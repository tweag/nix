from cffi import FFI

ffi = FFI()

def extract_cffi(fname):
    with open(fname) as f:
        contents = f.read()
        return contents[contents.index("// cffi start"):contents.index("// cffi end")]

headers = [
    "nix_api_util.h",
    "nix_api_store.h",
    "nix_api_expr.h",
    "nix_value_api.h"
]
header_content = "\n".join([extract_cffi("../src/libnixc/" + p) for p in headers])

# Define C declarations
ffi.cdef(header_content)

# Set the C source file
ffi.set_source('_nix_api', '''
#include "nix_api_util.h"
#include "nix_api_store.h"
#include "nix_api_expr.h"
#include "nix_value_api.h"
''',
               libraries=["nixc"],
               library_dirs=["../src/libnixc"],
               include_dirs=["../src/libnixc"])

# Compile the CFFI extension
if __name__ == '__main__':
    ffi.compile(verbose=True)
