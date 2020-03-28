from platform import uname
from TestWriteSpeed import Test
from typing import AnyStr, Optional, IO


class ResultsFile():
    f: Optional[IO]

    def __init__(self, filename: str = None):
        if filename is not None and len(filename) > 0:
            self.f = open(filename, "a+")
        else:
            self.f = None

    def write(self, content: AnyStr):
        if self.f is not None and self.f.writable:
            self.f.write(content)

    def __del__(self):
        if self.f is not None:
            if not self.f.closed():
                self.f.close()


resultsFilename = "results.txt"
testRepeatsCount = 10
tests = {
    "1KB": {
        "filesize": 1024 ** 3,
        "blocksize": 1024
    },
    "4KB": {
        "filesize": 1024 ** 3,
        "blocksize": 4096
    },
    "8KB": {
        "filesize": 1024 ** 3,
        "blocksize": 8192
    },
    "16KB": {
        "filesize": 1024 ** 3,
        "blocksize": 16384
    },
    "4MB": {
        "filesize": 1024 ** 3,
        "blocksize": 4*1024*1024
    }
}

resultsFile = ResultsFile(resultsFilename)
deviceName = ' '.join([uname().machine, uname().system, uname().version])
resultsFile.write(f"{deviceName}, (tests repeats {testRepeatsCount}):\n")

for testName, test in tests.items():
    timeSumm: float = 0
    for i in range(testRepeatsCount):
        timeSumm += Test(test["filesize"], test["blocksize"])
    averageTime = round(timeSumm/testRepeatsCount, 6)
    averageSpeed = round(
        test['filesize'] * testRepeatsCount / timeSumm / (1024**2), 4)
    print(f"{testName} average time: {averageTime} s")
    print(f"{testName} average speed: {averageSpeed} Mib/s")
    resultsFile.write(
        f"\t{testName} (filesize: {round(test['filesize']/(1024**2), 6)} Mib, "
        f"blocksize: {round(test['blocksize'] / 1024, 4)} Kib)\n"
        f"\t\taverage speed: {averageTime} Mib/s\n"
        f"\t\taverage time: {averageSpeed} s\n")
resultsFile.write(f"\n")
