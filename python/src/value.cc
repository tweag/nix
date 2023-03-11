#include "nix/python/value.hh"
#include "internal/eval.hh"
#include "internal/nix-to-python.hh"
#include "internal/errors.hh"

using namespace nix;

namespace nix::python {

static void dealloc(PyObject *pyObject) {
    auto value = (ValuePyObject *) pyObject;
    assert(value->value);
    delete value->value;
    Py_TYPE(pyObject)->tp_free(pyObject);
}

static PyObject *toPythonStrict(PyObject *obj, PyObject *_no_args) {
    try {
        auto value = (ValuePyObject *)obj;
        auto &state = getState();

        state.forceValueDeep(****value);

        nix::PathSet context;
        return nixToPythonObject(state, ****value, context);

    } catch (nix::ThrownError & e) {
        return PyErr_Format(ThrownNixError, "%s", e.message().c_str());
    } catch (nix::Error & e) {
        return PyErr_Format(NixError, "%s", e.what());
    } catch (...) {
        return PyErr_Format(NixError, "unexpected C++ exception: '%s'", currentExceptionTypeName());
    }
};

static PyMethodDef methods[] = {
    {"toPythonStrict", &toPythonStrict, METH_NOARGS,
        "Convert a Nix value to a python value and return it. This is behaves approximately as if Nix builtins.deepSeq was invoked: if it returns a value, it will not contain any exceptions or recursion."
    },
    {NULL}
};

PyTypeObject ValuePyType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "nix.Value",

    .tp_basicsize = sizeof(ValuePyObject),
    // types with fixed-length instances have a zero tp_itemsize field
    .tp_itemsize = 0,

    .tp_dealloc = &dealloc,

    .tp_flags = Py_TPFLAGS_DEFAULT,

    .tp_doc = PyDoc_STR("A Nix value. This can hold an actual value, such as a string or a function, or it can hold a thunk that can be \"forced\" to an actual value (or throw an exception)."),

    .tp_methods = methods,


    // The type cannot be called. This is like not having a constructor.
    // All construction is performed by functions instead.
    .tp_new = NULL,
};

ValuePyObject * allocValuePyObject(nix::EvalState * state) {
    ValuePyObject * r = (ValuePyObject *) PyType_GenericAlloc(&ValuePyType, 1);
    r->value = new RootValue(allocRootValue(state->allocValue()));
    return r;
}


}
