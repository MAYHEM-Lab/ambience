import re
import sys
import argparse
import subprocess
from shutil import copyfile
import os
from jinja2 import Template

script_dir = os.path.dirname(os.path.realpath(__file__))
pagetemplate = Template(open(os.path.join(script_dir, "templates/page.html.jinja")).read())


parser = argparse.ArgumentParser(description='Compile a tosdoc document')

parser.add_argument('input', metavar='input', type=str, nargs=1,
                    help='path')

parser.add_argument('-o', dest="output", type=str, nargs=1,
                    help='Output File')

parser.add_argument("-R", dest="root", type=str, help="Root of the project")


def compile(src: str) -> str:
    args = ["asciidoctor", "-s", "-b", "html5", "-q", "-a", "source-highlighter=pygments", "-a", "pygments-css=style",
            "-a", "pygments-style=xcode", "-a", "showtitle", "-"]
    # print(args, file=sys.stderr)
    cmake_proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    cmake_proc.stdin.write(src.encode("utf-8"))
    cmake_proc.stdin.close()
    cmake_proc.wait()
    return pagetemplate.render({
        "body": cmake_proc.stdout.read().decode("utf-8")
    })


def extract_images(src: str) -> [str]:
    adoc_image_pattern = re.compile('image\:+(.*?)\[.*?\]', re.M)
    return adoc_image_pattern.findall(src)


def root_dir_pass(src: str, root_dir: str) -> str:
    absolute_path_pattern = re.compile('((?:include|image)\:+)(\/(?:.*?))\[(.*?)\]', re.M)
    return absolute_path_pattern.sub(r'\1{}\2[\3]'.format(root_dir), src)


def compile_one(content: str, root_dir: str) -> (str, [str]):
    converted = root_dir_pass(content, root_dir)
    compiled = compile(converted)
    return (compiled, extract_images(content))


def compile_highlevel(inpath: str, outpath: str, rootdir: str):
    with open(inpath) as file:
        content = file.read()
        (compiled, assets) = compile_one(content, rootdir)
        in_dir = os.path.dirname(inpath)
        out_dir = os.path.dirname(outpath)
        for src in assets:
            copyfile(os.path.join(in_dir, src), os.path.join(out_dir, src))
        with open(outpath, 'w+') as out:
            out.write(compiled)


if __name__ == "__main__":
    args = parser.parse_args()
    compile_highlevel(args.input[0], args.output[0], args.root)
