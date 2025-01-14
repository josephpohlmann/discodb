#define PY_SSIZE_T_CLEAN

// #if PY_VERSION_HEX < 0x02060000
// #define PyVarObject_HEAD_INIT(type, size) PyObject_HEAD_INIT(type) size,
// #define Py_TYPE(ob)   (((PyObject*)(ob))->ob_type)
// #define PyBytes_AsString PyString_AsString
// #define PyBytes_FromFormat PyString_FromFormat
// #endif

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#include "discodb.h"
#include "discodbmodule.h"


static PyObject *DiscoDBError;


/* discodb Module Methods */

static PyMethodDef discodb_methods[] = {
    {NULL}                         /* Sentinel          */
};


/* DiscoDB Object Definition */

static PySequenceMethods DiscoDB_as_sequence = {
    NULL,                          /* sq_length         */
    NULL,                          /* sq_concat         */
    NULL,                          /* sq_repeat         */
    NULL,                          /* sq_item           */
    NULL,                          /* sq_slice          */
    NULL,                          /* sq_ass_item       */
    NULL,                          /* sq_ass_slice      */
    (objobjproc)DiscoDB_contains,  /* sq_contains       */
    NULL,                          /* sq_inplace_concat */
    NULL,                          /* sq_inplace_repeat */
};

static PyMappingMethods DiscoDB_as_mapping = {
    0,                             /* mp_length         */
    (binaryfunc)DiscoDB_getitem,   /* mp_subscript      */
    NULL,                          /* mp_ass_subscript  */
};

static PyMethodDef DiscoDB_methods[] = {
    {"keys", (PyCFunction)DiscoDB_keys, METH_NOARGS,
     "d.keys() -> an iterator over the keys of d."},
    {"values", (PyCFunction)DiscoDB_values, METH_NOARGS,
     "d.values() -> an iterator over the values of d."},
    {"unique_values", (PyCFunction)DiscoDB_unique_values, METH_NOARGS,
     "d.unique_values() -> an iterator over the unique values of d."},
    {"query", (PyCFunction)DiscoDB_query, METH_KEYWORDS | METH_VARARGS,
     "d.query(q) -> an iterator over the values of d whose keys satisfy q."},
    {"dumps", (PyCFunction)DiscoDB_dumps, METH_NOARGS,
     "d.dumps() -> a serialization of d."},
    {"dump", (PyCFunction)DiscoDB_dump, METH_O,
     "d.dump(o) -> write serialization of d to file object o."},
    {"loads", (PyCFunction)DiscoDB_loads, METH_CLASS | METH_O,
     "D.loads(s) -> a deserialized instance of D from serialization s."},
    {"load", (PyCFunction)DiscoDB_load, METH_CLASS | METH_VARARGS,
     "D.load(o[, n=0]]) -> a deserialized instance of D from file object o with offset n."},
    {NULL}                         /* Sentinel          */
};

static PyMemberDef DiscoDB_members[] = {
    {NULL}                         /* Sentinel          */
};

static PyTypeObject DiscoDBType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "_DiscoDB",                     /* tp_name           */
    sizeof(DiscoDB),               /* tp_basicsize      */
    0,                             /* tp_itemsize       */
    (destructor)DiscoDB_dealloc,   /* tp_dealloc        */
    0,                             /* tp_print          */
    0,                             /* tp_getattr        */
    0,                             /* tp_setattr        */
    0,                             /* tp_compare        */
    0,                             /* tp_repr           */
    0,                             /* tp_as_number      */
    &DiscoDB_as_sequence,          /* tp_as_sequence    */
    &DiscoDB_as_mapping,           /* tp_as_mapping     */
    0,                             /* tp_hash           */
    0,                             /* tp_call           */
    0,                             /* tp_str            */
    0,                             /* tp_getattro       */
    0,                             /* tp_setattro       */
    0,                             /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,           /* tp_flags          */
    0,                             /* tp_doc            */
    0,                             /* tp_traverse       */
    0,                             /* tp_clear          */
    0,                             /* tp_richcompare    */
    0,                             /* tp_weaklistoffset */
    (getiterfunc)DiscoDB_keys,     /* tp_iter           */
    0,                             /* tp_iternext       */
    DiscoDB_methods,               /* tp_methods        */
    DiscoDB_members,               /* tp_members        */
    0,                             /* tp_getset         */
    0,                             /* tp_base           */
    0,                             /* tp_dict           */
    0,                             /* tp_descr_get      */
    0,                             /* tp_descr_set      */
    0,                             /* tp_dictoffset     */
    0,                             /* tp_init           */
    0,                             /* tp_alloc          */
    DiscoDB_new,                   /* tp_new            */
    0,                             /* tp_free           */
};


