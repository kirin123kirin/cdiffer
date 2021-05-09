

# Python C Extention 2 Sequence Compare
[![Upload pypi.org](https://github.com/kirin123kirin/cdiffer/actions/workflows/pypi.yml/badge.svg?branch=v0.2.1)](https://github.com/kirin123kirin/cdiffer/actions/workflows/pypi.yml)

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
3
>>> dist(list('coffee'), list('cafe'))
3
>>> dist(tuple('coffee'), tuple('cafe'))
3
>>> dist(iter('coffee'), iter('cafe'))
3
>>> dist(range(4), range(5))
1
>>> dist('coffee', 'xxxxxx')
6
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
0.7
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
['insert', None, 1, None, 'a']
['delete', 1, None, 'o', None]
['equal', 2, 2, 'f', 'f']
['delete', 3, None, 'f', None]
['delete', 4, None, 'e', None]
['equal', 5, 3, 'e', 'e']
>>> for x in differ('coffee', 'cafe', diffonly=True):
...     print(x)
...
['insert', None, 1, None, 'a']
['delete', 1, None, 'o', None]
['delete', 3, None, 'f', None]
['delete', 4, None, 'e', None]
>>> # Matching rate option is `rep_rate` (default is 60(%))
>>> for x in differ('coffee', 'cafe', rep_rate=0):
...     print(x)
['equal', 0, 0, 'c', 'c']
['replace', 1, 1, 'o', 'a']
['equal', 2, 2, 'f', 'f']
['delete', 3, None, 'f', None]
['delete', 4, None, 'e', None]
['equal', 5, 3, 'e', 'e']
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
162 ns ± 0.752 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
709 ns ± 3.33 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
658 ns ± 7.6 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.08 µs ± 6 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.14 µs ± 5.15 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
199 ns ± 0.38 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
50.3 ns ± 0.103 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)

In [3]: %timeit similar('coffee', 'cafe')
    ...: %timeit similar(list('coffee'), list('cafe'))
    ...: %timeit similar(tuple('coffee'), tuple('cafe'))
    ...: %timeit similar(iter('coffee'), iter('cafe'))
    ...: %timeit similar(range(4), range(5))
    ...: %timeit similar('coffee', 'xxxxxx')
    ...: %timeit similar('coffee', 'coffee')
161 ns ± 0.079 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
708 ns ± 5.43 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
671 ns ± 2.35 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.11 µs ± 15.6 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.15 µs ± 7.85 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
196 ns ± 0.242 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
50.9 ns ± 0.628 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)

In [4]: %timeit differ('coffee', 'cafe')
    ...: %timeit differ(list('coffee'), list('cafe'))
    ...: %timeit differ(tuple('coffee'), tuple('cafe'))
    ...: %timeit differ(iter('coffee'), iter('cafe'))
    ...: %timeit differ(range(4), range(5))
    ...: %timeit differ('coffee', 'xxxxxx')
    ...: %timeit differ('coffee', 'coffee')
683 ns ± 1.41 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.21 µs ± 9.12 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.16 µs ± 13.9 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.63 µs ± 9.98 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
2.1 µs ± 8.4 ns per loop (mean ± std. dev. of 7 runs, 100000 loops each)
1 µs ± 3.05 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
493 ns ± 1.28 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)

In [5]: a = dict(zip('012345', 'coffee'))
    ...: b = dict(zip('0123', 'cafe'))
    ...: %timeit dist(a, b)
    ...: %timeit similar(a, b)
    ...: %timeit differ(a, b)
436 ns ± 1.45 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
434 ns ± 2.07 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
932 ns ± 3.91 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)


```
