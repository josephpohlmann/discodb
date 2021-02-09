
.. structure:

structure -- C library to read/write DiscoDBs
====================================================================

The C library is the native control application for reading and writing DiscoDBs

.. image:: images/discodb_format.png

Security
--------
This means that the DiscoDB keys are littered throughout the database file and the and values are at the end of the database in memory and on disk and in memory.


If the content of the keys/values are secure data there are a few options.

Loosly using the term "key" to refer to the key/value pair.  Using the terms "symmetric key" and "asymmetric key" to refer to cryptographic keys.

In all cases, the DiscoDB file would be presumed widely available, and the symmetric key would be a held secret or the asymmetric keys would be used as intended.


1) Encrypt the keys and values before inserting with :code:`add`.  The end user may not need to know this key if the system provides it at an intermediate layer. (**Note** Relationships of encrypted keys to encrypted values would be visible).

   a) Encrypt the search token (key) with a known symmetric key the tokens of the key before calling :code:`get` or preparing a :code:`Q` object
   b) Decrypt each value with a known symmetric key before returning each key to the end user.

2) Encrypt the entire DiscoDB file with a symmetric or asymmetric key

   a) use an encryption utility (examples: GPG, sodium, etc) to encrypt the entire DiscoDB file once it is written to disk locally.
   b) make the encrypted version of the DiscoDB file available on shared systems
   c) decrypt and access a local copy of the DiscoDB as needed


