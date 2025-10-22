## Installing dependencies

This app depends on the HCL Domino C API. Download it from the software center, and then add them as follows:
```
libs/include/domino/<name>.h
libs/lib/<name>.lib
libs/lib/<name>.obj
```

## Building

```shell
cmake --preset release && cmake --build --preset release
```

You can find the output exe in `build32/Release/onotes.exe`.