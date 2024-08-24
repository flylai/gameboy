# Google Test Report

Commit: [39df18afd8d9e914b414c162cc9aabb3b6d6ce18](https://github.com/flylai/gameboy/commit/39df18afd8d9e914b414c162cc9aabb3b6d6ce18)

## Suite: gb_test_roms_cpu_instrs/GBTest
- ✅ **Test**: ../tests/gb-test-roms/cpu_instrs/cpu_instrs.gb
- ✅ **Test**: ../tests/gb-test-roms/cpu_instrs/individual/01-special.gb
- ✅ **Test**: ../tests/gb-test-roms/cpu_instrs/individual/02-interrupts.gb
- ✅ **Test**: ../tests/gb-test-roms/cpu_instrs/individual/03-op sp,hl.gb
- ✅ **Test**: ../tests/gb-test-roms/cpu_instrs/individual/04-op r,imm.gb
- ✅ **Test**: ../tests/gb-test-roms/cpu_instrs/individual/05-op rp.gb
- ✅ **Test**: ../tests/gb-test-roms/cpu_instrs/individual/06-ld r,r.gb
- ✅ **Test**: ../tests/gb-test-roms/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb
- ✅ **Test**: ../tests/gb-test-roms/cpu_instrs/individual/08-misc instrs.gb
- ✅ **Test**: ../tests/gb-test-roms/cpu_instrs/individual/09-op r,r.gb
- ✅ **Test**: ../tests/gb-test-roms/cpu_instrs/individual/10-bit ops.gb
- ✅ **Test**: ../tests/gb-test-roms/cpu_instrs/individual/11-op a,(hl).gb
## Suite: gb_test_roms_instr_timing/GBTest
- ✅ **Test**: ../tests/gb-test-roms/instr_timing/instr_timing.gb
## Suite: gb_test_roms_interrupt_time/GBTest
- ❌ **Test**: ../tests/gb-test-roms/interrupt_time/interrupt_time.gb
## Suite: gb_test_roms_mem_timing/GBTest
- ✅ **Test**: ../tests/gb-test-roms/mem_timing/individual/01-read_timing.gb
- ✅ **Test**: ../tests/gb-test-roms/mem_timing/individual/02-write_timing.gb
- ✅ **Test**: ../tests/gb-test-roms/mem_timing/individual/03-modify_timing.gb
- ✅ **Test**: ../tests/gb-test-roms/mem_timing/mem_timing.gb
## Suite: gb_test_roms_mem_timing2/GBTest
- ❌ **Test**: ../tests/gb-test-roms/mem_timing-2/mem_timing.gb
- ❌ **Test**: ../tests/gb-test-roms/mem_timing-2/rom_singles/01-read_timing.gb
- ❌ **Test**: ../tests/gb-test-roms/mem_timing-2/rom_singles/02-write_timing.gb
- ❌ **Test**: ../tests/gb-test-roms/mem_timing-2/rom_singles/03-modify_timing.gb
## Suite: mts_emulator_only_mbc1/GBTest
- ✅ **Test**: ../tests/mts/emulator-only/mbc1/bits_bank1.gb
- ✅ **Test**: ../tests/mts/emulator-only/mbc1/bits_bank2.gb
- ✅ **Test**: ../tests/mts/emulator-only/mbc1/bits_mode.gb
- ✅ **Test**: ../tests/mts/emulator-only/mbc1/bits_ramg.gb
- ❌ **Test**: ../tests/mts/emulator-only/mbc1/multicart_rom_8Mb.gb
- ✅ **Test**: ../tests/mts/emulator-only/mbc1/ram_256kb.gb
- ✅ **Test**: ../tests/mts/emulator-only/mbc1/ram_64kb.gb
- ✅ **Test**: ../tests/mts/emulator-only/mbc1/rom_16Mb.gb
- ✅ **Test**: ../tests/mts/emulator-only/mbc1/rom_1Mb.gb
- ✅ **Test**: ../tests/mts/emulator-only/mbc1/rom_2Mb.gb
- ✅ **Test**: ../tests/mts/emulator-only/mbc1/rom_4Mb.gb
- ✅ **Test**: ../tests/mts/emulator-only/mbc1/rom_512kb.gb
- ✅ **Test**: ../tests/mts/emulator-only/mbc1/rom_8Mb.gb
## Suite: mts_acceptance_bits/GBTest
- ✅ **Test**: ../tests/mts/acceptance/bits/mem_oam.gb
- ✅ **Test**: ../tests/mts/acceptance/bits/reg_f.gb
- ❌ **Test**: ../tests/mts/acceptance/bits/unused_hwio-GS.gb
## Suite: mts_acceptance_instr/GBTest
- ✅ **Test**: ../tests/mts/acceptance/instr/daa.gb
## Suite: mts_acceptance_interrupts/GBTest
- ❌ **Test**: ../tests/mts/acceptance/interrupts/ie_push.gb
## Suite: mts_acceptance_ppu/GBTest
- ❌ **Test**: ../tests/mts/acceptance/ppu/hblank_ly_scx_timing-GS.gb
- ❌ **Test**: ../tests/mts/acceptance/ppu/intr_1_2_timing-GS.gb
- ❌ **Test**: ../tests/mts/acceptance/ppu/intr_2_0_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/ppu/intr_2_mode0_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/ppu/intr_2_mode0_timing_sprites.gb
- ❌ **Test**: ../tests/mts/acceptance/ppu/intr_2_mode3_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/ppu/intr_2_oam_ok_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/ppu/lcdon_timing-GS.gb
- ❌ **Test**: ../tests/mts/acceptance/ppu/lcdon_write_timing-GS.gb
- ❌ **Test**: ../tests/mts/acceptance/ppu/stat_irq_blocking.gb
- ❌ **Test**: ../tests/mts/acceptance/ppu/stat_lyc_onoff.gb
- ❌ **Test**: ../tests/mts/acceptance/ppu/vblank_stat_intr-GS.gb
## Suite: mts_acceptance_timer/GBTest
- ✅ **Test**: ../tests/mts/acceptance/timer/div_write.gb
- ❌ **Test**: ../tests/mts/acceptance/timer/rapid_toggle.gb
- ✅ **Test**: ../tests/mts/acceptance/timer/tim00.gb
- ✅ **Test**: ../tests/mts/acceptance/timer/tim00_div_trigger.gb
- ✅ **Test**: ../tests/mts/acceptance/timer/tim01.gb
- ✅ **Test**: ../tests/mts/acceptance/timer/tim01_div_trigger.gb
- ✅ **Test**: ../tests/mts/acceptance/timer/tim10.gb
- ✅ **Test**: ../tests/mts/acceptance/timer/tim10_div_trigger.gb
- ✅ **Test**: ../tests/mts/acceptance/timer/tim11.gb
- ✅ **Test**: ../tests/mts/acceptance/timer/tim11_div_trigger.gb
- ✅ **Test**: ../tests/mts/acceptance/timer/tima_reload.gb
- ❌ **Test**: ../tests/mts/acceptance/timer/tima_write_reloading.gb
- ❌ **Test**: ../tests/mts/acceptance/timer/tma_write_reloading.gb
## Suite: mts_acceptance_root/GBTest
- ❌ **Test**: ../tests/mts/acceptance/add_sp_e_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/boot_div-S.gb
- ❌ **Test**: ../tests/mts/acceptance/boot_div-dmg0.gb
- ❌ **Test**: ../tests/mts/acceptance/boot_div-dmgABCmgb.gb
- ❌ **Test**: ../tests/mts/acceptance/boot_div2-S.gb
- ❌ **Test**: ../tests/mts/acceptance/boot_hwio-S.gb
- ❌ **Test**: ../tests/mts/acceptance/boot_hwio-dmg0.gb
- ❌ **Test**: ../tests/mts/acceptance/boot_hwio-dmgABCmgb.gb
- ❌ **Test**: ../tests/mts/acceptance/boot_regs-dmg0.gb
- ✅ **Test**: ../tests/mts/acceptance/boot_regs-dmgABC.gb
- ❌ **Test**: ../tests/mts/acceptance/boot_regs-mgb.gb
- ❌ **Test**: ../tests/mts/acceptance/boot_regs-sgb.gb
- ❌ **Test**: ../tests/mts/acceptance/boot_regs-sgb2.gb
- ❌ **Test**: ../tests/mts/acceptance/call_cc_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/call_cc_timing2.gb
- ❌ **Test**: ../tests/mts/acceptance/call_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/call_timing2.gb
- ❌ **Test**: ../tests/mts/acceptance/di_timing-GS.gb
- ✅ **Test**: ../tests/mts/acceptance/div_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/ei_sequence.gb
- ✅ **Test**: ../tests/mts/acceptance/ei_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/halt_ime0_ei.gb
- ❌ **Test**: ../tests/mts/acceptance/halt_ime0_nointr_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/halt_ime1_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/halt_ime1_timing2-GS.gb
- ❌ **Test**: ../tests/mts/acceptance/if_ie_registers.gb
- ❌ **Test**: ../tests/mts/acceptance/intr_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/jp_cc_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/jp_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/ld_hl_sp_e_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/oam_dma_restart.gb
- ❌ **Test**: ../tests/mts/acceptance/oam_dma_start.gb
- ❌ **Test**: ../tests/mts/acceptance/oam_dma_timing.gb
- ✅ **Test**: ../tests/mts/acceptance/pop_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/push_timing.gb
- ✅ **Test**: ../tests/mts/acceptance/rapid_di_ei.gb
- ❌ **Test**: ../tests/mts/acceptance/ret_cc_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/ret_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/reti_intr_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/reti_timing.gb
- ❌ **Test**: ../tests/mts/acceptance/rst_timing.gb

