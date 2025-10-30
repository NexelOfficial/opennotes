# OpenNotes

OpenNotes is a fast, open-source CLI tool that can quickly log View columns and evaluate [@Formula](https://help.hcl-software.com/dom_designer/12.0.2/basic/H_NOTES_FORMULA_LANGUAGE.html) on them. Based on the [HCL Domino C API](https://opensource.hcltechsw.com/domino-c-api-docs/) but in a modern C++ jacket.

This project is mainly for personal use. The company I work at uses Domino, and I wanted to use some of Domino outside of the clients, and without writing complex code for every use case.

### Highlights

- 🚀 Automatic multithreading (based on the amount of entries)
- 🧩 Evaluate @Formula directly on all entries in a View
- 📈 Compatible with Domino 12 and 14

> Note: Currently Windows only. The build preset is 32-bit, but 64-bit compilation is possible (and should work).

## Installing dependencies

This app depends on the HCL Domino C API. Download it from the software center, and then add as follows:

```
libs/include/domino/<name>.h
libs/lib/<name>.lib
libs/lib/<name>.obj
```

Now install the VCPKG dependencies by running `vcpkg install --triplet x86-windows`.

## Building

```shell
cmake --preset release && cmake --build --preset release
```

You can find the output exe in `build/Release/onotes.exe`.

## Usage

Add your Notes directory to the PATH environment variable. It's recommended to do this system-wide so you don't have to add it again with every shell restart.

After that, use the commands to access items in views and run @Formula on them. Here are some example commands that can be used:

```shell
# Start working in a database (port is optional)
onotes use "<server>" "<file>" "<port>"

# Get 10 items from the view, log the first column
onotes view "<view name>" --column 0 --count 10

# Run the formula for all items in the view
onotes view "<view name>" --formula '@Text(@DocumentUniqueId) + ", " + @Text(@Modified)'
```
