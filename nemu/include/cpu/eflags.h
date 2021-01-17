#ifndef __EFLAGS_H__
#define __EFLAGS_H__

#include "common.h"

void update_eflags_pf_zf_sf(uint32_t);


static inline bool check_cc_b() {
	return cpu.EFLAGS.CF;
}


static inline bool check_cc_e() {
	return cpu.EFLAGS.ZF;
}

static inline bool check_cc_ne() {
	return !cpu.EFLAGS.ZF;
}

static inline bool check_cc_be() {
	return cpu.EFLAGS.CF | cpu.EFLAGS.ZF;
}

static inline bool check_cc_a() {
	return !(cpu.EFLAGS.CF | cpu.EFLAGS.ZF);
}

static inline bool check_cc_s() {
	return cpu.EFLAGS.SF;
}

static inline bool check_cc_ns() {
	return !cpu.EFLAGS.SF;
}


static inline bool check_cc_l() {
	return cpu.EFLAGS.SF ^ cpu.EFLAGS.OF;
}

static inline bool check_cc_ge() {
	return !(cpu.EFLAGS.SF ^ cpu.EFLAGS.OF);
}

static inline bool check_cc_le() {
	return (cpu.EFLAGS.SF ^ cpu.EFLAGS.OF) | cpu.EFLAGS.ZF;
}

static inline bool check_cc_g() {
	return !((cpu.EFLAGS.SF ^ cpu.EFLAGS.OF) | cpu.EFLAGS.ZF);
}

#endif
