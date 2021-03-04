import unittest

class TestEditDistance(unittest.TestCase):
    def test_editdistance(self):
        from cdiffer import distance
        self.assertEqual(1, distance('abc', 'aec'))
    

if __name__ == '__main__':
    unittest.main()