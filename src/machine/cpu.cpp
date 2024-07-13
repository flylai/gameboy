#include "cpu.h"

#include <memory>
#include <string>

#include "common/type.h"

namespace gb {

// https://gbdev.io/gb-opcodes//optables/
// https://rgbds.gbdev.io/docs/v0.7.0/gbz80.7

struct Instruction {
  static constexpr u16 OFFSET = 0xff00;

  constexpr Instruction(const std::string &name, u8 op, u8 bytes) //
      : name(name),                                               //
        op(op),                                                   //
        bytes(bytes) {}

  const std::string name;
  const u8 op{};
  const u8 bytes{};
};

#define DEF_INST(NAME, OP, BYTES, CYCLE)                 \
  struct _##OP : Instruction {                           \
    constexpr _##OP() : Instruction(#NAME, OP, BYTES) {} \
    static u8 update(CPU *cpu) {                         \
      u8 cycle = (CYCLE);

#define DEF_INST_END \
  return cycle;      \
  }                  \
  }                  \
  ;


#define A(...) cpu->A(__VA_ARGS__)
#define F(...) cpu->F(__VA_ARGS__)
#define AF(...) cpu->AF(__VA_ARGS__)
#define B(...) cpu->B(__VA_ARGS__)
#define C(...) cpu->C(__VA_ARGS__)
#define BC(...) cpu->BC(__VA_ARGS__)
#define D(...) cpu->D(__VA_ARGS__)
#define E(...) cpu->E(__VA_ARGS__)
#define DE(...) cpu->DE(__VA_ARGS__)
#define H(...) cpu->H(__VA_ARGS__)
#define L(...) cpu->L(__VA_ARGS__)
#define HL(...) cpu->HL(__VA_ARGS__)
#define zf(...) cpu->zf(__VA_ARGS__)
#define nf(...) cpu->nf(__VA_ARGS__)
#define hf(...) cpu->hf(__VA_ARGS__)
#define cf(...) cpu->cf(__VA_ARGS__)
#define PC(...) cpu->PC(__VA_ARGS__)
#define SP(...) cpu->SP(__VA_ARGS__)
#define halt(...) cpu->halt(__VA_ARGS__)
#define imm8(...) cpu->imm8(__VA_ARGS__)
#define imm16(...) cpu->imm16(__VA_ARGS__)
#define set(...) cpu->set(__VA_ARGS__)
#define set16(...) cpu->set16(__VA_ARGS__)
#define get(...) cpu->get(__VA_ARGS__)
#define inc8(...) cpu->inc8(__VA_ARGS__)
#define dec8(...) cpu->dec8(__VA_ARGS__)
#define rlc8(...) cpu->rlc8(__VA_ARGS__)
#define addHL(...) cpu->addHL(__VA_ARGS__)
#define add8(...) cpu->add8(__VA_ARGS__)
#define add16(...) cpu->add16(__VA_ARGS__)
#define adc8(...) cpu->adc8(__VA_ARGS__)
#define sub8(...) cpu->sub8(__VA_ARGS__)
#define sub16(...) cpu->sub16(__VA_ARGS__)
#define sbc8(...) cpu->sbc8(__VA_ARGS__)
#define and8(...) cpu->and8(__VA_ARGS__)
#define xor8(...) cpu->xor8(__VA_ARGS__)
#define or8(...) cpu->or8(__VA_ARGS__)
#define cp8(...) cpu->cp8(__VA_ARGS__)
#define jr(...) cpu->jr(__VA_ARGS__)
#define push16(...) cpu->push16(__VA_ARGS__)
#define pop16(...) cpu->pop16(__VA_ARGS__)
#define IME(...) cpu->IME(__VA_ARGS__)
#define rrc8(...) cpu->rrc8(__VA_ARGS__)
#define rl8(...) cpu->rl8(__VA_ARGS__)
#define rr8(...) cpu->rr8(__VA_ARGS__)
#define sla8(...) cpu->sla8(__VA_ARGS__)
#define sra8(...) cpu->sra8(__VA_ARGS__)
#define swap8(...) cpu->swap8(__VA_ARGS__)
#define srl8(...) cpu->srl8(__VA_ARGS__)
#define bit8(...) cpu->bit8(__VA_ARGS__)
#define res8(...) cpu->res8(__VA_ARGS__)
#define set8(...) cpu->set8(__VA_ARGS__)

/*
let unprefixed = json["unprefixed"]
for (let op in unprefixed) {
  let obj = unprefixed[op];
  let op_name = obj["mnemonic"];
  let str = op_name;
  for (let operand in obj['operands']) {
    str += "_"
    if (!obj['operands'][operand]['immediate']) {
      str += "x" + obj['operands'][operand]['name'] + "x";
    } else {
      str += obj['operands'][operand]['name'];
    }
  }
  let flags = obj["flags"];
  str += "_" + (flags['Z'] == '-' ? "x" : flags['Z']);
  str += "_" + (flags['N'] == '-' ? "x" : flags['N']);
  str += "_" + (flags['H'] == '-' ? "x" : flags['H']);
  str += "_" + (flags['C'] == '-' ? "x" : flags['C']);
  str += "," + op + "," + obj["bytes"] + "," + obj["cycles"].at(-1);
  console.log(str)
}
*/

DEF_INST(NOP_x_x_x_x, 0x00, 1, 4)
DEF_INST_END

DEF_INST(LD_BC_n16_x_x_x_x, 0x01, 3, 12)
BC(imm16());
DEF_INST_END

DEF_INST(LD_xBCx_A_x_x_x_x, 0x02, 1, 8)
set(BC(), A());
DEF_INST_END

DEF_INST(INC_BC_x_x_x_x, 0x03, 1, 8)
BC(BC() + 1);
DEF_INST_END

DEF_INST(INC_B_Z_0_H_x, 0x04, 1, 4)
B(inc8(B()));
DEF_INST_END

DEF_INST(DEC_B_Z_1_H_x, 0x05, 1, 4)
B(dec8(B()));
DEF_INST_END

DEF_INST(LD_B_n8_x_x_x_x, 0x06, 2, 8)
B(imm8());
DEF_INST_END

DEF_INST(RLCA_0_0_0_C, 0x07, 1, 4)
A(rlc8(A()));
zf(0);
DEF_INST_END

DEF_INST(LD_xa16x_SP_x_x_x_x, 0x08, 3, 20)
set16(imm16(), SP());
DEF_INST_END

DEF_INST(ADD_HL_BC_x_0_H_C, 0x09, 1, 8)
HL(addHL(BC()));
DEF_INST_END

