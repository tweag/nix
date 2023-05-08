from __future__ import annotations

from typing import TypeAlias, Optional

from .util import err_check, null_check
from ._nix_api_store import lib, ffi

CData: TypeAlias = ffi.CData


def nix_store_init() -> None:
    err_check(lib.nix_libstore_init())


class StorePath:
    def __init__(self, ptr: ffi.CData) -> None:
        self._path = ptr


class Store:
    def __init__(self, url: Optional[str] = None, params: Optional[dict[str, str]] = None) -> None:
        ffi.init_once(nix_store_init, "init_libstore")
        url_c = ffi.NULL
        params_c = ffi.NULL
        if url is not None:
            url_c = ffi.new("char[]", url.encode())
        # store references because they have ownership
        pm = []
        kvs = []
        if params is not None:
            for k, v in params.items():
                kv = [ffi.new("char[]", k.encode()), ffi.new("char[]", v.encode())]
                kvs.append(kv)
                pm.append(ffi.new("char*[]", kv))
            pm.append(ffi.NULL)
            params_c = ffi.new("char**[]", pm)
        self._store = ffi.gc(
            null_check(lib.nix_store_open(url_c, params_c)), lib.nix_store_unref
        )

    def get_uri(self) -> str:
        dest = ffi.new("char[256]")
        err_check(lib.nix_store_get_uri(self._store, dest, len(dest)))
        return ffi.string(dest).decode()

    def get_version(self) -> str:
        dest = ffi.new("char[256]")
        err_check(lib.nix_store_get_version(self._store, dest, len(dest)))
        return ffi.string(dest).decode()

    def parse_path(self, path: str) -> StorePath:
        path_ct = ffi.new("char[]", path.encode())
        sp = null_check(lib.nix_store_parse_path(self._store, path_ct))
        return StorePath(ffi.gc(sp, lib.nix_store_path_free))

    def _ensure_store_path(self, path: StorePath | str) -> StorePath:
        if isinstance(path, StorePath):
            return path
        if isinstance(path, str):
            return self.parse_path(path)
        # value
        # if value is string: storepath(str)
        if "type" in path and str(path["type"]) == "derivation":
            return self.parse_path(str(path["drvPath"]))

    def is_valid_path(self, path: StorePath | str) -> bool:
        return bool(
            lib.nix_store_is_valid_path(
                self._store, self._ensure_store_path(path)._path
            )
        )

    def build(self, path: StorePath | str) -> dict[str, str]:
        path = self._ensure_store_path(path)
        res = {}

        # todo extern "Python"
        @ffi.callback("void(char*, char*)")
        def iter_callback(key: CData, path: CData) -> None:
            res[ffi.string(key).decode()] = ffi.string(path).decode()

        err_check(lib.nix_store_build(self._store, path._path, iter_callback))
        return res
