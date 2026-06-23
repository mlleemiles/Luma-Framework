// standalone_patch.cpp
#include <vector>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>

#include "hash.h"

// Minimal required structures
struct DXBCHeader
{
  static constexpr uint32_t hash_size = 16;

  char format_name[4]; // 'DXBC'
  uint8_t hash[hash_size]; // Checksum MD5
  uint32_t version; // Seemengly always 1
  uint32_t file_size; // Total size in bytes (including the header)
  uint32_t chunk_count;
  uint32_t chunk_offsets[]; // Array of DWORD offsets in bytes, from the beginning of the object/header
};

// From "dxbc-spirv", developed by Philip Rebohle, MIT license
Hash::MD5::Digest CalcDXBCHash(const void* data, size_t size)
{
  constexpr size_t BlockSize = 64u;

  /* Skip initial part of the header including the hash digest */
  size_t offset = offsetof(DXBCHeader, version);

  if (size < offset)
     return Hash::MD5::Digest();

  auto bytes = reinterpret_cast<const unsigned char*>(data) + offset;
  size -= offset;

  /* Compute byte representations of the bit count and a derived
   * number that will be appended to the stream */
  const uint32_t aNum = uint32_t(size) * 8u;
  const uint32_t bNum = (aNum >> 2u) | 1u;

  std::array<uint8_t, sizeof(uint32_t)> a = { };
  std::array<uint8_t, sizeof(uint32_t)> b = { };

  for (uint32_t i = 0u; i < sizeof(uint32_t); i++) {
     a[i] = Bextract(aNum, 8u * i, 8u);
     b[i] = Bextract(bNum, 8u * i, 8u);
  }

  /* Hash remaining header and all chunk data */
  size_t remainder = size % BlockSize;
  size_t paddingSize = BlockSize - remainder;

  Hash::MD5::Hasher hasher = { };
  hasher.update(bytes, size - remainder);

  /* DXBC hashing does not finalize the last block properly, instead
   * padding behaviour depends on the size of the byte stream */
  static const std::array<uint8_t, BlockSize> s_padding = { 0x80u };

  if (remainder >= 56u) {
     /* Append last block and pad to multiple of 64 bytes */
     hasher.update(&bytes[size - remainder], remainder);
     hasher.update(s_padding.data(), paddingSize);

     /* Pad with null block and custom finalizer */
     hasher.update(a.data(), a.size());
     hasher.update(s_padding.data() + a.size(), s_padding.size() - a.size() - b.size());
     hasher.update(b.data(), b.size());
  }
  else {
     /* Append bit count */
     hasher.update(a.data(), a.size());

     /* Append last block */
     if (remainder)
        hasher.update(&bytes[size - remainder], remainder);

     /* Append regular padding sequence */
     hasher.update(s_padding.data(), paddingSize - a.size() - b.size());

     /* Append final magic number */
     hasher.update(b.data(), b.size());
  }

  return hasher.getDigest();
}

#define OFFLINE_PATCHER 1
#include "../includes/shader_patches.h"

std::vector<std::byte> ReadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<std::byte> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file: " + filename);
    }
    
    return buffer;
}

void WriteFile(const std::string& filename, const std::vector<std::byte>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to create file: " + filename);
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <input_shader> <output_shader>" << std::endl;
        return 1;
    }
    
    try {
        std::string inputFile = argv[1];
        std::string outputFile = argv[2];
        
        std::cout << "Reading shader from: " << inputFile << std::endl;
        std::vector<std::byte> shaderCode = ReadFile(inputFile);
        std::cout << "Original shader size: " << shaderCode.size() << " bytes" << std::endl;
        
        std::cout << "Patching shader..." << std::endl;
        uint16_t type = GetShaderProgramType(shaderCode);
        if (type == 1)
        {
            PatchVertexShader(shaderCode);
        }
        else if (type == 0)
        {
            PatchPixelShader(shaderCode);
        }
        else if (type == 5)
        {
            PatchComputeShader(shaderCode);
        }
        else
        {
            std::cout << "Unimplemented shader type: " << type << std::endl;
            return 1;
        }
        std::cout << "Patched shader size: " << shaderCode.size() << " bytes" << std::endl;
        
        std::cout << "Writing patched shader to: " << outputFile << std::endl;
        WriteFile(outputFile, shaderCode);
        
        std::cout << "Success!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}