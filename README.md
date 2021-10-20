discodb
=======

An efficient, immutable, persistent mapping object.

See documentation at http://discodb.readthedocs.org

Use the Makefile to build, e.g.::

    make
    make erlang
    make python
    make python CMD=install
    make utils

NOTE: This is a python3 port, and is woefully incomplete.  It only meets my current needs.

Tests completed:
  - Open an existing DB and iterate over it.
  - Create a new DB using Constructor, finalize, and dump(s).

Tests not yet performed:
  - Query of existing DB
  - Merging of DBs

Limitations:
  - Keys & values must be added to DB as bytes (at present)