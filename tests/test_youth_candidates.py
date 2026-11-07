#!/usr/bin/env python3
"""Full-program regression: python3 tests/test_youth_candidates.py [--sanitize]."""
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SANITIZE = '--sanitize' in sys.argv
if SANITIZE:
    sys.argv.remove('--sanitize')


class YouthCandidatesTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        cls.driver = str(Path(cls.temp.name) / 'driver')
        cls.demo = str(Path(cls.temp.name) / 'demo')
        flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic']
        if SANITIZE:
            flags += ['-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                      '-fno-omit-frame-pointer', '-g', '-no-pie']
        for source, binary in [
            ('tests/youth_candidates_driver.c', cls.driver),
            ('DataStructure-Project/实验12-校园十大优秀青年评比.c', cls.demo),
        ]:
            subprocess.run(shlex.split(os.environ.get('CC', 'cc')) + flags +
                           [str(ROOT / source), '-o', binary], check=True)

    def run_case(self, *args):
        result = subprocess.run([self.driver, *map(str, args)], capture_output=True,
                                text=True, encoding='utf-8', timeout=10)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(result.stderr, '')
        return result.stdout

    def test_all_field_boundaries(self):
        for field, capacity in enumerate([30, 40, 40, 20, 160]):
            for length in [0, 1, capacity - 1, capacity, capacity + 1, 319, 4095]:
                with self.subTest(field=field, length=length):
                    self.run_case('boundary', field, length)
        # Original reported 32-byte name corruption.
        self.run_case('boundary', 0, 32)

    def test_null_empty_and_unterminated_fields(self):
        self.run_case('invalid')

    def test_duplicate_lookup_voting_and_ranking(self):
        output = self.run_case('flow')
        self.assertIn('第1名：Bob  票数=2\n第2名：Alice  票数=1\n', output)
        self.assertIn('姓名：Bob  班级：class', output)

    def test_collisions_wraparound_and_full_table(self):
        self.run_case('collisions')

    def test_exact_names_and_utf8_byte_boundaries(self):
        self.run_case('identity')

    def test_original_demo_output(self):
        result = subprocess.run([self.demo], capture_output=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stderr, b'')
        self.assertEqual(result.stdout, (ROOT / 'tests/fixtures/youth_demo.txt').read_bytes())


if __name__ == '__main__':
    unittest.main()