/* General Object Protocol */

static PyObject *
DiscoDB_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    DiscoDBConstructor *cons = NULL;
    DiscoDB *self = NULL;
    PyObject
        *arg = NULL,
        *emptytuple = PyTuple_New(0),
        *item = NULL,
        *items = NULL,
        *iteritems = NULL,
        *none = NULL,
        *typedict = Py_BuildValue("{s:O}", "ddb_type", type);

    if (typedict == NULL || emptytuple == NULL)
      goto Done;

    cons = (DiscoDBConstructor *)DiscoDBConstructor_new(&DiscoDBConstructorType,
                                                        emptytuple,
                                                        typedict);
    if (cons == NULL)
      goto Done;

    if (!PyArg_ParseTuple(args, "|O", &arg))
      goto Done;

    if (arg == NULL)                /* null constructor */
      items = PyTuple_New(0);
    else if (PyMapping_Check(arg))  /* copy constructor */
      items = PyMapping_Items(arg);
    else                            /* iter constructor */
      Py_INCREF(items = arg);

    iteritems = PyObject_GetIter(items);
    if (iteritems == NULL)
      goto Done;

    while ((item = PyIter_Next(iteritems))) {
      none = DiscoDBConstructor_add(cons, item);
      if (none == NULL)
        goto Done;
      Py_CLEAR(item);
      Py_CLEAR(none);
    }
    self = (DiscoDB *)DiscoDBConstructor_finalize(cons, emptytuple, kwds);

 Done:
    Py_CLEAR(emptytuple);
    Py_CLEAR(cons);
    Py_CLEAR(item);
    Py_CLEAR(items);
    Py_CLEAR(iteritems);
    Py_CLEAR(none);
    Py_CLEAR(typedict);

    if (PyErr_Occurred()) {
        Py_CLEAR(self);
        return NULL;
    }
    return (PyObject *)self;
}