DEF_INST(LD_A_xBCx_x_x_x_x, 0x0A, 1, 8)
A(get(BC()));
DEF_INST_END

DEF_INST(DEC_BC_x_x_x_x, 0x0B, 1, 8)
BC(BC() - 1);
DEF_INST_END

DEF_INST(INC_C_Z_0_H_x, 0x0C, 1, 4)
C(inc8(C()));
DEF_INST_END

DEF_INST(DEC_C_Z_1_H_x, 0x0D, 1, 4)
C(dec8(C()));
DEF_INST_END

DEF_INST(LD_C_n8_x_x_x_x, 0x0E, 2, 8)
C(imm8());
DEF_INST_END

DEF_INST(RRCA_0_0_0_C, 0x0F, 1, 4)
zf(0);
nf(0);
hf(0);
cf(A() & 0x1);
A(A() >> 1 | ((A() & 0x7f) << 7));
DEF_INST_END

DEF_INST(STOP_n8_x_x_x_x, 0x10, 2, 4)
// nothing to do.
DEF_INST_END

DEF_INST(LD_DE_n16_x_x_x_x, 0x11, 3, 12)
DE(imm16());
DEF_INST_END

DEF_INST(LD_xDEx_A_x_x_x_x, 0x12, 1, 8)
set(DE(), A());
DEF_INST_END

DEF_INST(INC_DE_x_x_x_x, 0x13, 1, 8)
DE(DE() + 1);
DEF_INST_END

DEF_INST(INC_D_Z_0_H_x, 0x14, 1, 4)
D(inc8(D()));
DEF_INST_END

DEF_INST(DEC_D_Z_1_H_x, 0x15, 1, 4)
D(dec8(D()));
DEF_INST_END

DEF_INST(LD_D_n8_x_x_x_x, 0x16, 2, 8)
D(imm8());
DEF_INST_END

DEF_INST(RLA_0_0_0_C, 0x17, 1, 4)
zf(0);
nf(0);
hf(0);
u8 c = cf();
cf(A() >> 7);
A(A() << 1 | c);
DEF_INST_END

DEF_INST(JR_e8_x_x_x_x, 0x18, 2, 12)
jr(imm8());
DEF_INST_END

DEF_INST(ADD_HL_DE_x_0_H_C, 0x19, 1, 8)
HL(addHL(DE()));
DEF_INST_END

DEF_INST(LD_A_xDEx_x_x_x_x, 0x1A, 1, 8)
A(get(DE()));
DEF_INST_END

DEF_INST(DEC_DE_x_x_x_x, 0x1B, 1, 8)
DE(DE() - 1);
DEF_INST_END

DEF_INST(INC_E_Z_0_H_x, 0x1C, 1, 4)
E(inc8(E()));
DEF_INST_END

DEF_INST(DEC_E_Z_1_H_x, 0x1D, 1, 4)
E(dec8(E()));
DEF_INST_END

DEF_INST(LD_E_n8_x_x_x_x, 0x1E, 2, 8)
E(imm8());
DEF_INST_END

DEF_INST(RRA_0_0_0_C, 0x1F, 1, 4)
A(rr8(A()));
zf(0);
DEF_INST_END

DEF_INST(JR_NZ_e8_x_x_x_x, 0x20, 2, 8)
u8 target = imm8();
if (!zf()) {
  jr(target);
  cycle = 12;
}
DEF_INST_END

DEF_INST(LD_HL_n16_x_x_x_x, 0x21, 3, 12)
HL(imm16());
DEF_INST_END

DEF_INST(LD_xHLx_A_x_x_x_x, 0x22, 1, 8)
set(HL(), A());
HL(HL() + 1);
DEF_INST_END

DEF_INST(INC_HL_x_x_x_x, 0x23, 1, 8)
HL(HL() + 1);
DEF_INST_END

DEF_INST(INC_H_Z_0_H_x, 0x24, 1, 4)
H(inc8(H()));
DEF_INST_END

DEF_INST(DEC_H_Z_1_H_x, 0x25, 1, 4)
H(dec8(H()));
DEF_INST_END

DEF_INST(LD_H_n8_x_x_x_x, 0x26, 2, 8)
H(imm8());
DEF_INST_END

DEF_INST(DAA_Z_x_0_C, 0x27, 1, 4)
u8 a = A();
u8 v = cf() ? 0x60 : 0;
if (hf()) {
  v |= 0x06;
}
if (!nf()) {
  if ((a & 0x0f) > 0x09) {
    v |= 0x06;
  }
  if (a > 0x99) {
    v |= 0x60;
  }
  a += v;
} else {
  a -= v;
}
cf(v >= 0x60);
hf(0);
zf(a == 0x0);
A(a);
DEF_INST_END

DEF_INST(JR_Z_e8_x_x_x_x, 0x28, 2, 8)
u8 target = imm8();
if (zf()) {
  jr(target);
  cycle = 12;
}
DEF_INST_END

DEF_INST(ADD_HL_HL_x_0_H_C, 0x29, 1, 8)
HL(addHL(HL()));
DEF_INST_END

DEF_INST(LD_A_xHLx_x_x_x_x, 0x2A, 1, 8)
A(get(HL()));
HL(HL() + 1);
DEF_INST_END

DEF_INST(DEC_HL_x_x_x_x, 0x2B, 1, 8)
HL(HL() - 1);
DEF_INST_END

DEF_INST(INC_L_Z_0_H_x, 0x2C, 1, 4)
L(inc8(L()));
DEF_INST_END

DEF_INST(DEC_L_Z_1_H_x, 0x2D, 1, 4)
L(dec8(L()));
DEF_INST_END

DEF_INST(LD_L_n8_x_x_x_x, 0x2E, 2, 8)
L(imm8());
DEF_INST_END

DEF_INST(CPL_x_1_1_x, 0x2F, 1, 4)
nf(1);
hf(1);
A(~A());
DEF_INST_END

DEF_INST(JR_NC_e8_x_x_x_x, 0x30, 2, 8)
u8 target = imm8();
if (!cf()) {
  jr(target);
  cycle = 12;
}
DEF_INST_END

DEF_INST(LD_SP_n16_x_x_x_x, 0x31, 3, 12)
SP(imm16());
DEF_INST_END

DEF_INST(LD_xHLx_A_x_x_x_x, 0x32, 1, 8)
set(HL(), A());
HL(HL() - 1);
DEF_INST_END

DEF_INST(INC_SP_x_x_x_x, 0x33, 1, 8)
SP(SP() + 1);
DEF_INST_END

