"""Full program/solver regression with an independent subset-per-semester oracle."""
from pathlib import Path
import itertools
import os
import random
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SAN = '--sanitize' in sys.argv
if SAN: sys.argv.remove('--sanitize')


def oracle(cs, semesters, credit, count):
    states = {0}
    for _ in range(semesters):
        following = set()
        for done in states:
            ready = [i for i, (_, pre) in enumerate(cs) if not done >> i & 1
                     and all(done >> p & 1 for p in pre)]
            for k in range(min(count, len(ready)) + 1):
                for subset in itertools.combinations(ready, k):
                    if sum(cs[i][0] for i in subset) <= credit:
                        following.add(done | sum(1 << i for i in subset))
        states = following
        if (1 << len(cs)) - 1 in states: return True
    return False


class TeachingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.bin = str(Path(cls.tmp.name) / 'driver')
        cls.full = str(Path(cls.tmp.name) / 'full')
        flags = ['-std=c++11', '-Wall', '-Wextra', '-Werror', '-pedantic']
        if SAN: flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g', '-no-pie']
        for source, binary in [('tests/teaching_plan_driver.cpp', cls.bin),
                               ('DataStructure-Project/实验2-教学计划编制.cpp', cls.full)]:
            subprocess.run(['g++', *flags, str(ROOT / source), '-o', binary], check=True)

    def run_case(self, cs, semesters, credit, count, mode=0):
        text = f'{len(cs)} {semesters} {credit} {count} {mode}\n'
        text += ''.join(f'C{i} {c} {len(pre)} ' + ' '.join(f'C{p}' for p in pre) + '\n'
                        for i, (c, pre) in enumerate(cs))
        target = Path(self.tmp.name) / 'plan.txt'
        text += str(target) + '\n'
        r = subprocess.run([self.bin], input=text, text=True, capture_output=True, timeout=15)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertEqual(r.stderr, '')
        assigned = {}
        for line in r.stdout.splitlines():
            if line.startswith('ASSIGN '):
                _, i, s = line.split(); i, s = int(i), int(s)
                self.assertNotIn(i, assigned); assigned[i] = s
        found = len(assigned) == len(cs)
        if found:
            for i, (c, pre) in enumerate(cs):
                self.assertTrue(all(assigned[p] < assigned[i] for p in pre))
            for s in range(semesters):
                items = [i for i in assigned if assigned[i] == s]
                self.assertLessEqual(len(items), count)
                self.assertLessEqual(sum(cs[i][0] for i in items), credit)
            if mode in (2, 4): self.assertIn('学期总数', target.read_text())
        return found, r.stdout, assigned

    def test_counterexample(self):
        self.assertTrue(self.run_case([(1, []), (10, []), (5, []), (1, [0])], 2, 15, 2)[0])

    def test_independent_packing(self):
        self.assertTrue(self.run_case([(c, []) for c in [6, 5, 3, 2, 2, 2]], 2, 10, 3)[0])

    def test_constraints(self):
        cases = [([(1, []), (1, [0]), (1, [1])], 3, 2, 2),
                 ([(1, []), (1, [0]), (1, [1])], 2, 2, 2),
                 ([(6, []), (6, []), (6, [])], 2, 10, 2),
                 ([(1, [])] * 5, 2, 10, 2),
                 ([(1, [1]), (1, [0])], 2, 10, 2),
                 ([(11, [])], 2, 10, 2),
                 ([(0.5, []), (1.5, []), (1, [0])], 2, 2, 2)]
        for cs, s, c, n in cases:
            with self.subTest(cs=cs):
                self.assertEqual(self.run_case(cs, s, c, n)[0], oracle(cs, s, c, n))

    def test_random_oracle_and_optimality(self):
        rng = random.Random(20261107)
        for _ in range(180):
            n = rng.randint(1, 8); s = rng.randint(1, 4)
            cap = rng.randint(3, 12); count = rng.randint(1, 4)
            cs = [(rng.randint(1, 7), [j for j in range(i) if rng.random() < .2]) for i in range(n)]
            found, out, assignment = self.run_case(cs, s, cap, count)
            expected = oracle(cs, s, cap, count)
            self.assertEqual(found, expected, (cs, s, cap, count, out))
            if found:
                optimal = next(h for h in range(1, s + 1) if oracle(cs, h, cap, count))
                self.assertEqual(max(assignment.values()) + 1, optimal)

    def test_scale_and_budget(self):
        self.assertTrue(self.run_case([(1, [])] * 40, 8, 15, 5)[0])
        self.assertFalse(self.run_case([(1, [])] * 100, 8, 15, 5)[0])
        out = self.run_case([(1, [])], 1, 15, 5, 3)[1]
        self.assertIn('BUDGET 2', out)
        self.assertIn('【搜索未完成】', out)
        self.assertNotIn('【无解】', out)

    def test_even_export_and_full_menu(self):
        cs = [(2, []), (3, []), (1, [0])]
        self.assertTrue(self.run_case(cs, 3, 15, 5, 4)[0])
        self.assertEqual((Path(self.tmp.name) / 'plan.txt').read_bytes(),
                         (ROOT / 'tests/fixtures/teaching_even.txt').read_bytes())
        self.assertTrue(self.run_case(cs, 3, 15, 5, 2)[0])
        menu = '2\n4\nA A 1 0\nB B 10 0\nD D 5 0\nC C 1 1 A\n5\n2\n15\n2\n7\n0\n'
        r = subprocess.run([self.full], input=menu, text=True, capture_output=True, timeout=10)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertIn('按尽可能短时间生成的教学计划', r.stdout)
        self.assertNotIn('【无解】', r.stdout)

if __name__ == '__main__': unittest.main()