static void
DiscoDB_dealloc(DiscoDB *self)
{
    Py_CLEAR(self->obuffer);
    free(self->cbuffer);
    ddb_free(self->discodb);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

/* Mapping Formal / Informal Protocol */

static int
DiscoDB_contains(register DiscoDB *self, register PyObject *key)
{
    struct ddb_cursor *cursor = NULL;
    struct ddb_entry kentry;
    int isfound = 1;

    if (ddb_string_to_entry(key, &kentry))
        goto Done;

    cursor = ddb_getitem(self->discodb, &kentry);
    if (cursor == NULL)
        if (ddb_has_error(self->discodb))
            goto Done;

    if (ddb_notfound(cursor))
        isfound = 0;

 Done:
    ddb_cursor_dealloc(cursor);

    if (PyErr_Occurred())
        return -1;
    return isfound;
}

static PyObject *
DiscoDB_getitem(register DiscoDB *self, register PyObject *key)
{
    struct ddb_cursor *cursor = NULL;
    struct ddb_entry kentry;

    if (ddb_string_to_entry(key, &kentry))
        goto Done;

    cursor = ddb_getitem(self->discodb, &kentry);
    if (cursor == NULL)
        if (ddb_has_error(self->discodb))
            goto Done;

    if (ddb_notfound(cursor))
        PyErr_Format(PyExc_KeyError, "%s", PyBytes_AsString(key));

 Done:
    if (PyErr_Occurred())
        return NULL;

    return DiscoDBIter_new(&DiscoDBIterType, self, cursor);
}

static PyObject *
DiscoDB_keys(DiscoDB *self)
{
    struct ddb_cursor *cursor = ddb_keys(self->discodb);
    if (cursor == NULL)
        if (ddb_has_error(self->discodb))
            return NULL;
    return DiscoDBIter_new(&DiscoDBIterType, self, cursor);
}

static PyObject *
DiscoDB_values(DiscoDB *self)
{
    struct ddb_cursor *cursor = ddb_values(self->discodb);
    if (cursor == NULL)
        if (ddb_has_error(self->discodb))
            return NULL;
    return DiscoDBIter_new(&DiscoDBIterType, self, cursor);
}

static PyObject *
DiscoDB_unique_values(DiscoDB *self)
{
    struct ddb_cursor *cursor = ddb_unique_values(self->discodb);
    if (cursor == NULL)
        if (ddb_has_error(self->discodb))
            return NULL;
    return DiscoDBIter_new(&DiscoDBIterType, self, cursor);
}

static PyObject *
DiscoDB_query(register DiscoDB *self, PyObject *args, PyObject *kwds)
{
    PyObject
        *query_ = NULL,
        *clause = NULL,
        *clauses = NULL,
        *literal = NULL,
        *literals = NULL,
        *iterclauses = NULL,
        *iterliterals = NULL,
        *negated = NULL,
        *term = NULL,
        *query = NULL;
    DiscoDBView *view = NULL;
    Py_ssize_t i = 0, j = 0;
    struct ddb_query_clause *ddb_clauses = NULL;
    struct ddb_cursor *cursor = NULL;

    static char *kwlist[] = {"query", "view", NULL};

    if (self == NULL)
      goto Done;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O!", kwlist,
                                     &query_, &DiscoDBViewType, &view))
      goto Done;

    Py_INCREF(query_);
    Py_XINCREF(view);

    query = PyObject_CallMethod(query_, "resolve", "O", self);
    if (query == NULL)
        goto Done;

    clauses = PyObject_GetAttrString(query, "clauses");
    if (clauses == NULL)
        goto Done;

    iterclauses = PyObject_GetIter(clauses);
    if (iterclauses == NULL)
        goto Done;

    if ((i = PyObject_Length(clauses)) < 0)
        goto Done;
    ddb_clauses = ddb_query_clause_alloc(i);

    for (i = 0; (clause = PyIter_Next(iterclauses)); i++) {
        literals = PyObject_GetAttrString(clause, "literals");
        if (literals == NULL)
            goto Done;

        iterliterals = PyObject_GetIter(literals);
        if (iterliterals == NULL)
            goto Done;

        if ((j = PyObject_Length(literals)) < 0)
            goto Done;
        ddb_clauses[i].num_terms = j;
        ddb_clauses[i].terms     = ddb_query_term_alloc(j);

        for (j = 0; (literal = PyIter_Next(iterliterals)); j++) {
            negated = PyObject_GetAttrString(literal, "negated");
            if (negated == NULL)
                goto Done;

            term = PyObject_GetAttrString(literal, "term");
            if (term == NULL)
                goto Done;

            ddb_clauses[i].terms[j].nnot = PyObject_IsTrue(negated);

            if (ddb_string_to_entry(term, &ddb_clauses[i].terms[j].key))
                goto Done;

            Py_CLEAR(literal);
            Py_CLEAR(negated);
            Py_CLEAR(term);
        }

        Py_CLEAR(clause);
        Py_CLEAR(literals);
        Py_CLEAR(iterliterals);
    }

    if (view)
        cursor = ddb_query_view(self->discodb, ddb_clauses, i, view->view);
    else
        cursor = ddb_query(self->discodb, ddb_clauses, i);

    if (cursor == NULL)
        if (ddb_has_error(self->discodb))
            goto Done;

 Done:
    Py_CLEAR(clause);
    Py_CLEAR(clauses);
    Py_CLEAR(literal);
    Py_CLEAR(literals);
    Py_CLEAR(iterclauses);
    Py_CLEAR(iterliterals);
    Py_CLEAR(negated);
    Py_CLEAR(term);
    Py_CLEAR(query_);
    Py_CLEAR(query);
    Py_CLEAR(view);
    ddb_query_clause_dealloc(ddb_clauses, i);

    if (PyErr_Occurred()) {
        ddb_cursor_dealloc(cursor);
        return NULL;
    }

    return DiscoDBIter_new(&DiscoDBIterType, self, cursor);
}



