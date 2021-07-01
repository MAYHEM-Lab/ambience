import ambictl.platforms
import ambictl.memories


class Node:
    name: str
    platform: Platform
    memories: Memories

    def __init__(self, name: str, platform: Platform, memories: Memories):
        self.name = name
        self.platform = platform
        self.memories = memories
