import sys
from ambictl.loading import *
from ambictl.ambidecl import *
import mkbuild
import pwd


def current_user_name():
    return pwd.getpwuid(os.getuid())[0]


if __name__ == "__main__":
    dir = sys.argv[1]
    load_dir(dir)
    deployment = ambidecl._finalize()
    script_path = os.path.dirname(os.path.abspath(__file__))
    mkbuild.create_build_root(
        f"/tmp/{current_user_name()}/mydeployment", os.path.join(script_path, "../.."))
    mkbuild.build_all(f"/tmp/{current_user_name()}/mydeployment", deployment)
