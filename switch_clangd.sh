#/bin/bash

rm -f compile_commands.json
rm -f .cache

touch $1/compile_commands.json
mkdir -p $1/.cache/clangd

ln -s "$1/compile_commands.json"
ln -s "$1/.cache"
