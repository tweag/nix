from cffi import FFI

ffi = FFI()

def extract_cffi(fname):
    with open(fname) as f:
        contents = f.read()
        return contents[contents.index("// cffi start"):contents.index("// cffi end")]
header_content = extract_cffi("../src/libnixc/nix_api.h") + "\n" + extract_cffi("../src/libnixc/nix_value_api.h")

# Define C declarations
ffi.cdef(header_content)

# Set the C source file
ffi.set_source('_nix_api', '''
#include "nix_api.h"
#include "nix_value_api.h"
''',
               libraries=["nixc"],
               library_dirs=["../src/libnixc"],
               include_dirs=["../src/libnixc"])

# Compile the CFFI extension
if __name__ == '__main__':
    ffi.compile(verbose=True)
