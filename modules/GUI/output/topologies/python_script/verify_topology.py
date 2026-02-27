#!/usr/bin/env python3
"""
Topology verification tool for .simcfg files.

Usage:
    python3 verify_topology.py <simcfg_file> [--max-dist 1000]

Checks performed:
    1. All node IDs are unique.
    2. Exactly one C3 node exists.
    3. All other nodes are C2 with nextHop and hopCount.
    4. Every nextHop reference points to a valid node ID.
    5. Distance between each node and its nextHop is < max_dist (default 1000).
    6. hopCount matches the actual trace length back to the C3.
    7. No routing loops exist.
    8. All nodes are reachable from the C3 (graph connectivity via adjacency < max_dist).
"""

import argparse
import math
import sys
from collections import deque


def parse_simcfg(filepath: str) -> dict:
    """
    Parse a .simcfg file.
    Returns dict keyed by node_id: {cls, x, y, nextHop, hopCount}
    """
    nodes = {}
    with open(filepath) as f:
        for lineno, raw in enumerate(f, start=1):
            line = raw.strip()
            if not line:
                continue

            parts = line.split()
            if parts[0] != "NODE":
                print(f"  WARN: line {lineno}: unexpected prefix '{parts[0]}', skipping")
                continue

            node_id = int(parts[1])
            cls = parts[2]
            x = int(parts[3])
            y = int(parts[4])

            next_hop = None
            hop_count = None
            for p in parts[5:]:
                if p.startswith("nextHop="):
                    next_hop = int(p.split("=")[1])
                elif p.startswith("hopCount="):
                    hop_count = int(p.split("=")[1])

            if node_id in nodes:
                print(f"  ERROR: line {lineno}: duplicate node ID {node_id}")

            nodes[node_id] = {
                "cls": cls, "x": x, "y": y,
                "nextHop": next_hop, "hopCount": hop_count,
                "line": lineno,
            }

    return nodes


