# Reversi

A simple and smooth Reversi game that uses OpenGL to render with the help of GLFW and GLAD.

# How to use

## Download (no build)

You can download Linux or Windows builds from [the latest release](https://github.com/Reminimalism/Reversi/releases/latest).

For Linux, make sure to run `chmod u+x path/to/Reversi`
in terminal to be able to run it (replace `path/to/Reversi` with the downloaded executable file path).
Also, you can use [reminimalism-reversi.desktop](https://github.com/Reminimalism/Reversi/blob/main/reminimalism-reversi.desktop)
for your desktop environment to show Reversi in its apps menu.
Replace `/path/to/Reversi` with the executable path and `/path/to/Reversi.svg` with the icon path.
The icon can be found [here](https://github.com/Reminimalism/Reversi/blob/main/icon/Reversi.svg) too.

## Building on Linux

Install cmake, make and a C++ compiler like GCC (g++) or Clang (clang++).
Here's how you can install them in some Linux distributions (only one of clang++/g++ is needed):

Arch Linux: `sudo pacman -S cmake make clang gcc`

Debian/Ubuntu: `sudo apt install cmake make clang g++`

Go to a directory where you want to download the repository to,
and enter the following commands to clone and build the latest version (may be work in progress):

```
git clone https://github.com/Reminimalism/Reversi.git
cd Reversi
mkdir Build
cd Build
cmake ..
cmake --build .
```

From here, you can enter `./Reversi/Reversi` in the same terminal session to run the game.

After building, the executable file can be found in `Reversi/Build/Reversi/`.

## Building on Windows

The easiest option is to open the project folder directly in the latest Visual Studio if you have its own CMake tools installed.
The build configuration can be changed using the configuration menu (usually x64-Debug by default, release configuration is recommended) and the project can be built using Build > Build All.

Alternatively, download and install CMake from [their website](https://cmake.org/download/).
Download and extract this repository from the "Code" menu > Download Zip.
Open the extracted project in CMake.
Generate the project for an IDE and use the supported IDE to build.