DEF_INST(INC_xHLx_Z_0_H_x, 0x34, 1, 12)
set(HL(), inc8(get(HL())));
DEF_INST_END

DEF_INST(DEC_xHLx_Z_1_H_x, 0x35, 1, 12)
set(HL(), dec8(get(HL())));
DEF_INST_END

DEF_INST(LD_xHLx_n8_x_x_x_x, 0x36, 2, 12)
set(HL(), imm8());
DEF_INST_END

DEF_INST(SCF_x_0_0_1, 0x37, 1, 4)
nf(0);
hf(0);
cf(1);
DEF_INST_END

DEF_INST(JR_C_e8_x_x_x_x, 0x38, 2, 8)
u8 target = imm8();
if (cf()) {
  jr(target);
  cycle = 12;
}
DEF_INST_END

DEF_INST(ADD_HL_SP_x_0_H_C, 0x39, 1, 8)
HL(addHL(SP()));
DEF_INST_END

DEF_INST(LD_A_xHLx_x_x_x_x, 0x3A, 1, 8)
A(get(HL()));
HL(HL() - 1);
DEF_INST_END

DEF_INST(DEC_SP_x_x_x_x, 0x3B, 1, 8)
SP(SP() - 1);
DEF_INST_END

DEF_INST(INC_A_Z_0_H_x, 0x3C, 1, 4)
A(inc8(A()));
DEF_INST_END

DEF_INST(DEC_A_Z_1_H_x, 0x3D, 1, 4)
A(dec8(A()));
DEF_INST_END

DEF_INST(LD_A_n8_x_x_x_x, 0x3E, 2, 8)
A(imm8());
DEF_INST_END

DEF_INST(CCF_x_0_0_C, 0x3F, 1, 4)
nf(0);
hf(0);
cf(!cf());
DEF_INST_END

DEF_INST(LD_B_B_x_x_x_x, 0x40, 1, 4)
// LD B B
// nothing to do.
DEF_INST_END

DEF_INST(LD_B_C_x_x_x_x, 0x41, 1, 4)
B(C());
DEF_INST_END

DEF_INST(LD_B_D_x_x_x_x, 0x42, 1, 4)
B(D());
DEF_INST_END

DEF_INST(LD_B_E_x_x_x_x, 0x43, 1, 4)
B(E());
DEF_INST_END

DEF_INST(LD_B_H_x_x_x_x, 0x44, 1, 4)
B(H());
DEF_INST_END

DEF_INST(LD_B_L_x_x_x_x, 0x45, 1, 4)
B(L());
DEF_INST_END

DEF_INST(LD_B_xHLx_x_x_x_x, 0x46, 1, 8)
B(get(HL()));
DEF_INST_END

DEF_INST(LD_B_A_x_x_x_x, 0x47, 1, 4)
B(A());
DEF_INST_END

DEF_INST(LD_C_B_x_x_x_x, 0x48, 1, 4)
C(B());
DEF_INST_END

DEF_INST(LD_C_C_x_x_x_x, 0x49, 1, 4)
// LD C C
// nothing to do
DEF_INST_END

DEF_INST(LD_C_D_x_x_x_x, 0x4A, 1, 4)
C(D());
DEF_INST_END

DEF_INST(LD_C_E_x_x_x_x, 0x4B, 1, 4)
C(E());
DEF_INST_END

DEF_INST(LD_C_H_x_x_x_x, 0x4C, 1, 4)
C(H());
DEF_INST_END

DEF_INST(LD_C_L_x_x_x_x, 0x4D, 1, 4)
C(L());
DEF_INST_END

DEF_INST(LD_C_xHLx_x_x_x_x, 0x4E, 1, 8)
C(get(HL()));
DEF_INST_END

DEF_INST(LD_C_A_x_x_x_x, 0x4F, 1, 4)
C(A());
DEF_INST_END

DEF_INST(LD_D_B_x_x_x_x, 0x50, 1, 4)
D(B());
DEF_INST_END

DEF_INST(LD_D_C_x_x_x_x, 0x51, 1, 4)
D(C());
DEF_INST_END

DEF_INST(LD_D_D_x_x_x_x, 0x52, 1, 4)
D(D());
DEF_INST_END

DEF_INST(LD_D_E_x_x_x_x, 0x53, 1, 4)
D(E());
DEF_INST_END

DEF_INST(LD_D_H_x_x_x_x, 0x54, 1, 4)
D(H());
DEF_INST_END

DEF_INST(LD_D_L_x_x_x_x, 0x55, 1, 4)
D(L());
DEF_INST_END

DEF_INST(LD_D_xHLx_x_x_x_x, 0x56, 1, 8)
D(get(HL()));
DEF_INST_END

DEF_INST(LD_D_A_x_x_x_x, 0x57, 1, 4)
D(A());
DEF_INST_END

DEF_INST(LD_E_B_x_x_x_x, 0x58, 1, 4)
E(B());
DEF_INST_END

DEF_INST(LD_E_C_x_x_x_x, 0x59, 1, 4)
E(C());
DEF_INST_END

DEF_INST(LD_E_D_x_x_x_x, 0x5A, 1, 4)
E(D());
DEF_INST_END

DEF_INST(LD_E_E_x_x_x_x, 0x5B, 1, 4)
// LD E E
// nothing to do
DEF_INST_END

DEF_INST(LD_E_H_x_x_x_x, 0x5C, 1, 4)
E(H());
DEF_INST_END

DEF_INST(LD_E_L_x_x_x_x, 0x5D, 1, 4)
E(L());
DEF_INST_END

DEF_INST(LD_E_xHLx_x_x_x_x, 0x5E, 1, 8)
E(get(HL()));
DEF_INST_END

DEF_INST(LD_E_A_x_x_x_x, 0x5F, 1, 4)
E(A());
DEF_INST_END

DEF_INST(LD_H_B_x_x_x_x, 0x60, 1, 4)
H(B());
DEF_INST_END

DEF_INST(LD_H_C_x_x_x_x, 0x61, 1, 4)
H(C());
DEF_INST_END

DEF_INST(LD_H_D_x_x_x_x, 0x62, 1, 4)
H(D());
DEF_INST_END

DEF_INST(LD_H_E_x_x_x_x, 0x63, 1, 4)
H(E());
DEF_INST_END

DEF_INST(LD_H_H_x_x_x_x, 0x64, 1, 4)
// LD H H
// nothing to do
DEF_INST_END

DEF_INST(LD_H_L_x_x_x_x, 0x65, 1, 4)
H(L());
DEF_INST_END

