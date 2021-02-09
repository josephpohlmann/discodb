
.. cnf:

cnf -- Conjunctive Normal Form (used by Query Language)
====================================================================

Both the Python API and C API will interpret CNF based queries into a normalized form for queries.



.. code-block:: python


    >>> Q.parse('~(B | C)') == Q.parse('~B & ~C')
    True
    >>> Q.parse('(A & B) | C') == Q.parse('(A | C) & (B | C)')
    True
    >>> Q.parse('A & (B | (D & E))') == Q.parse('A & (B | D) & (B | E)')
    True
    >>> Q.parse(str(Q.parse('a | b | c & d | e'))) == Q.parse('a | b | c & d | e')
    True

