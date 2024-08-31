#pragma once

#include <array>

#include "common/type.h"
#include "common/utils.h"

namespace gb {

class MemoryBus;

struct Instruction {
  const char* name{};
  const u8 idx{};
  const u8 size{}; // cb prefixed inst must minus 1.
};

struct DisassembleRecord {
  const Instruction* instruction{};
  u16 pc{};
  u8 operand[2]{};
};

class Disassembler {
public:
  void enable() { enable_ = true; }

  void disable() { enable_ = false; }

  void memoryBus(MemoryBus* memory_bus) {
    memory_bus_ = memory_bus;
    std::fill(records_.begin(), records_.end(), DisassembleRecord{});
    records_count_ = 0;
    disassembleROM();
  }

  void disassembleROM();

  u8 disassemble(u16 pc, bool force_update = false);

  const auto& records() const { return records_; }

  u16 recordCount() const { return records_count_; }

  static bool isReturn(u8 op) { return inOr(op, 0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7, 0xFF); }

  static bool isJump(u8 op) {
    return inOr(op, 0xC3, 0xE9, 0x18, 0xC2, 0xCA, 0xD2, 0xDA, 0x20, 0x28, 0x30, 0x38);
  }

  static bool isBasicBlockEnd(u8 op) { return isJump(op) || isReturn(op); }

private:
  bool enable_{};
  const MemoryBus* memory_bus_{};
  std::array<DisassembleRecord, 0x10000> records_;
  u16 records_count_{};


