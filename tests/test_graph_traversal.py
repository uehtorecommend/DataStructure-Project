#!/usr/bin/env python3
"""Exercise graph adjacency and traversal: python3 tests/test_graph_traversal.py [--sanitize]."""
from collections import deque
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


def traversal(adj, bfs=False, root=None):
    visited, result = set(), []

    def dfs(v):
        visited.add(v)
        result.append(v)
        for w in adj[v]:
            if w not in visited:
                dfs(w)

    for start in range(len(adj)) if root is None else [root]:
        if start in visited:
            continue
        if not bfs:
            dfs(start)
            continue
        visited.add(start)
        queue = deque([start])
        while queue:
            v = queue.popleft()
            result.append(v)
            for w in adj[v]:
                if w not in visited:
                    visited.add(w)
                    queue.append(w)
    return [chr(65 + v) for v in result]


class GraphTraversalTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        cls.binary = str(Path(cls.temp.name) / 'graph_traversal')
        flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic']
        if SANITIZE:
            flags += ['-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                      '-fno-omit-frame-pointer', '-g', '-no-pie']
        subprocess.run(shlex.split(os.environ.get('CC', 'cc')) + flags +
                       [str(ROOT / 'tests/graph_traversal_driver.c'), '-o', cls.binary], check=True)

    def run_graph(self, n, edges):
        data = f'{n} {len(edges)}\n' + ''.join(f'{a} {b} {w}\n' for a, b, w in edges)
        proc = subprocess.run([self.binary], input=data, text=True, capture_output=True, timeout=10)
        self.assertEqual(proc.returncode, 0, proc.stderr)
        return {key: value.split() for key, value in
                (line.split(':', 1) for line in proc.stdout.splitlines())}

    def check_graph(self, n, edges):
        out = self.run_graph(n, edges)
        matrix = [[] for _ in range(n)]
        linked = [[] for _ in range(n)]
        for a, b, _ in edges:
            matrix[a].append(b)
            matrix[b].append(a)
            linked[a].insert(0, b)
            linked[b].insert(0, a)
        matrix = [sorted(neighbors) for neighbors in matrix]
        for i in range(n):
            self.assertEqual(out[f'M{i}'], list(map(str, matrix[i])))
            self.assertEqual(out[f'L{i}'], list(map(str, linked[i])))
            self.assertEqual(sorted(out[f'M{i}']), sorted(out[f'L{i}']))
            self.assertEqual(out[f'MR{i}'], traversal(matrix, root=i))
            self.assertEqual(out[f'LR{i}'], traversal(linked, root=i))
            self.assertEqual(set(out[f'MR{i}']), set(out[f'LR{i}']))
        for prefix, adj in [('M', matrix), ('L', linked)]:
            self.assertEqual(out[prefix + 'D'], traversal(adj))
            self.assertEqual(out[prefix + 'B'], traversal(adj, bfs=True))
        self.assertEqual(out['bounds'], ['-1'] * 6)
        return out

    def test_reported_zero_weight_counterexample(self):
        out = self.run_graph(2, [(0, 1, 0)])
        self.assertEqual(out['L0'], ['1'])
        self.assertEqual(out['M0'], ['1'])
        self.check_graph(2, [(0, 1, 0)])

    def test_zero_weight_as_next_neighbor(self):
        self.check_graph(4, [(0, 1, 5), (0, 3, 0), (1, 2, 2)])

    def test_mixed_weight_traversal_order(self):
        # Missing A-C changes both forest traversals, not just neighbor queries.
        out = self.check_graph(4, [(0, 2, 0), (2, 1, 5), (1, 3, 2)])
        for key in ['MD', 'MB', 'LD', 'LB']:
            self.assertEqual(out[key], ['A', 'C', 'B', 'D'])

    def test_positive_edges(self):
        self.check_graph(4, [(0, 1, 5), (0, 2, 2), (2, 3, 7)])

    def test_isolated_vertices_and_diagonal(self):
        self.check_graph(5, [])
        self.check_graph(1, [])

    def test_disconnected_graph(self):
        self.check_graph(7, [(0, 2, 0), (2, 1, 4), (3, 5, 0), (5, 4, 6)])

    def test_all_zero_cycle(self):
        self.check_graph(4, [(0, 1, 0), (1, 2, 0), (2, 3, 0), (3, 0, 0)])

    def test_connected_demo_graph(self):
        self.check_graph(6, [(0, 1, 5), (0, 2, 2), (1, 3, 7), (2, 3, 4),
                             (2, 4, 8), (3, 5, 3), (4, 5, 6)])

    def test_weight_boundary_and_negative_weight(self):
        self.check_graph(4, [(0, 2, 99999999), (2, 1, 0), (1, 3, -1)])

    def test_empty_graph(self):
        self.check_graph(0, [])

    def test_maximum_complete_zero_graph(self):
        self.check_graph(20, [(i, j, 0) for i in range(20) for j in range(i + 1, 20)])

    def test_random_graphs(self):
        for seed in range(60):
            rng = random.Random(seed)
            n = rng.randint(1, 20)
            edges = [(i, j, rng.choice([0, 0, 1, 7, 99999999]))
                     for i in range(n) for j in range(i + 1, n) if rng.random() < .22]
            rng.shuffle(edges)
            with self.subTest(seed=seed):
                self.check_graph(n, edges)

    def test_demo_output_unchanged(self):
        proc = subprocess.run([self.binary, 'demo'], capture_output=True, timeout=10)
        self.assertEqual(proc.returncode, 0, proc.stderr)
        self.assertEqual(proc.stdout, (ROOT / 'tests/fixtures/graph_traversal_demo.txt').read_bytes())


if __name__ == '__main__':
    unittest.main()
