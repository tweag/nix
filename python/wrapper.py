import os
import builtins
import types
from _nix_api import ffi, lib

# todo: estimate size for ffi.gc calls

class NixAPIError(Exception):
    pass

def nix_setting_get(key):
    value = ffi.new("char[1024]")
    err_check(lib.nix_setting_get(key.encode(), value, len(value)))
    return ffi.string(value).decode()

def nix_setting_set(key, value):
    err_check(lib.nix_setting_set(key.encode(), value.encode()))

def nix_init() -> None:
    err_check(lib.nix_init())

def nix_err_msg():
    msg = ffi.new("char[1024]")
    lib.nix_err_msg(msg, len(msg))
    return ffi.string(msg).decode()

class GCpin:
    def __init__(self, ptr):
        self._ref = ffi.gc(lib.nix_gc_ref(ptr), lib.nix_gc_free)

class Store:
    def __init__(self):
        self._store = ffi.gc(lib.nix_store_open(),
                             lib.nix_store_unref)


class Expr:
    def __init__(self, state_wrapper, expr):
        self._state = state_wrapper
        self._expr = expr
        self._gc = GCpin(self._expr)

    def eval(self):
        value = Value(self._state._state)
        err_check(lib.nix_expr_eval(self._state._state, self._expr, value._value))

        return value


class State:
    def __init__(self, search_path, store_wrapper):
        search_path_c = [ffi.new("char[]", path.encode()) for path in search_path]
        search_path_c.append(ffi.NULL)
        search_path_ptr = ffi.new("char*[]", search_path_c)
        self._state = ffi.gc(
            lib.nix_state_create(search_path_ptr, store_wrapper._store),
            lib.nix_state_free)

    def parse_expr_from_string(self, expr_string, path):
        expr = null_check(lib.nix_parse_expr_from_string(
            self._state, expr_string.encode(), path.encode()
        ))
        return Expr(self, expr)
        
#Evaluated = lambda: int | float | str | types.NoneType | dict[str, Value] | list[Value] # Function | String
Evaluated = lambda: int | float | str | types.NoneType | dict | list # Function | String
DeepEvaluated = lambda: int | float | str | "String" | types.NoneType | dict[string, DeepEvaluated] | list[DeepEvaluated] | Function

def err_check(err_code):
    if err_code != lib.NIX_OK:
        raise NixAPIError(nix_err_msg())

def null_check(obj):
    if not obj:
        raise NixAPIError(nix_err_msg())
    return obj

class Value:
    def __init__(self, state_ptr, value_ptr=None):
        self._state = state_ptr
        if value_ptr is None:
            self._value = lib.nix_alloc_value(state_ptr)
        else:
            self._value = value_ptr
        self._gc = GCpin(self._value)

    def _get_type(self) -> int:
        return lib.nix_get_type(self._value)

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
                raise NotImplementedError
            case lib.NIX_TYPE_EXTERNAL:
                raise NotImplementedError
            case _:
                raise RuntimeError("invalid type from nix_get_type")

    def _force(self, deep=False):
        if deep:
            err_check(lib.nix_value_force_deep(self._state, self._value))
        else:
            err_check(lib.nix_value_force(self._state, self._value))
        

    def __repr__(self):
        # todo: state.print
        return "<Nix Value ({})>".format(self.get_typename())

    def _to_python(self, deep=False):
        match self.get_type():
            case builtins.int:
                return lib.nix_get_int(self._value)
            case builtins.float:
                return lib.nix_get_double(self._value)
            case builtins.bool:
                return bool(lib.nix_get_bool(self._value))
            case builtins.str:
                return self.get_string()
            case builtins.dict:
                res = {}
                if deep:
                    self.get_attr_iterate(lambda k,v: res.__setitem__(k, v._to_python(deep)))
                else:
                    self.get_attr_iterate(lambda k,v: res.__setitem__(k, v))
                return res
            case builtins.list:
                l = list(self)
                if deep:
                    # todo don't need another force call
                    l = [x._to_python(deep=True) for x in l]
                return l
            case _:
                raise NotImplementedError

    def force(self, typeCheck=Evaluated, convert=True, deep=False):
        if isinstance(typeCheck, types.FunctionType):
            typeCheck = typeCheck()

        self._force(deep=deep)
        if not issubclass(self.get_type(), typeCheck):
            raise TypeError("nix value is {} while {} was expected".format(self.get_typename(), str(typeCheck)))

        return self._to_python(deep) if convert else None

    def get_typename(self) -> str:
        return ffi.string(lib.nix_get_typename(self._value)).decode()

    def get_string(self) -> str:
        return ffi.string(lib.nix_get_string(self._value)).decode()

    def get_list_byid(self, ix: int) -> "Value":
        value_ptr = lib.nix_get_list_byid(self._value, ix)
        return Value(self._state, value_ptr)

    def get_attr_byname(self, name: str) -> "Value":
        value_ptr = lib.nix_get_attr_byname(
            self._value,
            self._state,
            name.encode())
        return Value(self._state, value_ptr)

    def get_attr_iterate(self, iter_func) -> None:
        @ffi.callback("void(char[], Value*, void*)")
        def iter_callback(name, value_ptr, data):
            iter_func(ffi.string(name).decode(), Value(self._state, value_ptr))

        lib.nix_get_attr_iterate(self._value, self._state, iter_callback, ffi.NULL)

    def __int__(self) -> int:
        return self.force(int)

    def __str__(self) -> str:
        return self.force(str)

    def __float__(self) -> float:
        return self.force(float)

    def __len__(self) -> int:
        self.force(dict | list, convert=False)
        match self.get_type():
            case builtins.dict:
                return lib.nix_get_attrs_size(self._value)
            case builtins.list:
                return lib.nix_get_list_size(self._value)
            case _:
                raise RuntimeError()

    def __getitem__(self, i: int | str): #-> Value:
        self.force(dict | list, convert=False)
        match self.get_type():
            case builtins.dict:
                return self.force(dict)[i]
            case builtins.list:
                if i >= len(self):
                    raise IndexError("list index out of range")
                return self.get_list_byid(i % len(self))
            case _:
                raise RuntimeError()

# Example usage:
if __name__ == "__main__":
    nix_init()

    try:
        state = State([], Store())
        e = state.parse_expr_from_string("((x: [1 2 x]) 1)", ".")
        v = e.eval()
        #print(v.get_typename(), v.get_string())
        
        # nix_setting_set("example_key", "example_value")
        # value = nix_setting_get("example_key")
        # print(f"example_key: {value}")
    except NixAPIError as e:
        print(f"Error: {e}")

def nix(expr):
    e = state.parse_expr_from_string(expr, ".")
    return e.eval()
    