/* Serialization / Deserialization Informal Protocol */

static PyObject *
DiscoDB_dumps(DiscoDB *self)
{
    uint64_t length;
    char *cbuffer = ddb_dumps(self->discodb, &length);
    PyObject *string = Py_BuildValue("y#", cbuffer, length);
    free(cbuffer);
    return string;
}

static PyObject *
DiscoDB_dump(DiscoDB *self, PyObject *file)
{
    PyObject *fileno = NULL;
    int fd;

    fileno = PyObject_CallMethod(file, "fileno", NULL);
    if (fileno == NULL)
        goto Done;

    fd = PyLong_AsLong(fileno);
    if (fd < 0)
        goto Done;

    if (ddb_dump(self->discodb, fd))
        if (ddb_has_error(self->discodb))
            goto Done;

 Done:
    Py_CLEAR(fileno);

    if (PyErr_Occurred())
        return NULL;

    Py_RETURN_NONE;
}

static PyObject *
DiscoDB_loads(PyTypeObject *type, PyObject *bytes)
{
    DiscoDB *self = (DiscoDB *)type->tp_alloc(type, 0);
    const char *buffer;
    Py_ssize_t n;

    if (self != NULL) {
        if (PyBytes_AsStringAndSize(bytes, (char**)&buffer, &n))
            goto Done;

        Py_INCREF(bytes);
        self->cbuffer = NULL;
        self->obuffer = bytes;
        self->discodb = ddb_alloc();
        if (self->discodb == NULL)
            goto Done;

        if (ddb_loads(self->discodb, buffer, n))
            if (ddb_has_error(self->discodb))
                goto Done;
    }

 Done:
    if (PyErr_Occurred()) {
        Py_CLEAR(self);
        return NULL;
    }

    return (PyObject *)self;
}

static PyObject *
DiscoDB_load(PyTypeObject *type, PyObject *args)
{
    DiscoDB *self = (DiscoDB *)type->tp_alloc(type, 0);
    PyObject
        *file = NULL,
        *fileno = NULL;
    long offset = 0;
    int fd;

    if (self != NULL) {
        if (!PyArg_ParseTuple(args, "O|l", &file, &offset))
            goto Done;

        fileno = PyObject_CallMethod(file, "fileno", NULL);
        if (fileno == NULL)
            goto Done;

        fd = PyLong_AsLong(fileno);
        if (fd < 0)
            goto Done;

        self->cbuffer = NULL;
        self->obuffer = NULL;
        self->discodb = ddb_alloc();
        if (self->discodb == NULL)
            goto Done;

        if (ddb_loado(self->discodb, fd, offset))
            if (ddb_has_error(self->discodb))
                goto Done;
    }

 Done:
    Py_CLEAR(fileno);

    if (PyErr_Occurred()) {
        Py_CLEAR(self);
        return NULL;
    }

    return (PyObject *)self;
}

/* Module Initialization */

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "_discodb",
    "Discodb Module",
    -1,
    discodb_methods,
    NULL,
    NULL,
    NULL,
    NULL,
};

PyMODINIT_FUNC PyInit__discodb(void)
{
    PyObject *module = PyModule_Create(&moduledef);

    if (PyType_Ready(&DiscoDBType) < 0)
        return (PyObject *)NULL;
    Py_INCREF(&DiscoDBType);
    PyModule_AddObject(module, "_DiscoDB",
                       (PyObject *)&DiscoDBType);

    if (PyType_Ready(&DiscoDBConstructorType) < 0)
        return (PyObject *)NULL;
    Py_INCREF(&DiscoDBConstructorType);
    PyModule_AddObject(module, "DiscoDBConstructor",
                       (PyObject *)&DiscoDBConstructorType);

    if (PyType_Ready(&DiscoDBIterType) < 0)
        return (PyObject *)NULL;
    Py_INCREF(&DiscoDBIterType);
    PyModule_AddObject(module, "DiscoDBIter",
                       (PyObject *)&DiscoDBIterType);

    if (PyType_Ready(&DiscoDBViewType) < 0)
        return (PyObject *)NULL;
    Py_INCREF(&DiscoDBViewType);
    PyModule_AddObject(module, "DiscoDBView",
                       (PyObject *)&DiscoDBViewType);

    DiscoDBError = PyErr_NewException("discodb.DiscoDBError", NULL, NULL);
    Py_INCREF(DiscoDBError);
    PyModule_AddObject(module, "DiscoDBError", DiscoDBError);
    return module;
}



