
.. capi:

capi -- C library to read/write DiscoDBs
====================================================================

The C library is the native control application for reading and writing DiscoDBs


Examples
========

Opening a discodb
------------------
Likely want to check a few things when opening a ddb file.


.. code-block:: c

    static struct ddb *open_discodb(const char *file)
    {
        struct ddb *db;
        int fd;
        if (!(db = ddb_new())){
            fprintf(stderr, "Couldn't initialize discodb: Out of memory\n");
            exit(1);
        }
        if ((fd = open(file, O_RDONLY)) == -1){
            fprintf(stderr, "Couldn't open discodb %s\n", file);
            exit(1);
        }
        if (ddb_load(db, fd)){
            const char *err;
            ddb_error(db, &err);
            fprintf(stderr, "Invalid discodb in %s: %s\n", file, err);
            exit(1);
        }
        return db;
    }



Adding key/values to a ddb
--------------------------

You'll need to create a :code:`ddb_entry` from memory available on the stack or heap.  During the :code:`ddb_cons_add` call, the entries will be duplicated into the discodb and the original stack/heap memory can be reused or freed.


.. code-block:: c

    uint64_t flags = 0;
    flags |= getenv("DONT_COMPRESS") ? DDB_OPT_DISABLE_COMPRESSION: 0;
    flags |= getenv("UNIQUE_ITEMS") ? DDB_OPT_UNIQUE_ITEMS: 0;
    flags |= DDB_OPT_UNIQUE_ITEMS;


    char key[MAX_KV_SIZE] = "Key\0";
    char val[MAX_KV_SIZE] = "Value\0";
    struct ddb_entry key_e = {.data = key, .length = strlen(key)};
    struct ddb_entry val_e = {.data = val, .length = strlen(val)};


    if (ddb_cons_add(db, &key_e, &val_e)){
        fprintf(stderr, "Adding '%s':'%s' failed\n", key, val);
        exit(1);
    }


    if (!(data = ddb_cons_finalize(db, &size, flags))){
        fprintf(stderr, "Packing the index failed\n");
        exit(1);
    }

    ddb_cons_free(db);

    if (!(out = fopen("/path/to/ddb", "w"))){
        fprintf(stderr, "Opening file %s failed\n", ddbfile);
        exit(1);
    }
    if (!fwrite(data, size, 1, out)){
        fprintf(stderr, "Writing file %s failed\n", ddbfile);
        exit(1);
    }
    fflush(out);
    fclose(out);
    free(data);
