# PSBW

## WARNING:
**THIS CODE IS A MESS. It's a school project finished on the last minute and I don't give any warranty with it. This code is to be rewritten on top of psyqo and this is pretty much a public archive. A better engine is comming. This is a proof of concept.**

A game engine for the PSX based on [spicyjpeg's bare metal tutorials](https://github.com/spicyjpeg/ps1-bare-metal/)
The toolchain, cmake and basic header files are taken from his project and will be built upon. They are and will be developed under the MIT license

## How to build

### Clone the repository
```
git clone --recursive https://github.com/pslib-cz/MP2023-24_Racek-Jan_Herni-engine-pro-PlayStation-
```
It's important to clone recursively on Linux due to mkpsxiso and it's submodules. You don't need to clone recursively on Windows as you will be required to download mkpsxiso by yourself as seen further below.

### Prerequisities
You're going to need the mips-none-elf gcc toolchain, Cmake>=3.25, Python>=3.10 and [Ninja](https://ninja-build.org/)

### Get the toolchain on Windows
You can install the toolchain from the [pcsx-redux](https://github.com/spicyjpeg/ps1-bare-metal) project
```
powershell -c "& { iwr -UseBasicParsing https://raw.githubusercontent.com/grumpycoders/pcsx-redux/main/mips.ps1 | iex }"
```
A reboot might be needed
```
mips install 13.2.0
```

### Get the toolchain on Linux
You can spawn it from source using this [compile script](https://github.com/grumpycoders/pcsx-redux/blob/main/tools/linux-mips/spawn-compiler.sh) by pcsx-redux
```
chmod +x spawn-compiler.sh
sudo ./spawn-compiler.sh
```

### Python prerequisities
```
py -m pip install -r tools/requirements.txt   # Windows
pip3 install -r tools/requirements.txt        # Linux/macOS
```

### Windows specific requirements
If you're on Windows you will need to download [mkpsxiso](https://github.com/Lameguy64/mkpsxiso/releases) and ensure it's in your PATH

This projects builds its own mkpsxiso from source on Linux

### Build
```
cmake --preset debug
cmake --build ./build
```
game.cue and game.bin will be found in the build directory

## How to use
You're gonna need [psxfudge](https://github.com/spicyjpeg/psxfudge/tree/refactor) make sure to use the refactor branch. If you get audio resampler errors
while using it, use [my fork](https://github.com/CloudMracek/psxfudge-fix) instead. Fudgebundle is pretty easy to use. Just read its [documentation](https://github.com/spicyjpeg/psxfudge/blob/refactor/doc/fudgebundle.md)

**SPRITESHEETS AND OTHER HANDY-DANDY FEATURES OF FUDGEBUNDLE ARE NOT SUPPORTED BY THIS ENGINE**

Once you create your bundle you need to put it into assets and tell mkpsxiso (which is called by cmake) to bundle it in in iso.xml in the root of the project.

If you want to see the source JSONs for the Tetris clone bundles they can be found in assets/tetrisfudge

Now go read game/main.cpp and game/MainMenu.cpp where the coding of the engine is explained.

# Disclaimer
This engine contains a lot of PSn00b code. If the original creators of that code want this taken down, please contact me and I will...

# Thanks
Thanks to [spicyjpeg](https://github.com/spicyjpeg/) who is the author of the register header files, the cmake files, the compiler configuration and who's been extemely helpful with debugging some complex issues

More thanks goes to [Nicolas Noble](https://github.com/nicolasnoble) who's intelligence is beyond my comprehension keeps wasting his time to help me solve trivial issues. <3