/* DiscoDB Constructor Type */

static PyMethodDef DiscoDBConstructor_methods[] = {
    {"add", (PyCFunction)DiscoDBConstructor_add, METH_VARARGS,
     "c.add(k, v) -> add (k, v) to the DiscoDB that will be produced."},
    {"finalize", (PyCFunction)DiscoDBConstructor_finalize, METH_VARARGS | METH_KEYWORDS,
     "c.finalize([flags]) -> a DiscoDB containing the mappings added to c."},
    {NULL}                                    /* Sentinel          */
};

static PyMemberDef DiscoDBConstructor_members[] = {
    {NULL}                                    /* Sentinel          */
};

static PyTypeObject DiscoDBConstructorType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "DiscoDBConstructor",                     /* tp_name           */
    sizeof(DiscoDBConstructor),               /* tp_basicsize      */
    0,                                        /* tp_itemsize       */
    (destructor)DiscoDBConstructor_dealloc,   /* tp_dealloc        */
    0,                                        /* tp_print          */
    0,                                        /* tp_getattr        */
    0,                                        /* tp_setattr        */
    0,                                        /* tp_compare        */
    0,                                        /* tp_repr           */
    0,                                        /* tp_as_number      */
    0,                                        /* tp_as_sequence    */
    0,                                        /* tp_as_mapping     */
    0,                                        /* tp_hash           */
    0,                                        /* tp_call           */
    0,                                        /* tp_str            */
    0,                                        /* tp_getattro       */
    0,                                        /* tp_setattro       */
    0,                                        /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,                      /* tp_flags          */
    0,                                        /* tp_doc            */
    0,                                        /* tp_traverse       */
    0,                                        /* tp_clear          */
    0,                                        /* tp_richcompare    */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter           */
    0,                                        /* tp_iternext       */
    DiscoDBConstructor_methods,               /* tp_methods        */
    DiscoDBConstructor_members,               /* tp_members        */
    0,                                        /* tp_getset         */
    0,                                        /* tp_base           */
    0,                                        /* tp_dict           */
    0,                                        /* tp_descr_get      */
    0,                                        /* tp_descr_set      */
    0,                                        /* tp_dictoffset     */
    0,                                        /* tp_init           */
    0,                                        /* tp_alloc          */
    DiscoDBConstructor_new,                   /* tp_new            */
    0,                                        /* tp_free           */
};

static PyObject *
DiscoDBConstructor_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    DiscoDBConstructor *self = (DiscoDBConstructor *)type->tp_alloc(type, 0);
    DiscoDB *ddb = NULL;
    PyTypeObject *ddb_type = &DiscoDBType;

    static char *kwlist[] = {"ddb", "ddb_type", NULL};

    if (self == NULL)
      goto Done;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OO", kwlist, &ddb, &ddb_type))
      goto Done;

    if (ddb && !PyObject_TypeCheck(ddb, &DiscoDBType)) {
      PyErr_SetString(DiscoDBError, "Not a discodb.");
      goto Done;
    }

    if (!PyType_Check(ddb_type) || !PyType_IsSubtype(ddb_type, &DiscoDBType)) {
      PyErr_SetString(DiscoDBError, "Not a valid type.");
      goto Done;
    }

    Py_INCREF(self->ddb_type = ddb_type);

    if (ddb)
      self->ddb_cons = ddb_cons_ddb(ddb->discodb);
    else
      self->ddb_cons = ddb_cons_alloc();

    if (self->ddb_cons == NULL)
      goto Done;

 Done:

    if (PyErr_Occurred()) {
        Py_CLEAR(self);
        return NULL;
    }
    return (PyObject *)self;
}

