#include <stdio.h>
#include <stdlib.h>

void set_n(int);

void set_z(int);

void set_c_add(int, int);

void set_c_sub(int, int);

void set_c_shift(int, int, int);

void set_v_add(int, int);

void set_v_sub(int, int);

void print_flags();

void print_memory();

void print_regs();

void store_memory(int, int);

int load_memory(int);

int load_register(int);

void store_register(int, int);

int shift_left(int, int);

int shift_right(int, int);

int rotate_right(int, int);

void and(int, int, int, int, int);

void eor(int, int, int, int, int);

void sub(int, int, int, int, int);

void rsb(int, int, int, int, int);

void add(int, int, int, int, int);

void adc(int, int, int, int, int);

void sbc(int, int, int, int, int);

void rsc(int, int, int, int, int);

void cmp(int, int, int);

void cmn(int, int, int);

void orr(int, int, int, int, int);

void mov(int, int, int, int);

void lsl(int, int, int, int, int);

void asr(int, int, int, int, int);

void rrx(int, int, int, int);

void ror(int, int, int, int, int);

void bic(int, int, int, int, int);

void mvn(int, int, int, int);

void mul(int, int, int, int, int);

void mla(int, int, int, int, int, int);

int str(int, int, int, int, int);

int ldr(int, int, int, int, int);

int strb(int, int, int, int, int);

int ldrb(int, int, int, int, int);

int b(int);

void load_data_mem();

int init();

int foo();

int eval_cond(int);
