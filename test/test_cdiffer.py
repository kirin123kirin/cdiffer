#!/usr/bin/python
# -*- coding: utf-8 -*-

from timeit import timeit
import os
import sys
from psutil import virtual_memory, Process
process = Process(os.getpid())
print("\n\n", __file__, ":PID -> ", os.getpid(), "\n\n")

try:
    from cdiffer import dist, differ, similar, compare

    smip = "from cdiffer import dist, differ, similar, compare"
except ImportError:
    from cdiffer.cdiffer import dist, differ, similar, compare

    smip = "from cdiffer.cdiffer import dist, differ, similar, compare"


def test_import_dist():
    assert dist


def test_import_differ():
    assert differ


def test_import_similar():
    assert similar

def test_dist_values():
    assert(dist("coffee", "cafe") == 4)
    assert(dist("c", "coffee") == 5)
    assert(dist("ca", "coffee") == 6)
    assert(dist("xxxxxx", "coffee") == 12)
    assert(dist("kafe", "coffee") == 6)
    assert(dist("cofef", "coffee") == 3)
    assert(dist("coffee" * 2, "cafe" * 2) == 8)
    assert(dist("coffee" * 5, "cafe" * 5) == 20)
    assert(dist("coffee" * 10, "cafe" * 10) == 40)
    assert(dist("coffee" * 20, "cafe" * 20) == 80)
    assert(dist("coffee" * 40, "cafe" * 40) == 160)
    assert(dist("coffee" * 80, "cafe" * 80) == 320)
    assert(dist(u'あいう', u'あえう!') == 3)
    assert(dist(u'ＣＯＦＦＥＥ', u'ＣＡＦＥ') == 4)

# # differ_test
def test_differ_binary_test():
    assert (differ(b'coffee', b'cafe'))


ans1 = [['equal', 0, 0, 'c', 'c'],
        ['insert', None, 1, None, 'a'],
        ['delete', 1, None, 'o', None],
        ['equal', 2, 2, 'f', 'f'],
        ['delete', 3, None, 'f', None],
        ['equal', 4, 3, 'e', 'e'],
        ['delete', 5, None, 'e', None]]

ans2 = [['equal', 0, 0, 'c', 'c'],
        ['insert', None, 1, None, 'z'],
        ['delete', 1, None, 'o', None],
        ['delete', 2, None, 'f', None],
        ['delete', 3, None, 'f', None],
        ['delete', 4, None, 'e', None],
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
    assert (dist(list('coffee'), list('cafe')) == 4)


def test_similar_binary_test():
    assert (similar(b'coffee', b'cafe') == 0.6)


def test_similar_string_test():
    assert (similar('coffee', 'cafe') == 0.6)


def test_similar_list_test():
    assert (similar(list('coffee'), list('cafe')) == 0.6)
    assert (similar(list('cafe'), list('cafe')) == 1)
    assert (similar(list('cafe'), list('')) == 0)
    assert (similar(list('cafe'), []) == 0)


def test_similar_tuple_test():
    assert (similar(tuple('coffee'), tuple('cafe')) == 0.6)
    assert (similar(tuple('cafe'), tuple('cafe')) == 1)
    assert (similar(tuple('cafe'), tuple('')) == 0)
    assert (similar(tuple('cafe'), []) == 0)


def test_similar_same_test():
    assert (similar([], []) == 1.0)
    assert (similar(1, 1) == 1.0)


def test_similar_iter_test():
    assert (dist(iter('coffee'), iter('cafe')) == 4)
    assert (similar(iter('coffee'), iter('cafe')) == 0.6)
    assert (differ(iter('cafexyz'), iter('coffeeabcdefghijk'), False, 0) in (
            [['equal', 0, 0, 'c', 'c'],
             ['replace', 1, 1, 'a', 'o'],
             ['insert', None, 2, None, 'f'],
             ['equal', 2, 3, 'f', 'f'],
             ['equal', 3, 4, 'e', 'e'],
             ['replace', 4, 5, 'x', 'e'],
             ['replace', 5, 6, 'y', 'a'],
             ['replace', 6, 7, 'z', 'b'],
             ['insert', None, 8, None, 'c'],
             ['insert', None, 9, None, 'd'],
             ['insert', None, 10, None, 'e'],
             ['insert', None, 11, None, 'f'],
             ['insert', None, 12, None, 'g'],
             ['insert', None, 13, None, 'h'],
             ['insert', None, 14, None, 'i'],
             ['insert', None, 15, None, 'j'],
             ['insert', None, 16, None, 'k']],
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
                ['replace', 3, 12, 'e', 'g'],
                ['replace', 4, 13, 'x', 'h'],
                ['replace', 5, 14, 'y', 'i'],
                ['replace', 6, 15, 'z', 'j'],
                ['insert', None, 16, None, 'k']]
            )
            )


