#!/usr/bin/python
# -*- coding: utf-8 -*-

from timeit import timeit
import os
import sys
from psutil import virtual_memory, Process
process = Process(os.getpid())
print("\n\n", __file__, ":PID -> ", os.getpid(), "\n\n")

try:
    from cdiffer import dist, differ, similar

    smip = "from cdiffer import dist, differ, similar"
except ImportError:
    from cdiffer.cdiffer import dist, differ, similar

    smip = "from cdiffer.cdiffer import dist, differ, similar"


def test_import_dist():
    assert dist


def test_import_differ():
    assert differ


def test_import_similar():
    assert similar


# differ_test
def test_differ_binary_test():
    assert (differ(b'coffee', b'cafe'))


ans1 = [['equal', 0, 0, 'c', 'c'],
        ['insert', None, 1, None, 'a'],
        ['delete', 1, None, 'o', None],
        ['equal', 2, 2, 'f', 'f'],
        ['delete', 3, None, 'f', None],
        ['delete', 4, None, 'e', None],
        ['equal', 5, 3, 'e', 'e']]

ans2 = [['equal', 0, 0, 'c', 'c'],
        ['delete', 1, None, 'o', None],
        ['delete', 2, None, 'f', None],
        ['delete', 3, None, 'f', None],
        ['delete', 4, None, 'e', None],
        ['insert', None, 1, None, 'z'],
        ['delete', 5, None, 'e', None]]


def test_differ_string_test():
    assert (differ('coffee', 'cafe') == ans1)


def test_differ_list_test():
    assert (differ(list('coffee'), list('cafe')) == ans1)
    assert (differ(list('coffee'), list('cz')) == ans2)


def test_differ_iter_test():
    assert (differ(iter('coffee'), iter('cafe')) == ans1)


def test_diffonly_flag_test():
    assert (differ('coffee', 'cafe', True) == [x for x in ans1 if x[0] != "equal"])


def test_dist_list_test():
    assert (dist(list('coffee'), list('cafe')) == 3)


def test_similar_binary_test():
    assert (similar(b'coffee', b'cafe') == 0.7)


def test_similar_string_test():
    assert (similar('coffee', 'cafe') == 0.7)


def test_similar_list_test():
    assert (similar(list('coffee'), list('cafe')) == 0.7)
    assert (similar(list('cafe'), list('cafe')) == 1)
    assert (similar(list('cafe'), list('')) == 0)
    assert (similar(list('cafe'), []) == 0)


def test_similar_tuple_test():
    assert (similar(tuple('coffee'), tuple('cafe')) == 0.7)
    assert (similar(tuple('cafe'), tuple('cafe')) == 1)
    assert (similar(tuple('cafe'), tuple('')) == 0)
    assert (similar(tuple('cafe'), []) == 0)


def test_similar_same_test():
    assert (similar([], []) == 1.0)
    assert (similar(1, 1) == 1.0)


def test_similar_iter_test():
    assert (dist(iter('coffee'), iter('cafe')) == 3)
    assert (similar(iter('coffee'), iter('cafe')) == 0.7)
    assert (differ(iter('cafexyz'), iter('coffeeabcdefghijk'), False, 0) ==
            [['equal', 0, 0, 'c', 'c'],
             ['insert', None, 1, None, 'o'],
             ['insert', None, 2, None, 'f'],
             ['insert', None, 3, None, 'f'],
             ['insert', None, 4, None, 'e'],
             ['insert', None, 5, None, 'e'],
             ['equal', 1, 6, 'a', 'a'],
             ['insert', None, 7, None, 'b'],
             ['insert', None, 8, None, 'c'],
             ['insert', None, 9, None, 'd'],
             ['insert', None, 10, None, 'e'],
             ['equal', 2, 11, 'f', 'f'],
             ['insert', None, 12, None, 'g'],
             ['replace', 3, 13, 'e', 'h'],
             ['replace', 4, 14, 'x', 'i'],
             ['replace', 5, 15, 'y', 'j'],
             ['replace', 6, 16, 'z', 'k']]
            )


def test_string_test():
    assert (dist('cdfaafe', 'cofeedfajj') == 7)


ans3 = [[u'equal', 0, 0, u'あ', u'あ'],
        [u'replace', 1, 1, u'い', u'え'],
        [u'equal', 2, 2, u'う', u'う']]

ans4 = [[u'equal', 0, 0, u'あ', u'あ'],
        [u'replace', 1, 1, u'い', u'え'],
        [u'equal', 2, 2, u'う', u'う'],
        [u'insert', None, 3, None, u'!']]

ans5 = [[u'equal', 0, 0, u'あ', u'あ'],
        [u'replace', 1, 1, u'い', u'え'],
        [u'equal', 2, 2, u'う', u'う'],
        [u'delete', 3, None, u'!', None]]


def test_multibyte_test():
    assert (dist(u'あいう', u'あえう') == 1)
    assert (dist(u'あいう', u'あえう!') == 2)
    assert (differ(u'あいう', u'あえう', False, 0) == ans3)
    assert (differ(u'あいう', u'あえう!', False, 0) == ans4)
    assert (differ(u'あいう!', u'あえう', False, 0) == ans5)


def test_list_test():
    assert (dist(list('cdfaafe'), list('cofeedfajj')) == 7)


ans6 = [[u'equal', 0, 0, '0', '0'],
        [u'equal', 1, 1, '1', '1'],
        [u'equal', 2, 2, '2', '2'],
        [u'equal', 3, 3, '3', '3'],
        [u'delete', 4, None, '4', None],
        [u'delete', 5, None, '5', None]]

