#!/usr/bin/env python3
"""Run with python3 tests/test_campus_routes.py [--sanitize]; requires a C11 compiler."""
import os
from pathlib import Path
import random
import re
import shlex
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
NAMES = ['正门', '图书馆', '启航广场', '教学楼', '体育馆', '宿舍区']
INF = 100000000
SANITIZE = '--sanitize' in sys.argv
if SANITIZE:
    sys.argv.remove('--sanitize')


class CampusRoutesTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        cls.binary = str(Path(cls.temp.name) / 'campus_routes')
        flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic']
        if SANITIZE:
            flags += ['-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                      '-fno-omit-frame-pointer', '-g', '-no-pie']
        subprocess.run(shlex.split(os.environ.get('CC', 'cc')) + flags +
                       [str(ROOT / 'tests/campus_routes_driver.c'), '-o', cls.binary], check=True)

    def query(self, edges=(), start=0, target=3):
        data = f'{len(edges)} {start} {target}\n'
        data += ''.join(f'{a} {b} {w}\n' for a, b, w in edges)
        result = subprocess.run([self.binary], input=data, text=True, encoding='utf-8',
                                capture_output=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)
        return result.stdout

    def route(self, start, target, path, distance):
        return (f'{NAMES[start]} 到 {NAMES[target]} 的最佳路线：' +
                ' -> '.join(NAMES[i] for i in path) + f'，距离 {distance}m\n')

    def test_reported_counterexample(self):
        self.assertEqual(self.query([(0, 1, 300)]), '正门 到 教学楼 不存在可达路径。\n')

    def test_disconnected_components(self):
        edges = [(0, 1, 300), (2, 3, 100), (4, 5, 50)]
        self.assertEqual(self.query(edges, 2, 0), '启航广场 到 正门 不存在可达路径。\n')
        self.assertEqual(self.query(edges, 2, 3), self.route(2, 3, [2, 3], 100))

    def test_all_mixed_reachability(self):
        self.assertEqual(self.query([(0, 1, 300)], target=-2),
                         '从正门到所有景点的最佳路线：\n'
                         '  到图书馆：正门 -> 图书馆，距离 300m\n' +
                         ''.join(f'  到{name}：不存在可达路径。\n' for name in NAMES[2:]))

    def test_isolated_source(self):
        edges = [(0, 1, 300)]
        self.assertEqual(self.query(edges, 5, 0), '宿舍区 到 正门 不存在可达路径。\n')
        self.assertEqual(self.query(edges, 5, -2), '从宿舍区到所有景点的最佳路线：\n' +
                         ''.join(f'  到{name}：不存在可达路径。\n' for name in NAMES[:5]))

    def test_no_roads(self):
        for start in range(6):
            for target in range(6):
                with self.subTest(start=start, target=target):
                    expected = (self.route(start, target, [start], 0) if start == target else
                                f'{NAMES[start]} 到 {NAMES[target]} 不存在可达路径。\n')
                    self.assertEqual(self.query(start=start, target=target), expected)

    def test_same_endpoint_with_roads(self):
        self.assertEqual(self.query([(0, 1, 300)], 0, 0), self.route(0, 0, [0], 0))

    def test_direct_and_reverse(self):
        self.assertEqual(self.query([(0, 1, 300)], 0, 1), self.route(0, 1, [0, 1], 300))
        self.assertEqual(self.query([(0, 1, 300)], 1, 0), self.route(1, 0, [1, 0], 300))

    def test_indirect_shortest_path(self):
        self.assertEqual(self.query([(0, 3, 900), (0, 1, 300), (1, 3, 350)]),
                         self.route(0, 3, [0, 1, 3], 650))

    def test_zero_weight_route(self):
        self.assertEqual(self.query([(0, 1, 0), (1, 3, 0)]), self.route(0, 3, [0, 1, 3], 0))

    def test_distance_below_sentinel(self):
        self.assertEqual(self.query([(0, 1, INF - 2), (1, 3, 1)]),
                         self.route(0, 3, [0, 1, 3], INF - 1))

    def test_zero_weight_cycle_and_equal_routes(self):
        edges = [(0, 1, 0), (1, 2, 0), (2, 0, 0), (1, 3, 5), (2, 3, 5)]
        self.assertEqual(self.query(edges), self.route(0, 3, [0, 1, 3], 5))
        self.assertEqual(self.query(edges, 2, 2), self.route(2, 2, [2], 0))

    def test_unreachable_before_reachable_target(self):
        self.assertEqual(self.query([(0, 5, 1)], target=-2),
                         '从正门到所有景点的最佳路线：\n' +
                         ''.join(f'  到{name}：不存在可达路径。\n' for name in NAMES[1:5]) +
                         '  到宿舍区：正门 -> 宿舍区，距离 1m\n')

    def test_sentinel_limit_and_alternative_route(self):
        # Distances >= INF are outside the original finite-distance representation.
        for length in (1, 2, INF - 1):
            edges = [(0, 1, INF - 1), (1, 3, length)]
            self.assertEqual(self.query(edges), '正门 到 教学楼 不存在可达路径。\n')
            self.assertEqual(self.query(edges + [(0, 3, 7)]),
                             self.route(0, 3, [0, 3], 7))

    def test_missing_endpoints(self):
        for start, target in [(-1, 0), (0, -1), (-1, -1)]:
            self.assertEqual(self.query(start=start, target=target), '起点或终点不存在。\n')
        self.assertEqual(self.query(start=-1, target=-2), '起点不存在。\n')

    def test_original_demo_unchanged(self):
        actual = subprocess.check_output([self.binary, 'demo'], timeout=10)
        self.assertEqual(actual, (ROOT / 'tests/fixtures/campus_demo.txt').read_bytes())

    def test_random_all_queries_against_floyd(self):
        rng = random.Random(20261104)
        for case in range(40):
            edges = [(a, b, rng.randrange(501)) for a in range(6) for b in range(a + 1, 6)
                     if rng.random() < 0.3]
            weights = [[0 if a == b else INF for b in range(6)] for a in range(6)]
            for a, b, w in edges:
                weights[a][b] = weights[b][a] = w
            distances = [row[:] for row in weights]
            for k in range(6):
                for a in range(6):
                    for b in range(6):
                        distances[a][b] = min(distances[a][b], distances[a][k] + distances[k][b])
            for start in range(6):
                lines = self.query(edges, start, -2).splitlines()
                self.assertEqual(len(lines), 6)
                for target, line in zip((i for i in range(6) if i != start), lines[1:]):
                    with self.subTest(case=case, start=start, target=target):
                        prefix = f'  到{NAMES[target]}：'
                        self.assertTrue(line.startswith(prefix))
                        result = line[len(prefix):]
                        if distances[start][target] >= INF:
                            self.assertEqual(result, '不存在可达路径。')
                            continue
                        match = re.fullmatch(r'(.+)，距离 (\d+)m', result)
                        self.assertIsNotNone(match)
                        path = [NAMES.index(name) for name in match[1].split(' -> ')]
                        self.assertEqual((path[0], path[-1]), (start, target))
                        self.assertEqual(len(path), len(set(path)))
                        self.assertTrue(all(weights[a][b] < INF for a, b in zip(path, path[1:])))
                        self.assertEqual(sum(weights[a][b] for a, b in zip(path, path[1:])),
                                         distances[start][target])
                        self.assertEqual(int(match[2]), distances[start][target])


if __name__ == '__main__':
    unittest.main()
