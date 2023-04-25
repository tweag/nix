from __future__ import annotations

import builtins
import collections.abc
import types
import typing
from collections.abc import Callable, Iterator
from typing import Any, TypeAlias, Union

from _nix_api import ffi, lib

CData: TypeAlias = ffi.CData

# todo: estimate size for ffi.gc calls

class NixAPIError(Exception):
    pass

def nix_setting_get(key: str) -> str:
    value = ffi.new("char[1024]")
    err_check(lib.nix_setting_get(key.encode(), value, len(value)))
    return ffi.string(value).decode()

def nix_setting_set(key: str, value: str) -> None:
    err_check(lib.nix_setting_set(key.encode(), value.encode()))

def nix_init() -> None:
    err_check(lib.nix_init())

def nix_err_msg() -> str:
    msg = ffi.new("char[1024]")
    lib.nix_err_msg(msg, len(msg))
    return ffi.string(msg).decode('utf-8', errors='replace')

class GCpin:
    def __init__(self, ptr: ffi.CData) -> None:
        self._ref = ffi.gc(lib.nix_gc_ref(ptr), lib.nix_gc_free)

class Store:
    def __init__(self) -> None:
        self._store = ffi.gc(lib.nix_store_open(),
                             lib.nix_store_unref)


class Expr:
    def __init__(self, state_wrapper: State, expr: ffi.CData) -> None:
        self._state = state_wrapper
        self._expr = expr
        self._gc = GCpin(self._expr)

    def eval(self) -> Value:
        value = Value(self._state._state)
        err_check(lib.nix_expr_eval(self._state._state, self._expr, value._value))

        return value


class State:
    def __init__(self, search_path: list[str], store_wrapper: Store) -> None:
        search_path_c = [ffi.new("char[]", path.encode()) for path in search_path]
        search_path_c.append(ffi.NULL)
        search_path_ptr = ffi.new("char*[]", search_path_c)
        self._state = ffi.gc(
            lib.nix_state_create(search_path_ptr, store_wrapper._store),
            lib.nix_state_free)

    def parse_expr_from_string(self, expr_string: str, path: str) -> Expr:
        expr = null_check(lib.nix_parse_expr_from_string(
            self._state, expr_string.encode(), path.encode(),
        ))
        return Expr(self, expr)

    def alloc_val(self) -> Value:
        return Value(self._state)

    def val_from_python(self, py_val: Evaluated) -> Value:
        v = self.alloc_val()
        v.set(py_val)
        return v

Evaluated: TypeAlias = Union[
    int,
    float,
    str,
    None,
    dict,
    list,
    "Function", # | String
]
DeepEvaluated = Union[
    int,
    float,
    str,
    None,
    dict[str, "DeepEvaluated"],
    list["DeepEvaluated"],
    "Function",
    # string
]

def err_check(err_code: int) -> None:
    if err_code != lib.NIX_OK:
        raise NixAPIError(nix_err_msg())

def null_check(obj: ffi.CData) -> ffi.CData:
    if not obj:
        raise NixAPIError(nix_err_msg())
    return obj

class Function:
    def __init__(self, val: Value) -> None:
        self.value = val
    def __repr__(self) -> str:
        return repr(self.value)
    def __call__(self, arg: Value | Evaluated) -> Value:
        if not isinstance(arg, Value):
            arg2 = Value(self.value._state)
            arg2.set(arg)
            arg = arg2
        res = Value(self.value._state)
        err_check(lib.nix_value_call(self.value._state, self.value._value, arg._value, res._value))
        return res

class X:
    Function = Function

T = typing.TypeVar("T")

