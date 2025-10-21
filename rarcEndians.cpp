#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
bool isLE;
uint32_t Swap32(uint32_t val) {
    return ((val >> 24) & 0x000000FF) |
           ((val >> 8)  & 0x0000FF00) |
           ((val << 8)  & 0x00FF0000) |
           ((val << 24) & 0xFF000000);
}

uint16_t Swap16(uint16_t val) {
    return ((val >> 8) & 0x00FF) |
           ((val << 8) & 0xFF00);
}

#pragma pack(push, 1)
struct RarcHeader {
    uint32_t magic;
    uint32_t fileSize;
    uint32_t dataHeaderOffset;
    uint32_t fileDataOffset;
    uint32_t fileDataLength;
    uint32_t mramSize;
    uint32_t aramSize;
    uint32_t dvdSize;
};

struct RarcDataHeader {
    uint32_t dirNodeCount;
    uint32_t dirNodeOffset;
    uint32_t fileNodeCount;
    uint32_t fileNodeOffset;
    uint32_t stringTableSize;
    uint32_t stringTableOffset;
    uint16_t nextFileIndex;
    uint8_t  fileIDSyncFlag;
    uint8_t  padding[5];
};

struct RarcDirectoryNode {
    uint32_t type;       
    uint32_t nameOffset;
    uint16_t nameHash;
    uint16_t fileNodeCount;
    uint32_t fileNodeOffset;
};

struct RarcFileNode {
    uint16_t nodeIndex;
    uint16_t nameHash;
    uint32_t  typeFlag;
    uint32_t nameOffset;
    uint32_t dataOffsetOrDirIndex;
    uint32_t dataSizeOrDirSize;
};
#pragma pack(pop)

void ConvertRarcHeader(RarcHeader& hdr) {
    hdr.magic             = Swap32(hdr.magic);
    hdr.fileSize          = Swap32(hdr.fileSize);
    hdr.dataHeaderOffset  = Swap32(hdr.dataHeaderOffset);
    hdr.fileDataOffset    = Swap32(hdr.fileDataOffset);
    hdr.fileDataLength    = Swap32(hdr.fileDataLength);
    hdr.mramSize          = Swap32(hdr.mramSize);
    hdr.aramSize          = Swap32(hdr.aramSize);
    hdr.dvdSize           = Swap32(hdr.dvdSize);
}

void ConvertRarcDataHeader(RarcDataHeader& dhdr) {
    dhdr.dirNodeCount       = Swap32(dhdr.dirNodeCount);
    dhdr.dirNodeOffset      = Swap32(dhdr.dirNodeOffset);
    dhdr.fileNodeCount      = Swap32(dhdr.fileNodeCount);
    dhdr.fileNodeOffset     = Swap32(dhdr.fileNodeOffset);
    dhdr.stringTableSize    = Swap32(dhdr.stringTableSize);
    dhdr.stringTableOffset  = Swap32(dhdr.stringTableOffset);
    dhdr.nextFileIndex      = Swap16(dhdr.nextFileIndex);
}

void ConvertDirectoryNode(RarcDirectoryNode& node) {
    node.type            = Swap32(node.type);
    node.nameOffset      = Swap32(node.nameOffset);
    node.nameHash        = Swap16(node.nameHash);
    node.fileNodeCount   = Swap16(node.fileNodeCount);
    node.fileNodeOffset  = Swap32(node.fileNodeOffset);
}

void ConvertFileNode(RarcFileNode& node) {
    node.nodeIndex             = Swap16(node.nodeIndex);
    node.nameHash              = Swap16(node.nameHash);
    node.typeFlag = Swap32(node.typeFlag);
    node.nameOffset            = Swap32(node.nameOffset);
    node.dataOffsetOrDirIndex = Swap32(node.dataOffsetOrDirIndex);
    node.dataSizeOrDirSize    = Swap32(node.dataSizeOrDirSize);
}