  static constexpr Instruction instructions_unprefixed_[] = {
          {"NOP", 0x00, 1},         {"LD BC,n16", 0x01, 3},  {"LD [BC],A", 0x02, 1},  {"INC BC", 0x03, 1},
          {"INC B", 0x04, 1},       {"DEC B", 0x05, 1},      {"LD B,n8", 0x06, 2},    {"RLCA", 0x07, 1},
          {"LD [a16],SP", 0x08, 3}, {"ADD HL,BC", 0x09, 1},  {"LD A,[BC]", 0x0A, 1},  {"DEC BC", 0x0B, 1},
          {"INC C", 0x0C, 1},       {"DEC C", 0x0D, 1},      {"LD C,n8", 0x0E, 2},    {"RRCA", 0x0F, 1},
          {"STOP n8", 0x10, 2},     {"LD DE,n16", 0x11, 3},  {"LD [DE],A", 0x12, 1},  {"INC DE", 0x13, 1},
          {"INC D", 0x14, 1},       {"DEC D", 0x15, 1},      {"LD D,n8", 0x16, 2},    {"RLA", 0x17, 1},
          {"JR e8", 0x18, 2},       {"ADD HL,DE", 0x19, 1},  {"LD A,[DE]", 0x1A, 1},  {"DEC DE", 0x1B, 1},
          {"INC E", 0x1C, 1},       {"DEC E", 0x1D, 1},      {"LD E,n8", 0x1E, 2},    {"RRA", 0x1F, 1},
          {"JR NZ,e8", 0x20, 2},    {"LD HL,n16", 0x21, 3},  {"LD [HL+],A", 0x22, 1}, {"INC HL", 0x23, 1},
          {"INC H", 0x24, 1},       {"DEC H", 0x25, 1},      {"LD H,n8", 0x26, 2},    {"DAA", 0x27, 1},
          {"JR Z,e8", 0x28, 2},     {"ADD HL,HL", 0x29, 1},  {"LD A,[HL+]", 0x2A, 1}, {"DEC HL", 0x2B, 1},
          {"INC L", 0x2C, 1},       {"DEC L", 0x2D, 1},      {"LD L,n8", 0x2E, 2},    {"CPL", 0x2F, 1},
          {"JR NC,e8", 0x30, 2},    {"LD SP,n16", 0x31, 3},  {"LD [HL-],A", 0x32, 1}, {"INC SP", 0x33, 1},
          {"INC [HL]", 0x34, 1},    {"DEC [HL]", 0x35, 1},   {"LD [HL],n8", 0x36, 2}, {"SCF", 0x37, 1},
          {"JR C,e8", 0x38, 2},     {"ADD HL,SP", 0x39, 1},  {"LD A,[HL-]", 0x3A, 1}, {"DEC SP", 0x3B, 1},
          {"INC A", 0x3C, 1},       {"DEC A", 0x3D, 1},      {"LD A,n8", 0x3E, 2},    {"CCF", 0x3F, 1},
          {"LD B,B", 0x40, 1},      {"LD B,C", 0x41, 1},     {"LD B,D", 0x42, 1},     {"LD B,E", 0x43, 1},
          {"LD B,H", 0x44, 1},      {"LD B,L", 0x45, 1},     {"LD B,[HL]", 0x46, 1},  {"LD B,A", 0x47, 1},
          {"LD C,B", 0x48, 1},      {"LD C,C", 0x49, 1},     {"LD C,D", 0x4A, 1},     {"LD C,E", 0x4B, 1},
          {"LD C,H", 0x4C, 1},      {"LD C,L", 0x4D, 1},     {"LD C,[HL]", 0x4E, 1},  {"LD C,A", 0x4F, 1},
          {"LD D,B", 0x50, 1},      {"LD D,C", 0x51, 1},     {"LD D,D", 0x52, 1},     {"LD D,E", 0x53, 1},
          {"LD D,H", 0x54, 1},      {"LD D,L", 0x55, 1},     {"LD D,[HL]", 0x56, 1},  {"LD D,A", 0x57, 1},
          {"LD E,B", 0x58, 1},      {"LD E,C", 0x59, 1},     {"LD E,D", 0x5A, 1},     {"LD E,E", 0x5B, 1},
          {"LD E,H", 0x5C, 1},      {"LD E,L", 0x5D, 1},     {"LD E,[HL]", 0x5E, 1},  {"LD E,A", 0x5F, 1},
          {"LD H,B", 0x60, 1},      {"LD H,C", 0x61, 1},     {"LD H,D", 0x62, 1},     {"LD H,E", 0x63, 1},
          {"LD H,H", 0x64, 1},      {"LD H,L", 0x65, 1},     {"LD H,[HL]", 0x66, 1},  {"LD H,A", 0x67, 1},
          {"LD L,B", 0x68, 1},      {"LD L,C", 0x69, 1},     {"LD L,D", 0x6A, 1},     {"LD L,E", 0x6B, 1},
          {"LD L,H", 0x6C, 1},      {"LD L,L", 0x6D, 1},     {"LD L,[HL]", 0x6E, 1},  {"LD L,A", 0x6F, 1},
          {"LD [HL],B", 0x70, 1},   {"LD [HL],C", 0x71, 1},  {"LD [HL],D", 0x72, 1},  {"LD [HL],E", 0x73, 1},
          {"LD [HL],H", 0x74, 1},   {"LD [HL],L", 0x75, 1},  {"HALT", 0x76, 1},       {"LD [HL],A", 0x77, 1},
          {"LD A,B", 0x78, 1},      {"LD A,C", 0x79, 1},     {"LD A,D", 0x7A, 1},     {"LD A,E", 0x7B, 1},
          {"LD A,H", 0x7C, 1},      {"LD A,L", 0x7D, 1},     {"LD A,[HL]", 0x7E, 1},  {"LD A,A", 0x7F, 1},
          {"ADD A,B", 0x80, 1},     {"ADD A,C", 0x81, 1},    {"ADD A,D", 0x82, 1},    {"ADD A,E", 0x83, 1},
          {"ADD A,H", 0x84, 1},     {"ADD A,L", 0x85, 1},    {"ADD A,[HL]", 0x86, 1}, {"ADD A,A", 0x87, 1},
          {"ADC A,B", 0x88, 1},     {"ADC A,C", 0x89, 1},    {"ADC A,D", 0x8A, 1},    {"ADC A,E", 0x8B, 1},
          {"ADC A,H", 0x8C, 1},     {"ADC A,L", 0x8D, 1},    {"ADC A,[HL]", 0x8E, 1}, {"ADC A,A", 0x8F, 1},
          {"SUB A,B", 0x90, 1},     {"SUB A,C", 0x91, 1},    {"SUB A,D", 0x92, 1},    {"SUB A,E", 0x93, 1},
          {"SUB A,H", 0x94, 1},     {"SUB A,L", 0x95, 1},    {"SUB A,[HL]", 0x96, 1}, {"SUB A,A", 0x97, 1},
          {"SBC A,B", 0x98, 1},     {"SBC A,C", 0x99, 1},    {"SBC A,D", 0x9A, 1},    {"SBC A,E", 0x9B, 1},
          {"SBC A,H", 0x9C, 1},     {"SBC A,L", 0x9D, 1},    {"SBC A,[HL]", 0x9E, 1}, {"SBC A,A", 0x9F, 1},
          {"AND A,B", 0xA0, 1},     {"AND A,C", 0xA1, 1},    {"AND A,D", 0xA2, 1},    {"AND A,E", 0xA3, 1},
          {"AND A,H", 0xA4, 1},     {"AND A,L", 0xA5, 1},    {"AND A,[HL]", 0xA6, 1}, {"AND A,A", 0xA7, 1},
          {"XOR A,B", 0xA8, 1},     {"XOR A,C", 0xA9, 1},    {"XOR A,D", 0xAA, 1},    {"XOR A,E", 0xAB, 1},
          {"XOR A,H", 0xAC, 1},     {"XOR A,L", 0xAD, 1},    {"XOR A,[HL]", 0xAE, 1}, {"XOR A,A", 0xAF, 1},
          {"OR A,B", 0xB0, 1},      {"OR A,C", 0xB1, 1},     {"OR A,D", 0xB2, 1},     {"OR A,E", 0xB3, 1},
          {"OR A,H", 0xB4, 1},      {"OR A,L", 0xB5, 1},     {"OR A,[HL]", 0xB6, 1},  {"OR A,A", 0xB7, 1},
          {"CP A,B", 0xB8, 1},      {"CP A,C", 0xB9, 1},     {"CP A,D", 0xBA, 1},     {"CP A,E", 0xBB, 1},
          {"CP A,H", 0xBC, 1},      {"CP A,L", 0xBD, 1},     {"CP A,[HL]", 0xBE, 1},  {"CP A,A", 0xBF, 1},
          {"RET NZ", 0xC0, 1},      {"POP BC", 0xC1, 1},     {"JP NZ,a16", 0xC2, 3},  {"JP a16", 0xC3, 3},
          {"CALL NZ,a16", 0xC4, 3}, {"PUSH BC", 0xC5, 1},    {"ADD A,n8", 0xC6, 2},   {"RST $00", 0xC7, 1},
          {"RET Z", 0xC8, 1},       {"RET", 0xC9, 1},        {"JP Z,a16", 0xCA, 3},   {"PREFIX", 0xCB, 1},
          {"CALL Z,a16", 0xCC, 3},  {"CALL a16", 0xCD, 3},   {"ADC A,n8", 0xCE, 2},   {"RST $08", 0xCF, 1},
          {"RET NC", 0xD0, 1},      {"POP DE", 0xD1, 1},     {"JP NC,a16", 0xD2, 3},  {"ILLEGAL_D3", 0xD3, 1},
          {"CALL NC,a16", 0xD4, 3}, {"PUSH DE", 0xD5, 1},    {"SUB A,n8", 0xD6, 2},   {"RST $10", 0xD7, 1},
          {"RET C", 0xD8, 1},       {"RETI", 0xD9, 1},       {"JP C,a16", 0xDA, 3},   {"ILLEGAL_DB", 0xDB, 1},
          {"CALL C,a16", 0xDC, 3},  {"ILLEGAL_DD", 0xDD, 1}, {"SBC A,n8", 0xDE, 2},   {"RST $18", 0xDF, 1},
          {"LDH [a8],A", 0xE0, 2},  {"POP HL", 0xE1, 1},     {"LD [C],A", 0xE2, 1},   {"ILLEGAL_E3", 0xE3, 1},
          {"ILLEGAL_E4", 0xE4, 1},  {"PUSH HL", 0xE5, 1},    {"AND A,n8", 0xE6, 2},   {"RST $20", 0xE7, 1},
          {"ADD SP,e8", 0xE8, 2},   {"JP HL", 0xE9, 1},      {"LD [a16],A", 0xEA, 3}, {"ILLEGAL_EB", 0xEB, 1},
          {"ILLEGAL_EC", 0xEC, 1},  {"ILLEGAL_ED", 0xED, 1}, {"XOR A,n8", 0xEE, 2},   {"RST $28", 0xEF, 1},
          {"LDH A,[a8]", 0xF0, 2},  {"POP AF", 0xF1, 1},     {"LD A,[C]", 0xF2, 1},   {"DI", 0xF3, 1},
          {"ILLEGAL_F4", 0xF4, 1},  {"PUSH AF", 0xF5, 1},    {"OR A,n8", 0xF6, 2},    {"RST $30", 0xF7, 1},
          {"LD HL,SP,e8", 0xF8, 2}, {"LD SP,HL", 0xF9, 1},   {"LD A,[a16]", 0xFA, 3}, {"EI", 0xFB, 1},
          {"ILLEGAL_FC", 0xFC, 1},  {"ILLEGAL_FD", 0xFD, 1}, {"CP A,n8", 0xFE, 2},    {"RST $38", 0xFF, 1},
  };

