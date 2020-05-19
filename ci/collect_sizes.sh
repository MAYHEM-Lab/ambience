#!/bin/bash

find . -regex ".*/[a-z0-9_-]+$" -type f | xargs size -B 2>/dev/null