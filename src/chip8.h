#pragma once

#include <cstdint>
#include <random>
#include <stdio.h>

/*
 * The Following are CHIP8 values that we can declare as global and use
 * later on in the code, the functionality of each global var is described above
 * it
 * */

// The CHIP8 Emulator uses a Hex Keypad
const unsigned int KEY_COUNT = 16;
// The CHIP8 has 4KB of Memory
const unsigned int MEMORY_SIZE = 4096;
// There are 15 Registers, 1 extra Register for a carry flag
const unsigned int REGISTER_COUNT = 16;
// The CHIP8 will use a stack to remember to remember a location
// before a jump occurs, this will reflect the register number of 16
const unsigned int STACK_VALUE = 16;
// CHIP8 has a graphics dimension of 2048 pixels [64 x 32]
const unsigned int CHIP8_HEIGHT = 64;
const unsigned int CHIP8_WIDTH = 32;

class chip8 {
public:
  chip8();
  void loadRom(char const *filename);
  void emulationCycle();
  ~chip8();

private:
};
