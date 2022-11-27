# ModernPDF
[![CodeFactor](https://www.codefactor.io/repository/github/aleksmurfitt/modernpdf/badge)](https://www.codefactor.io/repository/github/aleksmurfitt/modernpdf)

>## ⚠️ **This project is in _very_ early development**
>Feel free to read the code and explore the repository, but unless you're actively working on the project _do not attempt to build or run the project; **it most likely will not work as expected**_

## Building the project

> ⚠️ **At this point in development, the CMake files may rarely be out of sync with the source code.  
> If you have any issues building the project, _do not try to debug them_ — it will almost always be a waste of your time. Create an issue so I can look into it.**  

### Dependencies

The CMakeLists have been written to automatically handle all project dependencies when building, but in order to even use CMake or any other build tools, you will need to have a C++ toolchain and CMake installed.

#### Dynamic libraries

Precompiled dynamic libraries are installed using HomeBrew at the moment, making `brew` a prerequisite. As explained above, however, `brew` is still optional, albeit highly recommended — if you use MacPorts or another package manager, either install the libraries yourself or, better yet, update the build scripts with the relevant commands for others. A list of requisite libraries will be printed on build if they cannot be resolved with `brew`.

#### Static libraries

Static libraries will be built as dependencies automatically during the build process from their respective upstream main branches. As a result you will find that the first build, or occasionally subsequent builds may be considerably slower than usual — this will be because changes have been made upstream that require a rebuild.

Source files for libraries will be in the `libs` folder, and should **_not_** be modified.

### Generating a binary

Clone the repo:

``` bash
git clone https://github.com/thirdfort/pdf-generator
```

Enter the project root and configure CMake:

``` bash
cmake -Bbuild
```

and finally, build the binary:

``` bash
cmake --build build
```

The build files will be generated in a new (or existing on subsequent builds) build directory — the generated binary will be located in the same directory.  
