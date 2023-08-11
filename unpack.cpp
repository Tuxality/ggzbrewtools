/*
pack.cpp -- GGZ BREW / Zeebo assets unpack tool

Copyright (C) 2021
 Przemyslaw Skryjomski (Tuxality)

For more visit: http://tuxality.net

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.
*/

#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <string>
#include <vector>

#include <cstdint>
#include <cstring>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#ifdef _WIN32
uint32_t __bswap_32(uint32_t value)
{
	return _byteswap_ulong(value);
}
#elif __APPLE__
#include <libkern/OSByteOrder.h>
uint32_t __bswap_32(uint32_t value)
{
	return OSSwapInt32(value);
}
#else
#include <byteswap.h>
#endif	// _WIN32

#include "version.h"

static bool isGzipHeader(const uint32_t& header)
{
    static const uint8_t GZIP_HEADER[sizeof(uint32_t)] = { 0x1F, 0x8B, 0x08, 0x08 };
    return (0 == std::memcmp(static_cast<const void*>(&GZIP_HEADER[0]), static_cast<const void*>(&header), sizeof(uint32_t)));
}

static void decompressGZIP(const std::vector<char>& stream, std::vector<char>& decompressed)
{
    boost::iostreams::filtering_ostream os;
    os.push(boost::iostreams::gzip_decompressor());
    os.push(boost::iostreams::back_inserter(decompressed));
    boost::iostreams::write(os, stream.data(), stream.size());
}

static void writeBinaryFile(const std::vector<char>& buffer, const std::string& path)
{
    std::fstream fp(path, std::ios::out | std::ios::binary);
    if (!fp.is_open())
    {
        throw std::runtime_error("Failed to open file = " + path + " for writing!");
    }

    fp.write((const char*)(buffer.data()), buffer.size() * sizeof(char));
    fp.close();
}

static void unpackGGZ(std::fstream& ggz, const std::string& file, const std::string& decompressedPath)
{
    std::vector<uint32_t> offsetsStart;
    std::vector<uint32_t> offsetsEnd;
    std::vector<uint32_t> originalSizes;

    ggz.seekg(0, ggz.end);
    size_t size = ggz.tellg();
    ggz.seekg(0, ggz.beg);

    do
    {
        uint32_t offset, originalSize;
        ggz.read((char*)(&offset), sizeof(uint32_t)); offset = __bswap_32(offset);
        ggz.read((char*)(&originalSize), sizeof(uint32_t)); originalSize = __bswap_32(originalSize);

        uint32_t header = __bswap_32(offset);
        if (isGzipHeader(header)) break;

        offsetsStart.push_back(offset);
        originalSizes.push_back(originalSize);
    } while (!ggz.eof());

    if (ggz.eof())
        throw std::runtime_error("End of file, this should not happen!");

    offsetsEnd = offsetsStart;
    offsetsEnd.erase(offsetsEnd.begin() + 0);
    offsetsEnd.push_back(size);

    for (size_t i = 0; i<offsetsStart.size(); i++)
    {
        uint32_t start        = offsetsStart.at(i);
        uint32_t end          = offsetsEnd.at(i);
        uint32_t originalSize = originalSizes.at(i);
        uint32_t size         = end - start;

        ggz.seekg(start);
        uint32_t magicId, flags;
        uint16_t flags2;
        ggz.read((char*)(&magicId), sizeof(uint32_t));
        ggz.read((char*)(&flags), sizeof(uint32_t));
        ggz.read((char*)(&flags2), sizeof(uint16_t));

        std::string name;
        char c = '\0';
        do
        {
            ggz.read((char*)(&c), sizeof(char));
            if (c == '\0') break;
            name += c;
        } while (!ggz.eof());

        const std::string decompressedFilePath = (std::filesystem::path(decompressedPath) / name).string();

        std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << start << " "
                                                << std::setfill('0') << std::setw(8) << end << " "
                                                << std::dec << originalSize << " " << size << " " << name << " " << decompressedFilePath << std::endl;

        ggz.seekg(start);

        std::vector<char> stream;
        stream.reserve(size);
        stream.resize(size);
        ggz.read((char*)(stream.data()), size * sizeof(char));

        std::vector<char> decompressed;
        decompressed.reserve(originalSize);

        decompressGZIP(stream, decompressed);
        writeBinaryFile(decompressed, decompressedFilePath);
    }
}

static void createDirectory(const std::string& path)
{
    if (std::filesystem::exists(path))
    {
        std::filesystem::remove_all(path);
    }

    std::filesystem::create_directory(path);
}

int main(int argc, char* argv[])
{
    std::cout << "GGZ BREW / Zeebo assets unpack tool " << __VERSION_VER__ << " - " << __VERSION_DATE__ << std::endl;
    std::cout << "by Przemyslaw Skryjomski (Tuxality)" << std::endl << std::endl;

    try
    {
        static const std::string decompressedPath = "decompressed";
        createDirectory(decompressedPath);

        static const std::string currentDirectory = (std::filesystem::path(".") / "").string();
        static const std::string sourceDirectory = (2 == argc) ? argv[1] : currentDirectory;
        std::regex re(".*.ggz", std::regex_constants::ECMAScript | std::regex_constants::icase);

        std::cout << "Checking directory current = " << currentDirectory << ", source = " << sourceDirectory << " for GGZ files..." << std::endl;
        size_t fileCount = 0;

        for (auto& p: std::filesystem::directory_iterator(sourceDirectory))
        {
            const std::string file = p.path().filename().string();
            const std::string fullPath = p.path().string();

            if (std::regex_match(file, re) && std::filesystem::is_regular_file(p))
            {
                const std::string decompressedDestPath = (std::filesystem::path(currentDirectory) / decompressedPath / file).string();
                createDirectory(decompressedDestPath);

                std::fstream fp(fullPath, std::ios::in | std::ios::binary);
                if (!fp.is_open())
                {
                    std::cerr << "Failed to open file = " << file << ", fullPath = " << fullPath << ", for processing!" << std::endl;
                    continue;
                }

                std::cout << "Processing file = " << file << ", fullPath = " << fullPath << ", decompressedPath = " << decompressedPath << "..." << std::endl;
                unpackGGZ(fp, file, decompressedDestPath);
                fileCount++;
            }
        }

        if (0 == fileCount)
        {
            std::cerr << "No input files found." << std::endl;
        }
    }

    catch (...)
    {
        std::cerr << std::endl << boost::current_exception_diagnostic_information() << std::endl;
        return 1;
    }

    return 0;
}
