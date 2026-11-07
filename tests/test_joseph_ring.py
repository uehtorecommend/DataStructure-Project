#!/usr/bin/env python3
"""Compile both complete interactive programs; run with optional --sanitize."""
import os
from pathlib import Path
import random
import shlex
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SANITIZE = '--sanitize' in sys.argv
if SANITIZE:
    sys.argv.remove('--sanitize')


def reference(n, m):
    """Independent array model: jump by modular arithmetic, then remove."""
    people = list(range(1, n + 1))
    order = []
    index = 0
    while people:
        index = (index + m - 1) % len(people)
        order.append(people.pop(index))
    return order


class JosephRingTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        cls.binaries = []
        flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic', '-O0', '-g']
        if SANITIZE:
            flags += ['-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                      '-fno-omit-frame-pointer', '-no-pie']
        for i, source in enumerate(['实验1-约瑟夫环.c', 'DataStructure-Project/实验1-约瑟夫环.c']):
            # Source and existing output strings use GBK; preserve their bytes.
            binary = str(Path(cls.temp.name) / f'joseph_{i}')
            subprocess.run(shlex.split(os.environ.get('CC', 'cc')) + flags +
                           [str(ROOT / source), '-o', binary], check=True)
            cls.binaries.append(binary)

    def run_input(self, data, expected, status=0):
        env = os.environ.copy()
        env.setdefault('ASAN_OPTIONS', 'detect_leaks=1:halt_on_error=1')
        env.setdefault('UBSAN_OPTIONS', 'halt_on_error=1:print_stacktrace=1')
        for binary in self.binaries:
            result = subprocess.run([binary], input=data.encode('ascii'),
                                    capture_output=True, timeout=10, env=env)
            self.assertEqual(result.returncode, status, result.stderr.decode(errors='replace'))
            self.assertEqual(result.stderr, b'')
            self.assertEqual(result.stdout, ('请输入人数n和密码m：' + expected).encode('gbk'))

    def valid(self, n, m):
        self.run_input(f'{n} {m}\n', '出圈顺序为：' +
                       ' '.join(map(str, reference(n, m))) + '\n')

    def test_single(self):
        for m in (1, 2, 17):
            with self.subTest(m=m):
                self.valid(1, m)

    def test_two(self):
        for m in (1, 2, 3, 20):
            with self.subTest(m=m):
                self.valid(2, m)

    def test_many(self):
        for n, m in ((7, 3), (10, 2), (41, 3), (100, 19)):
            with self.subTest(n=n, m=m):
                self.valid(n, m)

    def test_password_one(self):
        for n in (3, 10, 100):
            self.valid(n, 1)

    def test_password_exceeds_population(self):
        for n, m in ((3, 11), (10, 101), (25, 1000)):
            self.valid(n, m)

    def test_random_reference(self):
        rng = random.Random(20261107)
        for _ in range(100):
            n, m = rng.randint(1, 60), rng.randint(1, 250)
            with self.subTest(n=n, m=m):
                self.valid(n, m)

    def test_invalid_parameters(self):
        for data in ('0 1', '-1 1', '1 0', '1 -1', '0 0', '-5 -7'):
            with self.subTest(data=data):
                self.run_input(data, 'n和m必须为正整数\n', 1)

    def test_invalid_format(self):
        for data in ('', 'abc', '1', '1 x'):
            with self.subTest(data=data):
                self.run_input(data, '输入格式错误\n', 1)


if __name__ == '__main__':
    unittest.main()
