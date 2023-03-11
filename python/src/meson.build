src = [
  'nix-to-python.cc',
  'python-to-nix.cc',
  'eval.cc',
  'value.cc',
  'python-module.cc',
]

incdir = include_directories('../include')

nix_bindings = py_installation.extension_module('nix', src,
    dependencies : [nix_expr_dep, nix_main_dep],
    install: true,
    include_directories: incdir,
    cpp_args: [
      '-std=c++17',
      # -Wnon-virtual-dtor is unnecessarily turned on by Meson
      # This is fixed in Meson 1.0.0 with https://github.com/mesonbuild/meson/pull/10339
      '-Wno-non-virtual-dtor',
      '-fvisibility=hidden',
      '-include', 'config.h'
    ])