def test_string_test():
    assert (dist('cdfaafe', 'cofeedfajj') == 9)


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
    assert (dist(u'あいう', u'あえう') == 2)
    assert (dist(u'あいう', u'あえう!') == 3)
    assert (differ(u'あいう', u'あえう', False, 0) == ans3)
    assert (differ(u'あいう', u'あえう!', False, 0) == ans4)
    assert (differ(u'あいう!', u'あえう', False, 0) == ans5)


def test_list_test():
    assert (dist(list('cdfaafe'), list('cofeedfajj')) == 9)


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
    pass
    # try:
    #     differ("", [])
    #     raise AssertionError
    # except ValueError:
    #     pass
    # except Exception as e:
    #     raise AssertionError(e)


def test_integer_test():
    assert (similar(10, 100) == 0)
    assert (dist(10, 100) == 2)
    assert (differ(10, 100) == [
        ['delete', 0, None, 10, None],
        ['insert', None, 0, None, 100],
    ])

def test_complex_type():
    assert (dist(list("coffee"), "cafe") == 10)
    assert (dist(list(u'あいう'), u'あえう!') == 7)

def test_dist_Notype():
    assert(dist(None, None) == 0)
    assert(dist("", "") == 0)
    assert(dist(b"", b"") == 0)
    assert(dist([], []) == 0)
    assert(dist({}, {}) == 0)
    assert(dist((), ()) == 0)

def test_dist_complex_Nottype():
    assert(dist([None], None) == 2)
    assert(dist([None], "") == 1)
    assert(dist([None], []) == 1) #@todo tamani 0 ninaru genin fumei

def test_similar_Notype():
    assert(similar(None, None) == 1.0)
    assert(similar("", "") == 1.0)
    assert(similar(b"", b"") == 1.0)
    assert(similar([], []) == 1.0)
    assert(similar({}, {}) == 1.0)
    assert(similar((), ()) == 1.0)

def test_similar_complex_Nottype():
    assert(similar([None], None) == 0.0)
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
    assert(differ([None], None) == [['delete', 0, None, [None], None], ['insert', None, 0, None, None]])
    assert(differ([None], "") == [['insert', None, 0, None, ''], ['delete', 0, None, [None], None]])
    assert(differ([None], []) == [['insert', None, 0, None, []], ['delete', 0, None, [None], None]] )
    assert(differ("", []) == [['delete', 0, None, '', None],['insert', None, 0, None, []]])
    assert(differ(None, "") == [['insert', None, 0, None, ''], ['delete', 0, None, None, None]])
    assert(differ(None, []) == [['insert', None, 0, None, []], ['delete', 0, None, None, None]])
    assert(differ("", []) == [['delete', 0, None, '', None], ['insert', None, 0, None, []]])
    assert(differ([], "") == [['delete', 0, None, [], None], ['insert', None, 0, None, '']])
    assert(differ("", None) == [['delete', 0, None, '', None], ['insert', None, 0, None, None]])
    assert(differ([], None) == [['delete', 0, None, [], None], ['insert', None, 0, None, None]])


def test_differ_value_test1():
    assert differ("c", "coffee") == [["equal", 0, 0, 'c', 'c'],
                                     ["insert", None, 1, None, 'o'],
                                     ["insert", None, 2, None, 'f'],
                                     ["insert", None, 3, None, 'f'],
                                     ["insert", None, 4, None, 'e'],
                                     ["insert", None, 5, None, 'e']]

def test_differ_value_test2():
    assert differ("ca", "coffee", rep_rate=0) == [["equal", 0, 0, 'c', 'c'],
                                                  ["replace", 1, 1, 'a', 'o'],
                                                  ["insert", None, 2, None, 'f'],
                                                  ["insert", None, 3, None, 'f'],
                                                  ["insert", None, 4, None, 'e'],
                                                  ["insert", None, 5, None, 'e']]

def test_differ_value_test3():
    assert differ("cafe", "coffee", rep_rate=0) == [["equal", 0, 0, 'c', 'c'],
                                                    ["replace", 1, 1, 'a', 'o'],
                                                    ["equal", 2, 2, 'f', 'f'],
                                                    ["insert", None, 3, None, 'f'],
                                                    ["equal", 3, 4, 'e', 'e'],
                                                    ["insert", None, 5, None, 'e']]

