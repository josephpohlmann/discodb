.. query:

query -- Access methods to DiscoDB
====================================================================

Overview of access methods

Direct Access
-------------
The same path as the default getter

.. code-block:: python

    d = DiscoDB({'A': ['B', 'C'], 'B': 'D', 'C': 'E', 'D': 'F', 'E': 'G'})
    sorted(d.query(Q.parse('A')))
    ['B', 'C']


Indirect Access
---------------
Gather the values which are accessed by the value at the named key

1) A points to B and C
2) B points to D
3) C points to E

therefore :code:`*A` points to :code:`[D, E]`

.. code-block:: python

    sorted(d.query(Q.parse('*A')))
    ['D', 'E']


Double Indirect Access
----------------------

:code:`**A`

1) A points to B and C
2) B points to D (first indirect)
3) D points to F (second indirect)
4) C points to E (first indirect)
5) E points to G (second indirect)

therefore :code:`**A` points to :code:`[F, G]`

.. code-block:: python

    sorted(d.query(Q.parse('**A')))
    ['F', 'G']


Arbitrary Depth Indirects
-------------------------

DiscoDB can execute arbitrary depth queries.

:code:`***A`

1) A points to B and C
2) B points to D (first indirect)
3) D points to F (second indirect)
4) C points to E (first indirect)
5) E points to G (second indirect)
6) F is not a valid key (third indirect)
7) G is not a valid key (third indirect)

therefore :code:`***A` points to :code:`[ ]`

**Note:**  generation of Q object will throw a :code:`TypeError` if the number of redirects exceeds the linkages provided by the underlying DDB

.. code-block:: python

    sorted(d.query(Q.parse('***A')))
    [ ]
    sorted(d.query(Q.parse('****A')))
    [ ]
    sorted(d.query(Q.parse('*****A')))
    TypeError: reduce() of empty sequence with no initial value


Or Operation
------------
Gather the values by any of the keys

1) A points to B and C
2) B points to D

therefore :code:`A | B` points to :code:`[B, C, D]`

.. code-block:: python

    sorted(d.query(Q.parse('A | B')))
    ['B', 'C', 'D']

Mixing Indirects with Logical
-----------------------------

.. code-block:: python

    sorted(d.query(Q.parse('*A | B')))
    ['D', 'E']
    sorted(d.query(Q.parse('**A | *B')))
    ['F', 'G']

Meta Queries
------------

Metaqueries document how DiscoDB arrived at the result of the query

This may be helpful when the CNF parser rearranges the query prior to runtime.

.. code-block:: python

    >>> sorted((str(k), sorted(vs)) for k, vs in d.metaquery(Q.parse('A')))
    [('A', ['B', 'C'])]
    >>> sorted((str(k), sorted(vs)) for k, vs in d.metaquery(Q.parse('*A')))
    [('B', ['D']), ('C', ['E'])]
    >>> sorted((str(k), sorted(vs)) for k, vs in d.metaquery(Q.parse('A | B')))
    [('A | B', ['B', 'C', 'D'])]
    >>> sorted((str(k), sorted(vs)) for k, vs in d.metaquery(Q.parse('*A | B')))
    [('B', ['D']), ('C | B', ['D', 'E'])]
    >>> sorted((str(k), sorted(vs)) for k, vs in d.metaquery(Q.parse('**A | *B')))
    [('D', ['F']), ('E | D', ['F', 'G'])]

