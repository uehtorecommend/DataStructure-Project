#!/usr/bin/env python3
"""Experiment 7 integer-key hash regression; optional --sanitize, CC override."""
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


class SearchHashTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        cls.binary = str(Path(cls.temp.name) / 'search_hash')
        flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic']
        if SANITIZE:
            flags += ['-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                      '-fno-omit-frame-pointer', '-g', '-no-pie']
        subprocess.run(shlex.split(os.environ.get('CC', 'cc')) + flags +
                       [str(ROOT / 'tests/search_hash_driver.c'), '-o', cls.binary], check=True)
        cls.minimum, cls.maximum = map(int, cls.run_commands([])[0].split())

    @classmethod
    def run_commands(cls, commands):
        result = subprocess.run([cls.binary], input='\n'.join(commands) + '\n',
                                text=True, encoding='utf-8', capture_output=True, timeout=10)
        if result.returncode:
            raise AssertionError(f'exit {result.returncode}: {result.stderr}')
        if result.stderr:
            raise AssertionError(result.stderr)
        return result.stdout.splitlines()

    def check_sequence(self, commands):
        # Membership/capacity oracle: no reproduction of C's probing algorithm.
        positions = {}
        lines = self.run_commands(commands)[1:]
        self.assertEqual(len(lines), len(commands))
        for command, line in zip(commands, lines):
            op, value = command.split()
            key = int(value)
            fields = line.split()
            self.assertEqual(fields[0], op)
            pos = int(fields[1])
            if op == 'H':
                self.assertEqual(pos, key % 13)
            elif op == 'I':
                if key in positions:
                    self.assertEqual(pos, positions[key])
                elif len(positions) == 13:
                    self.assertEqual(pos, -1)
                else:
                    self.assertTrue(0 <= pos < 13)
                    self.assertNotIn(pos, positions.values())
                    positions[key] = pos
            else:
                self.assertEqual(pos, positions.get(key, -1), command)
                self.assertTrue(1 <= int(fields[2]) <= 13)
        return lines

    def test_hash_all_signs_and_integer_limits(self):
        keys = [self.minimum, self.minimum + 1, -27, -26, -13, -2, -1,
                0, 1, 2, 12, 13, 26, self.maximum - 1, self.maximum]
        self.check_sequence([f'H {key}' for key in keys])

    def test_empty_table_missing_keys(self):
        self.assertEqual(self.check_sequence([f'S {k}' for k in
                         [-2, -1, 0, 1, self.minimum, self.maximum]]), ['S -1 1'] * 6)

    def test_negative_two_reproducer(self):
        self.assertEqual(self.check_sequence(['I -2', 'S -2', 'S 11']),
                         ['I 11', 'S 11 1', 'S -1 2'])

    def test_minus_one_is_real_key(self):
        self.assertEqual(self.check_sequence(['S -1', 'I -1', 'S -1', 'I 12',
                                             'S -1', 'S 12', 'I -1']),
                         ['S -1 1', 'I 12', 'S 12 1', 'I 0', 'S 12 1', 'S 0 2', 'I 12'])

    def test_mixed_keys_and_limits(self):
        keys = [45, -2, 0, -1, self.minimum, self.maximum, -13, 13, -26, 26]
        self.check_sequence([f'I {k}' for k in keys] +
                            [f'S {k}' for k in keys + [-999, 999]])

    def test_collision_wraparound_and_counts(self):
        keys = [-1, 12, -14, 25, -27]
        lines = self.check_sequence([f'I {k}' for k in keys] +
                                    [f'S {k}' for k in keys] + ['S 38'])
        self.assertEqual(lines[:5], ['I 12', 'I 0', 'I 1', 'I 2', 'I 3'])
        self.assertEqual(lines[5:], ['S 12 1', 'S 0 2', 'S 1 3', 'S 2 4', 'S 3 5', 'S -1 6'])

    def test_duplicates_do_not_consume_capacity(self):
        keys = [-1, -2, 0, self.minimum, self.maximum]
        self.check_sequence([f'I {k}' for k in keys for _ in range(5)] +
                            [f'I {k}' for k in range(1, 9)] +
                            [f'S {k}' for k in keys + list(range(1, 9))] + ['I 999', 'S 999'])

    def test_full_collision_table_duplicate_and_missing(self):
        keys = [-1 + 13 * i for i in range(13)]
        lines = self.check_sequence([f'I {k}' for k in keys] +
                                    [f'I {k}' for k in keys] +
                                    [f'S {k}' for k in keys] + ['I -14', 'S -14'])
        self.assertEqual(lines[-2:], ['I -1', 'S -1 13'])

    def test_reinitialize_clears_occupancy(self):
        lines = self.run_commands(['I -1', 'I 0', 'R', 'S -1', 'S 0', 'I -1', 'S -1'])
        self.assertEqual(lines[1:], ['I 12', 'I 0', 'S -1 1', 'S -1 1', 'I 12', 'S 12 1'])

    def test_print_distinguishes_minus_one_and_empty(self):
        lines = self.run_commands(['I -1', 'P'])
        self.assertEqual(lines[-1], '哈希表数据: ' + ' --' * 12 + ' -1')

    def test_original_demo_unchanged(self):
        actual = subprocess.run([self.binary, 'demo'], capture_output=True, timeout=10)
        self.assertEqual(actual.returncode, 0, actual.stderr)
        self.assertEqual(actual.stderr, b'')
        self.assertEqual(actual.stdout, (ROOT / 'tests/fixtures/search_hash_demo.txt').read_bytes())

    def test_random_sequences_against_membership_oracle(self):
        rng = random.Random(20261105)
        for case in range(100):
            keys = [self.minimum, self.maximum, -1, 0, -2]
            keys += [rng.randint(self.minimum, self.maximum) for _ in range(20)]
            rng.shuffle(keys)
            commands = [f'S {k}' for k in keys]
            for key in keys:
                commands += [f'I {key}', f'I {key}']
                commands += [f'S {k}' for k in keys]
            with self.subTest(case=case):
                self.check_sequence(commands)


if __name__ == '__main__':
    unittest.main()
