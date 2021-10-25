#!/usr/bin/python
# -*- coding: utf-8 -*-

from timeit import timeit
import os
import sys
os.environ['PYTHONIOENCODING'] = 'utf8'
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
    assert (sorted(differ(iter('coffee'), iter('cafe'))) == sorted(ans1))

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
    assert(dist([None], []) == 1)  # @todo tamani 0 ninaru genin fumei

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
    assert(similar([None], []) == 0.0)  # @todo tamani 0 ninaru genin fumei

def test_differ_Notype():
    assert(differ(None, None) == [['equal', 0, 0, None, None]])
    assert(differ("", "") == [['equal', 0, 0, '', '']])
    assert(differ(b"", b"") == [['equal', 0, 0, b'', b'']])
    assert(differ([], []) == [['equal', 0, 0, [], []]])
    assert(differ({}, {}) == [['equal', 0, 0, {}, {}]])
    assert(differ((), ()) == [['equal', 0, 0, (), ()]])

def test_differ_complex_Nottype():
    assert(differ([None], None) == [['delete', 0, None, None, None]])
    assert(differ([None], "") == [['insert', None, 0, None, ''], ['delete', 0, None, None, None]])
    assert(differ([None], []) == [['insert', None, 0, None, []], ['delete', 0, None, None, None]])
    assert(differ("", []) == [['delete', 0, None, '', None], ['insert', None, 0, None, []]])
    assert(differ(None, "") == [['insert', None, 0, None, '']])
    assert(differ(None, []) == [['insert', None, 0, None, []]])
    assert(differ("", []) == [['delete', 0, None, '', None], ['insert', None, 0, None, []]])
    assert(differ([], "") == [['delete', 0, None, [], None], ['insert', None, 0, None, '']])
    assert(differ("", None) == [['delete', 0, None, '', None]])
    assert(differ([], None) == [['delete', 0, None, [], None]])


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

def test_differ_value_test17():
    assert(differ("aaf", "caf", rep_rate=-1) == [['replace', 0, 0, 'a', 'c'], ['equal', 1, 1, 'a', 'a'], ['equal', 2, 2, 'f', 'f']])
    assert(differ("aaf", "caf") == [['delete', 0, None, 'a', None], ['insert', None, 0, None, 'c'], ['equal', 1, 1, 'a', 'a'], ['equal', 2, 2, 'f', 'f']])


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
        ['equal', 0, 0, ('a', 'b', 'c'), ('a', 'b', 'c')],
        ['replace', 1, 1, ('a', 'b', 'c'), ('a', 'c', 'c')],
        ['insert', None, 2, None, ('x', 't', 'z')]
    ])

def test_compare_1d_array():
    assert(compare("aaf", "caf", rep_rate=-1) == [['tag', 'index_a', 'index_b', 'data'],
                                                  ['replace', 0, 0, 'a ---> c'], ['equal', 1, 1, 'a'], ['equal', 2, 2, 'f']])
    assert(compare('coffee', 'cafe', header=False, delete_sign_value="DEL", insert_sign_value="ADD") == [['equal', 0, 0, 'c'], ['insert', '-', 1, 'ADD ---> a'], [
           'delete', 1, '-', 'o ---> DEL'], ['equal', 2, 2, 'f'], ['delete', 3, '-', 'f ---> DEL'], ['equal', 4, 3, 'e'], ['delete', 5, '-', 'e ---> DEL']])
    assert(compare('coffee', 'cafe', header=False, rep_rate=-1, delete_sign_value="DEL", insert_sign_value="ADD") ==
           [['equal', 0, 0, 'c'], ['replace', 1, 1, 'o ---> a'], ['equal', 2, 2, 'f'], ['delete', 3, '-', 'f ---> DEL'], ['equal', 4, 3, 'e'], ['delete', 5, '-', 'e ---> DEL']])
    assert(compare(["abc", "abc"], ["abc", "acc", "xtz"], rep_rate=40) == [['tag', 'index_a', 'index_b', 'data'],
                                                                           ['equal', 0, 0, 'abc'], ['replace', 1, 1, 'abc ---> acc'], ['insert', '-', 2, 'ADD ---> xtz']])
    assert(compare(["abc", "abc"], ["abc", "acc", "xtz"], rep_rate=50) == [['tag', 'index_a', 'index_b', 'data'],
                                                                           ['equal', 0, 0, 'abc'], ['replace', 1, 1, 'abc ---> acc'], ['insert', '-', 2, 'ADD ---> xtz']])

