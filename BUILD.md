# BUILD - Table of Contents

<!-- TOC -->

- [BUILD - Table of Contents](#build-table-of-contents)
- [Prerequisites](#prerequisites)
- [Windows Build](#windows-build)
- [Ubuntu 16.04 Xenial Build](#ubuntu-1604-xenial-build)
- [Ubuntu 15.10 Wily Build](#ubuntu-1510-wily-build)
- [Ubuntu 14.04 Trusty Build](#ubuntu-1404-trusty-build)
- [Linux Build](#linux-build)
- [OS X Build](#os-x-build)

<!-- /TOC -->

# Prerequisites

Building the gtest-runner from source requires having the following programs installed

* A working compiler
  * Windows:
    * _Recommended_: [Visual Studio 2015](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx). 
    * _Minimum_: Visual Studio 2013 is the minimum compatible compiler.
  * Linux:
    * _Minimum_: gcc 4.9.3.
* Qt
  * _Recommended_: [Qt 5.6](http://www.qt.io/download/) in order to support high-DPI monitors.
  * _Minimum_: Qt 5.3 for non high-DPI monitors.
* CMake
  * _Recommended_: [CMake 3.6.2](https://cmake.org/download/)
  * _Minimum_: CMake 2.8.12
* _OPTIONAL_
  * git 2.7.0 or later
  * NSIS 3.0 or later if you want to build the .exe installer.

# Windows Build

These steps have been tested on Windows 10, but should be similar or the same for Windows 7/8.

_Make sure you've installed all the [prerequisites](https://github.com/nholthaus/gtest-runner/wiki/Build-Instructions#prerequisites) before proceeding!_

1. Download the [Latest Source .zip](https://github.com/nholthaus/gtest-runner/releases), then unzip the source files.
  * example: extract the files to `C:\workspace\gtest-runner`
2. Open a command line terminal by typing `cmd` into the cortana/start menu bar.
3. use `cd` to navigate to the root directory of the source code.
  * example: `cd C:\workspace\gtest-runner`
4. Ensure that the `QTDIR` environment variable is properly set by typing `echo %QTDIR%`. You should see something like

  > C:\Qt\5.6\msvc2015

  where msvc2015 is the name of your compiler. If you are building for x64, it should be in the form msvc2015_64.
  * If your `QTDIR` is not set, then set it using `set QTDIR=[path\to\Qt]`, where `[path\to\Qt] is replaced with your install path, example: `set QTDIR=C:\Qt\5.6\msvc2015`
5. Ensure the cmake path is visible to the command line by typing `cmake --version`. If you see
  > cmake version 3.4.3

  or similar, proceed to step 6. Otherwise, determine the path to you cmake install, and add it to the path:

  `set PATH="C:\Program Files (x86)\CMake\bin";%PATH%` (be sure to use the proper path for YOUR install) 
6. Type the following commands into the prompt, and press enter after each one
  * `md build`
  * `cd build`
  * `cmake -Wno-dev ..`
  * `cmake --config Release --build .`
  * you can now run `gtest-runner.exe` from the `build\Release` directory!
7. (optional) If you have `NSIS` installed, you can create the installer package and install gtest-runner into your `Program Files`.
  * `cmake --target PACKAGE --config Release --build .`  
  * `cd Release`
  * Run the installer using `gtest-runner-v[VERSION]-[TARGET].exe`, where version is the gtest-version that you downloaded, and target is win32 or win64 depending on your platform. If you're not sure what to use, type the `dir` command to see which executable was generated.
    * example: `gtest-runner-v1.4.0-win32.exe`

The installer is used to create program shortcuts and links in your application menu. If you prefer not to use the installer (or can't use it), you can still run `gtest-runner.exe` directly from the Release directory.

# Ubuntu 16.04 Xenial Build

1. Open a terminal window
2. Clone the repository and checkout the latest version of the code
  - `git clone https://github.com/nholthaus/gtest-runner.git ~/gtest-runner`
  - `cd ~/gtest-runner`
  - `git tag -l` to see the available versions
  - `git checkout [version]`, where [version] was the newest version tag, example: `git checkout v1.4.0`
3. Build and install the code
  - `cd ~/gtest-runner`
  - `mkdir build`
  - `cd build`
  - `cmake -DCMAKE_BUILD_TYPE=Release ..`
  - `make` (or use `make -j2` if you have a dual-core machine)
  - `sudo make install`
 
You can now run the gtest-runner by typing `gtest-runner` into your console.

# Ubuntu 15.10 Wily Build

1. Open a terminal window
2. Clone the repository and checkout the latest version of the code
  - `git clone https://github.com/nholthaus/gtest-runner.git ~/gtest-runner`
  - `cd ~/gtest-runner`
  - `git tag -l` to see the available versions
  - `git checkout [version]`, where [version] was the newest version tag, example: `git checkout v1.4.0`
3. Build and install the code
  - `cd ~/gtest-runner`
  - `mkdir build`
  - `cd build`
  - `cmake -DCMAKE_BUILD_TYPE=Release ..`
  - `make` (or use `make -j2` if you have a dual-core machine)
  - `sudo make install`
 
You can now run the gtest-runner by typing `gtest-runner` into your console.

# Ubuntu 14.04 Trusty Build

1. Open a terminal window
2. Clone the repository and checkout the latest version of the code
  - `git clone https://github.com/nholthaus/gtest-runner.git ~/gtest-runner`
  - `cd ~/gtest-runner`
  - `git tag -l`
  - `git checkout [version]`, where [version] was the newest version tag, example: `git checkout v1.4.0`
3. Get gcc 4.9.3 (skip this step if you already have it, or have a newer version)
  - `sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y`
  - `sudo apt-get update -qq`
  - `sudo apt-get install g++-4.9 -y`
  - `export CXX="g++-4.9"`
4. Get Qt 5.6 (skip this step if you already have it, or have a newer version)
  - `sudo add-apt-repository ppa:beineri/opt-qt57-trusty -y`
  - `sudo apt-get update -qq`
  - `sudo apt-get install qt57base -y`
  - `sudo apt-get install qt57xmlpatterns -y`
  - `source /opt/qt57/bin/qt57-env.sh`
5. Get CMake 3.4.3 (skip this step if you already have it, or have a newer version)
  - `sudo add-apt-repository ppa:george-edison55/cmake-3.x -y`
  - `sudo apt-get update -qq`
  - `sudo apt-get install cmake -y`
6. Build and install the code
  - `cd ~/gtest-runner`
  - `mkdir build`
  - `cd build`
  - `cmake -DCMAKE_BUILD_TYPE=Release ..`
  - `make` (use `make -j2` if you have a dual-core machine)
  - `sudo make install`
 
You can now run the gtest-runner by typing `gtest-runner` into your console.

# Linux Build

Follow the instructions for the [Ubuntu 14.04 Build](https://github.com/nholthaus/gtest-runner/wiki/Build-Instructions#ubuntu-1404-trusty-build), with the following exceptions:

Steps 3-5:
  - use your systems package manager to get the prerequisites.
  - if they are not available, download and build them from source. Make sure to set your `QTDIR`. Make sure your `PATH` variable includes the the cmake `bin` directory.

# OS X Build

No testing has been done with gtest-runner/OS X to date. If you're interested in using gtest-runner on a Mac, get in contact.  


