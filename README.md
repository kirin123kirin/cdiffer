

# Python C Extention 2 Sequence Compare
[![Upload pypi.org](https://github.com/kirin123kirin/cdiffer/actions/workflows/pypi.yml/badge.svg?branch=v0.4.2)](https://github.com/kirin123kirin/cdiffer/actions/workflows/pypi.yml)

**Edit distance, Similarity and 2 sequence differences printing.**

# How to Install?
```shell
pip install cdiffer
```

# Requirement
* python3.6 or later
* python2.7

# cdiffer.dist
Compute absolute Levenshtein distance of two strings.

## Usage
dist(sequence, sequence)

## Examples (it's hard to spell Levenshtein correctly):

```python
Help on built-in function dist in module cdiffer:

dist(...)
    Compute absolute Levenshtein distance of two strings.

    dist(sequence, sequence)

    Examples (it's hard to spell Levenshtein correctly):

    >>> dist('coffee', 'cafe')
    4
    >>> dist(list('coffee'), list('cafe'))
    4
    >>> dist(tuple('coffee'), tuple('cafe'))
    4
    >>> dist(iter('coffee'), iter('cafe'))
    4
    >>> dist(range(4), range(5))
    1
    >>> dist('coffee', 'xxxxxx')
    12
    >>> dist('coffee', 'coffee')
    0
```

# cdiffer.similar

Compute similarity of two strings.

## Usage
similar(sequence, sequence)

The similarity is a number between 0 and 1,
base on levenshtein edit distance.

## Examples
```python
>>> from cdiffer import similar
>>>
>>> similar('coffee', 'cafe')
0.6
>>> similar('hoge', 'bar')
0.0

```

# cdiffer.differ

Find sequence of edit operations transforming one string to another.

## Usage
differ(source_sequence, destination_sequence, diffonly=False, rep_rate=60)

## Examples

```python
>>> from cdiffer import differ
>>>
    >>> for x in differ('coffee', 'cafe'):
    ...     print(x)
    ...
    ['equal',   0, 0,   'c', 'c']
    ['delete',  1, None,'o',None]
    ['insert',  None, 1,None,'a']
    ['equal',   2, 2,   'f', 'f']
    ['delete',  3, None,'f',None]
    ['delete',  4, None,'e',None]
    ['equal',   5, 3,   'e', 'e']
    >>> for x in differ('coffee', 'cafe', diffonly=True):
    ...     print(x)
    ...
    ['delete',  1, None,'o',None]
    ['insert',  None, 1,None,'a']
    ['delete',  3, None,'f',None]
    ['delete',  4, None,'e',None]

    >>> for x in differ('coffee', 'cafe', rep_rate = 0):
    ...     print(x)
    ...
    ['equal',   0, 0,   'c', 'c']
    ['replace', 1, 1,   'o', 'a']
    ['equal',   2, 2,   'f', 'f']
    ['delete',  3, None,'f',None]
    ['delete',  4, None,'e',None]
    ['equal',   5, 3,   'e', 'e']
    >>> for x in differ('coffee', 'cafe', diffonly=True, rep_rate = 0):
    ...     print(x)
    ...
    ['replace', 1, 1,   'o', 'a']
    ['delete',  3, None,'f',None]
    ['delete',  4, None,'e',None]

```

## Performance


```python
C:\Windows\system>ipython
Python 3.7.7 (tags/v3.7.7:d7c567b08f, Mar 10 2020, 10:41:24) [MSC v.1900 64 bit (AMD64)]
Type 'copyright', 'credits' or 'license' for more information
IPython 7.21.0 -- An enhanced Interactive Python. Type '?' for help.

In [1]: from cdiffer import *

In [2]: %timeit dist('coffee', 'cafe')
   ...: %timeit dist(list('coffee'), list('cafe'))
   ...: %timeit dist(tuple('coffee'), tuple('cafe'))
   ...: %timeit dist(iter('coffee'), iter('cafe'))
   ...: %timeit dist(range(4), range(5))
   ...: %timeit dist('coffee', 'xxxxxx')
   ...: %timeit dist('coffee', 'coffee')
125 ns ± 0.534 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
677 ns ± 2.3 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
638 ns ± 3.42 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
681 ns ± 2.16 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
843 ns ± 3.66 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
125 ns ± 0.417 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
50.5 ns ± 0.338 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)

In [3]: %timeit similar('coffee', 'cafe')
   ...: %timeit similar(list('coffee'), list('cafe'))
   ...: %timeit similar(tuple('coffee'), tuple('cafe'))
   ...: %timeit similar(iter('coffee'), iter('cafe'))
   ...: %timeit similar(range(4), range(5))
   ...: %timeit similar('coffee', 'xxxxxx')
   ...: %timeit similar('coffee', 'coffee')
123 ns ± 0.301 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
680 ns ± 2.64 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
647 ns ± 1.78 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
680 ns ± 7.57 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
848 ns ± 4.19 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
130 ns ± 0.595 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
54.8 ns ± 0.691 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)

In [4]: %timeit differ('coffee', 'cafe')
    ...: %timeit differ(list('coffee'), list('cafe'))
    ...: %timeit differ(tuple('coffee'), tuple('cafe'))
    ...: %timeit differ(iter('coffee'), iter('cafe'))
    ...: %timeit differ(range(4), range(5))
    ...: %timeit differ('coffee', 'xxxxxx')
    ...: %timeit differ('coffee', 'coffee')
735 ns ± 4.18 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.36 µs ± 5.17 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.31 µs ± 5.25 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.37 µs ± 5.04 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.33 µs ± 5.32 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.07 µs ± 6.75 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
638 ns ± 3.67 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)

In [5]: a = dict(zip('012345', 'coffee'))
    ...: b = dict(zip('0123', 'cafe'))
    ...: %timeit dist(a, b)
    ...: %timeit similar(a, b)
    ...: %timeit differ(a, b)
524 ns ± 2.6 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
539 ns ± 2.23 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.07 µs ± 1.9 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
```