DEF_INST(LD_H_xHLx_x_x_x_x, 0x66, 1, 8)
H(get(HL()));
DEF_INST_END

DEF_INST(LD_H_A_x_x_x_x, 0x67, 1, 4)
H(A());
DEF_INST_END

DEF_INST(LD_L_B_x_x_x_x, 0x68, 1, 4)
L(B());
DEF_INST_END

DEF_INST(LD_L_C_x_x_x_x, 0x69, 1, 4)
L(C());
DEF_INST_END

DEF_INST(LD_L_D_x_x_x_x, 0x6A, 1, 4)
L(D());
DEF_INST_END

DEF_INST(LD_L_E_x_x_x_x, 0x6B, 1, 4)
L(E());
DEF_INST_END

DEF_INST(LD_L_H_x_x_x_x, 0x6C, 1, 4)
L(H());
DEF_INST_END

DEF_INST(LD_L_L_x_x_x_x, 0x6D, 1, 4)
// LD L L
DEF_INST_END

DEF_INST(LD_L_xHLx_x_x_x_x, 0x6E, 1, 8)
L(get(HL()));
DEF_INST_END

DEF_INST(LD_L_A_x_x_x_x, 0x6F, 1, 4)
L(A());
DEF_INST_END

DEF_INST(LD_xHLx_B_x_x_x_x, 0x70, 1, 8)
set(HL(), B());
DEF_INST_END

DEF_INST(LD_xHLx_C_x_x_x_x, 0x71, 1, 8)
set(HL(), C());
DEF_INST_END

DEF_INST(LD_xHLx_D_x_x_x_x, 0x72, 1, 8)
set(HL(), D());
DEF_INST_END

DEF_INST(LD_xHLx_E_x_x_x_x, 0x73, 1, 8)
set(HL(), E());
DEF_INST_END

DEF_INST(LD_xHLx_H_x_x_x_x, 0x74, 1, 8)
set(HL(), H());
DEF_INST_END

DEF_INST(LD_xHLx_L_x_x_x_x, 0x75, 1, 8)
set(HL(), L());
DEF_INST_END

DEF_INST(HALT_x_x_x_x, 0x76, 1, 4)
halt(true);
DEF_INST_END

DEF_INST(LD_xHLx_A_x_x_x_x, 0x77, 1, 8)
set(HL(), A());
DEF_INST_END

DEF_INST(LD_A_B_x_x_x_x, 0x78, 1, 4)
A(B());
DEF_INST_END

DEF_INST(LD_A_C_x_x_x_x, 0x79, 1, 4)
A(C());
DEF_INST_END

DEF_INST(LD_A_D_x_x_x_x, 0x7A, 1, 4)
A(D());
DEF_INST_END

DEF_INST(LD_A_E_x_x_x_x, 0x7B, 1, 4)
A(E());
DEF_INST_END

DEF_INST(LD_A_H_x_x_x_x, 0x7C, 1, 4)
A(H());
DEF_INST_END

DEF_INST(LD_A_L_x_x_x_x, 0x7D, 1, 4)
A(L());
DEF_INST_END

DEF_INST(LD_A_xHLx_x_x_x_x, 0x7E, 1, 8)
A(get(HL()));
DEF_INST_END

DEF_INST(LD_A_A_x_x_x_x, 0x7F, 1, 4)
// LD A A
DEF_INST_END

DEF_INST(ADD_A_B_Z_0_H_C, 0x80, 1, 4)
A(add8(A(), B()));
DEF_INST_END

DEF_INST(ADD_A_C_Z_0_H_C, 0x81, 1, 4)
A(add8(A(), C()));
DEF_INST_END

DEF_INST(ADD_A_D_Z_0_H_C, 0x82, 1, 4)
A(add8(A(), D()));
DEF_INST_END

DEF_INST(ADD_A_E_Z_0_H_C, 0x83, 1, 4)
A(add8(A(), E()));
DEF_INST_END

DEF_INST(ADD_A_H_Z_0_H_C, 0x84, 1, 4)
A(add8(A(), H()));
DEF_INST_END

DEF_INST(ADD_A_L_Z_0_H_C, 0x85, 1, 4)
A(add8(A(), L()));
DEF_INST_END

DEF_INST(ADD_A_xHLx_Z_0_H_C, 0x86, 1, 8)
A(add8(A(), get(HL())));
DEF_INST_END

DEF_INST(ADD_A_A_Z_0_H_C, 0x87, 1, 4)
A(add8(A(), A()));
DEF_INST_END

DEF_INST(ADC_A_B_Z_0_H_C, 0x88, 1, 4)
A(adc8(A(), B()));
DEF_INST_END

DEF_INST(ADC_A_C_Z_0_H_C, 0x89, 1, 4)
A(adc8(A(), C()));
DEF_INST_END

DEF_INST(ADC_A_D_Z_0_H_C, 0x8A, 1, 4)
A(adc8(A(), D()));
DEF_INST_END

DEF_INST(ADC_A_E_Z_0_H_C, 0x8B, 1, 4)
A(adc8(A(), E()));
DEF_INST_END

DEF_INST(ADC_A_H_Z_0_H_C, 0x8C, 1, 4)
A(adc8(A(), H()));
DEF_INST_END

DEF_INST(ADC_A_L_Z_0_H_C, 0x8D, 1, 4)
A(adc8(A(), L()));
DEF_INST_END

DEF_INST(ADC_A_xHLx_Z_0_H_C, 0x8E, 1, 8)
A(adc8(A(), get(HL())));
DEF_INST_END

DEF_INST(ADC_A_A_Z_0_H_C, 0x8F, 1, 4)
A(adc8(A(), A()));
DEF_INST_END

DEF_INST(SUB_A_B_Z_1_H_C, 0x90, 1, 4)
A(sub8(A(), B()));
DEF_INST_END

DEF_INST(SUB_A_C_Z_1_H_C, 0x91, 1, 4)
A(sub8(A(), C()));
DEF_INST_END

DEF_INST(SUB_A_D_Z_1_H_C, 0x92, 1, 4)
A(sub8(A(), D()));
DEF_INST_END

DEF_INST(SUB_A_E_Z_1_H_C, 0x93, 1, 4)
A(sub8(A(), E()));
DEF_INST_END

DEF_INST(SUB_A_H_Z_1_H_C, 0x94, 1, 4)
A(sub8(A(), H()));
DEF_INST_END

DEF_INST(SUB_A_L_Z_1_H_C, 0x95, 1, 4)
A(sub8(A(), L()));
DEF_INST_END