def verify(nodes: dict, max_dist: float = 1000.0) -> tuple[int, int]:
    """
    Run all verification checks.
    Returns (error_count, warning_count).
    """
    errors = 0
    warnings = 0

    n = len(nodes)
    print(f"  Total nodes: {n}")

    # --- Check 1: unique IDs (already caught during parse) ---

    # --- Check 2: exactly one C3 ---
    c3_ids = [nid for nid, nd in nodes.items() if nd["cls"] == "C3"]
    c2_ids = [nid for nid, nd in nodes.items() if nd["cls"] == "C2"]
    other = [nid for nid, nd in nodes.items() if nd["cls"] not in ("C2", "C3")]

    if len(c3_ids) != 1:
        print(f"  ERROR: expected exactly 1 C3, found {len(c3_ids)}: {c3_ids}")
        errors += 1
    else:
        print(f"  C3 node: {c3_ids[0]}")

    print(f"  C2 nodes: {len(c2_ids)}")

    if other:
        print(f"  WARN: unknown classes for nodes: {other}")
        warnings += len(other)

    if not c3_ids:
        print("  FATAL: no C3 node found, cannot verify routing.")
        return errors + 1, warnings

    c3_id = c3_ids[0]

    # --- Check 3: C2 nodes must have nextHop and hopCount ---
    for nid in c2_ids:
        nd = nodes[nid]
        if nd["nextHop"] is None:
            print(f"  ERROR: Node {nid} (C2) missing nextHop")
            errors += 1
        if nd["hopCount"] is None:
            print(f"  ERROR: Node {nid} (C2) missing hopCount")
            errors += 1

    # --- Check 4: nextHop references valid node ---
    for nid, nd in nodes.items():
        if nd["nextHop"] is not None and nd["nextHop"] not in nodes:
            print(f"  ERROR: Node {nid} nextHop={nd['nextHop']} does not exist")
            errors += 1

    # --- Check 5: nextHop distance < max_dist ---
    max_nh_dist = 0.0
    for nid, nd in nodes.items():
        if nd["nextHop"] is None:
            continue
        nh = nodes.get(nd["nextHop"])
        if nh is None:
            continue
        dx = nd["x"] - nh["x"]
        dy = nd["y"] - nh["y"]
        dist = math.sqrt(dx * dx + dy * dy)
        max_nh_dist = max(max_nh_dist, dist)
        if dist >= max_dist:
            print(f"  ERROR: Node {nid} -> nextHop {nd['nextHop']}: "
                  f"distance {dist:.1f} >= {max_dist}")
            errors += 1

    print(f"  Max nextHop distance: {max_nh_dist:.1f} (limit: {max_dist})")

    # --- Check 6 & 7: hopCount correctness and loop detection ---
    for nid, nd in nodes.items():
        if nd["cls"] == "C3":
            if nd["hopCount"] is not None and nd["hopCount"] != 0:
                print(f"  WARN: C3 node {nid} has hopCount={nd['hopCount']} (expected 0 or None)")
                warnings += 1
            continue

        if nd["nextHop"] is None or nd["hopCount"] is None:
            continue

        hops = 0
        cur = nid
        visited = set()
        loop = False
        while cur != c3_id:
            if cur in visited:
                print(f"  ERROR: Node {nid}: routing loop detected "
                      f"(revisited node {cur} after {hops} hops)")
                errors += 1
                loop = True
                break
            visited.add(cur)
            cur_node = nodes.get(cur)
            if cur_node is None or cur_node["nextHop"] is None:
                print(f"  ERROR: Node {nid}: broken chain at node {cur} "
                      f"(no nextHop) after {hops} hops")
                errors += 1
                loop = True
                break
            cur = cur_node["nextHop"]
            hops += 1
            if hops > n:
                print(f"  ERROR: Node {nid}: chain exceeds node count, likely loop")
                errors += 1
                loop = True
                break

        if not loop and hops != nd["hopCount"]:
            print(f"  ERROR: Node {nid}: declared hopCount={nd['hopCount']} "
                  f"but traced {hops} hops to C3")
            errors += 1

    # --- Check 8: graph connectivity ---
    # Build adjacency based on Euclidean distance < max_dist
    ids = list(nodes.keys())
    adj = {nid: [] for nid in ids}
    for i, nid_i in enumerate(ids):
        for j in range(i + 1, len(ids)):
            nid_j = ids[j]
            dx = nodes[nid_i]["x"] - nodes[nid_j]["x"]
            dy = nodes[nid_i]["y"] - nodes[nid_j]["y"]
            if math.sqrt(dx * dx + dy * dy) < max_dist:
                adj[nid_i].append(nid_j)
                adj[nid_j].append(nid_i)

    # BFS from C3
    visited_bfs = set()
    queue = deque([c3_id])
    visited_bfs.add(c3_id)
    while queue:
        u = queue.popleft()
        for v in adj[u]:
            if v not in visited_bfs:
                visited_bfs.add(v)
                queue.append(v)

    unreachable = set(ids) - visited_bfs
    if unreachable:
        print(f"  ERROR: {len(unreachable)} nodes unreachable from C3 "
              f"via adjacency graph (dist < {max_dist}): {sorted(unreachable)[:20]}...")
        errors += 1
    else:
        print(f"  Connectivity: all {n} nodes reachable from C3 (adjacency < {max_dist})")

    # --- Stats ---
    hop_counts = [nd["hopCount"] for nd in nodes.values()
                  if nd["hopCount"] is not None and nd["cls"] != "C3"]
    if hop_counts:
        print(f"  Hop count range: {min(hop_counts)} - {max(hop_counts)}, "
              f"avg: {sum(hop_counts)/len(hop_counts):.1f}")

    return errors, warnings


def main():
    parser = argparse.ArgumentParser(
        description="Verify a .simcfg hexagonal topology file."
    )
    parser.add_argument("file", help="Path to the .simcfg file")
    parser.add_argument(
        "--max-dist", type=float, default=1000.0,
        help="Maximum allowed distance between node and its nextHop (default: 1000)"
    )
    args = parser.parse_args()

    print(f"Verifying: {args.file} (max distance: {args.max_dist})")
    print("-" * 60)

    nodes = parse_simcfg(args.file)
    if not nodes:
        print("  FATAL: no nodes parsed from file.")
        sys.exit(1)

    errs, warns = verify(nodes, args.max_dist)

    print("-" * 60)
    if errs == 0:
        print(f"RESULT: PASS ({warns} warning(s))")
    else:
        print(f"RESULT: FAIL ({errs} error(s), {warns} warning(s))")
        sys.exit(1)


if __name__ == "__main__":
    main()
