#!/usr/bin/env python3
import os
from collections import defaultdict

INFILE = '/project/.project_tree.txt'
OUTFILE = '/project/project_tree_formatted.txt'

# Build nested dict tree
root = {}

def node():
    return {}

with open(INFILE, 'r', encoding='utf-8') as f:
    lines = [l.rstrip('\n') for l in f if l.strip()]

# Normalize and remove any entries that are the root directory itself
prefix = '/project'
paths = []
for p in lines:
    if p == prefix:
        continue
    if p.startswith(prefix + '/'):
        rel = p[len(prefix) + 1:]
    else:
        rel = p
    paths.append(rel)

# Insert into tree
tree = {}
for p in paths:
    parts = p.split('/')
    cur = tree
    for part in parts:
        if part not in cur:
            cur[part] = {}
        cur = cur[part]

# Render tree
lines_out = []

def render(d, prefix_str=''):
    keys = sorted(d.keys())
    for i, k in enumerate(keys):
        is_last = (i == len(keys) - 1)
        connector = '└── ' if is_last else '├── '
        lines_out.append(prefix_str + connector + k)
        child_prefix = prefix_str + ('    ' if is_last else '│   ')
        if d[k]:
            render(d[k], child_prefix)

lines_out.append('.')
render(tree, '')

with open(OUTFILE, 'w', encoding='utf-8') as f:
    f.write('\n'.join(lines_out))

print('Wrote', OUTFILE)