DEF_INST(SUB_A_xHLx_Z_1_H_C, 0x96, 1, 8)
A(sub8(A(), get(HL())));
DEF_INST_END

DEF_INST(SUB_A_A_1_1_0_0, 0x97, 1, 4)
A(sub8(A(), A()));
DEF_INST_END

DEF_INST(SBC_A_B_Z_1_H_C, 0x98, 1, 4)
A(sbc8(A(), B()));
DEF_INST_END

DEF_INST(SBC_A_C_Z_1_H_C, 0x99, 1, 4)
A(sbc8(A(), C()));
DEF_INST_END

DEF_INST(SBC_A_D_Z_1_H_C, 0x9A, 1, 4)
A(sbc8(A(), D()));
DEF_INST_END

DEF_INST(SBC_A_E_Z_1_H_C, 0x9B, 1, 4)
A(sbc8(A(), E()));
DEF_INST_END

DEF_INST(SBC_A_H_Z_1_H_C, 0x9C, 1, 4)
A(sbc8(A(), H()));
DEF_INST_END

DEF_INST(SBC_A_L_Z_1_H_C, 0x9D, 1, 4)
A(sbc8(A(), L()));
DEF_INST_END

DEF_INST(SBC_A_xHLx_Z_1_H_C, 0x9E, 1, 8)
A(sbc8(A(), get(HL())));
DEF_INST_END

DEF_INST(SBC_A_A_Z_1_H_x, 0x9F, 1, 4)
A(sbc8(A(), A()));
DEF_INST_END

DEF_INST(AND_A_B_Z_0_1_0, 0xA0, 1, 4)
A(and8(A(), B()));
DEF_INST_END

DEF_INST(AND_A_C_Z_0_1_0, 0xA1, 1, 4)
A(and8(A(), C()));
DEF_INST_END

DEF_INST(AND_A_D_Z_0_1_0, 0xA2, 1, 4)
A(and8(A(), D()));
DEF_INST_END

DEF_INST(AND_A_E_Z_0_1_0, 0xA3, 1, 4)
A(and8(A(), E()));
DEF_INST_END

DEF_INST(AND_A_H_Z_0_1_0, 0xA4, 1, 4)
A(and8(A(), H()));
DEF_INST_END

DEF_INST(AND_A_L_Z_0_1_0, 0xA5, 1, 4)
A(and8(A(), L()));
DEF_INST_END

DEF_INST(AND_A_xHLx_Z_0_1_0, 0xA6, 1, 8)
A(and8(A(), get(HL())));
DEF_INST_END

DEF_INST(AND_A_A_Z_0_1_0, 0xA7, 1, 4)
A(and8(A(), A()));
DEF_INST_END

DEF_INST(XOR_A_B_Z_0_0_0, 0xA8, 1, 4)
A(xor8(A(), B()));
DEF_INST_END

DEF_INST(XOR_A_C_Z_0_0_0, 0xA9, 1, 4)
A(xor8(A(), C()));
DEF_INST_END

DEF_INST(XOR_A_D_Z_0_0_0, 0xAA, 1, 4)
A(xor8(A(), D()));
DEF_INST_END

DEF_INST(XOR_A_E_Z_0_0_0, 0xAB, 1, 4)
A(xor8(A(), E()));
DEF_INST_END

DEF_INST(XOR_A_H_Z_0_0_0, 0xAC, 1, 4)
A(xor8(A(), H()));
DEF_INST_END

DEF_INST(XOR_A_L_Z_0_0_0, 0xAD, 1, 4)
A(xor8(A(), L()));
DEF_INST_END

DEF_INST(XOR_A_xHLx_Z_0_0_0, 0xAE, 1, 8)
A(xor8(A(), get(HL())));
DEF_INST_END

DEF_INST(XOR_A_A_1_0_0_0, 0xAF, 1, 4)
A(xor8(A(), A()));
DEF_INST_END

DEF_INST(OR_A_B_Z_0_0_0, 0xB0, 1, 4)
A(or8(A(), B()));
DEF_INST_END

DEF_INST(OR_A_C_Z_0_0_0, 0xB1, 1, 4)
A(or8(A(), C()));
DEF_INST_END

DEF_INST(OR_A_D_Z_0_0_0, 0xB2, 1, 4)
A(or8(A(), D()));
DEF_INST_END

DEF_INST(OR_A_E_Z_0_0_0, 0xB3, 1, 4)
A(or8(A(), E()));
DEF_INST_END

DEF_INST(OR_A_H_Z_0_0_0, 0xB4, 1, 4)
A(or8(A(), H()));
DEF_INST_END

DEF_INST(OR_A_L_Z_0_0_0, 0xB5, 1, 4)
A(or8(A(), L()));
DEF_INST_END

DEF_INST(OR_A_xHLx_Z_0_0_0, 0xB6, 1, 8)
A(or8(A(), get(HL())));
DEF_INST_END

DEF_INST(OR_A_A_Z_0_0_0, 0xB7, 1, 4)
A(or8(A(), A()));
DEF_INST_END

DEF_INST(CP_A_B_Z_1_H_C, 0xB8, 1, 4)
cp8(A(), B());
DEF_INST_END

DEF_INST(CP_A_C_Z_1_H_C, 0xB9, 1, 4)
cp8(A(), C());
DEF_INST_END

DEF_INST(CP_A_D_Z_1_H_C, 0xBA, 1, 4)
cp8(A(), D());
DEF_INST_END

DEF_INST(CP_A_E_Z_1_H_C, 0xBB, 1, 4)
cp8(A(), E());
DEF_INST_END

DEF_INST(CP_A_H_Z_1_H_C, 0xBC, 1, 4)
cp8(A(), H());
DEF_INST_END

DEF_INST(CP_A_L_Z_1_H_C, 0xBD, 1, 4)
cp8(A(), L());
DEF_INST_END

DEF_INST(CP_A_xHLx_Z_1_H_C, 0xBE, 1, 8)
cp8(A(), get(HL()));
DEF_INST_END

DEF_INST(CP_A_A_1_1_0_0, 0xBF, 1, 4)
cp8(A(), A());
DEF_INST_END

DEF_INST(RET_NZ_x_x_x_x, 0xC0, 1, 8)
if (!zf()) {
  PC(pop16());
  cycle = 20;
}
DEF_INST_END

DEF_INST(POP_BC_x_x_x_x, 0xC1, 1, 12)
BC(pop16());
DEF_INST_END

DEF_INST(JP_NZ_a16_x_x_x_x, 0xC2, 3, 12)
u16 target = imm16();
if (!zf()) {
  PC(target);
  cycle = 16;
}
DEF_INST_END

