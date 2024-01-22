# GGZ BREW / Zeebo assets (un)packer tool
Simple (un)packing tool for GGZ assets found in BREW / Zeebo games such as "Devil May Cry: Dante X Vergil" and "Sonic the Hedgehog" for BREW as well as "Double Dragon" for Zeebo.

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

### Supported games

List of currently supported BREW / Zeebo games by (un)packing tool:

- Devil May Cry: Dante X Vergil for BREW
- Sonic the Hedgehog for BREW
- Double Dragon for Zeebo

Tool supports extracting and injecting modified assets to all of the supported games mentioned above. Modyfing game assets needs to be done by proper, manually selected external tools. Most likely some other games are supported too, compatibility list will be extended when such will be found.

### Devil May Cry: Dante X Vergil for BREW file formats
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

### Sonic the Hedgehog for BREW file formats
Currently tool allows to replace in-game text, MIDI and SFX sounds as well as various bitmaps including in-game sprites. Some of the GZIP streams do not have name flag set due to which unique ordinal number is assigned to the resource.

Known content:
 - data.ggz
   - font.bmp - game font, PC bitmap, Windows 3.x format, 230 x 39 x 8
   - _various unnamed files_ - sprites, PC bitmap, Windows 3.x format, various size
   - *.mid - BGM files, Standard MIDI data (format 0) using 1 track at 1/480
   - c5.WAV - SFX file, RIFF (little-endian) data, WAVE audio, Microsoft PCM, 16 bit, mono 6000 Hz
   - _148_ - SEGA jingle, RIFF (little-endian) data
   - *.act - unknown, zone related assets
   - *.bct - unknown, zone related assets
   - *.blt - unknown, zone related assets
   - *.bmd - unknown, zone related assets
   - world.dat - unknown
   - scdtblwk.scd - unknown
   - lang_0.txt - game menu / text
   - manual_0.txt - game hints / text

### Double Dragon for Zeebo file formats
Currently tool allows to replace in-game sprites as well as SFX and BGM (MIDI) music.

Known content:
 - data.ggz
   - *.obm1 - sprites / animations / tiles, custom file format
 - sound.ggz
   - *.wav - SFX files, RIFF (little-endian) data, WAVE audio, Microsoft PCM, 16 bit, mono 22050 Hz
   - *.mid - BGM files, Standard MIDI data (format 0) using 1 track at 1/960

### Information
Tools are provided as-is, please use at your own risk.