def test_compare_2d_array():
    assert(compare([list("abc"), list("abc")], [list("abc"), list("acc"), list("xtz")], rep_rate=50) == [['tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'equal', 0, 0, 'a', 'b', 'c'], ['replace', 1, 1, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    if sys.version_info[0] == 3:
        assert(compare([None], [None]) == [['tag', 'index_a', 'index_b', 'data'], ['equal', 0, 0, None]])
    else:
        assert(compare([None], [None]) == [['tag', 'index_a', 'index_b', 'data'], ['equal', 0, 0, [None]]])

def test_compare_Nonetype_complex():
    assert(compare(None, None) == [['tag', 'index_a', 'index_b', 'data'], ['equal', 0, 0, None]])
    assert(repr(compare([], [])) == repr([['tag', 'index_a', 'index_b', 'data'], ['equal', 0, 0, []]]))
    assert(compare("", "") == [['tag', 'index_a', 'index_b', 'data'], ['equal', 0, 0, '']])
    assert(compare(None, "") == [['tag', 'index_a', 'index_b', 'data'], ['insert', '-', 0, 'ADD ---> ']])
    assert(compare("", None) == [['tag', 'index_a', 'index_b', 'data'], ['delete', 0, '-', ' ---> DEL']])
    assert(compare(None, []) ==[['tag', 'index_a', 'index_b', 'data'], ['insert', '-', 0, 'ADD ---> []']])
    assert(compare([], None) == [['tag', 'index_a', 'index_b', 'data'], ['delete', 0, '-', '[] ---> DEL']])
    assert(compare("", []) == [['tag', 'index_a', 'index_b', 'data'], ['delete', 0, '-', ' ---> DEL'], ['insert', '-', 0, 'ADD ---> []']])
    assert(compare([], "") == [['tag', 'index_a', 'index_b', 'data'], ['delete', 0, '-', '[] ---> DEL'], ['insert', '-', 0, 'ADD ---> ']])

def test_compare_rep_rate():
    assert(compare([list('coffee')], [list('cafe')], rep_rate=70) == [['tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03', 'COL_04', 'COL_05'], [
           'delete', 0, '-', 'c ---> DEL', 'o ---> DEL', 'f ---> DEL', 'f ---> DEL', 'e ---> DEL', 'e ---> DEL'], ['insert', '-', 0, 'ADD ---> c', 'ADD ---> a', 'ADD ---> f', 'ADD ---> e']])
    assert(compare([list('coffee')], [list('cafe')], rep_rate=-1) == [['tag', 'index_a', 'index_b', 'COL_00', 'COL_01',
                                                                       'COL_02', 'COL_03', 'COL_04', 'COL_05'], ['replace', 0, 0, 'c', 'o ---> a', 'f', 'f ---> DEL', 'e', 'e ---> DEL']])
    assert(compare([list('coffee')], [list('cafe')], rep_rate=60) == [['tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03',
                                                                       'COL_04', 'COL_05', 'COL_06'], ['replace', 0, 0, 'c', 'ADD ---> a', 'o ---> DEL', 'f', 'f ---> DEL', 'e', 'e ---> DEL']])
    assert(compare([list('cafe')], [list('coffee')], rep_rate=60) == [['tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03',
                                                                       'COL_04', 'COL_05', 'COL_06'], ['replace', 0, 0, 'c', 'a ---> DEL', 'ADD ---> o', 'f', 'ADD ---> f', 'e', 'ADD ---> e']])
    assert(compare([list('cafe')], [list('coffee')], rep_rate=-1) == [['tag', 'index_a', 'index_b', 'COL_00', 'COL_01',
                                                                       'COL_02', 'COL_03', 'COL_04', 'COL_05'], ['replace', 0, 0, 'c', 'a ---> o', 'f', 'ADD ---> f', 'e', 'ADD ---> e']])
    assert(compare([list('cafe')], [list('cafe'), list('cafe')]) == [['tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'],
                                                                     ['equal', 0, 0, 'c', 'a', 'f', 'e'], ['insert', '-', 1, 'ADD ---> c', 'ADD ---> a', 'ADD ---> f', 'ADD ---> e']])
    assert(compare([list('aafe'), list('cafe')], [list('cafe'), list('cafe')], rep_rate=-1) == [['tag', 'index_a', 'index_b',
                                                                                                 'COL_00', 'COL_01', 'COL_02', 'COL_03'], ['replace', 0, 0, 'a ---> c', 'a', 'f', 'e'], ['equal', 1, 1, 'c', 'a', 'f', 'e']])

def test_compare_complex_datatype():
    assert(compare([iter('cafe'), iter('cafe'), iter('cafe')], [iter('cafe'), iter('cafe'), iter('cafe')]) == [['tag', 'index_a', 'index_b', 'COL_00',
                                                                                                                'COL_01', 'COL_02', 'COL_03'], ['equal', 0, 0, 'c', 'a', 'f', 'e'], ['equal', 1, 1, 'c', 'a', 'f', 'e'], ['equal', 2, 2, 'c', 'a', 'f', 'e']])
    assert(compare([tuple('cafe')], [tuple('cafo')], rep_rate=-1) == [['tag', 'index_a', 'index_b',
                                                                       'COL_00', 'COL_01', 'COL_02', 'COL_03'], ['replace', 0, 0, 'c', 'a', 'f', 'e ---> o']])
    assert(compare([list("abc"), list("abc")], [list("abc"), list("acc"), list("xtz")], rep_rate=50) == [['tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'equal', 0, 0, 'a', 'b', 'c'], ['replace', 1, 1, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])

def test_comapare_3d():
    assert(compare(dict(hoge=None), dict(hoge=[list("abc"), list("def")])) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
        'hoge', 'insert', '-', 0, 'ADD ---> a', 'ADD ---> b', 'ADD ---> c'], ['hoge', 'insert', '-', 1, 'ADD ---> d', 'ADD ---> e', 'ADD ---> f']])
    assert(compare(dict(hoge=[list("abc")]), dict(hoge=[list("abc")])) == [
        ['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], ['hoge', 'equal', 0, 0, 'a', 'b', 'c']])
    assert(compare(dict(hoge=[list("abc")]), dict(hoge=[list("abZ")])) == [['group', 'tag', 'index_a', 'index_b',
                                                                            'COL_00', 'COL_01', 'COL_02', 'COL_03'], ['hoge', 'replace', 0, 0, 'a', 'b', 'c ---> DEL', 'ADD ---> Z']])
    assert(compare(dict(hoge=[list("abc")]), dict(hoge=[list("abCefg")])) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03', 'COL_04', 'COL_05'], [
        'hoge', 'delete', 0, '-', 'a ---> DEL', 'b ---> DEL', 'c ---> DEL'], ['hoge', 'insert', '-', 0, 'ADD ---> a', 'ADD ---> b', 'ADD ---> C', 'ADD ---> e', 'ADD ---> f', 'ADD ---> g']])
    assert(compare(dict(hoge=[list("abc"), list("def")]), dict(hoge=[list("abc"), list("def")])) == [['group', 'tag', 'index_a',
                                                                                                      'index_b', 'COL_00', 'COL_01', 'COL_02'], ['hoge', 'equal', 0, 0, 'a', 'b', 'c'], ['hoge', 'equal', 1, 1, 'd', 'e', 'f']])
    assert(compare(dict(hoge=[list("abc"), list("def")]), dict(hoge=[list("abc"), list("DEF")])) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
        'hoge', 'equal', 0, 0, 'a', 'b', 'c'], ['hoge', 'delete', 1, '-', 'd ---> DEL', 'e ---> DEL', 'f ---> DEL'], ['hoge', 'insert', '-', 1, 'ADD ---> D', 'ADD ---> E', 'ADD ---> F']])
    assert(compare(dict(hoge=[list("abc"), list("def")]), dict(hoge=[list("abc"), list("def"), list("ghi")])) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
        'hoge', 'equal', 0, 0, 'a', 'b', 'c'], ['hoge', 'equal', 1, 1, 'd', 'e', 'f'], ['hoge', 'insert', '-', 2, 'ADD ---> g', 'ADD ---> h', 'ADD ---> i']])
    assert(compare(dict(hoge=[list("abc"), list("def"), list("GHI")]), dict(hoge=[list("abc"), list("def"), list("ghi")])) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
        'hoge', 'equal', 0, 0, 'a', 'b', 'c'], ['hoge', 'equal', 1, 1, 'd', 'e', 'f'], ['hoge', 'delete', 2, '-', 'G ---> DEL', 'H ---> DEL', 'I ---> DEL'], ['hoge', 'insert', '-', 2, 'ADD ---> g', 'ADD ---> h', 'ADD ---> i']])
    assert(compare(dict(hoge="foo"), dict(hoge="foo")) == [['group', 'tag', 'index_a', 'index_b'], ['hoge', 'equal', 0, 0, 'foo']])
    assert(compare(dict(hoge="abc"), dict(hoge="abZ")) == [['group', 'tag', 'index_a', 'index_b'], ['hoge', 'replace', 0, 0, 'abc ---> abZ']])

