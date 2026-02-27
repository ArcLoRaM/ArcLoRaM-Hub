#!/usr/bin/env python3
"""
Hexagonal topology generator for .simcfg simulator configuration files.

Usage:
    python3 generate_hex_topology.py <num_nodes> [output_file]

Examples:
    python3 generate_hex_topology.py 50
    python3 generate_hex_topology.py 100 my_topology.simcfg

Layout:
    - Concentric hexagonal rings radiating from a central C3 node.
    - Adjacent node spacing = 900 units (< 1000 reachability threshold).
    - One C3 at center (Node 1), all others are C2.
    - Routing via BFS shortest-path tree from the C3.
"""

import argparse
import math
import sys
from collections import deque


def generate_hex_grid_positions(num_nodes: int, spacing: int = 900) -> list[tuple[float, float]]:
    """
    Generate hex grid positions in concentric rings around the origin.
    Ring 0 = center (1 node), ring k = 6*k nodes.
    """
    positions = [(0.0, 0.0)]
    ring = 1
    dy_half = spacing * math.sqrt(3) / 2

    # Direction vectors for traversing each edge of a hex ring
    # Starting position: (ring * spacing, 0), moving counter-clockwise
    directions = [
        (-spacing / 2,  dy_half),   # edge 0: upper-right
        (-spacing,       0.0),      # edge 1: left
        (-spacing / 2, -dy_half),   # edge 2: lower-left
        ( spacing / 2, -dy_half),   # edge 3: lower-right
        ( spacing,       0.0),      # edge 4: right
        ( spacing / 2,  dy_half),   # edge 5: upper-right
    ]

    while len(positions) < num_nodes:
        cx, cy = ring * spacing, 0.0
        for d in range(6):
            for _ in range(ring):
                if len(positions) >= num_nodes:
                    break
                positions.append((cx, cy))
                cx += directions[d][0]
                cy += directions[d][1]
            if len(positions) >= num_nodes:
                break
        ring += 1

    return positions[:num_nodes]


def build_adjacency(positions: list[tuple[float, float]], max_dist: float = 950.0) -> dict[int, list[int]]:
    """Build adjacency list: two nodes are neighbors if Euclidean distance < max_dist."""
    n = len(positions)
    adj = {i: [] for i in range(n)}
    for i in range(n):
        for j in range(i + 1, n):
            dx = positions[i][0] - positions[j][0]
            dy = positions[i][1] - positions[j][1]
            if math.sqrt(dx * dx + dy * dy) < max_dist:
                adj[i].append(j)
                adj[j].append(i)
    return adj


def bfs_routing(adj: dict[int, list[int]], root: int = 0) -> tuple[list[int], list[int]]:
    """
    BFS from root (C3 node).
    Returns (next_hop, hop_count) arrays indexed by node index.
    """
    n = len(adj)
    hop_count = [-1] * n
    next_hop = [-1] * n

    hop_count[root] = 0
    parent = [-1] * n
    queue = deque([root])

    while queue:
        u = queue.popleft()
        for v in adj[u]:
            if hop_count[v] == -1:
                hop_count[v] = hop_count[u] + 1
                parent[v] = u
                queue.append(v)

    for i in range(n):
        if i != root:
            next_hop[i] = parent[i]

    return next_hop, hop_count


def write_simcfg(filename: str, positions: list[tuple[float, float]],
                 next_hop: list[int], hop_count: list[int], root: int = 0) -> None:
    """Write the .simcfg file."""
    lines = []
    for i in range(len(positions)):
        node_id = i + 1
        cls = "C3" if i == root else "C2"
        x = int(round(positions[i][0]))
        y = int(round(positions[i][1]))
        if i == root:
            lines.append(f"NODE {node_id} {cls} {x} {y}")
        else:
            nh_id = next_hop[i] + 1
            hc = hop_count[i]
            lines.append(f"NODE {node_id} {cls} {x} {y} nextHop={nh_id} hopCount={hc}")

    with open(filename, "w") as f:
        f.write("\n".join(lines) + "\n")


def main():
    parser = argparse.ArgumentParser(
        description="Generate a hexagonal-grid .simcfg topology file."
    )
    parser.add_argument("num_nodes", type=int, help="Number of nodes to generate")
    parser.add_argument(
        "output", nargs="?", default=None,
        help="Output filename (default: topology_<N>.simcfg)"
    )
    parser.add_argument(
        "--spacing", type=int, default=900,
        help="Distance between adjacent hex nodes (default: 900)"
    )
    parser.add_argument(
        "--max-dist", type=float, default=950.0,
        help="Max distance for adjacency / reachability (default: 950)"
    )
    args = parser.parse_args()

    if args.num_nodes < 2:
        print("Error: need at least 2 nodes.", file=sys.stderr)
        sys.exit(1)

    filename = args.output or f"topology_{args.num_nodes}.simcfg"

    print(f"Generating {args.num_nodes}-node hexagonal topology (spacing={args.spacing}) ...")
    positions = generate_hex_grid_positions(args.num_nodes, args.spacing)
    adj = build_adjacency(positions, args.max_dist)

    root = 0
    next_hop, hop_count = bfs_routing(adj, root)

    # Quick sanity check
    unreachable = [i for i in range(len(positions)) if hop_count[i] == -1]
    if unreachable:
        print(f"FATAL: {len(unreachable)} unreachable nodes. Adjacency graph is disconnected.",
              file=sys.stderr)
        sys.exit(1)

    write_simcfg(filename, positions, next_hop, hop_count, root)

    max_hops = max(hop_count)
    avg_hops = sum(hop_count) / args.num_nodes
    print(f"Written: {filename}")
    print(f"  Nodes: {args.num_nodes} (1 C3 + {args.num_nodes - 1} C2)")
    print(f"  Max hop count: {max_hops}, Avg: {avg_hops:.1f}")
    xs = [p[0] for p in positions]
    ys = [p[1] for p in positions]
    print(f"  Bounding box: x=[{min(xs):.0f}, {max(xs):.0f}], y=[{min(ys):.0f}, {max(ys):.0f}]")


if __name__ == "__main__":
    main()