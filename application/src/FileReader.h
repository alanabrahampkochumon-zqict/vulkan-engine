#pragma once
/**
 * @file FileReader.h
 * @author Alan Abraham P Kochumon
 * @date Created on: July 24, 2026
 *
 * @brief Implementation for file reading logic.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */


#include <fstream>
#include <vector>


std::vector<char> readFile(const std::string& filename)
{
    // Create a fstream with read binary and start at end of file set.
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    // Create a buffer
    const auto fileSize = static_cast<std::size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    // Move the file ptr to the first
    file.seekg(0);
    // Read the file
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}