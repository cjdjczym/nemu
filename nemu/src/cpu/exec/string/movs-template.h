#include "cpu/exec/template-start.h"

#define instr movs

make_helper(concat(movs_, SUFFIX)) {

    MEM_W(R_DS, cpu.edi, MEM_R(R_DS, cpu.esi));
	if (cpu.DF == 0) {
        reg_l(R_EDI) += DATA_BYTE;
        reg_l(R_ESI) += DATA_BYTE;
    }else {
        reg_l(R_EDI) -= DATA_BYTE;
        reg_l(R_ESI) -= DATA_BYTE;
    }
	print_asm("movs");
    return 1;
}



#include "cpu/exec/template-end.h"