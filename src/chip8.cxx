#include "chip8.h"
#include <cstring>
#include <fstream>
#include <iosfwd>

const unsigned int START_ADDRESS = 0x200;

void chip8::loadRom(char const *filename) {
  // Open the file using fstream
  std::fstream file(filename, std::ios::binary | std::ios::ate);

  if (file.is_open()) {

    // Get the required size of the file, and allocate a buffer
    std::streampos size = file.tellg();
    char *buffer = new char[size];

    // Go back to the beginning of the file and fill the buffer
    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
    file.close();

    // Load the ROM contents into the Chip8 Memory, starting at 0x200
    for (long i = 0; i < size; ++i) {
      memory[START_ADDRESS + i] = buffer[i];
    }

    delete[] buffer;
  }
}
