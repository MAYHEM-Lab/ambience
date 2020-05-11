import re
import sys
import argparse
import subprocess
from shutil import copyfile
import os

parser = argparse.ArgumentParser(description='Compile a tosdoc document')

parser.add_argument('input', metavar='input', type=str, nargs=1,
                    help='path')

parser.add_argument('-o', dest="output", type=str, nargs=1,
                    help='Output File')

parser.add_argument("-R", dest="root", type=str, help="Root of the project")

def compile(src: str) -> str:
    dir_path = os.path.dirname(os.path.realpath(__file__))
    args = ["asciidoctor", "-b", "html5", "-q", "-a", "source-highlighter=pygments", "-a", "pygments-css=style", "-a", "pygments-style=xcode", "-a", "stylesheet=" + os.path.join(dir_path, "tosdoc.css"), "-"]
    #print(args, file=sys.stderr)
    cmake_proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    cmake_proc.stdin.write(src.encode("utf-8"))
    cmake_proc.stdin.close()
    cmake_proc.wait()
    return cmake_proc.stdout.read().decode("utf-8")


def extract_images(src: str) -> [str]:
    adoc_image_pattern = re.compile('image\:+(.*?)\[.*?\]', re.M)
    return adoc_image_pattern.findall(src)

def root_dir_pass(src: str) -> str:
    absolute_path_pattern = re.compile('((?:include|image)\:+)(\/(?:.*?))\[(.*?)\]', re.M)
    return absolute_path_pattern.sub(r'\1/home/fatih/tos\2[\3]', src)

if __name__ == "__main__":
    args = parser.parse_args()
    with open(args.input[0]) as file:
        content = file.read()
        converted = root_dir_pass(content)
        compiled = compile(converted)
        in_dir = os.path.dirname(args.input[0])
        out_dir = os.path.dirname(args.output[0])
        for src in extract_images(content):
            copyfile(os.path.join(in_dir, src), os.path.join(out_dir, src))
        with open(args.output[0], 'w+') as out:
            out.write(compiled)