def test_comapare_3d_Complex():
    assert(compare(dict(hoge=[list("あいうえお"), list("あいうえお")]), dict(hoge=[list("あいうえお"), list("あいうあお")])) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01',
                                                                                                              'COL_02', 'COL_03', 'COL_04', 'COL_05'], ['hoge', 'equal', 0, 0, 'あ', 'い', 'う', 'え', 'お'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> DEL', 'ADD ---> あ', 'お']])
    assert(compare(dict(hoge=[list("あいうえお"), list("あいうえお")]), dict(hoge=[list("あいうえお"), list("あいうあお")]), rep_rate=-1) == [['group', 'tag', 'index_a', 'index_b',
                                                                                                                           'COL_00', 'COL_01', 'COL_02', 'COL_03', 'COL_04'], ['hoge', 'equal', 0, 0, 'あ', 'い', 'う', 'え', 'お'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> あ', 'お']])
    assert(compare(dict(sheet1=[list("abc"), list("abc")], sheet2=[list("abc"), list("abc")]), dict(
        sheet1=[list("abc"), list("acc"), list("xtz")], sheet2=[list("abc"), list("abc")]), rep_rate=50) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], ['sheet1', 'equal', 0, 0, 'a', 'b', 'c'], ['sheet1', 'replace', 1, 1, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z'], ['sheet2', 'equal', 0, 0, 'a', 'b', 'c'], ['sheet2', 'equal', 1, 1, 'a', 'b', 'c']])
    assert(compare(dict(sheet1=[list("abc"), list("abc")]), dict(sheet1=[list("abc"), list("acc"), list("xtz")], sheet2=[list("abc"), list("abc")]), rep_rate=50) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], ['sheet1', 'equal', 0, 0, 'a', 'b', 'c'], [
        'sheet1', 'replace', 1, 1, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z'], ['sheet2', 'insert', '-', 0, 'ADD ---> a', 'ADD ---> b', 'ADD ---> c'], ['sheet2', 'insert', '-', 1, 'ADD ---> a', 'ADD ---> b', 'ADD ---> c']])
    assert(compare(dict(sheet1=[list("abc"), list("abc")]), dict(sheet1=[list("acc"), list("abc"), list("xtz")]), rep_rate=50) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
        'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(dict(sheet1=iter([list("abc"), list("abc")])), dict(sheet1=iter([list("abc"), list("acc"), list("xtz")])), rep_rate=50) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
        'sheet1', 'equal', 0, 0, 'a', 'b', 'c'], ['sheet1', 'replace', 1, 1, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(repr(compare(dict(sheet1=iter([])), dict(sheet1=iter([])), rep_rate=50)) == repr(
        [['group', 'tag', 'index_a', 'index_b', 'data'], ['sheet1', 'equal', 0, 0, []]]))

def test_comapare_3d_Nonetype():
    assert(compare(dict(hoge=None), dict(hoge=None)) == [['group', 'tag', 'index_a', 'index_b'], ['hoge', 'equal', 0, 0, None]])
    assert(repr(compare(dict(hoge=[]), dict(hoge=[]))) == repr([['group', 'tag', 'index_a', 'index_b', 'data'], ['hoge', 'equal', 0, 0, []]]))
    assert(compare(dict(hoge=tuple()), dict(hoge=tuple())) == [['group', 'tag', 'index_a', 'index_b', 'data'], ['hoge', 'equal', 0, 0, ()]])
    assert(compare(dict(hoge=tuple([1, 2])), dict(hoge=tuple([1, 2]))) == [
        ['group', 'tag', 'index_a', 'index_b'], ['hoge', 'equal', 0, 0, 1], ['hoge', 'equal', 1, 1, 2]])
    assert(compare(dict(hoge=""), dict(hoge="")) == [['group', 'tag', 'index_a', 'index_b'], ['hoge', 'equal', 0, 0, '']])
    assert(compare(dict(hoge=[]), dict(hoge=None)) == [['group', 'tag', 'index_a', 'index_b', 'data'], ['hoge', 'delete', 0, '-', '[] ---> DEL']])
    assert(compare(dict(hoge=""), dict(hoge=None)) == [['group', 'tag', 'index_a', 'index_b'], ['hoge', 'delete', 0, '-', ' ---> DEL']])
    assert(compare(dict(hoge=None), dict(hoge=[])) == [['group', 'tag', 'index_a', 'index_b', 'data'], ['hoge', 'insert', '-', 0, 'ADD ---> []']])
    assert(compare(dict(hoge=None), dict(hoge="")) == [['group', 'tag', 'index_a', 'index_b'], ['hoge', 'insert', '-', 0, 'ADD ---> ']])
    assert(compare(dict(hoge=tuple([])), dict(hoge=tuple([]))) == [['group', 'tag', 'index_a', 'index_b', 'data'], ['hoge', 'equal', 0, 0, ()]])
    assert(compare(dict(hoge=[[]]), dict(hoge=None)) == [['group', 'tag', 'index_a', 'index_b', 'data'], ['hoge', 'delete', 0, '-', '[] ---> DEL']])
    assert(compare(dict(hoge=None), dict(hoge=[[]])) == [['group', 'tag', 'index_a', 'index_b', 'data'], ['hoge', 'insert', '-', 0, 'ADD ---> []']])
    assert(compare(dict(hoge=1), dict(hoge=1)) == [['group', 'tag', 'index_a', 'index_b'], ['hoge', 'equal', 0, 0, 1]])
    assert(compare(dict(hoge=1), dict(hoge=2)) == [['group', 'tag', 'index_a', 'index_b'], [
        'hoge', 'delete', 0, '-', '1 ---> DEL'], ['hoge', 'insert', '-', 0, 'ADD ---> 2']])


def memusage():
    return process.memory_info()[0] / 1024


def runtimeit(funcstr, setup=smip, number=100000, normalize=10000):
    if (setup == smip):
        st = setup.strip()
    else:
        st = smip.strip() + ";" + setup.strip()

    i = 0

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
    compare([iter('cafe'), iter('cafe'), iter('cafe')], [iter('cafe'), iter('cafe'), iter('cafe')])
    compare([tuple('cafe')], [tuple('cafo')], rep_rate=-1)
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

def test_perf_comapare_2d_3d_default_option():
    runtimeit('compare(dict(hoge=None), dict(hoge=[list("abc"), list("def")]))')
    runtimeit('compare(dict(hoge=[list("abc")]), dict(hoge=[list("abc")]))')
    runtimeit('compare(dict(hoge=[list("abc")]), dict(hoge=[list("abZ")]))')
    runtimeit('compare(dict(hoge=[list("abc")]), dict(hoge=[list("abCefg")]))')
    runtimeit('compare(dict(hoge=[list("abc"), list("def")]), dict(hoge=[list("abc"), list("def")]))')
    runtimeit('compare(dict(hoge=[list("abc"), list("def")]), dict(hoge=[list("abc"), list("DEF")]))')
    runtimeit('compare(dict(hoge=[list("abc"), list("def")]), dict(hoge=[list("abc"), list("def"), list("ghi")]))')
    runtimeit('compare(dict(hoge=[list("abc"), list("def"), list("GHI")]), dict(hoge=[list("abc"), list("def"), list("ghi")]))')
    runtimeit('compare(dict(hoge="foo"), dict(hoge="foo"))')
    runtimeit('compare(dict(hoge="abc"), dict(hoge="abZ"))')

    # runtimeit('compare(dict(hoge=[list("あいうえお"), list("あいうえお")]), dict(hoge=[list("あいうえお"), list("あいうあお")]))')
    # runtimeit('compare(dict(hoge=[list("あいうえお"), list("あいうえお")]), dict(hoge=[list("あいうえお"), list("あいうあお")]), rep_rate = -1)')
    runtimeit(
        'compare(dict(sheet1=[list("abc"), list("abc")], sheet2=[list("abc"), list("abc")]), dict(sheet1=[list("abc"), list("acc"), list("xtz")], sheet2=[list("abc"), list("abc")]), rep_rate=50)')
    runtimeit(
        'compare(dict(sheet1=[list("abc"), list("abc")]), dict(sheet1=[list("abc"), list("acc"), list("xtz")], sheet2=[list("abc"), list("abc")]), rep_rate=50)')
    runtimeit('compare(dict(sheet1=[list("abc"), list("abc")]), dict(sheet1=[list("acc"), list("abc"), list("xtz")]), rep_rate=50)')
    runtimeit('compare(dict(sheet1=iter([list("abc"), list("abc")])), dict(sheet1=iter([list("abc"), list("acc"), list("xtz")])), rep_rate=50)')
    runtimeit('compare(dict(sheet1=iter([])), dict(sheet1=iter([])), rep_rate=50)')

    runtimeit('compare(dict(hoge=None), dict(hoge=None))')
    runtimeit('compare(dict(hoge=[]), dict(hoge=[]))')
    runtimeit('compare(dict(hoge=tuple()), dict(hoge=tuple()))')
    runtimeit('compare(dict(hoge=tuple([1, 2])), dict(hoge=tuple([1, 2])))')
    runtimeit('compare(dict(hoge=""), dict(hoge=""))')
    runtimeit('compare(dict(hoge=[]), dict(hoge=None))')
    runtimeit('compare(dict(hoge=""), dict(hoge=None))')
    runtimeit('compare(dict(hoge=None), dict(hoge=[]))')
    runtimeit('compare(dict(hoge=None), dict(hoge=""))')
    runtimeit('compare(dict(hoge=tuple([])), dict(hoge=tuple([])))')
    runtimeit('compare(dict(hoge=[[]]), dict(hoge=None))')
    runtimeit('compare(dict(hoge=None), dict(hoge=[[]]))')
    runtimeit('compare(dict(hoge=1), dict(hoge=1))')
    runtimeit('compare(dict(hoge=1), dict(hoge=2))')

def test_compare_list_and_perf():
    aa = dict(sheet1=[list("abc"), list("abc")])
    ab = dict(sheet1=[list("acc"), list("abc"), list("xtz")])
    # header
    assert(compare(aa, ab, header=True) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, header=False) == [['sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'],
                                             ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # diffonly
    assert(compare(aa, ab, diffonly=True) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> c', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, diffonly=False) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, aa, diffonly=True) == [['group', 'tag', 'index_a', 'index_b']])
    assert(compare(aa, aa, header=False, diffonly=True) == [])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # rep_rate ##@note 2d array inner rep_rate forcus.
    assert(compare(aa, ab, rep_rate=100) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], ['sheet1', 'delete', 0, '-', 'a ---> DEL', 'b ---> DEL', 'c ---> DEL'], [
           'sheet1', 'insert', '-', 0, 'ADD ---> a', 'ADD ---> c', 'ADD ---> c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=68) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], ['sheet1', 'delete', 0, '-', 'a ---> DEL', 'b ---> DEL', 'c ---> DEL'], [
           'sheet1', 'insert', '-', 0, 'ADD ---> a', 'ADD ---> c', 'ADD ---> c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=67) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=41) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=40) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=-1) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], ['sheet1', 'replace', 0, 0,
                                                                                                                   'a', 'b ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, rep_rate=68)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    runtimeit('compare(aa, ab, rep_rate=40)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # startidx
    assert(compare(aa, ab, startidx=100) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], ['sheet1', 'replace', 100, 100, 'a',
                                                                                                                              'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 101, 101, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 102, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # condition_value
    assert(compare(aa, ab, rep_rate=-1, condition_value="@@changed@@") == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
           'sheet1', 'replace', 0, 0, 'a', 'b@@changed@@c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD@@changed@@x', 'ADD@@changed@@t', 'ADD@@changed@@z']])
    runtimeit('compare(aa, ab, rep_rate=-1, condition_value="@@changed@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # na_value
    assert(compare(aa, ab, header=False, na_value="@@") == [['sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'],
                                                            ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '@@', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, header=False, na_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # delete_sign_value
    assert(compare(aa, ab, header=False, delete_sign_value="@@") == [['sheet1', 'replace', 0, 0, 'a', 'b ---> @@', 'ADD ---> c', 'c'], [
           'sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, header=False, delete_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # # insert_sign_value
    assert(compare(aa, ab, header=False, insert_sign_value="@@") == [['sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', '@@ ---> c', 'c'], [
           'sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, '@@ ---> x', '@@ ---> t', '@@ ---> z']])
    runtimeit('compare(aa, ab, header=False, insert_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))

def test_compare_unicode_of_list_and_perf():
    aa = dict(hoge=[list(u"あいうえお"), list(u"あいうえお")])
    ab = dict(hoge=[list(u"あいうえお"), list(u"あいうあお")])
    # header
    assert(compare(aa, ab, header=True) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03', 'COL_04', 'COL_05'], [
           'hoge', 'equal', 0, 0, 'あ', 'い', 'う', 'え', 'お'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> DEL', 'ADD ---> あ', 'お']])
    assert(compare(aa, ab, header=False) == [['hoge', 'equal', 0, 0, 'あ', 'い', 'う', 'え', 'お'],
                                             ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> DEL', 'ADD ---> あ', 'お']])
    # runtimeit('compare(aa, ab, header=False)', 'aa = dict(hoge=[list(r"あいうえお"), list(r"あいうえお")]);ab = dict(hoge=[list(r"あいうえお"), list(r"あいうあお")])')
    # diffonly
    assert(compare(aa, ab, diffonly=True) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01',
                                               'COL_02', 'COL_03', 'COL_04'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> あ', 'お']])
    assert(compare(aa, ab, diffonly=False) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03', 'COL_04'], [
           'hoge', 'equal', 0, 0, 'あ', 'い', 'う', 'え', 'お'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> あ', 'お']])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # #rep_rate ##@note 2d array inner rep_rate forcus.
    assert(compare(aa, ab, rep_rate=100) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03', 'COL_04'], ['hoge', 'equal', 0, 0, 'あ', 'い', 'う', 'え', 'お'], [
           'hoge', 'delete', 1, '-', 'あ ---> DEL', 'い ---> DEL', 'う ---> DEL', 'え ---> DEL', 'お ---> DEL'], ['hoge', 'insert', '-', 1, 'ADD ---> あ', 'ADD ---> い', 'ADD ---> う', 'ADD ---> あ', 'ADD ---> お']])
    assert(compare(aa, ab, rep_rate=81) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03', 'COL_04'], ['hoge', 'equal', 0, 0, 'あ', 'い', 'う', 'え', 'お'], [
           'hoge', 'delete', 1, '-', 'あ ---> DEL', 'い ---> DEL', 'う ---> DEL', 'え ---> DEL', 'お ---> DEL'], ['hoge', 'insert', '-', 1, 'ADD ---> あ', 'ADD ---> い', 'ADD ---> う', 'ADD ---> あ', 'ADD ---> お']])
    assert(compare(aa, ab, rep_rate=80) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03', 'COL_04', 'COL_05'], [
           'hoge', 'equal', 0, 0, 'あ', 'い', 'う', 'え', 'お'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> DEL', 'ADD ---> あ', 'お']])
    assert(compare(aa, ab, rep_rate=51) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03', 'COL_04', 'COL_05'], [
           'hoge', 'equal', 0, 0, 'あ', 'い', 'う', 'え', 'お'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> DEL', 'ADD ---> あ', 'お']])
    assert(compare(aa, ab, rep_rate=50) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03', 'COL_04', 'COL_05'], [
           'hoge', 'equal', 0, 0, 'あ', 'い', 'う', 'え', 'お'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> DEL', 'ADD ---> あ', 'お']])
    assert(compare(aa, ab, rep_rate=-1) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03',
                                             'COL_04'], ['hoge', 'equal', 0, 0, 'あ', 'い', 'う', 'え', 'お'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> あ', 'お']])
    runtimeit('compare(aa, ab, rep_rate=81)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    runtimeit('compare(aa, ab, rep_rate=50)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # startidx
    assert(compare(aa, ab, startidx=100) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03', 'COL_04', 'COL_05'], [
           'hoge', 'equal', 100, 100, 'あ', 'い', 'う', 'え', 'お'], ['hoge', 'replace', 101, 101, 'あ', 'い', 'う', 'え ---> DEL', 'ADD ---> あ', 'お']])
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # condition_value
    assert(compare(aa, ab, rep_rate=-1, condition_value="@@changed@@") == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03', 'COL_04'], [
           'hoge', 'equal', 0, 0, 'あ', 'い', 'う', 'え', 'お'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え@@changed@@あ', 'お']])
    runtimeit('compare(aa, ab, rep_rate=-1, condition_value="@@changed@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # na_value
    assert(compare(aa, ab, header=False, na_value="@@") == [['hoge', 'equal', 0, 0, 'あ', 'い', 'う',
                                                             'え', 'お'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> DEL', 'ADD ---> あ', 'お']])
    runtimeit('compare(aa, ab, header=False, na_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # delete_sign_value
    assert(compare(aa, ab, header=False, delete_sign_value="@@") == [['hoge', 'equal', 0, 0, 'あ',
                                                                      'い', 'う', 'え', 'お'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> @@', 'ADD ---> あ', 'お']])
    runtimeit('compare(aa, ab, header=False, delete_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # insert_sign_value
    assert(compare(aa, ab, header=False, insert_sign_value="@@") == [['hoge', 'equal', 0, 0, 'あ',
                                                                      'い', 'う', 'え', 'お'], ['hoge', 'replace', 1, 1, 'あ', 'い', 'う', 'え ---> DEL', '@@ ---> あ', 'お']])
    runtimeit('compare(aa, ab, header=False, insert_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))

def test_compare_tuple_and_perf():
    aa = dict(sheet1=[tuple("abc"), tuple("abc")])
    ab = dict(sheet1=[tuple("acc"), tuple("abc"), tuple("xtz")])
    # header
    assert(compare(aa, ab, header=True) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, header=False) == [['sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'],
                                             ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # diffonly
    assert(compare(aa, ab, diffonly=True) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> c', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, diffonly=False) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # rep_rate ##@note 2d array inner rep_rate forcus.
    assert(compare(aa, ab, rep_rate=100) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], ['sheet1', 'delete', 0, '-', 'a ---> DEL', 'b ---> DEL', 'c ---> DEL'], [
           'sheet1', 'insert', '-', 0, 'ADD ---> a', 'ADD ---> c', 'ADD ---> c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=68) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], ['sheet1', 'delete', 0, '-', 'a ---> DEL', 'b ---> DEL', 'c ---> DEL'], [
           'sheet1', 'insert', '-', 0, 'ADD ---> a', 'ADD ---> c', 'ADD ---> c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=67) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=41) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=40) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=-1) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], ['sheet1', 'replace', 0, 0,
                                                                                                                   'a', 'b ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, rep_rate=68)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    runtimeit('compare(aa, ab, rep_rate=40)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # startidx
    assert(compare(aa, ab, startidx=100) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], ['sheet1', 'replace', 100, 100, 'a',
                                                                                                                              'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 101, 101, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 102, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # condition_value
    assert(compare(aa, ab, rep_rate=-1, condition_value="@@changed@@") == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
           'sheet1', 'replace', 0, 0, 'a', 'b@@changed@@c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD@@changed@@x', 'ADD@@changed@@t', 'ADD@@changed@@z']])
    runtimeit('compare(aa, ab, rep_rate=-1, condition_value="@@changed@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # na_value
    assert(compare(aa, ab, header=False, na_value="@@") == [['sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'],
                                                            ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '@@', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, header=False, na_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # delete_sign_value
    assert(compare(aa, ab, header=False, delete_sign_value="@@") == [['sheet1', 'replace', 0, 0, 'a', 'b ---> @@', 'ADD ---> c', 'c'], [
           'sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, header=False, delete_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # # insert_sign_value
    assert(compare(aa, ab, header=False, insert_sign_value="@@") == [['sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', '@@ ---> c', 'c'], [
           'sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, '@@ ---> x', '@@ ---> t', '@@ ---> z']])
    runtimeit('compare(aa, ab, header=False, insert_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))

def test_compare_Nonetype_and_perf():
    aa = dict(sheet1=None)
    ab = dict(sheet1=None)
    # header
    assert(compare(aa, ab, header=True) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'equal', 0, 0, None]])
    assert(compare(aa, ab, header=False) == [['sheet1', 'equal', 0, 0, None]])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # diffonly
    assert(compare(aa, ab, diffonly=True) == [['group', 'tag', 'index_a', 'index_b']])
    assert(compare(aa, ab, diffonly=False) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'equal', 0, 0, None]])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # rep_rate ##@note 2d array inner rep_rate forcus.
    assert(compare(aa, ab, rep_rate=100) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'equal', 0, 0, None]])
    assert(compare(aa, ab, rep_rate=50) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'equal', 0, 0, None]])
    assert(compare(aa, ab, rep_rate=-1) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'equal', 0, 0, None]])
    runtimeit('compare(aa, ab, rep_rate=50)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # startidx
    assert(compare(aa, ab, startidx=100) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'equal', 100, 100, None]])
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # condition_value
    assert(compare(aa, ab, rep_rate=-1, condition_value="@@changed@@") == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'equal', 0, 0, None]])
    runtimeit('compare(aa, ab, rep_rate=-1, condition_value="@@changed@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # na_value
    assert(compare(aa, ab, header=False, na_value="@@") == [['sheet1', 'equal', 0, 0, None]])
    runtimeit('compare(aa, ab, header=False, na_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # delete_sign_value
    assert(compare(aa, ab, header=False, delete_sign_value="@@") == [['sheet1', 'equal', 0, 0, None]])
    runtimeit('compare(aa, ab, header=False, delete_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # # insert_sign_value
    assert(compare(aa, ab, header=False, insert_sign_value="@@") == [['sheet1', 'equal', 0, 0, None]])
    runtimeit('compare(aa, ab, header=False, insert_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))

def test_compare_iterator_and_perf():
    aa = dict(sheet1=[iter("abc"), iter("abc")])
    ab = dict(sheet1=[iter("acc"), iter("abc"), iter("xtz")])
    # header
    assert(compare(aa, ab, header=True) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, header=False) == [['sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'],
                                             ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # diffonly
    assert(compare(aa, ab, diffonly=True) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> c', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, diffonly=False) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # rep_rate ##@note 2d array inner rep_rate forcus.
    assert(compare(aa, ab, rep_rate=100) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], ['sheet1', 'delete', 0, '-', 'a ---> DEL', 'b ---> DEL', 'c ---> DEL'], [
           'sheet1', 'insert', '-', 0, 'ADD ---> a', 'ADD ---> c', 'ADD ---> c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=68) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], ['sheet1', 'delete', 0, '-', 'a ---> DEL', 'b ---> DEL', 'c ---> DEL'], [
           'sheet1', 'insert', '-', 0, 'ADD ---> a', 'ADD ---> c', 'ADD ---> c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=67) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=41) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=40) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], [
           'sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    assert(compare(aa, ab, rep_rate=-1) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], ['sheet1', 'replace', 0, 0,
                                                                                                                   'a', 'b ---> c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, rep_rate=68)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    runtimeit('compare(aa, ab, rep_rate=40)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # startidx
    assert(compare(aa, ab, startidx=100) == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], ['sheet1', 'replace', 100, 100, 'a',
                                                                                                                              'b ---> DEL', 'ADD ---> c', 'c'], ['sheet1', 'equal', 101, 101, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 102, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # condition_value
    assert(compare(aa, ab, rep_rate=-1, condition_value="@@changed@@") == [['group', 'tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02'], [
           'sheet1', 'replace', 0, 0, 'a', 'b@@changed@@c', 'c'], ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD@@changed@@x', 'ADD@@changed@@t', 'ADD@@changed@@z']])
    runtimeit('compare(aa, ab, rep_rate=-1, condition_value="@@changed@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # na_value
    assert(compare(aa, ab, header=False, na_value="@@") == [['sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', 'ADD ---> c', 'c'],
                                                            ['sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '@@', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, header=False, na_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # delete_sign_value
    assert(compare(aa, ab, header=False, delete_sign_value="@@") == [['sheet1', 'replace', 0, 0, 'a', 'b ---> @@', 'ADD ---> c', 'c'], [
           'sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, 'ADD ---> x', 'ADD ---> t', 'ADD ---> z']])
    runtimeit('compare(aa, ab, header=False, delete_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # # insert_sign_value
    assert(compare(aa, ab, header=False, insert_sign_value="@@") == [['sheet1', 'replace', 0, 0, 'a', 'b ---> DEL', '@@ ---> c', 'c'], [
           'sheet1', 'equal', 1, 1, 'a', 'b', 'c'], ['sheet1', 'insert', '-', 2, '@@ ---> x', '@@ ---> t', '@@ ---> z']])
    runtimeit('compare(aa, ab, header=False, insert_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))

def test_compare_number_and_perf():
    aa = dict(sheet1=1)
    ab = dict(sheet1=2)
    # header
    assert(compare(aa, ab, header=True) == [['group', 'tag', 'index_a', 'index_b'], [
           'sheet1', 'delete', 0, '-', '1 ---> DEL'], ['sheet1', 'insert', '-', 0, 'ADD ---> 2']])
    assert(compare(aa, ab, header=False) == [['sheet1', 'delete', 0, '-', '1 ---> DEL'], ['sheet1', 'insert', '-', 0, 'ADD ---> 2']])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # diffonly
    assert(compare(aa, ab, diffonly=True) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'replace', 0, 0, '1 ---> 2']])
    assert(compare(aa, ab, diffonly=False) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'replace', 0, 0, '1 ---> 2']])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # rep_rate ##@note 2d array inner rep_rate forcus.
    assert(compare(aa, ab, rep_rate=100) == [['group', 'tag', 'index_a', 'index_b'], [
           'sheet1', 'delete', 0, '-', '1 ---> DEL'], ['sheet1', 'insert', '-', 0, 'ADD ---> 2']])
    assert(compare(aa, ab, rep_rate=50) == [['group', 'tag', 'index_a', 'index_b'], [
           'sheet1', 'delete', 0, '-', '1 ---> DEL'], ['sheet1', 'insert', '-', 0, 'ADD ---> 2']])
    assert(compare(aa, ab, rep_rate=-1) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'replace', 0, 0, '1 ---> 2']])
    runtimeit('compare(aa, ab, rep_rate=50)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # startidx
    assert(compare(aa, ab, startidx=100) == [['group', 'tag', 'index_a', 'index_b'], [
           'sheet1', 'delete', 100, '-', '1 ---> DEL'], ['sheet1', 'insert', '-', 100, 'ADD ---> 2']])
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # condition_value
    assert(compare(aa, ab, rep_rate=-1, condition_value="@@changed@@") ==
           [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'replace', 0, 0, '1@@changed@@2']])
    runtimeit('compare(aa, ab, rep_rate=-1, condition_value="@@changed@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # na_value
    assert(compare(aa, ab, header=False, na_value="@@") == [['sheet1', 'delete', 0, '@@', '1 ---> DEL'], ['sheet1', 'insert', '@@', 0, 'ADD ---> 2']])
    runtimeit('compare(aa, ab, header=False, na_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # delete_sign_value
    assert(compare(aa, ab, header=False, delete_sign_value="@@") == [['sheet1', 'delete', 0, '-', '1 ---> @@'], ['sheet1', 'insert', '-', 0, 'ADD ---> 2']])
    runtimeit('compare(aa, ab, header=False, delete_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # # insert_sign_value
    assert(compare(aa, ab, header=False, insert_sign_value="@@") == [['sheet1', 'delete', 0, '-', '1 ---> DEL'], ['sheet1', 'insert', '-', 0, '@@ ---> 2']])
    runtimeit('compare(aa, ab, header=False, insert_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))

def test_compare_1d_string_and_perf():
    aa = dict(sheet1="foo")
    ab = dict(sheet1="bar")
    # header
    assert(compare(aa, ab, header=True) == [['group', 'tag', 'index_a', 'index_b'], [
           'sheet1', 'delete', 0, '-', 'foo ---> DEL'], ['sheet1', 'insert', '-', 0, 'ADD ---> bar']])
    assert(compare(aa, ab, header=False) == [['sheet1', 'delete', 0, '-', 'foo ---> DEL'], ['sheet1', 'insert', '-', 0, 'ADD ---> bar']])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # diffonly
    assert(compare(aa, ab, diffonly=True) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'replace', 0, 0, 'foo ---> bar']])
    assert(compare(aa, ab, diffonly=False) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'replace', 0, 0, 'foo ---> bar']])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # rep_rate ##@note 2d array inner rep_rate forcus.
    assert(compare(aa, ab, rep_rate=100) == [['group', 'tag', 'index_a', 'index_b'], [
           'sheet1', 'delete', 0, '-', 'foo ---> DEL'], ['sheet1', 'insert', '-', 0, 'ADD ---> bar']])
    assert(compare(aa, ab, rep_rate=50) == [['group', 'tag', 'index_a', 'index_b'], [
           'sheet1', 'delete', 0, '-', 'foo ---> DEL'], ['sheet1', 'insert', '-', 0, 'ADD ---> bar']])
    assert(compare(aa, ab, rep_rate=-1) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'replace', 0, 0, 'foo ---> bar']])
    runtimeit('compare(aa, ab, rep_rate=50)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # startidx
    assert(compare(aa, ab, startidx=100) == [['group', 'tag', 'index_a', 'index_b'], [
           'sheet1', 'delete', 100, '-', 'foo ---> DEL'], ['sheet1', 'insert', '-', 100, 'ADD ---> bar']])
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # condition_value
    assert(compare(aa, ab, rep_rate=-1, condition_value="@@changed@@") ==
           [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'replace', 0, 0, 'foo@@changed@@bar']])
    runtimeit('compare(aa, ab, rep_rate=-1, condition_value="@@changed@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # na_value
    assert(compare(aa, ab, header=False, na_value="@@") == [['sheet1', 'delete', 0, '@@', 'foo ---> DEL'], ['sheet1', 'insert', '@@', 0, 'ADD ---> bar']])
    runtimeit('compare(aa, ab, header=False, na_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # delete_sign_value
    assert(compare(aa, ab, header=False, delete_sign_value="@@") == [['sheet1', 'delete', 0, '-', 'foo ---> @@'], ['sheet1', 'insert', '-', 0, 'ADD ---> bar']])
    runtimeit('compare(aa, ab, header=False, delete_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # # insert_sign_value
    assert(compare(aa, ab, header=False, insert_sign_value="@@") == [['sheet1', 'delete', 0, '-', 'foo ---> DEL'], ['sheet1', 'insert', '-', 0, '@@ ---> bar']])
    runtimeit('compare(aa, ab, header=False, insert_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))

def test_compare_1d_bytes_and_perf():
    aa = dict(sheet1=b"foo")
    ab = dict(sheet1=b"bar")
    # header
    assert(compare(aa, ab, header=True) == [['group', 'tag', 'index_a', 'index_b'], [
           'sheet1', 'delete', 0, '-', "b'foo' ---> DEL"], ['sheet1', 'insert', '-', 0, "ADD ---> b'bar'"]])
    assert(compare(aa, ab, header=False) == [['sheet1', 'delete', 0, '-', "b'foo' ---> DEL"], ['sheet1', 'insert', '-', 0, "ADD ---> b'bar'"]])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # diffonly
    assert(compare(aa, ab, diffonly=True) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'replace', 0, 0, "b'foo' ---> b'bar'"]])
    assert(compare(aa, ab, diffonly=False) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'replace', 0, 0, "b'foo' ---> b'bar'"]])
    runtimeit('compare(aa, ab, diffonly=True)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # rep_rate ##@note 2d array inner rep_rate forcus.
    assert(compare(aa, ab, rep_rate=100) == [['group', 'tag', 'index_a', 'index_b'], [
           'sheet1', 'delete', 0, '-', "b'foo' ---> DEL"], ['sheet1', 'insert', '-', 0, "ADD ---> b'bar'"]])
    assert(compare(aa, ab, rep_rate=50) == [['group', 'tag', 'index_a', 'index_b'], [
           'sheet1', 'delete', 0, '-', "b'foo' ---> DEL"], ['sheet1', 'insert', '-', 0, "ADD ---> b'bar'"]])
    assert(compare(aa, ab, rep_rate=-1) == [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'replace', 0, 0, "b'foo' ---> b'bar'"]])
    runtimeit('compare(aa, ab, rep_rate=50)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # startidx
    assert(compare(aa, ab, startidx=100) == [['group', 'tag', 'index_a', 'index_b'], [
           'sheet1', 'delete', 100, '-', "b'foo' ---> DEL"], ['sheet1', 'insert', '-', 100, "ADD ---> b'bar'"]])
    runtimeit('compare(aa, ab, rep_rate=-1)', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # condition_value
    assert(compare(aa, ab, rep_rate=-1, condition_value="@@changed@@") ==
           [['group', 'tag', 'index_a', 'index_b'], ['sheet1', 'replace', 0, 0, "b'foo'@@changed@@b'bar'"]])
    runtimeit('compare(aa, ab, rep_rate=-1, condition_value="@@changed@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # na_value
    assert(compare(aa, ab, header=False, na_value="@@") == [['sheet1', 'delete', 0, '@@', "b'foo' ---> DEL"], ['sheet1', 'insert', '@@', 0, "ADD ---> b'bar'"]])
    runtimeit('compare(aa, ab, header=False, na_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # delete_sign_value
    assert(compare(aa, ab, header=False, delete_sign_value="@@") ==
           [['sheet1', 'delete', 0, '-', "b'foo' ---> @@"], ['sheet1', 'insert', '-', 0, "ADD ---> b'bar'"]])
    runtimeit('compare(aa, ab, header=False, delete_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))
    # # insert_sign_value
    assert(compare(aa, ab, header=False, insert_sign_value="@@") ==
           [['sheet1', 'delete', 0, '-', "b'foo' ---> DEL"], ['sheet1', 'insert', '-', 0, "@@ ---> b'bar'"]])
    runtimeit('compare(aa, ab, header=False, insert_sign_value = "@@")', 'aa = {aa};ab = {ab}'.format(aa=aa, ab=ab))


data_a = """24,2372,15,toyota corona mark ii
18,2774,15.5,amc hornet
21,2587,16,ford maverick
25,"2489",15,honda civic
24,あ2430,14.5,audi 100 ls
26,1835,20.5,volkswagen 1131 deluxe sedan
""".splitlines()
la = [x.split(",") for x in data_a]

data_b = """24,2372,15,toyota corona mark ii
22,2833,15.5,plymouth duster
18,2774,15.5,amc hornet
21,2587,16,ford maverick
27,2130,14.5,datsun pl510
25,"2489?",15,honda civic
""".splitlines()
lb = [x.split(",") for x in data_b]

def test_compare_key_1d_keysort_and_perf():
    assert(compare(data_a, data_b, keya=lambda x: x, keyb=lambda x: x) == [['tag', 'index_a', 'index_b', 'data'], ['equal', 0, 0, '24,2372,15,toyota corona mark ii'], ['insert', '-', 1, 'ADD ---> 22,2833,15.5,plymouth duster'], ['equal', 1, 2, '18,2774,15.5,amc hornet'], ['equal', 2, 3, '21,2587,16,ford maverick'], ['delete', 3, '-', '25,"2489",15,honda civic ---> DEL'], ['delete', 4, '-', '24,あ2430,14.5,audi 100 ls ---> DEL'], ['insert', '-', 4, 'ADD ---> 27,2130,14.5,datsun pl510'], ['delete', 5, '-', '26,1835,20.5,volkswagen 1131 deluxe sedan ---> DEL'], ['insert', '-', 5, 'ADD ---> 25,"2489?",15,honda civic']])
    runtimeit('compare(data_a, data_b, keya=lambda x: x, keyb=lambda x: x)', 'data_a={};data_b={}'.format(data_a, data_b))

def test_compare_key_list_of_list_and_perf():
    assert(compare(la, lb, keya=lambda x: x[-1], keyb=lambda x: x[-1]) == [['tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], ['equal', 0, 0, '24', '2372', '15', 'toyota corona mark ii'], ['insert', '-', 1, 'ADD ---> 22', 'ADD ---> 2833', 'ADD ---> 15.5', 'ADD ---> plymouth duster'], ['equal', 1, 2, '18', '2774', '15.5', 'amc hornet'], ['equal', 2, 3, '21', '2587', '16', 'ford maverick'], ['replace', 3, 5, '25', '"2489" ---> "2489?"', '15', 'honda civic'], ['delete', 4, '-', '24 ---> DEL', 'あ2430 ---> DEL', '14.5 ---> DEL', 'audi 100 ls ---> DEL'], ['insert', '-', 4, 'ADD ---> 27', 'ADD ---> 2130', 'ADD ---> 14.5', 'ADD ---> datsun pl510'], ['delete', 5, '-', '26 ---> DEL', '1835 ---> DEL', '20.5 ---> DEL', 'volkswagen 1131 deluxe sedan ---> DEL']])
    assert(compare(la, lb, keya=lambda x: x[-1], keyb=lambda x: x[-1]) == compare(la, lb, keya=lambda x: x[-1], keyb=lambda x: x[-1]))
    runtimeit('compare(la, lb, keya=lambda x: x[-1], keyb=lambda x: x[-1])', 'la={};lb={}'.format(la, lb))

def test_compare_key_iter_of_list_and_perf():
    a = list(map(iter, la))
    b = list(map(iter, lb))
    assert(compare(a, b, keya=lambda x: x[-1], keyb=lambda x: x[-1]) == [['tag', 'index_a', 'index_b', 'COL_00', 'COL_01', 'COL_02', 'COL_03'], ['equal', 0, 0, '24', '2372', '15', 'toyota corona mark ii'], ['insert', '-', 1, 'ADD ---> 22', 'ADD ---> 2833', 'ADD ---> 15.5', 'ADD ---> plymouth duster'], ['equal', 1, 2, '18', '2774', '15.5', 'amc hornet'], ['equal', 2, 3, '21', '2587', '16', 'ford maverick'], ['replace', 3, 5, '25', '"2489" ---> "2489?"', '15', 'honda civic'], ['delete', 4, '-', '24 ---> DEL', 'あ2430 ---> DEL', '14.5 ---> DEL', 'audi 100 ls ---> DEL'], ['insert', '-', 4, 'ADD ---> 27', 'ADD ---> 2130', 'ADD ---> 14.5', 'ADD ---> datsun pl510'], ['delete', 5, '-', '26 ---> DEL', '1835 ---> DEL', '20.5 ---> DEL', 'volkswagen 1131 deluxe sedan ---> DEL']])
    try:
        compare(a, b, keya=lambda x: x[-1], keyb=lambda x: x[-1])
    except RuntimeError:
        assert(True)
    else:
        assert(False)
    runtimeit('compare(list(map(iter, la)), list(map(iter, lb)), keya=lambda x: x[-1], keyb=lambda x: x[-1])', 'la={};lb={}'.format(la, lb))


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