static void
DiscoDBConstructor_dealloc(DiscoDBConstructor *self)
{
    Py_CLEAR(self->ddb_type);
    ddb_cons_dealloc(self->ddb_cons);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
DiscoDBConstructor_add(DiscoDBConstructor *self, PyObject *item)
{
    PyObject
        *itervalues = NULL,
        *value = NULL,
        *values = NULL,
        *valueseq = NULL;
    uint64_t n;
    struct ddb_entry kentry, ventry;

    if (!PyArg_ParseTuple(item, "y#O", &kentry.data, &kentry.length, &values))
      goto Done;

    Py_XINCREF(values);

    if (values == NULL)
      values = PyTuple_New(0);

    if (PyBytes_Check(values))
      valueseq = Py_BuildValue("(O)", values);
    else
      Py_XINCREF(valueseq = values);

    if (valueseq == NULL)
      goto Done;

    itervalues = PyObject_GetIter(valueseq);
    if (itervalues == NULL)
      goto Done;

    for (n = 0; (value = PyIter_Next(itervalues)); n++) {
      if (ddb_string_to_entry(value, &ventry))
          goto Done;

      if (ddb_cons_add(self->ddb_cons, &kentry, &ventry)) {
        PyErr_SetString(DiscoDBError, "Construction failed");
        goto Done;
      }

      Py_CLEAR(value);
    }

    if (n == 0)
      if (ddb_cons_add(self->ddb_cons, &kentry, NULL)) {
        PyErr_SetString(DiscoDBError, "Construction failed");
        goto Done;
      }

 Done:
    Py_CLEAR(itervalues);
    Py_CLEAR(value);
    Py_CLEAR(values);
    Py_CLEAR(valueseq);

    if (PyErr_Occurred())
      return NULL;

    Py_RETURN_NONE;
}

static PyObject *
DiscoDBConstructor_finalize(DiscoDBConstructor *self, PyObject *args, PyObject *kwds)
{
    DiscoDB *discodb = (DiscoDB *)DiscoDBType.tp_alloc(self->ddb_type, 0);
    uint64_t n,
      flags = 0,
      disable_compression = 0,
      unique_items = 0;

    static char *kwlist[] = {"disable_compression",
                             "unique_items", NULL};

    if (discodb == NULL)
      goto Done;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|II", kwlist,
                                     &disable_compression,
                                     &unique_items))
      goto Done;

    if (disable_compression)
      flags |= DDB_OPT_DISABLE_COMPRESSION;
    if (unique_items)
      flags |= DDB_OPT_UNIQUE_ITEMS;

    discodb->obuffer = NULL;
    discodb->cbuffer = ddb_cons_finalize(self->ddb_cons, &n, flags);
    if (discodb->cbuffer == NULL) {
        PyErr_SetString(DiscoDBError, "Construction finalization failed");
        goto Done;
    }

    discodb->discodb = ddb_alloc();
    if (discodb->discodb == NULL)
        goto Done;

    if (ddb_loads(discodb->discodb, discodb->cbuffer, n))
            if (ddb_has_error(discodb->discodb))
                goto Done;

 Done:
    if (PyErr_Occurred()) {
        Py_CLEAR(discodb);
        return NULL;
    }
    return (PyObject *)discodb;
}


/* DiscoDB Iterator Types */

static PyMethodDef DiscoDBIter_methods[] = {
    {"count", (PyCFunction)DiscoDBIter_count, METH_NOARGS,
     "i.count() -> count the remaining entries in the iterator."},
    {"size", (PyCFunction)DiscoDBIter_size, METH_NOARGS,
     "i.size() -> the size of the underlying cursor."},
    {NULL}                                   /* Sentinel          */
};