def test_differ_value_test4():
    assert differ("cofef", "coffee", rep_rate=0) == [["equal", 0, 0, 'c', 'c'],
                                                     ["equal", 1, 1, 'o', 'o'],
                                                     ["equal", 2, 2, 'f', 'f'],
                                                     ["insert", None, 3, None, 'f'],
                                                     ["equal", 3, 4, 'e', 'e'],
                                                     ["replace", 4, 5, 'f', 'e']]

def test_differ_value_test5():
    assert differ("kafe", "coffee", rep_rate=0) == [["replace", 0, 0, 'k', 'c'],
                                                    ["replace", 1, 1, 'a', 'o'],
                                                    ["equal", 2, 2, 'f', 'f'],
                                                    ["insert", None, 3, None, 'f'],
                                                    ["equal", 3, 4, 'e', 'e'],
                                                    ["insert", None, 5, None, 'e']]

def test_differ_value_test6():
    assert differ("xxxxxx", "coffee", rep_rate=0) == [["replace", 0, 0, 'x', 'c'],
                                                      ["replace", 1, 1, 'x', 'o'],
                                                      ["replace", 2, 2, 'x', 'f'],
                                                      ["replace", 3, 3, 'x', 'f'],
                                                      ["replace", 4, 4, 'x', 'e'],
                                                      ["replace", 5, 5, 'x', 'e']]

def test_differ_value_test7():
    assert differ("", "coffee", rep_rate=0) == [["insert", None, 0, None, 'c'],
                                                ["insert", None, 1, None, 'o'],
                                                ["insert", None, 2, None, 'f'],
                                                ["insert", None, 3, None, 'f'],
                                                ["insert", None, 4, None, 'e'],
                                                ["insert", None, 5, None, 'e']]

def test_differ_value_test8():
    assert differ("", "") == [["equal", 0, 0, "", ""]]

def test_differ_value_test9():
    assert differ("c", "coffee", True) == [["insert", None, 1, None, 'o'],
                                           ["insert", None, 2, None, 'f'],
                                           ["insert", None, 3, None, 'f'],
                                           ["insert", None, 4, None, 'e'],
                                           ["insert", None, 5, None, 'e']]

def test_differ_value_test10():
    assert differ("ca", "coffee", True, 0) == [["replace", 1, 1, 'a', 'o'],
                                               ["insert", None, 2, None, 'f'],
                                               ["insert", None, 3, None, 'f'],
                                               ["insert", None, 4, None, 'e'],
                                               ["insert", None, 5, None, 'e']]

def test_differ_value_test11():
    assert differ("cafe", "coffee", True, 0) == [["replace", 1, 1, 'a', 'o'], ["insert", None, 3, None, 'f'], ["insert", None, 5, None, 'e']]

def test_differ_value_test12():
    assert differ("cofef", "coffee", True, 0) == [["insert", None, 3, None, 'f'], ["replace", 4, 5, 'f', 'e']]

def test_differ_value_test13():
    assert differ("kafe", "coffee", True, 0) == [["replace", 0, 0, 'k', 'c'],
                                                 ["replace", 1, 1, 'a', 'o'],
                                                 ["insert", None, 3, None, 'f'],
                                                 ["insert", None, 5, None, 'e']]

def test_differ_value_test14():
    assert differ("xxxxxx", "coffee", True, 0) == [["replace", 0, 0, 'x', 'c'],
                                                   ["replace", 1, 1, 'x', 'o'],
                                                   ["replace", 2, 2, 'x', 'f'],
                                                   ["replace", 3, 3, 'x', 'f'],
                                                   ["replace", 4, 4, 'x', 'e'],
                                                   ["replace", 5, 5, 'x', 'e']]

def test_differ_value_test15():
    assert differ("", "coffee", True, 0) == [["insert", None, 0, None, 'c'],
                                             ["insert", None, 1, None, 'o'],
                                             ["insert", None, 2, None, 'f'],
                                             ["insert", None, 3, None, 'f'],
                                             ["insert", None, 4, None, 'e'],
                                             ["insert", None, 5, None, 'e']]

def test_differ_value_test16():
    assert differ("", "", True) == []

