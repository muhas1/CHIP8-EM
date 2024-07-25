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

  uint8_t keypad[KEY_COUNT]{};
  uint32_t window[CHIP8_WIDTH * CHIP8_HEIGHT]{};

private:
  /*
   * When we use {} it will initialize an array to contain all zeroes
   * if we don't use {}, and leave it as something like uint8_t
   * memory[MEMORY_SIZE]; then the values in the array are uninitialized and can
   * be random
   */

  uint8_t memory[MEMORY_SIZE]{}; // Memory of the emulator is initialized to 4K
  uint8_t registers[REGISTER_COUNT]{}; // register array that contains the 16
                                       // 8-bit registers that we will need
  uint8_t stack[STACK_VALUE]{}; // Our stack to store our location during jumps
  uint8_t sp;                   // our stack pointer

  /*
   * There is an index register and a program counter that can store values
   * between 0x000 to 0xFFF
   */
  uint16_t index{};
  uint16_t pc{};

  /*
   * The CHIP8 contains 2 timer registers that count at 60Hz.
   * When set above 0, they'll count down to 0 (zero)
   */
  uint8_t delay_timer;
  uint8_t sound_timer;

  /*
   * The Chip8 has 35 opcodes that are all two bytes long,
   * we can use a uint16_t to represent this and store our values
   */
  uint16_t opcodes{};

  /*
   * There is an instruction which places a random number in a register.
   * If we were using physical hardware this could be achieved by,
   * reading the value from a noisy disconnected pin or using a dedicated RNG
   * Chip For our case we can use C++ built in random facilities
   */
  std::default_random_engine randGen;
  std::uniform_int_distribution<uint8_t> randByte;

  // Clear display instruction 00E0 CLS
  void op_00E0();
};
