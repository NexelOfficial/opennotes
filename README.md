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

## Usage

OpenNotes is used from the command line. First add your port, server and database to your environment variables. This can be done in your system, or from the command line:
```ps
$env:DOMINO_PORT= "<port>"
$env:DOMINO_SERVER = "<server>"
$env:DOMINO_FILE = "<database file>"
```

After that, use the command to access items in views and run @formula's on them. Here are some example commands that can be used:

```ps
# Get 10 items from the view, log the first column
onotes "<view name>" --column 1 --count 10

# Run the formula for all items in the view
onotes "<view name>" --formula '@Text(@DocumentUniqueId) + " | " + @Text(@Modified)'
```