DEF_INST(JP_a16_x_x_x_x, 0xC3, 3, 16)
PC(imm16());
DEF_INST_END

DEF_INST(CALL_NZ_a16_x_x_x_x, 0xC4, 3, 12)
u16 target = imm16();
if (!zf()) {
  push16(PC());
  PC(target);
  cycle = 24;
}
DEF_INST_END

DEF_INST(PUSH_BC_x_x_x_x, 0xC5, 1, 16)
push16(BC());
DEF_INST_END

DEF_INST(ADD_A_n8_Z_0_H_C, 0xC6, 2, 8)
A(add8(A(), imm8()));
DEF_INST_END

DEF_INST(RST_0x00_x_x_x_x, 0xC7, 1, 16)
push16(PC());
PC(0x00);
DEF_INST_END

DEF_INST(RET_Z_x_x_x_x, 0xC8, 1, 8)
if (zf()) {
  PC(pop16());
  cycle = 20;
}
DEF_INST_END

DEF_INST(RET_x_x_x_x, 0xC9, 1, 16)
PC(pop16());
DEF_INST_END

DEF_INST(JP_Z_a16_x_x_x_x, 0xCA, 3, 12)
u16 target = imm16();
if (zf()) {
  PC(target);
  cycle = 16;
}
DEF_INST_END

DEF_INST(PREFIX_x_x_x_x, 0xCB, 1, 4)
// extend instructions
// https://gbdev.io/pandocs/CPU_Instruction_Set.html#cb-prefix-instructions
/*
eg. RLC B
00000   000
  op    reg
*/
cycle           = 8;
u8 inst         = imm8();
u8 reg_bit      = inst & 0x7;
u8 shift_op_bit = (inst >> 3) & 0x7;
u8 bit_op_bit   = inst >> 6;
u8 unhandled_data{};
u8 handled_data{};

switch (reg_bit) {
  case 0:
    unhandled_data = B();
    break;
  case 1:
    unhandled_data = C();
    break;
  case 2:
    unhandled_data = D();
    break;
  case 3:
    unhandled_data = E();
    break;
  case 4:
    unhandled_data = H();
    break;
  case 5:
    unhandled_data = L();
    break;
  case 6:
    unhandled_data = get(HL());
    cycle          = 16;
    break;
  case 7:
    unhandled_data = A();
    break;
  default:
    GB_UNREACHABLE();
}

if (bit_op_bit != 0) {
  u8 bit_n = (inst >> 3) & 0x7; // for set8 res8 bit8
  if (bit_op_bit == 1) {
    handled_data = bit8(bit_n, unhandled_data);
    if (reg_bit == 6) {
      cycle = 12;
    }
  } else if (bit_op_bit == 2) {
    handled_data = res8(bit_n, unhandled_data);
  } else if (bit_op_bit == 3) {
    handled_data = set8(bit_n, unhandled_data);
  } else {
    GB_UNREACHABLE();
  }
} else {
  switch (shift_op_bit) {
    case 0:
      handled_data = rlc8(unhandled_data);
      break;
    case 1:
      handled_data = rrc8(unhandled_data);
      break;
    case 2:
      handled_data = rl8(unhandled_data);
      break;
    case 3:
      handled_data = rr8(unhandled_data);
      break;
    case 4:
      handled_data = sla8(unhandled_data);
      break;
    case 5:
      handled_data = sra8(unhandled_data);
      break;
    case 6:
      handled_data = swap8(unhandled_data);
      break;
    case 7:
      handled_data = srl8(unhandled_data);
      break;
    default: {
      GB_UNREACHABLE();
      break;
    }
  }
}
if (bit_op_bit != 1) {
  switch (reg_bit) {
    case 0:
      B(handled_data);
      break;
    case 1:
      C(handled_data);
      break;
    case 2:
      D(handled_data);
      break;
    case 3:
      E(handled_data);
      break;
    case 4:
      H(handled_data);
      break;
    case 5:
      L(handled_data);
      break;
    case 6:
      set(HL(), handled_data);
      break;
    case 7:
      A(handled_data);
      break;
    default:
      GB_UNREACHABLE();
  }
}
DEF_INST_END

DEF_INST(CALL_Z_a16_x_x_x_x, 0xCC, 3, 12) u16 target = imm16();
if (zf()) {
  push16(PC());
  PC(target);
  cycle = 24;
}
DEF_INST_END

DEF_INST(CALL_a16_x_x_x_x, 0xCD, 3, 24)
u16 target = imm16();
push16(PC());
PC(target);
DEF_INST_END

DEF_INST(ADC_A_n8_Z_0_H_C, 0xCE, 2, 8)
A(adc8(A(), imm8()));
DEF_INST_END

DEF_INST(RST_0x08_x_x_x_x, 0xCF, 1, 16)
push16(PC());
PC(0x08);
DEF_INST_END

DEF_INST(RET_NC_x_x_x_x, 0xD0, 1, 8)
if (!cf()) {
  PC(pop16());
  cycle = 20;
}
DEF_INST_END

DEF_INST(POP_DE_x_x_x_x, 0xD1, 1, 12)
DE(pop16());
DEF_INST_END

DEF_INST(JP_NC_a16_x_x_x_x, 0xD2, 3, 12)
u16 target = imm16();
if (!cf()) {
  PC(target);
  cycle = 16;
}
DEF_INST_END

DEF_INST(ILLEGAL_D3_x_x_x_x, 0xD3, 1, 4)
DEF_INST_END

DEF_INST(CALL_NC_a16_x_x_x_x, 0xD4, 3, 12)
u16 target = imm16();
if (!cf()) {
  push16(PC());
  PC(target);
  cycle = 24;
}
DEF_INST_END

DEF_INST(PUSH_DE_x_x_x_x, 0xD5, 1, 16)
push16(DE());
DEF_INST_END

DEF_INST(SUB_A_n8_Z_1_H_C, 0xD6, 2, 8)
A(sub8(A(), imm8()));
DEF_INST_END

DEF_INST(RST_0x10_x_x_x_x, 0xD7, 1, 16)
push16(PC());
PC(0x10);
DEF_INST_END

DEF_INST(RET_C_x_x_x_x, 0xD8, 1, 8)
if (cf()) {
  PC(pop16());
  cycle = 20;
}
DEF_INST_END

DEF_INST(RETI_x_x_x_x, 0xD9, 1, 16)
IME(1);
PC(pop16());
DEF_INST_END

