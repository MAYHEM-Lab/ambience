import sys
from ambictl.loading import *
from ambictl.ambidecl import *
import mkbuild

if __name__ == "__main__":
    dir = sys.argv[1]
    load_dir(dir)
    deployment = ambidecl._finalize()
    mkbuild.build_all("/tmp/mydeployment", deployment)