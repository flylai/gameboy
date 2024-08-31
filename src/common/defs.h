#pragma once

#include "common/type.h"

namespace gb {

// IO Registers
static constexpr u16 P1_BASE           = 0xFF00;
static constexpr u16 SB_BASE           = 0xFF01;
static constexpr u16 SC_BASE           = 0xFF02;
static constexpr u16 DIV_BASE          = 0xFF04;
static constexpr u16 TIMA_BASE         = 0xFF05;
static constexpr u16 TMA_BASE          = 0xFF06;
static constexpr u16 TAC_BASE          = 0xFF07;
static constexpr u16 IF_BASE           = 0xFF0F;
static constexpr u16 NR10_BASE         = 0xFF10;
static constexpr u16 NR11_BASE         = 0xFF11;
static constexpr u16 NR12_BASE         = 0xFF12;
static constexpr u16 NR13_BASE         = 0xFF13;
static constexpr u16 NR14_BASE         = 0xFF14;
static constexpr u16 NR21_BASE         = 0xFF16;
static constexpr u16 NR22_BASE         = 0xFF17;
static constexpr u16 NR23_BASE         = 0xFF18;
static constexpr u16 NR24_BASE         = 0xFF19;
static constexpr u16 NR30_BASE         = 0xFF1A;
static constexpr u16 NR31_BASE         = 0xFF1B;
static constexpr u16 NR32_BASE         = 0xFF1C;
static constexpr u16 NR33_BASE         = 0xFF1D;
static constexpr u16 NR34_BASE         = 0xFF1E;
static constexpr u16 NR41_BASE         = 0xFF20;
static constexpr u16 NR42_BASE         = 0xFF21;
static constexpr u16 NR43_BASE         = 0xFF22;
static constexpr u16 NR44_BASE         = 0xFF23;
static constexpr u16 NR50_BASE         = 0xFF24;
static constexpr u16 NR51_BASE         = 0xFF25;
static constexpr u16 NR52_BASE         = 0xFF26;
static constexpr u16 LCDC_BASE         = 0xFF40;
static constexpr u16 STAT_BASE         = 0xFF41;
static constexpr u16 SCY_BASE          = 0xFF42;
static constexpr u16 SCX_BASE          = 0xFF43;
static constexpr u16 LY_BASE           = 0xFF44;
static constexpr u16 LYC_BASE          = 0xFF45;
static constexpr u16 DMA_BASE          = 0xFF46;
static constexpr u16 BGP_BASE          = 0xFF47;
static constexpr u16 OBP0_BASE         = 0xFF48;
static constexpr u16 OBP1_BASE         = 0xFF49;
static constexpr u16 WY_BASE           = 0xFF4A;
static constexpr u16 WX_BASE           = 0xFF4B;
static constexpr u16 IE_BASE           = 0xFFFF;

// Memory

static constexpr u16 ROM_BANK_0_BASE   = 0x0000;
static constexpr u16 ROM_BANK_N_BASE   = 0x4000;
static constexpr u16 VRAM_BASE         = 0x8000;
static constexpr u16 EXTERNAL_RAM_BASE = 0xA000;
static constexpr u16 WRAM_0_BASE       = 0xC000;
static constexpr u16 WRAM_N_BASE       = 0xD000;
static constexpr u16 ECHO_RAM_BASE     = 0xE000;
static constexpr u16 OAM_BASE          = 0xFE00;
static constexpr u16 PROHIBITED_BASE   = 0xFEA0;
static constexpr u16 HRAM_BASE         = 0xFF80;

// CGB Registers

static constexpr u16 HDMA1_BASE        = 0xFF51;
static constexpr u16 HDMA2_BASE        = 0xFF52;
static constexpr u16 HDMA3_BASE        = 0xFF53;
static constexpr u16 HDMA4_BASE        = 0xFF54;
static constexpr u16 HDMA5_BASE        = 0xFF55;
static constexpr u16 VBK_BASE          = 0xFF4F;
static constexpr u16 KEY1_BASE         = 0xFF4D;
static constexpr u16 RP_BASE           = 0xFF56;
static constexpr u16 OPRI_BASE         = 0xFF6C;
static constexpr u16 SVBK_BASE         = 0xFF70;

// Misc

static constexpr u8 LCD_WIDTH          = 160;
static constexpr u8 LCD_HEIGHT         = 144;

static constexpr u16 WINDOW_WIDTH      = 1280;
static constexpr u16 WINDOW_HEIGHT     = 720;

} // namespace gb
