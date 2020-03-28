from distutils.core import setup, Extension

TestWriteSpeedModule = Extension(
    "TestWriteSpeed",
    sources=["basicStorageSpeedTest.c"])

setup(name="TestWriteSpeed",
      version="1.0",
      description="Basic disc writing speed tester. Args: filesize, blocksize",
      ext_modules=[TestWriteSpeedModule])
