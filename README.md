

# Python C Extention 2 Sequence Compare
[![Upload pypi.org](https://github.com/kirin123kirin/cdiffer/actions/workflows/pypi.yml/badge.svg?branch=v0.1.7)](https://github.com/kirin123kirin/cdiffer/actions/workflows/pypi.yml)

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

The similarity is a number between 0 and 1, it's usually equal or
somewhat higher than difflib.SequenceMatcher.ratio(), because it's
based on real minimal edit distance.

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
differ(source_sequence, destination_sequence, diffonly=False)

## Examples

```python
>>> from cdiffer import differ
>>>
>>> for x in differ('coffee', 'cafe'):
...     print(x)
...
['equal',   0, 0,   'c', 'c']
['replace', 1, 1,   'o', 'a']
['equal',   2, 2,   'f', 'f']
['delete',  3, None,'f',None]
['delete',  4, None,'e',None]
['equal',   5, 3,   'e', 'e']
>>> for x in differ('coffee', 'cafe', diffonly=True):
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
   ...:
173 ns ± 0.206 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
741 ns ± 2.4 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
702 ns ± 2.15 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
706 ns ± 7.79 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
882 ns ± 7.51 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
210 ns ± 0.335 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
51.8 ns ± 1.18 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)

In [3]: %timeit similar('coffee', 'cafe')
   ...: %timeit similar(list('coffee'), list('cafe'))
   ...: %timeit similar(tuple('coffee'), tuple('cafe'))
   ...: %timeit similar(iter('coffee'), iter('cafe'))
   ...: %timeit similar(range(4), range(5))
   ...: %timeit similar('coffee', 'xxxxxx')
   ...: %timeit similar('coffee', 'coffee')
   ...:
186 ns ± 0.476 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)
718 ns ± 0.878 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
691 ns ± 1.42 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
706 ns ± 2.01 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
920 ns ± 8.2 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
223 ns ± 0.938 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
55 ns ± 0.308 ns per loop (mean ± std. dev. of 7 runs, 10000000 loops each)

In [4]: %timeit differ('coffee', 'cafe')
   ...: %timeit differ(list('coffee'), list('cafe'))
   ...: %timeit differ(tuple('coffee'), tuple('cafe'))
   ...: %timeit differ(iter('coffee'), iter('cafe'))
   ...: %timeit differ(range(4), range(5))
   ...: %timeit differ('coffee', 'xxxxxx')
   ...: %timeit differ('coffee', 'coffee')
   ...:
814 ns ± 2.79 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.36 µs ± 2.02 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.33 µs ± 4.19 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
1.37 µs ± 4.64 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
2.03 µs ± 19.1 ns per loop (mean ± std. dev. of 7 runs, 100000 loops each)
865 ns ± 1.89 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
724 ns ± 1.72 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)

In [5]: a = dict(zip('012345', 'coffee'))
   ...: b = dict(zip('0123', 'cafe'))
   ...: %timeit dist(a, b)
   ...: %timeit similar(a, b)
   ...: %timeit differ(a, b)
320 ns ± 1.26 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
327 ns ± 1.2 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
983 ns ± 17.1 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)


```