  static constexpr Instruction instruction_prefixed_[] = {
          {"RLC B", 0x00, 2},   {"RLC C", 0x01, 2},   {"RLC D", 0x02, 2},      {"RLC E", 0x03, 2},
          {"RLC H", 0x04, 2},   {"RLC L", 0x05, 2},   {"RLC [HL]", 0x06, 2},   {"RLC A", 0x07, 2},
          {"RRC B", 0x08, 2},   {"RRC C", 0x09, 2},   {"RRC D", 0x0A, 2},      {"RRC E", 0x0B, 2},
          {"RRC H", 0x0C, 2},   {"RRC L", 0x0D, 2},   {"RRC [HL]", 0x0E, 2},   {"RRC A", 0x0F, 2},
          {"RL B", 0x10, 2},    {"RL C", 0x11, 2},    {"RL D", 0x12, 2},       {"RL E", 0x13, 2},
          {"RL H", 0x14, 2},    {"RL L", 0x15, 2},    {"RL [HL]", 0x16, 2},    {"RL A", 0x17, 2},
          {"RR B", 0x18, 2},    {"RR C", 0x19, 2},    {"RR D", 0x1A, 2},       {"RR E", 0x1B, 2},
          {"RR H", 0x1C, 2},    {"RR L", 0x1D, 2},    {"RR [HL]", 0x1E, 2},    {"RR A", 0x1F, 2},
          {"SLA B", 0x20, 2},   {"SLA C", 0x21, 2},   {"SLA D", 0x22, 2},      {"SLA E", 0x23, 2},
          {"SLA H", 0x24, 2},   {"SLA L", 0x25, 2},   {"SLA [HL]", 0x26, 2},   {"SLA A", 0x27, 2},
          {"SRA B", 0x28, 2},   {"SRA C", 0x29, 2},   {"SRA D", 0x2A, 2},      {"SRA E", 0x2B, 2},
          {"SRA H", 0x2C, 2},   {"SRA L", 0x2D, 2},   {"SRA [HL]", 0x2E, 2},   {"SRA A", 0x2F, 2},
          {"SWAP B", 0x30, 2},  {"SWAP C", 0x31, 2},  {"SWAP D", 0x32, 2},     {"SWAP E", 0x33, 2},
          {"SWAP H", 0x34, 2},  {"SWAP L", 0x35, 2},  {"SWAP [HL]", 0x36, 2},  {"SWAP A", 0x37, 2},
          {"SRL B", 0x38, 2},   {"SRL C", 0x39, 2},   {"SRL D", 0x3A, 2},      {"SRL E", 0x3B, 2},
          {"SRL H", 0x3C, 2},   {"SRL L", 0x3D, 2},   {"SRL [HL]", 0x3E, 2},   {"SRL A", 0x3F, 2},
          {"BIT 0,B", 0x40, 2}, {"BIT 0,C", 0x41, 2}, {"BIT 0,D", 0x42, 2},    {"BIT 0,E", 0x43, 2},
          {"BIT 0,H", 0x44, 2}, {"BIT 0,L", 0x45, 2}, {"BIT 0,[HL]", 0x46, 2}, {"BIT 0,A", 0x47, 2},
          {"BIT 1,B", 0x48, 2}, {"BIT 1,C", 0x49, 2}, {"BIT 1,D", 0x4A, 2},    {"BIT 1,E", 0x4B, 2},
          {"BIT 1,H", 0x4C, 2}, {"BIT 1,L", 0x4D, 2}, {"BIT 1,[HL]", 0x4E, 2}, {"BIT 1,A", 0x4F, 2},
          {"BIT 2,B", 0x50, 2}, {"BIT 2,C", 0x51, 2}, {"BIT 2,D", 0x52, 2},    {"BIT 2,E", 0x53, 2},
          {"BIT 2,H", 0x54, 2}, {"BIT 2,L", 0x55, 2}, {"BIT 2,[HL]", 0x56, 2}, {"BIT 2,A", 0x57, 2},
          {"BIT 3,B", 0x58, 2}, {"BIT 3,C", 0x59, 2}, {"BIT 3,D", 0x5A, 2},    {"BIT 3,E", 0x5B, 2},
          {"BIT 3,H", 0x5C, 2}, {"BIT 3,L", 0x5D, 2}, {"BIT 3,[HL]", 0x5E, 2}, {"BIT 3,A", 0x5F, 2},
          {"BIT 4,B", 0x60, 2}, {"BIT 4,C", 0x61, 2}, {"BIT 4,D", 0x62, 2},    {"BIT 4,E", 0x63, 2},
          {"BIT 4,H", 0x64, 2}, {"BIT 4,L", 0x65, 2}, {"BIT 4,[HL]", 0x66, 2}, {"BIT 4,A", 0x67, 2},
          {"BIT 5,B", 0x68, 2}, {"BIT 5,C", 0x69, 2}, {"BIT 5,D", 0x6A, 2},    {"BIT 5,E", 0x6B, 2},
          {"BIT 5,H", 0x6C, 2}, {"BIT 5,L", 0x6D, 2}, {"BIT 5,[HL]", 0x6E, 2}, {"BIT 5,A", 0x6F, 2},
          {"BIT 6,B", 0x70, 2}, {"BIT 6,C", 0x71, 2}, {"BIT 6,D", 0x72, 2},    {"BIT 6,E", 0x73, 2},
          {"BIT 6,H", 0x74, 2}, {"BIT 6,L", 0x75, 2}, {"BIT 6,[HL]", 0x76, 2}, {"BIT 6,A", 0x77, 2},
          {"BIT 7,B", 0x78, 2}, {"BIT 7,C", 0x79, 2}, {"BIT 7,D", 0x7A, 2},    {"BIT 7,E", 0x7B, 2},
          {"BIT 7,H", 0x7C, 2}, {"BIT 7,L", 0x7D, 2}, {"BIT 7,[HL]", 0x7E, 2}, {"BIT 7,A", 0x7F, 2},
          {"RES 0,B", 0x80, 2}, {"RES 0,C", 0x81, 2}, {"RES 0,D", 0x82, 2},    {"RES 0,E", 0x83, 2},
          {"RES 0,H", 0x84, 2}, {"RES 0,L", 0x85, 2}, {"RES 0,[HL]", 0x86, 2}, {"RES 0,A", 0x87, 2},
          {"RES 1,B", 0x88, 2}, {"RES 1,C", 0x89, 2}, {"RES 1,D", 0x8A, 2},    {"RES 1,E", 0x8B, 2},
          {"RES 1,H", 0x8C, 2}, {"RES 1,L", 0x8D, 2}, {"RES 1,[HL]", 0x8E, 2}, {"RES 1,A", 0x8F, 2},
          {"RES 2,B", 0x90, 2}, {"RES 2,C", 0x91, 2}, {"RES 2,D", 0x92, 2},    {"RES 2,E", 0x93, 2},
          {"RES 2,H", 0x94, 2}, {"RES 2,L", 0x95, 2}, {"RES 2,[HL]", 0x96, 2}, {"RES 2,A", 0x97, 2},
          {"RES 3,B", 0x98, 2}, {"RES 3,C", 0x99, 2}, {"RES 3,D", 0x9A, 2},    {"RES 3,E", 0x9B, 2},
          {"RES 3,H", 0x9C, 2}, {"RES 3,L", 0x9D, 2}, {"RES 3,[HL]", 0x9E, 2}, {"RES 3,A", 0x9F, 2},
          {"RES 4,B", 0xA0, 2}, {"RES 4,C", 0xA1, 2}, {"RES 4,D", 0xA2, 2},    {"RES 4,E", 0xA3, 2},
          {"RES 4,H", 0xA4, 2}, {"RES 4,L", 0xA5, 2}, {"RES 4,[HL]", 0xA6, 2}, {"RES 4,A", 0xA7, 2},
          {"RES 5,B", 0xA8, 2}, {"RES 5,C", 0xA9, 2}, {"RES 5,D", 0xAA, 2},    {"RES 5,E", 0xAB, 2},
          {"RES 5,H", 0xAC, 2}, {"RES 5,L", 0xAD, 2}, {"RES 5,[HL]", 0xAE, 2}, {"RES 5,A", 0xAF, 2},
          {"RES 6,B", 0xB0, 2}, {"RES 6,C", 0xB1, 2}, {"RES 6,D", 0xB2, 2},    {"RES 6,E", 0xB3, 2},
          {"RES 6,H", 0xB4, 2}, {"RES 6,L", 0xB5, 2}, {"RES 6,[HL]", 0xB6, 2}, {"RES 6,A", 0xB7, 2},
          {"RES 7,B", 0xB8, 2}, {"RES 7,C", 0xB9, 2}, {"RES 7,D", 0xBA, 2},    {"RES 7,E", 0xBB, 2},
          {"RES 7,H", 0xBC, 2}, {"RES 7,L", 0xBD, 2}, {"RES 7,[HL]", 0xBE, 2}, {"RES 7,A", 0xBF, 2},
          {"SET 0,B", 0xC0, 2}, {"SET 0,C", 0xC1, 2}, {"SET 0,D", 0xC2, 2},    {"SET 0,E", 0xC3, 2},
          {"SET 0,H", 0xC4, 2}, {"SET 0,L", 0xC5, 2}, {"SET 0,[HL]", 0xC6, 2}, {"SET 0,A", 0xC7, 2},
          {"SET 1,B", 0xC8, 2}, {"SET 1,C", 0xC9, 2}, {"SET 1,D", 0xCA, 2},    {"SET 1,E", 0xCB, 2},
          {"SET 1,H", 0xCC, 2}, {"SET 1,L", 0xCD, 2}, {"SET 1,[HL]", 0xCE, 2}, {"SET 1,A", 0xCF, 2},
          {"SET 2,B", 0xD0, 2}, {"SET 2,C", 0xD1, 2}, {"SET 2,D", 0xD2, 2},    {"SET 2,E", 0xD3, 2},
          {"SET 2,H", 0xD4, 2}, {"SET 2,L", 0xD5, 2}, {"SET 2,[HL]", 0xD6, 2}, {"SET 2,A", 0xD7, 2},
          {"SET 3,B", 0xD8, 2}, {"SET 3,C", 0xD9, 2}, {"SET 3,D", 0xDA, 2},    {"SET 3,E", 0xDB, 2},
          {"SET 3,H", 0xDC, 2}, {"SET 3,L", 0xDD, 2}, {"SET 3,[HL]", 0xDE, 2}, {"SET 3,A", 0xDF, 2},
          {"SET 4,B", 0xE0, 2}, {"SET 4,C", 0xE1, 2}, {"SET 4,D", 0xE2, 2},    {"SET 4,E", 0xE3, 2},
          {"SET 4,H", 0xE4, 2}, {"SET 4,L", 0xE5, 2}, {"SET 4,[HL]", 0xE6, 2}, {"SET 4,A", 0xE7, 2},
          {"SET 5,B", 0xE8, 2}, {"SET 5,C", 0xE9, 2}, {"SET 5,D", 0xEA, 2},    {"SET 5,E", 0xEB, 2},
          {"SET 5,H", 0xEC, 2}, {"SET 5,L", 0xED, 2}, {"SET 5,[HL]", 0xEE, 2}, {"SET 5,A", 0xEF, 2},
          {"SET 6,B", 0xF0, 2}, {"SET 6,C", 0xF1, 2}, {"SET 6,D", 0xF2, 2},    {"SET 6,E", 0xF3, 2},
          {"SET 6,H", 0xF4, 2}, {"SET 6,L", 0xF5, 2}, {"SET 6,[HL]", 0xF6, 2}, {"SET 6,A", 0xF7, 2},
          {"SET 7,B", 0xF8, 2}, {"SET 7,C", 0xF9, 2}, {"SET 7,D", 0xFA, 2},    {"SET 7,E", 0xFB, 2},
          {"SET 7,H", 0xFC, 2}, {"SET 7,L", 0xFD, 2}, {"SET 7,[HL]", 0xFE, 2}, {"SET 7,A", 0xFF, 2},
  };
};

} // namespace gb