DEF_INST(JP_C_a16_x_x_x_x, 0xDA, 3, 12)
u16 target = imm16();
if (cf()) {
  PC(target);
  cycle = 16;
}
DEF_INST_END

DEF_INST(ILLEGAL_DB_x_x_x_x, 0xDB, 1, 4)
DEF_INST_END

DEF_INST(CALL_C_a16_x_x_x_x, 0xDC, 3, 12)
u16 target = imm16();
if (cf()) {
  push16(PC());
  PC(target);
  cycle = 24;
}
DEF_INST_END

DEF_INST(ILLEGAL_DD_x_x_x_x, 0xDD, 1, 4)
DEF_INST_END

DEF_INST(SBC_A_n8_Z_1_H_C, 0xDE, 2, 8)
A(sbc8(A(), imm8()));
DEF_INST_END

DEF_INST(RST_0x18_x_x_x_x, 0xDF, 1, 16)
push16(PC());
PC(0x18);
DEF_INST_END

DEF_INST(LDH_xa8x_A_x_x_x_x, 0xE0, 2, 12)
set(OFFSET + imm8(), A());
DEF_INST_END

DEF_INST(POP_HL_x_x_x_x, 0xE1, 1, 12)
HL(pop16());
DEF_INST_END

DEF_INST(LD_xCx_A_x_x_x_x, 0xE2, 1, 8)
set(OFFSET + C(), A());
DEF_INST_END

DEF_INST(ILLEGAL_E3_x_x_x_x, 0xE3, 1, 4)
DEF_INST_END
DEF_INST(ILLEGAL_E4_x_x_x_x, 0xE4, 1, 4)
DEF_INST_END

DEF_INST(PUSH_HL_x_x_x_x, 0xE5, 1, 16)
push16(HL());
DEF_INST_END

DEF_INST(AND_A_n8_Z_0_1_0, 0xE6, 2, 8)
A(and8(A(), imm8()));
DEF_INST_END

DEF_INST(RST_0x20_x_x_x_x, 0xE7, 1, 16)
push16(PC());
PC(0x20);
DEF_INST_END

DEF_INST(ADD_SP_e8_0_0_H_C, 0xE8, 2, 16)
SP(add16(SP(), imm8()));
DEF_INST_END

DEF_INST(JP_HL_x_x_x_x, 0xE9, 1, 4)
PC(HL());
DEF_INST_END

DEF_INST(LD_xa16x_A_x_x_x_x, 0xEA, 3, 16)
set(imm16(), A());
DEF_INST_END

DEF_INST(ILLEGAL_EB_x_x_x_x, 0xEB, 1, 4)
DEF_INST_END
DEF_INST(ILLEGAL_EC_x_x_x_x, 0xEC, 1, 4)
DEF_INST_END
DEF_INST(ILLEGAL_ED_x_x_x_x, 0xED, 1, 4)
DEF_INST_END

DEF_INST(XOR_A_n8_Z_0_0_0, 0xEE, 2, 8)
A(xor8(A(), imm8()));
DEF_INST_END

DEF_INST(RST_0x28_x_x_x_x, 0xEF, 1, 16)
push16(PC());
PC(0x28);
DEF_INST_END

DEF_INST(LDH_A_xa8x_x_x_x_x, 0xF0, 2, 12)
A(get(OFFSET + imm8()));
DEF_INST_END

DEF_INST(POP_AF_Z_N_H_C, 0xF1, 1, 12)
AF(pop16());
DEF_INST_END

DEF_INST(LD_A_xCx_x_x_x_x, 0xF2, 1, 8)
A(get(OFFSET + C()));
DEF_INST_END

DEF_INST(DI_x_x_x_x, 0xF3, 1, 4)
IME(0);
DEF_INST_END

DEF_INST(ILLEGAL_F4_x_x_x_x, 0xF4, 1, 4)
DEF_INST_END

DEF_INST(PUSH_AF_x_x_x_x, 0xF5, 1, 16)
push16(AF());
DEF_INST_END

DEF_INST(OR_A_n8_Z_0_0_0, 0xF6, 2, 8)
A(or8(A(), imm8()));
DEF_INST_END

DEF_INST(RST_0x30_x_x_x_x, 0xF7, 1, 16)
push16(PC());
PC(0x30);
DEF_INST_END

DEF_INST(LD_HL_SP_e8_0_0_H_C, 0xF8, 2, 12)
HL(add16(SP(), imm8()));
DEF_INST_END

DEF_INST(LD_SP_HL_x_x_x_x, 0xF9, 1, 8)
SP(HL());
DEF_INST_END

DEF_INST(LD_A_xa16x_x_x_x_x, 0xFA, 3, 16)
A(get(imm16()));
DEF_INST_END

DEF_INST(EI_x_x_x_x, 0xFB, 1, 4)
IME(1);
DEF_INST_END

DEF_INST(ILLEGAL_FC_x_x_x_x, 0xFC, 1, 4)
DEF_INST_END
DEF_INST(ILLEGAL_FD_x_x_x_x, 0xFD, 1, 4)
DEF_INST_END

DEF_INST(CP_A_n8_Z_1_H_C, 0xFE, 2, 8)
cp8(A(), imm8());
DEF_INST_END

DEF_INST(RST_0x38_x_x_x_x, 0xFF, 1, 16)
push16(PC());
PC(0x38);
DEF_INST_END

#undef A
#undef F
#undef AF
#undef B
#undef C
#undef BC
#undef D
#undef E
#undef DE
#undef H
#undef L
#undef HL
#undef zf
#undef nf
#undef hf
#undef cf
#undef PC
#undef SP
#undef halt
#undef imm8
#undef imm16
#undef set
#undef set16
#undef get
#undef inc8
#undef dec8
#undef rlc8
#undef addHL
#undef add8
#undef add16
#undef adc8
#undef sub8
#undef sub16
#undef sbc8
#undef and8
#undef xor8
#undef or8
#undef cp8
#undef jr
#undef push16
#undef pop16
#undef IME
#undef rrc8
#undef rl8
#undef rr8
#undef sla8
#undef sra8
#undef swap8
#undef srl8
#undef bit8
#undef res8
#undef set8

