__all__ = ["util", "store", "expr", "eval"]

from __future__ import annotations
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from .expr import Value

_state = None
_store = None

def eval(string: str, path: str=".") -> Value:
    from .store import Store
    from .expr import State

    global _store, _state
    if _store is None or _state is None:
        _store = Store()
        _state = State([], _store)
    e = _state.parse_expr_from_string(string, path)
    return e.eval()