static PyTypeObject DiscoDBIterType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "DiscoDB-iterator",                      /* tp_name           */
    sizeof(DiscoDBIter),                     /* tp_basicsize      */
    0,                                       /* tp_itemsize       */
    (destructor)DiscoDBIter_dealloc,         /* tp_dealloc        */
    0,                                       /* tp_print          */
    0,                                       /* tp_getattr        */
    0,                                       /* tp_setattr        */
    0,                                       /* tp_compare        */
    0,                                       /* tp_repr           */
    0,                                       /* tp_as_number      */
    0,                                       /* tp_as_sequence    */
    0,                                       /* tp_as_mapping     */
    0,                                       /* tp_hash           */
    0,                                       /* tp_call           */
    0,                                       /* tp_str            */
    PyObject_GenericGetAttr,                 /* tp_getattro       */
    0,                                       /* tp_setattro       */
    0,                                       /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,                     /* tp_flags          */
    0,                                       /* tp_doc            */
    0,                                       /* tp_traverse       */
    0,                                       /* tp_clear          */
    0,                                       /* tp_richcompare    */
    0,                                       /* tp_weaklistoffset */
    PyObject_SelfIter,                       /* tp_iter           */
    (iternextfunc)DiscoDBIter_iternext,      /* tp_iternext       */
    DiscoDBIter_methods,                     /* tp_methods        */
};

static PyObject *
DiscoDBIter_new(PyTypeObject *type, DiscoDB *owner, struct ddb_cursor *cursor)
{
    DiscoDBIter *self = PyObject_New(DiscoDBIter, type);
    if (self != NULL) {
        Py_INCREF(owner);
        self->owner  = owner;
        self->cursor = cursor;
    }
    return (PyObject *)self;
}

static void
DiscoDBIter_dealloc(DiscoDBIter *self)
{
    Py_CLEAR(self->owner);
    ddb_cursor_dealloc(self->cursor);
    PyObject_Del(self);
}

static PyObject *
DiscoDBIter_count(DiscoDBIter *self)
{
    int errcode;
    Py_ssize_t n = ddb_cursor_count(self->cursor, &errcode);
    if (errcode)
        return PyErr_NoMemory();
    return PyInt_FromSsize_t(n);
}

static PyObject *
DiscoDBIter_size(DiscoDBIter *self)
{
    return PyInt_FromSsize_t(ddb_resultset_size(self->cursor));
}

static PyObject *
DiscoDBIter_iternext(DiscoDBIter *self)
{
    int errcode;
    const struct ddb_entry *next = ddb_next(self->cursor, &errcode);

    if (errcode)
        return PyErr_NoMemory();

    if (next == NULL)
        return NULL;

    return Py_BuildValue("y#", next->data, next->length);
}

/* DiscoDB View Type */

static PySequenceMethods DiscoDBView_sequence = {
    (lenfunc)DiscoDBView_len /* sq length */
};

static PyMethodDef DiscoDBView_methods[] = {
    {NULL}                                   /* Sentinel          */
};

static PyTypeObject DiscoDBViewType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "DiscoDBView",                           /* tp_name           */
    sizeof(DiscoDBView),                     /* tp_basicsize      */
    0,                                       /* tp_itemsize       */
    (destructor)DiscoDBView_dealloc,         /* tp_dealloc        */
    0,                                       /* tp_print          */
    0,                                       /* tp_getattr        */
    0,                                       /* tp_setattr        */
    0,                                       /* tp_compare        */
    0,                                       /* tp_repr           */
    0,                                       /* tp_as_number      */
    &DiscoDBView_sequence,                   /* tp_as_sequence    */
    0,                                       /* tp_as_mapping     */
    0,                                       /* tp_hash           */
    0,                                       /* tp_call           */
    0,                                       /* tp_str            */
    PyObject_GenericGetAttr,                 /* tp_getattro       */
    0,                                       /* tp_setattro       */
    0,                                       /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,                     /* tp_flags          */
    0,                                       /* tp_doc            */
    0,                                       /* tp_traverse       */
    0,                                       /* tp_clear          */
    0,                                       /* tp_richcompare    */
    0,                                       /* tp_weaklistoffset */
    0,                                       /* tp_iter           */
    0,                                       /* tp_iternext       */
    DiscoDBView_methods,                     /* tp_methods        */
    0,                                       /* tp_members        */
    0,                                       /* tp_getset         */
    0,                                       /* tp_base           */
    0,                                       /* tp_dict           */
    0,                                       /* tp_descr_get      */
    0,                                       /* tp_descr_set      */
    0,                                       /* tp_dictoffset     */
    0,                                       /* tp_init           */
    0,                                       /* tp_alloc          */
    DiscoDBView_new,                         /* tp_new            */
    0,                                       /* tp_free           */
};