static const std::function<u8(CPU *)> *instruction_table() {
  static std::function<u8(CPU *)> f[]{
          _0x00::update, _0x01::update, _0x02::update, _0x03::update, _0x04::update, _0x05::update,
          _0x06::update, _0x07::update, _0x08::update, _0x09::update, _0x0A::update, _0x0B::update,
          _0x0C::update, _0x0D::update, _0x0E::update, _0x0F::update, _0x10::update, _0x11::update,
          _0x12::update, _0x13::update, _0x14::update, _0x15::update, _0x16::update, _0x17::update,
          _0x18::update, _0x19::update, _0x1A::update, _0x1B::update, _0x1C::update, _0x1D::update,
          _0x1E::update, _0x1F::update, _0x20::update, _0x21::update, _0x22::update, _0x23::update,
          _0x24::update, _0x25::update, _0x26::update, _0x27::update, _0x28::update, _0x29::update,
          _0x2A::update, _0x2B::update, _0x2C::update, _0x2D::update, _0x2E::update, _0x2F::update,
          _0x30::update, _0x31::update, _0x32::update, _0x33::update, _0x34::update, _0x35::update,
          _0x36::update, _0x37::update, _0x38::update, _0x39::update, _0x3A::update, _0x3B::update,
          _0x3C::update, _0x3D::update, _0x3E::update, _0x3F::update, _0x40::update, _0x41::update,
          _0x42::update, _0x43::update, _0x44::update, _0x45::update, _0x46::update, _0x47::update,
          _0x48::update, _0x49::update, _0x4A::update, _0x4B::update, _0x4C::update, _0x4D::update,
          _0x4E::update, _0x4F::update, _0x50::update, _0x51::update, _0x52::update, _0x53::update,
          _0x54::update, _0x55::update, _0x56::update, _0x57::update, _0x58::update, _0x59::update,
          _0x5A::update, _0x5B::update, _0x5C::update, _0x5D::update, _0x5E::update, _0x5F::update,
          _0x60::update, _0x61::update, _0x62::update, _0x63::update, _0x64::update, _0x65::update,
          _0x66::update, _0x67::update, _0x68::update, _0x69::update, _0x6A::update, _0x6B::update,
          _0x6C::update, _0x6D::update, _0x6E::update, _0x6F::update, _0x70::update, _0x71::update,
          _0x72::update, _0x73::update, _0x74::update, _0x75::update, _0x76::update, _0x77::update,
          _0x78::update, _0x79::update, _0x7A::update, _0x7B::update, _0x7C::update, _0x7D::update,
          _0x7E::update, _0x7F::update, _0x80::update, _0x81::update, _0x82::update, _0x83::update,
          _0x84::update, _0x85::update, _0x86::update, _0x87::update, _0x88::update, _0x89::update,
          _0x8A::update, _0x8B::update, _0x8C::update, _0x8D::update, _0x8E::update, _0x8F::update,
          _0x90::update, _0x91::update, _0x92::update, _0x93::update, _0x94::update, _0x95::update,
          _0x96::update, _0x97::update, _0x98::update, _0x99::update, _0x9A::update, _0x9B::update,
          _0x9C::update, _0x9D::update, _0x9E::update, _0x9F::update, _0xA0::update, _0xA1::update,
          _0xA2::update, _0xA3::update, _0xA4::update, _0xA5::update, _0xA6::update, _0xA7::update,
          _0xA8::update, _0xA9::update, _0xAA::update, _0xAB::update, _0xAC::update, _0xAD::update,
          _0xAE::update, _0xAF::update, _0xB0::update, _0xB1::update, _0xB2::update, _0xB3::update,
          _0xB4::update, _0xB5::update, _0xB6::update, _0xB7::update, _0xB8::update, _0xB9::update,
          _0xBA::update, _0xBB::update, _0xBC::update, _0xBD::update, _0xBE::update, _0xBF::update,
          _0xC0::update, _0xC1::update, _0xC2::update, _0xC3::update, _0xC4::update, _0xC5::update,
          _0xC6::update, _0xC7::update, _0xC8::update, _0xC9::update, _0xCA::update, _0xCB::update,
          _0xCC::update, _0xCD::update, _0xCE::update, _0xCF::update, _0xD0::update, _0xD1::update,
          _0xD2::update, _0xD3::update, _0xD4::update, _0xD5::update, _0xD6::update, _0xD7::update,
          _0xD8::update, _0xD9::update, _0xDA::update, _0xDB::update, _0xDC::update, _0xDD::update,
          _0xDE::update, _0xDF::update, _0xE0::update, _0xE1::update, _0xE2::update, _0xE3::update,
          _0xE4::update, _0xE5::update, _0xE6::update, _0xE7::update, _0xE8::update, _0xE9::update,
          _0xEA::update, _0xEB::update, _0xEC::update, _0xED::update, _0xEE::update, _0xEF::update,
          _0xF0::update, _0xF1::update, _0xF2::update, _0xF3::update, _0xF4::update, _0xF5::update,
          _0xF6::update, _0xF7::update, _0xF8::update, _0xF9::update, _0xFA::update, _0xFB::update,
          _0xFC::update, _0xFD::update, _0xFE::update, _0xFF::update,
  };
  return f;
}

u8 CPU::update(u64 cycle) {
  if (halt()) {
    // https://gbdev.io/pandocs/halt.html#halt-bug
    // todo: if IME() not set but ie/if is set, we need to handle this bug.
    if (IME() || (memory_bus_->get(0xff0f) & memory_bus_->get(0xffff))) {
      halt(false);
    } else {
      // idle
    }
    return 4;
  } else {
    // handle interrupt first
    u8 irq = handleInterrupt();
    if (irq != 0) {
      return irq;
    }
    u8 inst_idx = imm8();
    GB_ASSERT(inst_idx <= 255 || inst_idx >= 0);
    return instruction_table()[inst_idx](this);
  }
  GB_UNREACHABLE();
}

u8 CPU::handleInterrupt() {
  if (!IME()) {
    return 0;
  }
  if (interrupt_delay_ != 0) {
    interrupt_delay_--;
    return 0;
  }
  u8 interrupt_mask = memory_bus_->get(0xffff) & memory_bus_->get(0xff0f);
  if (interrupt_mask == 0) {
    return 0;
  }
  constexpr u16 interrupt_table[] = {
          0x40, // VBLANK
          0x48, // STAT
          0x50, // TIMER
          0x58, // SERIAL
          0x60, // JOYPAD
  };
  // interrupt priority
  // https://gbdev.io/pandocs/Interrupts.html#interrupt-priorities
  for (u8 i = 0; i < sizeof(interrupt_table) / sizeof(interrupt_table[0]); i++) {
    if (interrupt_mask & (1 << i)) {
      push16(PC());
      PC(interrupt_table[i]);
      memory_bus_->set(0xff0f, memory_bus_->get(0xff0f) & ~(1 << i));
      break;
    }
  }
  IME(false);
  // irq wasted 5 M-cycles = 20 T-cycles
  return 20;
}


} // namespace gb
