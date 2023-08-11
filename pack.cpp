/*
pack.cpp -- GGZ BREW / Zeebo assets pack tool

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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

static void compressGZIP(const std::vector<char>& decompressed, std::vector<char>& compressed, const std::string& fileName, const std::time_t& mtime)
{
    boost::iostreams::filtering_ostream os;
    boost::iostreams::gzip_params params;
    params.file_name = fileName;
    params.mtime = mtime;
    os.push(boost::iostreams::gzip_compressor(params));
    os.push(boost::iostreams::back_inserter(compressed));
    boost::iostreams::write(os, decompressed.data(), decompressed.size());
}

static void readBinaryFile(std::vector<char>& decompressed, const std::string& path)
{
    std::fstream fp(path, std::ios::in | std::ios::binary);
    if (!fp.is_open())
    {
        throw std::runtime_error(std::string("Failed to open file = ") + path + " for reading!");
    }

    fp.seekg(0, fp.end);
    size_t size = fp.tellg();
    fp.seekg(0, fp.beg);
    decompressed.reserve(size);
    std::copy(std::istreambuf_iterator<char>(fp), std::istreambuf_iterator<char>(), std::back_inserter(decompressed));
    fp.close();
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

static void checkFile(const std::string& path)
{
    if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
    {
        throw std::runtime_error(std::string("Path ") + path + " does not exist or is not a file!");
    }
}

static void packGGZ(std::fstream& ggz, const std::string& file, const std::string& decompressedPath, const std::string& compressedPath)
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
    } while (1);

    offsetsEnd = offsetsStart;
    offsetsEnd.erase(offsetsEnd.begin() + 0);
    offsetsEnd.push_back(size);

    const std::string compressedFilePath = (std::filesystem::path(compressedPath) / file).string();

    std::vector<std::pair<size_t, std::vector<char>>> files;

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

        if (ggz.eof())
            throw std::runtime_error("End of file, this should not happen!");

        const std::string decompressedFilePath = (std::filesystem::path(decompressedPath) / name).string();
        std::vector<char> decompressed;
        readBinaryFile(decompressed, decompressedFilePath);

        struct stat s;
        if (0 != stat(decompressedFilePath.c_str(), &s))
            throw std::runtime_error(std::string("Failed to stat file = ") + decompressedFilePath);

        std::vector<char> compressed;
        compressGZIP(decompressed, compressed, name, s.st_mtime);

        std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << start << " "
                                                << std::setfill('0') << std::setw(8) << end << " "
                                                << std::dec << originalSize << " " << size << " " << decompressed.size() << " " << compressed.size() << " " << name << " " << decompressedFilePath << " " << compressedFilePath << std::endl;

        files.push_back(std::make_pair(decompressed.size(), compressed));
    }

    std::vector<char> ggzFile;
    std::vector<char> ggzFileData;

    const size_t ggzDataOffset = files.size() * sizeof(uint32_t) * 2;

    auto write32 = [&](const uint32_t& value) -> void
    {
        const uint32_t swappedValue = __bswap_32(value);
        std::copy((char*)(&swappedValue), (char*)(&swappedValue) + sizeof(uint32_t), std::back_inserter(ggzFile));
    };

    for (auto& e : files)
    {
        const auto& [ originalSize, compressedData ] = e;
        write32(ggzDataOffset + ggzFileData.size());
        write32(originalSize);
        std::copy(compressedData.begin(), compressedData.end(), std::back_inserter(ggzFileData));
    }

    std::copy(ggzFileData.begin(), ggzFileData.end(), std::back_inserter(ggzFile));

    std::cout << "Writing file = " << file << ", compressedFilePath = " << compressedFilePath << std::endl;
    writeBinaryFile(ggzFile, compressedFilePath);
}

static void createDirectory(const std::string& path)
{
    if (std::filesystem::exists(path))
    {
        std::filesystem::remove_all(path);
    }

    std::filesystem::create_directory(path);
}

static void checkDirectory(const std::string& path)
{
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
    {
        throw std::runtime_error(std::string("Path ") + path + " does not exist or is not a directory!");
    }
}

int main(int argc, char* argv[])
{
    std::cout << "GGZ BREW / Zeebo assets pack tool " << __VERSION_VER__ << " - " << __VERSION_DATE__ << std::endl;
    std::cout << "by Przemyslaw Skryjomski (Tuxality)" << std::endl << std::endl;

    try
    {
        static const std::string compressedPath = "compressed";
        createDirectory(compressedPath);

        static const std::string decompressedPath = "decompressed";
        checkDirectory(decompressedPath);

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
                checkDirectory(decompressedDestPath);

                std::fstream fp(fullPath, std::ios::in | std::ios::binary);
                if (!fp.is_open())
                {
                    std::cerr << "Failed to open file = " << file << ", fullPath = " << fullPath << ", for processing!" << std::endl;
                    continue;
                }

                std::cout << "Processing file = " << file << ", fullPath = " << fullPath << ", decompressedPath = " << decompressedPath << ", compressedPath = " << compressedPath << "..." << std::endl;
                packGGZ(fp, file, decompressedDestPath, compressedPath);
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
