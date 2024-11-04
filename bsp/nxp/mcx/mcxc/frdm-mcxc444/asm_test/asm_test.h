#include <stdint.h>

void asm_test_bfi(void);
void asm_test_ror(void);
void asm_test_mov(void);
void asm_test_mvn(void);
void asm_test_asr(void);
void asm_test_add(void);
void asm_test_sub(void);
long long asm_test_ldrd(long long *a,long long *b);
void asm_test_strcpy(char * dst,char * src);
void asm_test_ldr(void);
void asm_test_nop(void);
void asm_test_nzcv(void);
void asm_test_cmn(void);
uint32_t asm_rev(uint32_t rev);
void asm_test_stm(uint32_t * p);
uint8_t xIsPrivileged(void);
void asm_test_ldr_adr(void);
