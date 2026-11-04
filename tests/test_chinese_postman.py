#!/usr/bin/env python3
"""Compile real C code; compare against an independent edge-coverage oracle.
Run from any directory: python3 tests/test_chinese_postman.py [--sanitize]
Requires Python 3 and a C11 compiler (CC, default cc); no third-party packages.
"""
import heapq
import itertools
import os
from pathlib import Path
import random
import re
import shlex
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'DataStructure-Project/实验11-中国邮路问题.c'


def oracle(n, edges, office):
    """Dijkstra on (position, covered street IDs), without Floyd or matching.

    Parallel edges have distinct bits, so every actual street must be covered.
    This exponential oracle is deliberately restricted to small test graphs.
    """
    adj = [[] for _ in range(n)]
    for i, (u, v, w) in enumerate(edges):
        adj[u].append((v, w, 1 << i))
        adj[v].append((u, w, 1 << i))
    goal = (1 << len(edges)) - 1
    dist = {(office, 0): 0}
    queue = [(0, office, 0)]
    while queue:
        cost, u, mask = heapq.heappop(queue)
        if cost != dist[u, mask]:
            continue
        if u == office and mask == goal:
            return cost
        for v, w, bit in adj[u]:
            state = v, mask | bit
            if cost + w < dist.get(state, float('inf')):
                dist[state] = cost + w
                heapq.heappush(queue, (cost + w, *state))
    return -1


def main():
    general = [(0, 1, 4), (0, 2, 3), (1, 2, 2), (1, 3, 5),
               (2, 4, 6), (3, 4, 1), (3, 5, 7), (4, 5, 2)]
    euler = [(0, 1, 2), (1, 2, 3), (2, 3, 4), (3, 0, 5)]
    counterexample = [(0, 1, 1), (0, 1, 10), (0, 2, 3), (1, 2, 10)]
    cases = []

    def add(name, n, edges, office=0, expected=None):
        if expected is None:
            expected = oracle(n, edges, office)
        cases.append((name, n, edges, office, expected))

    add('original general', 6, general, expected=33)
    add('original Euler', 4, euler, expected=14)
    # All 24 insertion orders; reversed endpoints are tested separately.
    for i, edges in enumerate(itertools.permutations(counterexample)):
        add(f'parallel permutation {i}', 3, list(edges), expected=25)
    add('reversed endpoints', 3, [(v, u, w) for u, v, w in counterexample], expected=25)
    add('equal parallel edges', 2, [(0, 1, 5)] * 3, expected=20)
    add('parallel Euler', 2, [(0, 1, 1), (0, 1, 10)], expected=11)
    add('zero parallel edge', 2, [(0, 1, 0), (0, 1, 9), (0, 1, 3)], expected=12)
    add('self loop', 2, [(0, 0, 7), (0, 1, 2)], expected=11)
    add('zero self loop', 1, [(0, 0, 0)], expected=0)
    add('isolated unused vertex', 3, [(0, 1, 2)], expected=4)
    add('isolated post office', 3, [(1, 2, 2)], expected=-1)
    add('disconnected even degrees', 4, [(0, 1, 2)] * 2 + [(2, 3, 3)] * 2, expected=-1)
    add('disconnected odd degrees', 4, [(0, 1, 2), (2, 3, 3)], expected=-1)
    add('disconnected self loop', 2, [(1, 1, 3)], expected=-1)
    add('empty graph', 1, [], expected=0)
    add('empty larger graph', 12, [], office=11, expected=0)
    add('indirect shortest path', 3, [(0, 1, 100), (0, 2, 1), (1, 2, 1), (0, 1, 200)], expected=304)
    add('all 12 vertices odd', 12, [(0, i, 1) for i in range(1, 12)], expected=22)
    max_int = 2**31 - 1
    add('large edge and matching sum', 12, [(0, i, max_int) for i in range(1, 12)], expected=22 * max_int)
    add('40 streets, large total', 2, [(0, 1, max_int)] * 40, expected=40 * max_int)
    add('39 streets, large total plus matching', 2, [(0, 1, max_int)] * 39, expected=40 * max_int)

    rng = random.Random(20261104)
    for i in range(300):
        n = rng.randint(1, 5)
        edges = [(rng.randrange(n), rng.randrange(n), rng.randrange(11))
                 for _ in range(rng.randrange(9))]
        add(f'oracle random {i}', n, edges, rng.randrange(n))

    flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic', '-g']
    if '--sanitize' in sys.argv:
        flags += ['-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-omit-frame-pointer', '-no-pie']
    compiler = shlex.split(os.environ.get('CC', 'cc'))
    with tempfile.TemporaryDirectory(prefix='postman-tests-') as tmp:
        exe = Path(tmp) / 'driver'
        subprocess.run(compiler + flags + [str(ROOT / 'tests/chinese_postman_driver.c'), '-o', str(exe)], check=True)
        data = ''.join(f'{n} {len(edges)} {office}\n' + ''.join(f'{u} {v} {w}\n' for u, v, w in edges)
                       for _, n, edges, office, _ in cases)
        run = subprocess.run([str(exe)], input=data.encode(), capture_output=True)
        if run.returncode:
            raise RuntimeError(f"driver exited {run.returncode}:\n{run.stderr.decode(errors='replace')}")
        blocks = run.stdout.decode().split('CASE\n')[1:]
        assert len(blocks) == len(cases)
        for (name, n, edges, office, expected), block in zip(cases, blocks):
            result = int(re.search(r'RESULT (-?\d+)', block)[1])
            assert result == expected, (name, expected, result, block)
            if expected >= 0:
                degree = [0] * n
                for u, v, _ in edges:
                    degree[u] += 1
                    degree[v] += 1
                pairs = [(int(u), int(v), int(w)) for u, v, w in
                         re.findall(r'\((\d+),(\d+),补长(\d+)\)', block)]
                assert sorted(vertex for u, v, _ in pairs for vertex in (u, v)) == [i for i in range(n) if degree[i] % 2], name
                assert sum(w for _, _, w in pairs) == expected - sum(w for _, _, w in edges), name
                # Check printed pair distances independently with single-source Dijkstra.
                for u, v, w in pairs:
                    adj = [[] for _ in range(n)]
                    for a, b, weight in edges:
                        adj[a].append((b, weight))
                        adj[b].append((a, weight))
                    distance = [float('inf')] * n
                    distance[u] = 0
                    queue = [(0, u)]
                    while queue:
                        d, a = heapq.heappop(queue)
                        if d != distance[a]:
                            continue
                        for b, weight in adj[a]:
                            if d + weight < distance[b]:
                                distance[b] = d + weight
                                heapq.heappush(queue, (d + weight, b))
                    assert distance[v] == w, (name, u, v, w, distance[v])
        demo = Path(tmp) / 'demo'
        subprocess.run(compiler + flags + [str(SOURCE), '-o', str(demo)], check=True)
        output = subprocess.check_output([str(demo)], text=True)
        assert '最短投递路线长度：33\n' in output
        assert '最短投递路线长度为：14\n' in output
    print(f'PASS: {len(cases)} route cases, input/capacity contracts, printed pairings, original demos')


if __name__ == '__main__':
    main()
