

# Python C Extention 2 Sequence Compare
[![Upload pypi.org](https://github.com/kirin123kirin/cdiffer/actions/workflows/pypi.yml/badge.svg?branch=v0.4.0)](https://github.com/kirin123kirin/cdiffer/actions/workflows/pypi.yml)

**Usefull differ function with Levenshtein distance.**

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
>>> from cdiffer import dist
>>>
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
['equal', 0, 0, 'c', 'c']
['delete', 1, None, 'o', None]
['insert', None, 1, None, 'a']
['delete', 2, None, 'f', None]
['equal', 3, 2, 'f', 'f']
['equal', 4, 3, 'e', 'e']
['delete', 5, None, 'e', None]
>>> for x in differ('coffee', 'cafe', diffonly=True):
...     print(x)
...
['delete', 1, None, 'o', None]
['insert', None, 1, None, 'a']
['delete', 2, None, 'f', None]
['delete', 5, None, 'e', None]
>>> # Matching rate option is `rep_rate` (default is 60(%))
>>> for x in differ('coffee', 'cafe', rep_rate=0):
...     print(x)
['equal', 0, 0, 'c', 'c']
['replace', 1, 1, 'o', 'a']
['delete', 2, None, 'f', None]
['equal', 3, 2, 'f', 'f']
['equal', 4, 3, 'e', 'e']
['delete', 5, None, 'e', None]
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
148 ns ± 1.1 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
728 ns ± 1.24 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
689 ns ± 1.73 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.13 µs ± 8.66 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.15 µs ± 6.32 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
163 ns ± 2.3 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
50.1 ns ± 1.05 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)

In [3]: %timeit similar('coffee', 'cafe')
    ...: %timeit similar(list('coffee'), list('cafe'))
    ...: %timeit similar(tuple('coffee'), tuple('cafe'))
    ...: %timeit similar(iter('coffee'), iter('cafe'))
    ...: %timeit similar(range(4), range(5))
    ...: %timeit similar('coffee', 'xxxxxx')
    ...: %timeit similar('coffee', 'coffee')
150 ns ± 1.01 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
729 ns ± 3.55 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
705 ns ± 9.76 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.14 µs ± 14.3 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.15 µs ± 6.97 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
166 ns ± 0.888 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
51 ns ± 0.698 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)

In [4]: %timeit differ('coffee', 'cafe')
    ...: %timeit differ(list('coffee'), list('cafe'))
    ...: %timeit differ(tuple('coffee'), tuple('cafe'))
    ...: %timeit differ(iter('coffee'), iter('cafe'))
    ...: %timeit differ(range(4), range(5))
    ...: %timeit differ('coffee', 'xxxxxx')
    ...: %timeit differ('coffee', 'coffee')
907 ns ± 4.58 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.47 µs ± 3.49 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.42 µs ± 2.57 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.89 µs ± 4.2 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
2.31 µs ± 35 ns per loop (mean ± std. dev. of 7 runs, 100000 loops each)
1.03 µs ± 1.18 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
409 ns ± 1.66 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)

In [5]: a = dict(zip('012345', 'coffee'))
    ...: b = dict(zip('0123', 'cafe'))
    ...: %timeit dist(a, b)
    ...: %timeit similar(a, b)
    ...: %timeit differ(a, b)
511 ns ± 2 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
519 ns ± 5.69 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.16 µs ± 2.81 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)

```