static PyObject *
DiscoDBView_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *data = NULL,
             *iter = NULL,
             *item = NULL;
    DiscoDB *ddb = NULL;
    struct ddb_view_cons *cons = NULL;
    DiscoDBView *self = (DiscoDBView*)type->tp_alloc(type, 0);

    if (!PyArg_ParseTuple(args, "O!O", &DiscoDBType, &ddb, &data))
        goto Done;

    Py_INCREF(data);
    Py_INCREF(ddb);

    if (!(cons = ddb_view_cons_new()))
        return NULL;

    if (!(iter = PyObject_GetIter(data)))
        goto Done;

    while ((item = PyIter_Next(iter))){
        struct ddb_entry e;
        if (ddb_string_to_entry(item, &e))
            goto Done;
        if (ddb_view_cons_add(cons, &e)){
            PyErr_SetString(PyExc_MemoryError, "Adding value to view failed");
            goto Done;
        }
        Py_CLEAR(item);
    }
    if (!(self->view = ddb_view_cons_finalize(cons, ddb->discodb)))
        PyErr_SetString(PyExc_RuntimeError, "Couldn't finalize the view");
Done:
    ddb_view_cons_free(cons);
    Py_CLEAR(data);
    Py_CLEAR(iter);
    Py_CLEAR(ddb);

    if (PyErr_Occurred()) {
        Py_CLEAR(self);
        return NULL;
    }
    return (PyObject*)self;
}

static void DiscoDBView_dealloc(DiscoDBView *self)
{
    ddb_view_free(self->view);
    PyObject_Del(self);
}

static Py_ssize_t DiscoDBView_len(DiscoDBView *self)
{
    return ddb_view_size(self->view);
}

/* ddb helpers */

static struct ddb *
ddb_alloc(void)
{
    struct ddb *ddb = ddb_new();
    if (!ddb)
        PyErr_NoMemory();
    return ddb;
}

static struct ddb_cons *
ddb_cons_alloc(void)
{
    struct ddb_cons *cons = ddb_cons_new();
    if (!cons)
        PyErr_NoMemory();
    return cons;
}

static struct ddb_query_clause *
ddb_query_clause_alloc(size_t count)
{
    struct ddb_query_clause *clause = (struct ddb_query_clause *)calloc(count, sizeof(struct ddb_query_clause));
    if (!clause)
        PyErr_NoMemory();
    return clause;
}

static struct ddb_query_term *
ddb_query_term_alloc(size_t count)
{
    struct ddb_query_term *term = (struct ddb_query_term *)calloc(count, sizeof(struct ddb_query_term));
    if (!term)
        PyErr_NoMemory();
    return term;
}

static void
ddb_cons_dealloc(struct ddb_cons *cons)
{
  if (cons)
      ddb_cons_free(cons);
}

static void
ddb_cursor_dealloc(struct ddb_cursor *cursor)
{
    if (cursor)
        ddb_free_cursor(cursor);
}

static void
ddb_query_clause_dealloc(struct ddb_query_clause *clauses, uint32_t num_clauses)
{
    int i;
    for (i = 0; i < num_clauses; i++)
        if (clauses[i].terms)
            free(clauses[i].terms);
    if (clauses)
        free(clauses);
}

static int
ddb_has_error(struct ddb *discodb)
{
    int errcode;
    const char *errstr;
    if ((errcode = ddb_error(discodb, &errstr)))
        PyErr_SetString(DiscoDBError, errstr);
    return errcode;
}

static int
ddb_string_to_entry(PyObject *str, struct ddb_entry *e)
{
    Py_ssize_t len = 0;
    if (PyString_AsStringAndSize(str, (char**)&e->data, &len))
        return 1;
    if (len < UINT32_MAX){
        e->length = len;
        return 0;
    }else{
        PyErr_SetString(PyExc_ValueError, "String too long");
        return 1;
    }
}

