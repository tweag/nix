__all__ = ["util", "store", "expr"]

_state = None
_store = None

def eval(string, path="."):
    from .store import Store
    from .expr import State

    global _store, _state
    if _store is None or _state is None:
        _store = Store()
        _state = State([], _store)
    e = _state.parse_expr_from_string(string, path)
    return e.eval()
