from typing import Any, TypeAlias, Callable, TypeVar

class CData:
    def __len__(self) -> int:
        pass
    def __getitem__(self, i: int) -> Any:
        pass

R = TypeVar("R")

class ffi:
    CData: TypeAlias = CData
    NULL: CData = ...
    @classmethod
    def string(cls, x: CData) -> bytes: ...
    @classmethod
    def new(cls, x: str, y: bytes | list[CData] | None = None) -> CData: ...
    @classmethod
    def gc(cls, x: CData, freer: Any) -> CData: ...
    @classmethod
    def callback(cls, x: str) -> Callable[..., CData]: ...
    @classmethod
    def init_once(cls, f: Callable[[], R], tag: str) -> R: ...

class Lib:
    def __getattribute__(self, name: str) -> Any: ...

lib: Lib = ...
