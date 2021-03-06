
.. pythonapi:

pythonapi -- Python library to read/write DiscoDBs
====================================================================

By default the python API links against libdiscodb at runtime and provides a thin wrapper to the functionality.




incrementally adding key/value pairs to a constructor
-----------------------------------------------------

to incrementally build a discodb, start a DiscoDBCostructor and add key values one at a time.

Keys and/or values may be duplicated, DiscoDB will take care of it.


.. code-block:: python

    import discodb
    from time import sleep
    a = discodb.DiscoDBConstructor()
    a.add(b"k", b"v")
    a.add(b"db1", b"a value")
    o = a.finalize(unique_items=True)
    with open("/path/to/ddb", 'wb') as f:
        o.dump(f)


loading the discodb from disk
-----------------------------------

.. code-block:: python

    import discodb
    with open("/path/to/ddb", 'rb') as f:
        ddb = discodb.DiscoDB.load(f)
    print("DB1 Keys and Values read from disk")
    first_keys = [k for k in ddb.keys()]
    first_values = [v for v in ddb.values()]



Merging multiple ddbs into a single ddb
---------------------------------------

.. code-block:: python

    import discodb
    c = discodb.DiscoDBConstructor()  # combined ddb
    c.add(b"k", b"2")  # feel free to add other keys before/after the merge call
    c.add(b"k", b"3")
    with open("/path/to/ddb", 'rb') as f:
        prior_ddb = discodb.DiscoDB.load(f)
    c.merge(prior_ddb)
    # can open / merge multiple ddbs
    # note CMPH algorithm preserves value order,
    # so the arrangement of merges will affect key order in final ddb
    merged_output = c.finalize(unique_items=True)
    with open("/path/to/merged_ddb", 'wb') as f:
        o.dump(f)



Overwriting DDB values during merge
-----------------------------------
merging multiple ddbs into a single ddb where they values are replaced by a single explicit value.

A DiscoDB contains key/value pairs.  In the case where a key/static-value pair is required (perhaps in a distributed index), :code:`merge_with_explicit_value` can be used during merge.  Every key will have the same explicit static value after the merge.

.. code-block:: python

    import discodb
    c = discodb.DiscoDBConstructor()  # combined ddb
    c.add(b"k", b"2")  # feel free to add other keys before/after the merge call
    c.add(b"k", b"3")
    with open("/path/to/ddb", 'rb') as f:
        prior_ddb = discodb.DiscoDB.load(f)
    c.merge_with_explicit_value(e, b"new_value_for_all_keys")
    # can open / merge multiple ddbs
    # note CMPH algorithm preserves value order,
    # so the arrangement of merges will affect key order in final ddb
    merged_output = c.finalize(unique_items=True)
    with open("/path/to/merged_ddb", 'wb') as f:
        o.dump(f)


Inverting a DDB
---------------
In cases where a ddb contains key/value pairs, and it needs inverted to value/key pairs (accessed by value rather than key)

.. code-block:: python

    import discodb
    c = discodb.DiscoDBConstructor()  # combined ddb
    c.add(b"k", b"2")  # feel free to add other keys before/after the merge call
    c.add(b"k", b"3")
    with open("/path/to/ddb", 'rb') as f:
        prior_ddb = discodb.DiscoDB.load(f)
    c.invert(prior_ddb)
    inverted_output = c.finalize(unique_items=True)
    with open("/path/to/inverted_ddb", 'wb') as f:
        o.dump(f)



DiscoDB memory maps the on-disk file, so the file can be quite large (technically 2^64 bytes) without requiring excessive process memory.

In cases where memory allows it, as determined by the user, Disco can load it's entire structure into a native python dictionary

**warning** DiscoDB will make no effort to verify the size of the file can be allocated into process memory as a dict

It is up to the user to check the size of the ddb and compare to available memory

Therefore, it is by design that the the :code:`.__dict__()` function will not automatically call the :code:`dump_dict` function

.. code-block:: python

    import discodb
    with open("/path/to/ddb", 'rb') as f:
        ddb = discodb.DiscoDB.load(f)
    native_dict = ddb.dump_dict()  # native dict




Passing around DDB as in-memory buffers
---------------------------------------

This may largely defeat a key point of DiscoDB in that DDBs can be generated on systems with large memory and read/used on systems with low memory due to memory mapping the structure rather than loading it in its entirety.

Passing around memory buffers may be useful in a cross-language system where a DDB can be exchanged between a C application, a Python application, and a Lua application all accessing the same DDB structure.

In general, there may better in-memory key value stores.

use the :code:`dumps` command on a **finalized** constructor to retreive a bytes buffer.  Transfer those bytes via any means to another system capable of loading the buffer.

use the :code:`discodb.DiscoDB.loads` command to load the buffer obtained from the other system.

.. code-block:: python

    import discodb

    c = discodb.DiscoDBConstructor()  # combined ddb
    c.add(b"key", b"value")
    o = c.finalize()

    ddb_buffer = o.dumps()

    ddb = discodb.DiscoDB.loads(ddb_buffer)
    native_dict = ddb.dump_dict()
    print("should be {b'key': [b'value']}")