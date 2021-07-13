import os
import ambictl

def create_build_root(build_at, tos_source: str):
    os.makedirs(build_at, exist_ok=True)

    if os.path.exists(os.path.join(build_at, "tos")):
        os.unlink(os.path.join(build_at, "tos"))

    os.symlink(tos_source, os.path.join(build_at, "tos"), target_is_directory=True)

def build_all(build_root, deployment: ambictl.Deployment):
    deployment.generate_build_dir(build_root)
    deployment.build_all()



if __name__ == "__main__":
    build_root = "/tmp/aebuild"
    create_build_root(build_root, "/home/fatih/tos")
