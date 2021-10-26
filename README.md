# Devil May Cry for BREW - (un)packer tool
Simple "Devil May Cry: Dante X Vergil for BREW" GGZ assets (un)packing tool

### Requirements
In order to compile you need to meet following requirements:
 - Compiler supporting C++17 (tested on GCC 11.2.0)
 - Boost library

### Compiling
Native compilation (GNU/Linux, macOS, MSYS2 on Windows):

```
make clean && make
```

Cross-compiling for Windows (x86):

```
make clean && make OS=Windows_NT CROSS=32
```

Cross-compiling for Windows (x86_64):

```
make clean && make OS=Windows_NT CROSS=64
```

### Usage
To unpack GGZ archives execute ```unpack``` tool directly from the terminal in working directory where GGZ files are located. You can also specify directory with archives as an argument for example ```unpack ggzfiles```. After processing ```decompressed``` directory will be created where all RAW assets can be accessed.

After making change in ```decompressed``` directory content, GGZ archives can be created by using ```pack``` tool. Tool lists all GGZ sources that reside in current directory in order to read metadata (list of files to pack), similarly to the unpacking tool there is a possibility to specify directory with GGZ archives as an argument for example ```pack ggzfiles```. After processing final archives will be placed in a newly created ```compressed``` path.

### File formats
Currently tool allows to replace in-game textures, static images, story text as well as SFX and BGM (MIDI) music. Please keep in mind that upscaling textures requires change in game code.

Known content:
 - mbactexs.ggz
   - *.mbac - static 3D meshes for models and maps
 - mtratexs.ggz
   - *.mtra - animations / non-static 3D models (?)
 - wqvgabmp.ggz
   - *.bmp - static 2D images, 4 and 8 bit not power of two (NPOT) PC bitmaps
 - wqvgadat.ggz
   - pallet_*.act - unknown
   - CINFO_.dat - unknown
   - MINFO.dat - unknown
   - TINFO2.dat - game story / text
   - *.wav - SFX files, RIFF (little-endian) data, WAVE audio, IMA ADPCM, mono 8000 Hz
   - *.mid - BGM files, Standard MIDI data format 1 using 1 track at 1/960 and format 0 using 1 track at 1/480
 - wqvgatex.ggz
   - *.bmp - textures for static and non-static 3D meshes, 8 bit 128x128 and 256x256 PC bitmaps

### Information
Tools are provided as-is, please use at your own risk.
