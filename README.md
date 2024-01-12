# PSBW
A game engine for the PSX based on [spicyjpeg's bare metal tutorials](https://github.com/spicyjpeg/ps1-bare-metal/)
The toolchain, cmake and basic header files are taken from his project and will be built upon. They are and will be developed under the MIT license

## How to build
You're going to need the mips-none-elf gcc toolchain, Cmake>=3.25, Python>=3.10 and [Ninja](https://ninja-build.org/)

### Get the toolchain on Windows
You can install the toolchain from the [pcsx-redux](https://github.com/spicyjpeg/ps1-bare-metal) project
```
powershell -c "& { iwr -UseBasicParsing https://raw.githubusercontent.com/grumpycoders/pcsx-redux/main/mips.ps1 | iex }"
```
A reboot might be possibly needed
```
mips install 13.2.0
```

### Get the toolchain on Linux
You can spawn int from source using this [compile script](https://github.com/grumpycoders/pcsx-redux/blob/main/tools/linux-mips/spawn-compiler.sh)
```
chmod +x spawn-compiler.sh
sudo ./spawn-compiler.sh
```

### Python prerequisities
```
py -m pip install -r tools/requirements.txt   # Windows
pip3 install -r tools/requirements.txt        # Linux/macOS
```

### Build
```
cmake --preset debug
cmake --build ./build
```

### Build documentation
```
doxygen Doxyfile
```

