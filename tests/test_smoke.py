import unittest

import surfex


class SurfexImportTests(unittest.TestCase):
    def test_import_exports(self):
        self.assertTrue(hasattr(surfex, "init"))
        self.assertTrue(hasattr(surfex, "show"))
        self.assertTrue(hasattr(surfex, "Surfex"))


if __name__ == "__main__":
    unittest.main()
