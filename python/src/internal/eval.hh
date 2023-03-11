#pragma once

#include <Python.h>

namespace nix {
struct EvalState;
}

namespace nix::python {

PyObject * eval(PyObject * self, PyObject * args, PyObject * kwdict);

nix::EvalState &getState();

}
