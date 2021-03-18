#!/usr/bin/python
# -*- coding: utf-8 -*-

# import cdiffer
from cdiffer import dist, differ, similar


def test_import_dist():
    assert dist
    # assert cdiffer.dist


def test_import_differ():
    assert differ
    # assert cdiffer.differ


def test_import_similar():
    assert similar
    # assert cdiffer.similar


# differ_test
def test_differ_binary_test():
    assert (differ(b'coffee', b'cafe'))


def test_differ_string_test():
    assert (differ('coffee', 'cafe'))


def test_differ_list_test():
    assert (differ(list('coffee'), list('cafe')))
    assert (differ(list('coffee'), list('cz')))


def test_differ_iter_test():
    assert (differ(iter('coffee'), iter('cafe')))


def test_diffonly_flag_test():
    assert (differ('coffee', 'cafe', True))


def test_dist_list_test():
    assert (dist(list('coffee'), list('cafe'))==3)


def test_similar_binary_test():
    assert (similar(b'coffee', b'cafe')==0.6)


def test_similar_string_test():
    assert (similar('coffee', 'cafe')==0.6)


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
    assert (similar([], [])==1.0)
    assert (similar(1, 1)==1.0)


def test_similar_iter_test():
    assert (similar(iter('coffee'), iter('cafe'))==0.6)
    assert (differ(iter('cafexyz'), iter('coffeeabcdefghijk')))


def test_string_test():
    assert (dist('cdfaafe', 'cofeedfajj')==7)


def test_list_test():
    assert (dist(list('cdfaafe'), list('cofeedfajj'))==7)


def test_dict_string_test():
    assert (similar(dict(zip('012345', 'coffee')), dict(zip('0123', 'cafe')))==0.8)
    assert (dist(dict(zip('012345', 'coffee')), dict(zip('0123', 'cafe')))==2)
    assert (differ(dict(zip('012345', 'coffee')), dict(zip('0123', 'cafe'))))


def test_Error_Test():
    try:
        dist(dict(zip('012345', 'coffee')).keys(), dict(zip('0123', 'cafe')).keys())
    except TypeError:
        pass
    except Exception as e:
        raise AssertionError(e)

    try:
        dist(dict(zip('012345', 'coffee')).values(), dict(zip('0123', 'cafe')).values())
    except TypeError:
        pass
    except Exception as e:
        raise AssertionError(e)

    try:
        similar(dict(zip('012345', 'coffee')).keys(), dict(zip('0123', 'cafe')).keys())
    except TypeError:
        pass
    except Exception as e:
        raise AssertionError(e)

    try:
        similar(dict(zip('012345', 'coffee')).values(), dict(zip('0123', 'cafe')).values())
    except TypeError:
        pass
    except Exception as e:
        raise AssertionError(e)

    try:
        differ(dict(zip('012345', 'coffee')).keys(), dict(zip('0123', 'cafe')).keys())
    except TypeError:
        pass
    except Exception as e:
        raise AssertionError(e)

    try:
        differ(dict(zip('012345', 'coffee')).values(), dict(zip('0123', 'cafe')).values())
    except TypeError:
        pass
    except Exception as e:
        raise AssertionError(e)


def test_integer_test():
    assert (similar(10, 100) == 0)
    assert (dist(10, 100) == 2)
    assert (differ(10, 100) == [['replace', 0, 0, 10, 100]])


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
        raise(e)
    finally:
        os.chdir(curdir)
