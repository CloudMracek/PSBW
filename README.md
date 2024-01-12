# PSBW
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
You can spawn it from source using this [compile script](https://github.com/grumpycoders/pcsx-redux/blob/main/tools/linux-mips/spawn-compiler.sh)
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

This projects builds it's own mkpsxiso from source on Linux

### Build
```
cmake --preset debug
cmake --build ./build
```

### Build documentation
```
doxygen Doxyfile
```

## How to run
If you have a PSX you can either burn a CD with the output bin/cue files or upload it directly to the PSX using a serial link cable.

Or you can use an emulator like [pcsx-redux](https://github.com/grumpycoders/pcsx-redux)

# Thanks
Thanks to [spicyjpeg](https://github.com/spicyjpeg/) who is the author of the register header files, the cmake files, the compiler configuration and who's been extemely helpful with debugging some complex issues

More thanks goes to [Nicolas Noble](https://github.com/nicolasnoble) who's intelligence is beyond my comprehension keeps wasting his time to help me solve trivial issues. <3
