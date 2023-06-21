from cffi import FFI
from pathlib import Path

ffi = FFI()

def extract_cffi(fname):
    with open(fname) as f:
        contents = f.read()
        return contents[contents.index("// cffi start"):contents.index("// cffi end")]

def make_ffi(name, headers, libraries, includes=[], extra_header=""):
    header_content = "\n".join([extract_cffi("../src/" + p) for p in headers])
    if extra_header:
        header_content += "\n" + extra_header

    ffi = FFI()

    for include in includes:
        ffi.include(include)

    # Define C declarations
    ffi.cdef(header_content)

    # Set the C source file
    ffi.set_source(name, '''
    #include "nix_api_util.h"
    #include "nix_api_store.h"
    #include "nix_api_expr.h"
    #include "nix_api_value.h"
    ''',
                   libraries=["nixutil", "nixstore", "nixexpr"],
                   library_dirs=["../outputs/out/lib"],
                   include_dirs=list(set(str(p.parent) for p in Path("../").glob("src/lib*/*.h"))))
    return ffi

libutil = make_ffi("nix._nix_api_util", ["libutil/nix_api_util.h"], ["nixutil"])
libstore = make_ffi("nix._nix_api_store", ["libstore/nix_api_store.h"], ["nixstore"], [libutil])
libexpr = make_ffi("nix._nix_api_expr", ["libexpr/nix_api_expr.h", "libexpr/nix_api_value.h"], ["nixexpr"], [libutil, libstore], """
extern "Python" void py_nix_primop_base(struct State*, int, void**, void*);
""")

# Compile the CFFI extension
if __name__ == '__main__':
    libutil.compile(verbose=True)
    libstore.compile(verbose=True)
    libexpr.compile(verbose=True)
