import telnetlib


class OCDDevice:
    tn: telnetlib.Telnet

    def __init__(self):
        self.tn = telnetlib.Telnet("localhost", 4444)
        self.tn.read_until(b'\n')

    def read_mem(self, address: int, length: int):
        res = [0] * length
        for x in range(0, length):
            res[x] = self.read_byte(address + x)
        return res

    def read_byte(self, address: int):
        res = self._exec_and_read("mdb {}".format(address))
        # print(res)
        return int(res[1].decode('utf-8').strip().split(' ')[1], 16)

    def _exec_and_read(self, cmd: str) -> [bytes]:
        self.tn.write(cmd.encode('utf-8') + b'\n')
        return [self.tn.read_until(b'\n'), self.tn.read_until(b'\n')]


if __name__ == "__main__":
    dev = OCDDevice()
    print(dev.read_mem(0x20000818, 256)[93])
