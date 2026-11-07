import unittest
import sys

sys.path.append("../amp-setup/scripts/")
import stats

TOLERANCE = 0.000001

class TestStringMethods(unittest.TestCase):

    def test_output(self):
        array1 = [0.1 for _ in range (0, 16)]
        array2 = [1.0 for _ in range (0, 16)]
        array3 = [0.1 for _ in range (0, 15)]
        array4 = [1.0 for _ in range (0, 16)]
        a1 = [3, 4, 1, 2.1]
        a2 = [490.2, 340, 433.9]

        p_val = stats.p_value(array1, len(array1), array1, len(array1))
        reference = 1.0
        self.assertEqual(p_val, reference)

        p_val = stats.p_value(array1, len(array1), array2, len(array2))
        reference = 0.0
        self.assertTrue(abs(p_val - reference) < TOLERANCE)

        p_val = stats.p_value(array1, len(array1), array3, len(array3))
        reference = 1.0
        self.assertTrue(abs(p_val - reference) < TOLERANCE)
        p_val = stats.p_value(array3, len(array3), array1, len(array1))
        reference = 1.0
        self.assertTrue(abs(p_val - reference) < TOLERANCE)

        array1[0] = 16.0
        p_val = stats.p_value(array1, len(array1), array2, len(array2))
        reference = 0.926088
        self.assertTrue(abs(p_val - reference) < TOLERANCE)

        array1[0:3] = [5.0, 5.0, 6.0]
        p_val = stats.p_value(array1, len(array1), array2, len(array2))
        reference = 0.880211
        self.assertTrue(abs(p_val - reference) < TOLERANCE)

        array4[0] = 0.0
        p_val = stats.p_value(array1, len(array1), array4, len(array4))
        reference = 0.791237
        self.assertTrue(abs(p_val - reference) < TOLERANCE)

        p_val = stats.p_value(a1, len(a1), a2, len(a2))
        reference = 0.010752
        self.assertTrue(abs(p_val - reference) < TOLERANCE)


if __name__ == '__main__':
    unittest.main()
