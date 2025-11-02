#pragma once

#include <lz4.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include "Macro.hpp"

namespace compression {

  /**
   * @brief LZ4-based compressor and decompressor for byte buffers.
   *
   * We use Raw LZ4 compression with a custom header:
   * - 4 bytes: Magic bytes 'L', 'Z', '4', 0
   * - 4 bytes: Original uncompressed size (big-endian)
   * - 4 bytes: Compressed size (big-endian)
   * - N bytes: Compressed data
   *
   * The header is 12 bytes total.
   */
  class Compressor {
    public:
      static std::vector<std::uint8_t> compress(
          const std::vector<std::uint8_t> &input, float ratio = 0.9f) {
        if (input.empty()) {
          return {};
        }

        const int srcSize = static_cast<int>(input.size());
        const int destSize = LZ4_compressBound(srcSize);

        if (destSize <= 0) {
          std::cerr << "[ERROR] LZ4_compressBound returned invalid size"
                    << std::endl;
          throw std::runtime_error("LZ4_compressBound failed");
        }

        std::vector<std::uint8_t> compressed;
        compressed.resize(destSize);

        const int compressedSize = LZ4_compress_default(
            reinterpret_cast<const char *>(input.data()),
            reinterpret_cast<char *>(compressed.data()), srcSize, destSize);

        if (compressedSize <= 0) {
          std::cerr << "[ERROR] LZ4 compression failed!" << std::endl;
          throw std::runtime_error("LZ4_compress_default failed");
        }

        const std::size_t finalSize = HEADER_SIZE_LZ4 + compressedSize;

        const float compressionRatio =
            static_cast<float>(finalSize) / static_cast<float>(srcSize);

        if (compressionRatio >= ratio) {
          return input;
        }

        std::vector<std::uint8_t> result;
        result.reserve(finalSize);

        /*
         * Magic bytes (4)
         */
        result.push_back('L');
        result.push_back('Z');
        result.push_back('4');
        result.push_back(0);

        /*
         * Original size (4) and Compressed size (4)
         */
        result.push_back(static_cast<std::uint8_t>((srcSize >> 24) & 0xFF));
        result.push_back(static_cast<std::uint8_t>((srcSize >> 16) & 0xFF));
        result.push_back(static_cast<std::uint8_t>((srcSize >> 8) & 0xFF));
        result.push_back(static_cast<std::uint8_t>(srcSize & 0xFF));

        result.push_back(
            static_cast<std::uint8_t>((compressedSize >> 24) & 0xFF));
        result.push_back(
            static_cast<std::uint8_t>((compressedSize >> 16) & 0xFF));
        result.push_back(
            static_cast<std::uint8_t>((compressedSize >> 8) & 0xFF));
        result.push_back(static_cast<std::uint8_t>(compressedSize & 0xFF));

        result.insert(result.end(), compressed.begin(),
                      compressed.begin() + compressedSize);

        return result;
      }

      static std::vector<std::uint8_t> decompress(
          const std::vector<std::uint8_t> &input) {
        if (input.empty()) {
          return input;
        }

        if (!isCompressed(input)) {
          return input;
        }

        if (input.size() < HEADER_SIZE_LZ4) {
          std::cerr << "[ERROR] Compressed packet too small (size: "
                    << input.size() << ")" << std::endl;
          return input;
        }

        /**
         * We read the original size from the header (bytes 4-7)
         */
        std::uint32_t originalSize =
            (static_cast<std::uint32_t>(input[4]) << 24) |
            (static_cast<std::uint32_t>(input[5]) << 16) |
            (static_cast<std::uint32_t>(input[6]) << 8) |
            static_cast<std::uint32_t>(input[7]);

        std::uint32_t compressedSize = input.size() - HEADER_SIZE_LZ4;

        if (originalSize == 0) {
          std::cerr << "[ERROR] Invalid original size: " << originalSize
                    << std::endl;
          return input;
        }

        if (compressedSize == 0 || compressedSize > input.size()) {
          std::cerr << "[ERROR] Invalid compressed size: " << compressedSize
                    << std::endl;
          return input;
        }

        if (input.size() < HEADER_SIZE_LZ4 + compressedSize) {
          std::cerr << "[ERROR] Input size mismatch: got " << input.size()
                    << " bytes, expected at least "
                    << (HEADER_SIZE_LZ4 + compressedSize) << std::endl;
          return input;
        }

        std::vector<std::uint8_t> decompressed;
        decompressed.resize(originalSize);

        const int decompressedSize = LZ4_decompress_safe(
            reinterpret_cast<const char *>(input.data() + HEADER_SIZE_LZ4),
            reinterpret_cast<char *>(decompressed.data()),
            static_cast<int>(compressedSize), static_cast<int>(originalSize));

        if (decompressedSize < 0) {
          std::cerr << "[ERROR] LZ4 decompression failed with code: "
                    << decompressedSize << std::endl;
          return input;
        }

        return decompressed;
      }

      static bool isCompressed(const std::vector<std::uint8_t> &buffer) {
        return buffer.size() >= HEADER_SIZE_LZ4 && buffer[0] == 'L' &&
               buffer[1] == 'Z' && buffer[2] == '4' && buffer[3] == 0;
      }
  };

}  // namespace compression
