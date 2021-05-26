#!/usr/bin/env python3

import os
import subprocess
import tempfile
import signal


def get_qemu_path() -> str:
    return "/home/fatih/qemu/bin/release/aarch64-softmmu/qemu-system-aarch64"


def make_fifos():
    fifo_dir = tempfile.mkdtemp()
    in_path = os.path.join(fifo_dir, "in")
    out_path = os.path.join(fifo_dir, "out")
    os.mkfifo(in_path)
    os.mkfifo(out_path)
    return in_path, out_path, fifo_dir


def launch_guest(kernel_path: str):
    in_path, out_path, fifo_dir = make_fifos()

    in_fd = os.open(in_path, os.O_RDWR | os.O_NONBLOCK)
    in_fifo = open(in_path, "rb")
    out_fd = os.open(out_path, os.O_RDWR | os.O_NONBLOCK)
    out_fifo = open(out_path, "wb")
    err_out = open("/dev/null", "wb")

    launch_args = [get_qemu_path(), "-M", "raspi3", "-semihosting", "-serial", "mon:stdio", "-gdb", "tcp::1234",
                   "-kernel", kernel_path, "-semihosting-config", "target=native"]
    qemu_proc = subprocess.Popen(launch_args, stdin=in_fifo, stdout=out_fifo)

    '''
        Closing the input FD to qemu could send EOF to it if the FIFO is not open anywhere else yet.
        To not risk it, we keep the FD open as long as qemu lives.
    '''
    # os.close(in_fd)
    os.close(out_fd)
    in_fifo.close()
    out_fifo.close()

    print("VM Is Running.")
    print("GDB Server Is Available At localhost:1234")
    print("FIFOs available at {}".format(fifo_dir))

    def signal_handler(sig, frame):
        print("Received SIGINT, stopping VM")
        os.kill(qemu_proc.pid, signal.SIGINT)

    old_handler = signal.signal(signal.SIGINT, signal_handler)

    qemu_proc.wait()

    signal.signal(signal.SIGINT, old_handler)

    os.unlink(in_path)
    os.unlink(out_path)
    os.close(in_fd)
    os.rmdir(fifo_dir)

    print("Cleaned up, exiting...")


if __name__ == "__main__":
    launch_guest("/home/fatih/tos/cmake-build-raspi3rel/bin/raspiboot")
