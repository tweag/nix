from cffi import FFI
from pathlib import Path

ffi = FFI()

def extract_cffi(fname):
    with open(fname) as f:
        contents = f.read()
        return contents[contents.index("// cffi start"):contents.index("// cffi end")]

headers = [
    "libutil/nix_api_util.h",
    "libstore/nix_api_store.h",
    "libexpr/nix_api_expr.h",
    "libexpr/nix_api_value.h"
]
header_content = "\n".join([extract_cffi("../src/" + p) for p in headers])

# Define C declarations
ffi.cdef(header_content)

# Set the C source file
ffi.set_source('_nix_api', '''
#include "nix_api_util.h"
#include "nix_api_store.h"
#include "nix_api_expr.h"
#include "nix_api_value.h"
''',
               libraries=["nixutil", "nixstore", "nixexpr"],
               library_dirs=["../outputs/out/lib"],
               include_dirs=list(set(str(p.parent) for p in Path("../").glob("src/lib*/*.h"))))

# Compile the CFFI extension
if __name__ == '__main__':
    ffi.compile(verbose=True)
