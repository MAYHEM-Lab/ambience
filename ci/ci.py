from git import Repo
import git
import os
import subprocess
import sys
import pprint
import time
import datetime
import tempfile
import glob

# root_dir = "/src"
root_dir = tempfile.mkdtemp()
# source_dir = os.path.join(root_dir, "repo")
source_dir = os.getcwd()


def prepare_repo_by_clone():
    cloneString = os.environ["CLONESTRING"]
    commit = os.environ["COMMITHASH"]

    repo = None

    def clone():
        class Progress(git.remote.RemoteProgress):
            def update(self, op_code, cur_count, max_count=None, message=''):
                # print('update(%s, %s, %s, %s)'%(op_code, cur_count, max_count, message), end='\r')
                pass

        nonlocal repo
        repo = Repo.clone_from(cloneString, source_dir, progress=Progress(),
                               env={"GIT_SSH_COMMAND": 'ssh -i /secret/ssh/ssh-privatekey'})
        return True

    def checkout():
        repo.git.checkout(commit)
        return True

    return [("Clone repo", clone), ("Checkout commit", checkout)]


def prepare_repo_by_existing(path):
    pass


def build_config(build_type, cpu, run_tests, env={}, base=None):
    directory = ""
    if base is None:
        directory = os.path.join(root_dir, "{}/{}".format(cpu, build_type))
    else:
        directory = os.path.join(root_dir, "{}/{}/{}".format(base, cpu, build_type))

    stage_env = os.environ.copy()
    for e in env:
        stage_env[e] = env[e]

    def createBuildDir():
        os.makedirs(directory, exist_ok=True)
        return True

    def generate():
        args = ["cmake", "-G", "Ninja"]
        args.append("-DTOS_CPU={}".format(cpu))
        args.append("-DCMAKE_BUILD_TYPE={}".format(build_type))
        args.append("-DCMAKE_CXX_COMPILER_LAUNCHER=ccache")
        args.append("-DCMAKE_C_COMPILER_LAUNCHER=ccache")
        if run_tests:
            args.append("-DBUILD_TESTS=ON")
        args.append(source_dir)
        print("Running {}".format(args), flush=True)
        cmake_proc = subprocess.Popen(args, cwd=directory, env=stage_env)
        res = cmake_proc.wait()
        return res == 0

    def build():
        args = ["ninja"]
        print("Running {}".format(args), flush=True)
        ninja_proc = subprocess.Popen(args, cwd=directory, env=stage_env)
        res = ninja_proc.wait()
        return res == 0

    def test():
        args = ["ctest", "--verbose"]
        print("Running {}".format(args), flush=True)
        ctest_proc = subprocess.Popen(args, cwd=directory, env=stage_env)
        res = ctest_proc.wait()
        return res == 0

    def bench():
        args = ["./bin/bench_main"]
        print("Running {}".format(args), flush=True)
        ctest_proc = subprocess.Popen(args, cwd=directory, env=stage_env)
        res = ctest_proc.wait()
        return res == 0

    def collect_sizes():
        args = [os.path.join(source_dir, "ci/collect_sizes.sh")]
        print("Running {}".format(args), flush=True)
        ctest_proc = subprocess.Popen(args, cwd=directory, env=stage_env)
        res = ctest_proc.wait()
        return True

    tasks = [("Create build directory", createBuildDir),
             ("Generate project", generate), ("Build", build), ("Collect sizes", collect_sizes)]
    if run_tests:
        tasks.append(("Run tests", test))
        tasks.append(("Run benchmarks", bench))

    return tasks


def get_pipeline():
    pipeline = []

    # pipeline.append(("Prepare Environment", prepare_repo_by_clone()))

    pipeline.append(
        ("x86 Hosted Debug", build_config("Debug", "x86/hosted", True)))
    pipeline.append(("x86 Hosted MinSizeRel", build_config(
        "MinSizeRel", "x86/hosted", True)))

    pipeline.append(("x86 Clang MinSizeRel", build_config(
        "MinSizeRel", "x86/hosted", True, env={"CC": "clang-10", "CXX": "clang++"}, base="clang")))

    pipeline.append(("STM32F103C8", build_config(
        "MinSizeRel", "stm32/f1/03c8", False)))
    pipeline.append(("STM32L053", build_config(
        "MinSizeRel", "stm32/l0/53", False)))
    pipeline.append(("STM32L475", build_config(
        "MinSizeRel", "stm32/l4/75", False)))

    pipeline.append(("ESP8266", build_config("MinSizeRel", "esp/8266", False)))

    pipeline.append(("nRF52840", build_config(
        "MinSizeRel", "nrf52/840", False)))
    pipeline.append(("nRF52832", build_config(
        "MinSizeRel", "nrf52/832", False)))

    pipeline.append(("atmega328p", build_config(
        "MinSizeRel", "atmega/328p", False)))

    pipeline.append(("CC3220SF", build_config(
        "MinSizeRel", "ti/cc3220sf", False)))

    return pipeline


# pp = pprint.PrettyPrinter(indent=4)
# pp.pprint(get_pipeline())

def execute_pipeline(pipeline):
    def write_log_end(depth, message):
        print("</{} {}> {}".format("#" * depth, datetime.datetime.now().isoformat(), message), flush=True)

    def write_log(depth, message):
        print("<{} {}> {}".format("#" * depth, datetime.datetime.now().isoformat(), message), flush=True)

    all_tasks = True
    write_log(1, "Pipeline")
    for (name, task) in pipeline:
        all_steps = True
        write_log(2, "{}/{}".format("Pipeline", name))
        for (step_name, step) in task:
            write_log(3, "{}/{}/{}".format("Pipeline", name, step_name))
            res = False
            try:
                res = step()
            except Exception as e:
                print("Exception: {}".format(e), flush=True)
                res = False
            all_steps = all_steps and res
            write_log_end(3, "Success" if res else "Failed")
        all_tasks = all_tasks and all_steps
        write_log_end(2, "Success" if all_steps else "Failed")
    write_log_end(1, "Success" if all_steps else "Failed")


execute_pipeline(get_pipeline())