def test_2d_list():
    a = ["hoge", "foo", "bar"]
    b = ["fuge", "faa", "bar"]
    assert(differ(a, b, rep_rate=50) == [
        ['replace', 0, 0, 'hoge', 'fuge'],
        ['delete', 1, None, 'foo', None],
        ['insert', None, 1, None, 'faa'],
        ['equal', 2, 2, 'bar', 'bar']
    ])

def test_differ2d():
    a = [list("abc"), list("abc")]
    b = [list("abc"), list("acc"), list("xtz")]
    assert(differ(a, b, rep_rate=50) == [
        ['equal', 0, 0, ['a', 'b', 'c'], ['a', 'b', 'c']],
        ['replace', 1, 1, ['a', 'b', 'c'], ['a', 'c', 'c']],
        ['insert', None, 2, None, ['x', 't', 'z']]
    ])

def test_compare():
    assert(compare('coffee', 'cafe') == [[60, 'replace', 'c', "ADD ---> 'a'", "'o' ---> DEL", 'f', "'f' ---> DEL", 'e', "'e' ---> DEL"]])
    assert(compare([list("abc"), list("abc")], [list("abc"), list("acc"), list("xtz")], rep_rate=50) == [[40, 'delete', ['a', 'b', 'c'], ['a', 'b', 'c']], [40, 'insert', ['a', 'b', 'c'], ['a', 'c', 'c'], ['x', 't', 'z']]])
    assert(compare(["abc", "abc"], ["abc", "acc", "xtz"], rep_rate=40) == [[40, 'replace', 'abc', "'abc' ---> 'acc'", "ADD ---> 'xtz'"]])
    assert(compare(["abc", "abc"], ["abc", "acc", "xtz"], rep_rate=50) == [[40, 'delete', 'abc', 'abc'], [40, 'insert', 'abc', 'acc', 'xtz']])

def test_compare_Nonetype():
    assert(compare(None, None) == [[100, 'equal', None]])
    if sys.version_info[0] == 3:
        assert(compare([None], [None]) == [[100, 'equal', None]])
    else:
        assert(compare([None], [None]) == [[100, 'equal', [None]]])
    assert(compare([], []) == [[100, 'equal', []]])
    assert(compare("", "") == [[100, 'equal', '']])
    assert(compare(None, "") == [[0, 'delete', "ADD ---> ''"], [0, 'insert', 'None ---> DEL']])
    assert(compare(None, []) == [[0, 'delete', 'ADD ---> []'], [0, 'insert', 'None ---> DEL']])
    assert(compare("", []) == [[0, 'delete', "'' ---> DEL"], [0, 'insert', 'ADD ---> []']])
    assert(compare([], "") == [[0, 'delete', '[] ---> DEL'], [0, 'insert', "ADD ---> ''"]])
    assert(compare("", None) == [[0, 'delete', "'' ---> DEL"], [0, 'insert', 'ADD ---> None']])
    assert(compare([], None) == [[0, 'delete', '[] ---> DEL'], [0, 'insert', 'ADD ---> None']])


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
        assert am < 1000, "{} function {}KB Memory Leak Error".format(fc, am)
        print("{}: {} ns (mem after {}KB)".format(fc, int(p * normalize), am))
        i += 1


def test_dist_perf():
    func = """
    dist('cafe', 'coffee')
    dist('coffee', 'cafe')
    dist('coffee'*2, 'cafe'*2)
    dist('coffee'*5, 'cafe'*5)
    dist('coffee'*10, 'cafe'*10)
    dist('coffee'*20, 'cafe'*20)
    dist('coffee'*40, 'cafe'*40)
    dist('coffee'*80, 'cafe'*80)
    dist(list('coffee'), list('cafe'))
    dist(tuple('coffee'), tuple('cafe'))
    dist(iter('coffee'), iter('cafe'))
    dist('coffee', 'xxxxxx')
    dist('coffee', 'coffee')
    dist(10, 100)
    dist(range(4), range(5))
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
    differ(10, 100)
    differ(range(4), range(5))
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

def test_compare_perf():
    func = """
    compare("coffee", "cafe")
    compare([list("abc"), list("abc")], [list("abc"), list("acc"), list("xtz")], rep_rate=50)
    compare(["abc", "abc"], ["abc", "acc", "xtz"], rep_rate=40)
    compare(["abc", "abc"], ["abc", "acc", "xtz"], rep_rate=50)
    compare(None, None)
    compare([None], [None])
    compare([], [])
    compare("", "")
    compare("", [])
    compare("", None)
    compare(None, "")
    """
    print("\n### Perfomance & memory leak check compare func ###")
    runtimeit(func, smip)


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
