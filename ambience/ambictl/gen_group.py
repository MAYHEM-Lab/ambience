"""
Per node, we need:

- Node schema with capabilities, tokens etc
- CMakeLists.txt
- load_groups

Per group
"""

from ambictl import sample_group

print(sample_group.sample_group().generateBody())