class Value:
    def __init__(self, state_ptr: CData, value_ptr: CData | None=None) -> None:
        self._state = state_ptr
        if value_ptr is None:
            self._value = lib.nix_alloc_value(state_ptr)
        else:
            self._value = value_ptr
        self._gc = GCpin(self._value)

    def _get_type(self) -> int:
        return int(lib.nix_get_type(self._value))

    def get_type(self) -> type:
        match self._get_type():
            case lib.NIX_TYPE_THUNK:
                return Value # todo?
            case lib.NIX_TYPE_INT:
                return int
            case lib.NIX_TYPE_FLOAT:
                return float
            case lib.NIX_TYPE_BOOL:
                return bool
            case lib.NIX_TYPE_STRING:
                return str
            case lib.NIX_TYPE_PATH:
                return str # todo
            case lib.NIX_TYPE_NULL:
                return types.NoneType
            case lib.NIX_TYPE_ATTRS:
                return dict
            case lib.NIX_TYPE_LIST:
                return list
            case lib.NIX_TYPE_FUNCTION:
                return Function
            case lib.NIX_TYPE_EXTERNAL:
                raise NotImplementedError
            case _:
                raise RuntimeError("invalid type from nix_get_type")

    def _force(self, deep: bool=False) -> None:
        if deep:
            err_check(lib.nix_value_force_deep(self._state, self._value))
        else:
            err_check(lib.nix_value_force(self._state, self._value))


    def __repr__(self) -> str:
        t = self._get_type()
        if t == lib.NIX_TYPE_ATTRS and "type" in self and self["type"].force() == "derivation":
            return "<Nix derivation {}>".format(self["drvPath"].force())
        elif t not in [lib.NIX_TYPE_THUNK, lib.NIX_TYPE_FUNCTION, lib.NIX_TYPE_ATTRS]:
            return f"<Nix: {self.force()}>"
        else:
            return f"<Nix Value ({self.get_typename()})>"

    def _to_python(self, deep: bool=False) -> Evaluated:
        match self.get_type():
            case builtins.int:
                return int(lib.nix_get_int(self._value))
            case builtins.float:
                return float(lib.nix_get_double(self._value))
            case builtins.bool:
                return bool(lib.nix_get_bool(self._value))
            case builtins.str:
                return ffi.string(lib.nix_get_string(self._value)).decode()
            case builtins.dict:
                res_dict: dict[str, Value | DeepEvaluated] = {}
                if deep:
                    self.get_attr_iterate(lambda k,v: res_dict.__setitem__(k, v._to_python(deep)))
                else:
                    self.get_attr_iterate(lambda k,v: res_dict.__setitem__(k, v))
                return res_dict
            case builtins.list:
                res_list: list[Value] = list(self)
                if deep:
                    # todo don't need another force call
                    return [x._to_python(deep=True) for x in res_list]
                return res_list
            case X.Function:
                return Function(self)
            case _:
                raise NotImplementedError

    # https://github.com/python/mypy/issues/9773
    def force(self, typeCheck: Any = Evaluated, deep: bool=False) -> typing.Any:
        self.force_type(typeCheck, deep=deep)
        return self._to_python(deep)

    def force_type(self, typeCheck: Any=Evaluated, deep: bool=False) -> type[Any]:
        if isinstance(typeCheck, types.FunctionType):
            typeCheck = typeCheck()

        self._force(deep=deep)
        tp = self.get_type()
        if not issubclass(tp, typeCheck):
            raise TypeError(f"nix value is {self.get_typename()} while {str(typeCheck)} was expected")

        return tp

    def get_typename(self) -> str:
        return str(ffi.string(lib.nix_get_typename(self._value)).decode())

    def get_list_byid(self, ix: int) -> Value:
        value_ptr = null_check(lib.nix_get_list_byid(self._value, ix))
        return Value(self._state, value_ptr)

    def get_attr_byname(self, name: str) -> Value:
        value_ptr = null_check(lib.nix_get_attr_byname(
            self._value,
            self._state,
            name.encode()))
        return Value(self._state, value_ptr)

    def get_attr_iterate(self, iter_func: Callable[[str, Value], None]) -> None:
        @ffi.callback("void(char*, Value*, void*)")
        def iter_callback(name: CData, value_ptr: CData, data: CData) -> None:
            iter_func(ffi.string(name).decode(), Value(self._state, value_ptr))

        lib.nix_get_attr_iterate(self._value, self._state, iter_callback, ffi.NULL)

    def __iter__(self) -> typing.Any:
        match self.force_type(dict | list):
            case builtins.list:
                return collections.abc.Sequence.__iter__(typing.cast(collections.abc.Sequence[Value], self))
            case builtins.dict:
                return iter(self.force())

    def __int__(self) -> int:
        return int(self.force(int | float | str))

    def __str__(self) -> str:
        return str(self.force(float | int | str | None))

    def __float__(self) -> float:
        return float(self.force(float | int | str))

    def __bool__(self) -> bool:
        match self.force_type():
            case builtins.dict | builtins.list:
                return bool(len(self))
            case _:
                return bool(self._to_python())

    def __len__(self) -> int:
        match self.force_type(dict | list):
            case builtins.dict:
                return int(lib.nix_get_attrs_size(self._value))
            case builtins.list:
                return int(lib.nix_get_list_size(self._value))
            case _:
                raise RuntimeError

    def __contains__(self, i: int | str) -> bool:
        match self.force_type(dict | list):
            case builtins.dict:
                assert type(i) == str
                return bool(lib.nix_has_attr_byname(self._value, self._state, i.encode()))
            case builtins.list:
                return i in list(self)
            case _:
                raise RuntimeError

    def __getitem__(self, i: int | str) -> Value:
        match self.force_type(dict | list):
            case builtins.dict:
                if not isinstance(i, str):
                    raise TypeError("key should be a string")
                return self.get_attr_byname(i)
            case builtins.list:
                if not isinstance(i, int):
                    raise TypeError("key should be a integer")
                if i >= len(self):
                    raise IndexError("list index out of range")
                return self.get_list_byid(i % len(self))
            case _:
                raise RuntimeError

    def keys(self) -> Iterator[str]:
        self.force_type(dict)
        return iter(self)

    def __call__(self, x: Value | Evaluated) -> Value:
        return typing.cast(Function, self.force(Function))(x)

    def set(self, py_val: Value | DeepEvaluated) -> None:
        if isinstance(py_val, Function):
            raise NotImplementedError
        elif isinstance(py_val, Value):
            raise NotImplementedError
        elif isinstance(py_val, bool):
            lib.nix_set_bool(self._value, py_val)
        elif isinstance(py_val, str):
            lib.nix_set_string(self._value, py_val.encode())
        elif isinstance(py_val, float):
            lib.nix_set_double(self._value, py_val)
        elif isinstance(py_val, int):
            lib.nix_set_int(self._value, py_val)
        elif py_val is None:
            lib.nix_set_null(self._value)
        elif isinstance(py_val, list):
            lib.nix_make_list(self._state, self._value, len(py_val))
            for i in range(len(py_val)):
                v = Value(self._state)
                v.set(py_val[i])
                lib.nix_set_list_byid(self._value, i, v._value)
        elif isinstance(py_val, dict):
            bb = ffi.gc(lib.nix_make_bindings_builder(self._state, len(py_val)), lib.nix_bindings_builder_unref)
            for k, dv in py_val.items():
                v = Value(self._state)
                v.set(dv)
                lib.nix_bindings_builder_insert(bb, k.encode(), v._value)
            lib.nix_make_attrs(self._value, bb)
        else:
            raise TypeError("tried to convert unknown type to nix")


# Example usage:
if __name__ == "__main__":
    nix_init()
    nix_setting_set("extra-experimental-features", "flakes")

    try:
        state = State([], Store())
        e = state.parse_expr_from_string("((x: [1 2 x]) 1)", ".")
        v = e.eval()

    except NixAPIError as e:
        print(f"Error: {e}")

def nix(expr: str) -> Value:
    e = state.parse_expr_from_string(expr, ".")
    return e.eval()

