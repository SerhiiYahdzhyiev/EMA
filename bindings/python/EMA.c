#include <Python.h>

#include <EMA.h>


PyObject* _EMA_init(PyObject* mod, PyObject* args)
{
    EMA_init(NULL);
    Py_RETURN_NONE;
}

PyObject* _EMA_finalize(PyObject* mod, PyObject* args)
{
    EMA_finalize();
    Py_RETURN_NONE;
}

PyObject* _EMA_region_define(PyObject* mod, PyObject* name_obj)
{
    const char *name = PyUnicode_AsUTF8(name_obj);
    Region *region = NULL;
    if( EMA_region_define(&region, name, NULL, "", 0, "") != 0 )
    {
        PyErr_SetString(PyExc_RuntimeError, "An error occurred.");
        return NULL;
    }
    return PyCapsule_New(region, NULL, NULL);
}

PyObject* _EMA_region_begin(PyObject* mod, PyObject* region_capsule)
{
    Region *region = PyCapsule_GetPointer(region_capsule, NULL);
    if( EMA_region_begin(region) != 0 )
    {
        PyErr_SetString(PyExc_RuntimeError, "An error occurred.");
        return NULL;
    }
    Py_RETURN_NONE;
}

PyObject* _EMA_region_end(PyObject* mod, PyObject* region_capsule)
{
    Region *region = PyCapsule_GetPointer(region_capsule, NULL);
    if( EMA_region_end(region) != 0 )
    {
        PyErr_SetString(PyExc_RuntimeError, "An error occurred.");
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
    {"EMA_init", _EMA_init, METH_NOARGS, NULL},
    {"EMA_finalize", _EMA_finalize, METH_NOARGS, NULL},
    {"EMA_region_define", _EMA_region_define, METH_O, NULL},
    {"EMA_region_begin", _EMA_region_begin, METH_O, NULL},
    {"EMA_region_end", _EMA_region_end, METH_O, NULL},
    {NULL, NULL, 0, NULL},
};

static PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    "EMA",
    "EMA Python bindings",
    0,
    methods
};

PyMODINIT_FUNC PyInit_EMA()
{
    return PyModule_Create(&module);
}
