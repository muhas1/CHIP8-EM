#include "chip8.h"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <random>
#include <sys/types.h>

const unsigned int START_ADDRESS = 0x200;
// The set is 80 as we have 16 characters each being 5 bytes
const unsigned int FONTSET_SIZE = 80;
// Initialize the font array
uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
// Starting address for putting the font into memory
const unsigned FONT_START_ADDRESS = 0x50;

chip8::chip8()
    : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
  // Initialize the program counter
  // CHIP8 memory from 0x000 to 0x1FF is reserved
  // so the ROM instructions must start from 0x200
  pc = START_ADDRESS;

  for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
    // Loading fonts into memory starting at the address and incrementing by one
    // each time
    memory[FONT_START_ADDRESS + i] = fontset[i];
  }
  // Get a random number between 0 and 255
  randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
}

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

void chip8::op_00E0() { memset(window, 0, sizeof(window)); }

// The top of the stack will have the address of one instruction PAST the one
// that called the subroutine so we can put that back in the program counter,
// this overwrites our preemptive pc += 2;
void chip8::OP_00EE() {
  --sp;
  pc = stack[sp];
}

// Jump to location nnn
// This instruction sets the program counter to nnn
// A jump doesn't need to remember its origin so we can ignore having to use sp
void chip8::OP_1nnn() {
  // Perform a bitwise AND operation, this extracts the lower 12 bits and and
  // discards the upper 4
  uint16_t address = opcodes & 0x0FFFu;
  pc = address;
}

// Call subroutine at nnn
// When we call a subroutine we want to return to it eventually,
// we put the current PC into the top of the stack, Remember we did
// PC += 2 in Cycle() so the current PC holds the next instruction after this
// CALL We dont want to return to the call instruction because it would cause an
// infinite loop
void chip8::OP_2nnn() {
  uint16_t address = opcodes & 0x0FFFu;

  stack[sp] = pc;
  ++sp;
  pc = address;
}

/*
 * Skip next instruction if Vx = kk.
 * Since our PC has already been incremented by 2 in Cycle(), we can just
 * increment by 2 again to skip the next instruction.
 */
void chip8::OP_3xkk() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t byte = (opcodes & 0x00FFu);
  if (registers[vx] == byte) {
    pc += 2;
  }
}

/*
 * Skip next instruction if Vx != kk.
 * Since our PC has already been incremented by 2 in Cycle(), we can just
 * increment by 2 again to skip the next instruction.
 */
void chip8::OP_4xkk() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t byte = (opcodes & 0x00FFu);
  if (registers[vx] != byte) {
    pc += 2;
  }
}

// Skip the instruction if vx == vy
void chip8::OP_5xy0() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t vy = (opcodes & 0x00F0u) >> 4u;
  if (registers[vx] == registers[vy]) {
    pc += 2;
  }
}

// Set Vx = kk
void chip8::OP_6xkk() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t byte = opcodes & 0x00FFu;

  registers[vx] = byte;
}

// Set vx = Vx + kk
void chip8::OP_7xkk() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t byte = (opcodes & 0x00FFu);
  registers[vx] += byte;
}

// Set vx = vy
void chip8::OP_8xy0() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t vy = (opcodes & 0x00F0u) >> 4u;
  registers[vx] = registers[vy];
}

// Set vx = vx OR vy
void chip8::OP_8xy1() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t vy = (opcodes & 0x00F0u) >> 4u;

  registers[vx] |= registers[vy];
}

// Set Vx = Vx AND Vy.
void chip8::OP_8xy2() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t vy = (opcodes & 0x00F0u) >> 4u;
  registers[vx] &= registers[vy];
}

// Set Vx = Vx XOR Vy.
void chip8::OP_8xy3() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t vy = (opcodes & 0x00F0u) >> 4u;
  registers[vx] ^= registers[vy];
}
// Set Vx = Vx + Vy, set VF = carry.

// The values of Vx and Vy are added together. If the result is greater than 8
// bits (i.e., > 255,) VF is set to 1, otherwise 0. Only the lowest 8 bits of
// the result are kept, and stored in Vx.

// This is an ADD with an overflow flag. If the sum is greater than what can fit
// into a byte (255), register VF will be set to 1 as a flag.
void chip8::OP_8xy4() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t vy = (opcodes & 0x00F0u) >> 4u;

  uint16_t sum = registers[vx] + registers[vy];
  if (sum > 255U) {
    registers[0xF] = 1;
  } else {
    registers[0xF] = 0;
  }

  registers[vx] = sum & 0xFFu;
}

// Set Vx = Vx - Vy, set VF = NOT borrow.
// If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx,
// and the results stored in Vx.
void chip8::OP_8xy5() {
  std::cout << "temp" << std::endl;
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t vy = (opcodes & 0x00F0u) >> 4u;

  if (registers[vx] > registers[vy]) {
    registers[0xF] = 1;
  } else {
    registers[0xF] = 0;
  }

  registers[vx] -= registers[vy];
}

// Set Vx = Vx SHR 1.
// If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0.
// Then Vx is divided by 2.
// A right shift is performed (division by 2), and the least significant bit is
// saved in Register VF.
void chip8::OP_8xy6() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  // Save LSB in vf
  registers[0xF] = (registers[vx] & 0x1u);
  registers[vx] >>= 1;
}

// Set vx = vy, set vf = NOT borrow
// if vy > vx, then vf is set to 1, otherwise its set to 0, then vx is
// subtracted from from vx and results are stored in vx
void chip8::OP_8xy7() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t vy = (opcodes & 0x00F0u) >> 4u;

  if (registers[vy] > registers[vx]) {
    registers[0xF] = 1;
  } else {
    registers[0xF] = 0;
  }

  registers[vx] = registers[vy] - registers[vx];
}

// Extracts the x registers value from the opcodes
// Saves the MSB into the carry flag register VF
// Shifts the value in vx one bit to the left
void chip8::OP_8xyE() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  // Save MSB into VF
  registers[0xF] = (registers[vx] & 0x80u) >> 7u;
  registers[vx] <<= 1;
}

// Extract both the x and y register values and store them
// compares the values in vx and vy registers
// if the values are not equal increment the pc counter and skip this
// instruction
void chip8::OP_9xy0() {
  uint8_t vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t vy = (opcodes & 0x00F0u) >> 4u;
  if (registers[vx] != registers[vy]) {
    pc += 2;
  }
}

// Set the index to the address
void chip8::OP_Annn() {
  uint16_t address = (opcodes & 0x0FFFu);
  index = address;
}

// Jump to a new address
void chip8::OP_Bnnn() {
  uint16_t address = (opcodes & 0x0FFFu);
  pc = registers[0] + address;
}

// Set the register x to a random byte AND kk
void chip8::OP_Cxkk() {
  uint8_t Vx = (opcodes & 0x0F00u) >> 8u;
  uint8_t byte = opcodes & 0x00FFu;

  registers[Vx] = randByte(randGen) & byte;
}
