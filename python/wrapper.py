import os
from _nix_api import ffi, lib

# todo: estimate size for ffi.gc calls

class NixAPIError(Exception):
    pass

def nix_setting_get(key):
    value = ffi.new("char[1024]")
    result = lib.nix_setting_get(key.encode(), value, len(value))

    if result == lib.NIX_OK:
        return ffi.string(value).decode()
    else:
        raise NixAPIError(nix_err_msg())

def nix_setting_set(key, value):
    result = lib.nix_setting_set(key.encode(), value.encode())

    if result != lib.NIX_OK:
        raise NixAPIError(nix_err_msg())

def nix_init() -> None:
    result = lib.nix_init()

    if result != lib.NIX_OK:
        raise NixAPIError(nix_err_msg())

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
        err_code = lib.nix_expr_eval(self._state._state, self._expr, value._value)
        if err_code != lib.NIX_OK:
            raise NixAPIError(nix_err_msg())

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
        expr = lib.nix_parse_expr_from_string(
            self._state, expr_string.encode(), path.encode()
        )
        return Expr(self, expr)
        

class Value:
    def __init__(self, state_ptr, value_ptr=None):
        self._state = state_ptr
        if value_ptr is None:
            self._value = lib.nix_alloc_value(state_ptr)
        else:
            self._value = value_ptr
        self._gc = GCpin(self._value)

    def get_type(self):
        return lib.nix_get_type(self._value)

    def get_typename(self) -> str:
        return ffi.string(lib.nix_get_typename(self._value)).decode()

    def get_bool(self) -> bool:
        return bool(lib.nix_get_bool(self._value))

    def get_string(self) -> str:
        return ffi.string(lib.nix_get_string(self._value)).decode()

    def get_list_size(self) -> int:
        return lib.nix_get_list_size(self._value)

    def get_double(self) -> float:
        return lib.nix_get_double(self._value)

    def get_int(self) -> int:
        return lib.nix_get_int(self._value)

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
        @ffi.callback("void(const char*, Value*, void*)")
        def iter_callback(name, value_ptr, data):
            iter_func(ffi.string(name).decode(), Value(self._state, value_ptr))

        lib.nix_get_attr_iterate(self._value, iter_callback, ffi.NULL)

# Example usage:
if __name__ == "__main__":
    nix_init()

    try:
        state = State([], Store())
        e = state.parse_expr_from_string("builtins.toJSON ((x: [1 2 x]) 1)", ".")
        v = e.eval()
        print(v.get_typename(), v.get_string())
        
        # nix_setting_set("example_key", "example_value")
        # value = nix_setting_get("example_key")
        # print(f"example_key: {value}")
    except NixAPIError as e:
        print(f"Error: {e}")