int main(int argc, char* argv[]) {

    std::string inputPath  = argv[1];
    std::string  outputPath;
    if (!argv[1]) {
        std::cerr << "Usage: rarcEndians.exe <input.arc> (output.arc)\n";
        return 1;
    }
    if (!argv[2])
    {
outputPath = inputPath + "_out.arc";
    }
    else
{
outputPath = argv[2];
}
    std::ifstream in(inputPath, std::ios::binary);
    std::ofstream out(outputPath, std::ios::binary);
    if (!in) {
        std::cerr << "OOPS!: " << "\n";
        return 1;
    }
    if (!out) {
        std::cerr << "OOPS...: " << "\n";
        return 1;
    }
in.seekg(0, std::ios::beg);
char ch[4];
in.read(ch, 4);
std::string magicStr1(ch, 4);
if (magicStr1 == "RARC")
{
isLE = false;
}
else if (magicStr1 == "CRAR")
{
isLE = true;
}
else
{
    std::cerr << "THIS IS NOT RARC FILE."<< "\n";
        return 1;
}
in.seekg(0, std::ios::beg);
    RarcHeader hdr;
    in.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    ConvertRarcHeader(hdr);
    out.write(reinterpret_cast<char*>(&hdr), sizeof(hdr));
if (isLE == false) in.seekg(hdr.dataHeaderOffset);
else if (isLE == true) in.seekg(Swap32(hdr.dataHeaderOffset));
    RarcDataHeader dhdr;
    in.read(reinterpret_cast<char*>(&dhdr), sizeof(dhdr));
    ConvertRarcDataHeader(dhdr);
    out.write(reinterpret_cast<char*>(&dhdr), sizeof(dhdr));
if (isLE == false) in.seekg(dhdr.dirNodeOffset + 0x20);
else in.seekg(Swap32(dhdr.dirNodeOffset) + 0x20);
if (isLE == true)
{
    std::vector<RarcDirectoryNode> dirNodes(Swap32(dhdr.dirNodeCount));
    for (auto& node : dirNodes) {
        in.read(reinterpret_cast<char*>(&node), sizeof(node));
        ConvertDirectoryNode(node);
        out.write(reinterpret_cast<char*>(&node), sizeof(node));
    }
}
else
{
        std::vector<RarcDirectoryNode> dirNodes(dhdr.dirNodeCount);
    for (auto& node : dirNodes) {
        in.read(reinterpret_cast<char*>(&node), sizeof(node));
        ConvertDirectoryNode(node);
        out.write(reinterpret_cast<char*>(&node), sizeof(node));
    }
}
if (isLE == false)
{
    in.seekg(dhdr.fileNodeOffset + 0x20);
    std::vector<RarcFileNode> fileNodes(dhdr.fileNodeCount);
    for (auto& node : fileNodes) {
        in.read(reinterpret_cast<char*>(&node), sizeof(node));
        ConvertFileNode(node);
        out.write(reinterpret_cast<char*>(&node), sizeof(node));
    }
}
else
{
    in.seekg(Swap32(dhdr.fileNodeOffset) + 0x20);
    std::vector<RarcFileNode> fileNodes(Swap32(dhdr.fileNodeCount));
    for (auto& node : fileNodes) {
        in.read(reinterpret_cast<char*>(&node), sizeof(node));
        ConvertFileNode(node);
        out.write(reinterpret_cast<char*>(&node), sizeof(node));
    }
}
std::streampos currentPos = in.tellg();

in.seekg(0, std::ios::end);
std::streampos endPos = in.tellg();
std::streamsize remainingSize = endPos - currentPos;

in.seekg(currentPos);
std::vector<char> remainingData(static_cast<size_t>(remainingSize));
if (!in.read(remainingData.data(), remainingSize)) {
    std::cerr << "FAILURE\n";
    return 1;
}
out.write(remainingData.data(), remainingSize);


    std::cout << "DONE" << "\n";





return 0;
}