#/bin/bash

rm compile_commands.json
rm .clangd

touch $1/compile_commands.json
mkdir -p $1/.clangd

ln -s "$1/compile_commands.json"
ln -s "$1/.clangd"
