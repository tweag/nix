#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "eval.hh"

namespace nix::python {

typedef struct {
    PyObject_HEAD
    // TODO: (performance) flatten this pointer, while making sure
    //       initialization is correct,
    //       or allocate these python objects with boehmgc
    nix::RootValue *value;

    inline nix::RootValue & operator *() {
        return *value;
    }
} ValuePyObject;

extern PyTypeObject ValuePyType;

ValuePyObject *allocValuePyObject(nix::EvalState * state);

}