def test_dict_string_test():
    assert (similar(dict(zip('012345', 'coffee')), dict(zip('0123', 'cafe'))) == 0.8)
    assert (dist(dict(zip('012345', 'coffee')), dict(zip('0123', 'cafe'))) == 2)
    if sys.version_info[0] > 2:
        assert (differ(dict(zip('012345', 'coffee')), dict(zip('0123', 'cafe'))) == ans6)

def test_Error_Test():
    try:
        differ("", [])
        raise AssertionError
    except ValueError:
        pass
    except Exception as e:
        raise AssertionError(e)


def test_integer_test():
    assert (similar(10, 100) == 0)
    assert (dist(10, 100) == 1)
    assert (differ(10, 100) == [
        ['insert', None, 0, None, 100],
        ['delete', 0, None, 10, None],
    ])

def test_complex_type():
    assert (dist(list("coffee"), "cafe") == 3)
    assert (dist(list('あいう'), 'あえう!') == 2)

def test_dist_Notype():
    assert(dist(None, None) == 0)
    assert(dist("", "") == 0)
    assert(dist(b"", b"") == 0)
    assert(dist([], []) == 0)
    assert(dist({}, {}) == 0)
    assert(dist((), ()) == 0)

def test_dist_complex_Nottype():
    assert(dist([None], None) == 0)
    assert(dist([None], "") == 1)
    assert(dist([None], []) == 1)

def test_similar_Notype():
    assert(similar(None, None) == 1.0)
    assert(similar("", "") == 1.0)
    assert(similar(b"", b"") == 1.0)
    assert(similar([], []) == 1.0)
    assert(similar({}, {}) == 1.0)
    assert(similar((), ()) == 1.0)

def test_similar_complex_Nottype():
    assert(similar([None], None) == 1.0)
    assert(similar([None], "") == 0.0)
    assert(similar([None], []) == 0.0)

def test_differ_Notype():
    assert(differ(None, None) == [['equal', 0, 0, None, None]])
    assert(differ("", "") == [['equal', 0, 0, '', '']])
    assert(differ(b"", b"") == [['equal', 0, 0, b'', b'']])
    assert(differ([], []) == [['equal', 0, 0, [], []]])
    assert(differ({}, {}) == [['equal', 0, 0, {}, {}]])
    assert(differ((), ()) == [['equal', 0, 0, (), ()]])

def test_differ_complex_Nottype():
    assert(differ([None], None) == [['equal', 0, 0, None, None]])
    assert(differ([None], "") == [['delete', 0, None, None, None]])
    assert(differ([None], []) == [['delete', 0, None, None, None]])

def test_2d_list():
    a = ["hoge", "foo", "bar"]
    b = ["fuge", "faa", "bar"]
    assert(differ(a, b, rep_rate=70) == [
        ['replace', 0, 0, 'hoge', 'fuge'],
        ['insert', None, 1, None, 'faa'],
        ['delete', 1, None, 'foo', None],
        ['equal', 2, 2, 'bar', 'bar']
    ])


def memusage():
    return process.memory_info()[0] / 1024


def runtimeit(funcstr, setup=smip, number=100000, normalize=10000):
    i = 0
    st = setup.strip()

    for fc in funcstr.strip().splitlines():
        fc = fc.strip()
        if i == 0:
            timeit(fc, st, number=number)
        bm = memusage()
        p = timeit(fc, st, number=number)
        am = (memusage() - bm)
        assert am < 10000, "{} function {}KB Memory Leak Error".format(fc, am)
        print("{}: {} ns (mem after {}KB)".format(fc, int(p * normalize), am))
        i += 1


def test_dist_perf():
    func = """
    dist('coffee', 'cafe')
    dist(list('coffee'), list('cafe'))
    dist(tuple('coffee'), tuple('cafe'))
    dist(iter('coffee'), iter('cafe'))
    dist('coffee', 'xxxxxx')
    dist('coffee', 'coffee')
    dist(range(4), range(5))
    dist(10, 100)
    """
    print("\n### Perfomance & memory leak check dist func ###")
    runtimeit(func, smip)


def test_similar_perf():
    func = """
    similar('coffee', 'cafe')
    similar(list('coffee'), list('cafe'))
    similar(tuple('coffee'), tuple('cafe'))
    similar(iter('coffee'), iter('cafe'))
    similar('coffee', 'xxxxxx')
    similar('coffee', 'coffee')
    similar(range(4), range(5))
    """
    print("\n### Perfomance & memory leak check similar func ###")
    runtimeit(func, smip)


def test_differ_perf():
    func = """
    differ('coffee', 'cafe')
    differ(list('coffee'), list('cafe'))
    differ(tuple('coffee'), tuple('cafe'))
    differ(iter('coffee'), iter('cafe'))
    differ('coffee', 'xxxxxx')
    differ('coffee', 'coffee')
    differ(range(4), range(5))
    differ(10, 100)
    """
    print("\n### Perfomance & memory leak check differ func ###")
    runtimeit(func, smip)


def test_other_perf():
    smipa = """
    a = dict(zip('012345', 'coffee'))
    b = dict(zip('0123', 'cafe'))
    """.splitlines()
    func = """
    dist(a, b)
    similar(a, b)
    differ(a, b)
    """
    print("\n### Perfomance & memory leak check other func ###")
    runtimeit(func, smip + "\n".join(map(str.strip, smipa)))


if __name__ == '__main__':
    import os
    import traceback

    curdir = os.getcwd()
    try:
        os.chdir(os.path.dirname(os.path.abspath(__file__)))
        for fn, func in dict(locals()).items():
            if fn.startswith("test_"):
                print("Runner: %s" % fn)
                func()
    except Exception as e:
        traceback.print_exc()
        raise (e)
    finally:
        os.chdir(curdir)
