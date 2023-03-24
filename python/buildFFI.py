from cffi import FFI

ffi = FFI()

header_content = open("../src/libnixc/nix_api.h").read()
header_content = header_content[header_content.index("// cffi start"):header_content.index("// cffi end")]

# Define C declarations
ffi.cdef(header_content)

# Set the C source file
ffi.set_source('_nix_api',
               '#include "nix_api.h"',
               libraries=["nixc"],
               library_dirs=["../src/libnixc"],
               include_dirs=["../src/libnixc"])

# Compile the CFFI extension
if __name__ == '__main__':
    ffi.compile(verbose=True)
