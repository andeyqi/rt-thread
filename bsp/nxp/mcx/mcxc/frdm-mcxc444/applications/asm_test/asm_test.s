    RSEG    ASM_TEST_CODE:CODE(2)
    thumb

    PUBLIC asm_test_bfi
    PUBLIC asm_test_ror
    PUBLIC asm_test_mov
    PUBLIC asm_test_mvn
    PUBLIC asm_test_asr
    PUBLIC asm_test_add
    PUBLIC asm_test_sub
    PUBLIC asm_test_ldrd
    PUBLIC asm_test_ldr
    PUBLIC asm_test_strcpy
    PUBLIC asm_test_nop
    PUBLIC asm_test_nzcv
    PUBLIC asm_test_cmn
    PUBLIC asm_rev
    PUBLIC asm_test_stm
    PUBLIC xIsPrivileged
    PUBLIC asm_test_ldr_adr

    EXTERN p_uint64

asm_test_bfi:
    ldr r0,=0xaabbccff
    ldr r1,=0x00000000
    ldr r2,=0x00000000
    //bfi r1,r0,#0,#8 /* copy r0 bit0-7 to r1 bit0-7 */
    //bfi r2,r0,#1,#10 /* cpoy r0 bit0-9 t0 r2 bit1-bit10 */

    bx lr

/*Rotate Right (immediate) provides the value of the contents of a register rotated by a constant value. The bits that
are rotated off the right end are inserted into the vacated bit positions on the left. It can optionally update the
condition flags based on the result.
*/
asm_test_ror:
    ldr r0,=0xaabbccdd
    ldr r1,=0x00000000
    ldr r2,=0x00000000
    //ror r1,r0,#16 /* r1 = 0xccddaabb */
    //ror r2,r0,#24 /* r2 = 0xbbccddaa */

    bx lr


asm_test_mov:
    movs r0,#1
    //mov r0,#0
    //mov r1,#1
    movs r0,#0
    movs r1,#1

    bx lr

asm_test_mvn:
//    mvns r0,#0          ;r0 = 0xffffffff N=1,Z=0
    mvns r0,#0XFFFFFFFF ;r0 = 0 N=0,Z=1
//    mvn  r1,r0
//    movt r1,#0xf123     ;MOVT writes a 16-bit immediate value, imm16, to the top halfword, Rd[31:16],
                        ;of its destination register. The write does not affect Rd[15:0].
                        ;MOVT{cond} Rd, #imm16,
                        ;Move Top.
                        ;Rd must not be SP and must not be PC.
//    movw r1,#0xf123     ;The MOVW instruction provides the same function as MOV,
                        ;but is restricted to using the imm16operand.

    bx lr

asm_test_asr:
//    mov r0,#0xf        ;r0 = 15 0b1111
//    asr r1,r0,#1       ;r1 = 7  0b0111
//    asr r1,r0,#2       ;r1 = 3  0b0011
//    asr r1,r0,#3       ;r1 = 1  0b0001
//    asr r1,r0,#4       ;r1 = 0  0b0000

//    mov r0,#0xfffffff0 ;r0 = -16
//    asr r1,r0,#1       ;r1 = -8 0xfffffff8
//    asr r1,r0,#2       ;r1 =-4  0xfffffffc
//    asr r1,r0,#3       ;r1=-2   0xfffffffe
//    asr r1,r0,#4       ;r1=-1   0xffffffff

    bx lr

#define TEST_LABEL  0xfffffff0

asm_test_add:
    ldr r0,=TEST_LABEL
    adds r1,r0,#1
    movs r0,#0x00
    ldr r0,=TEST_LABEL
    adds r1,r0,#1
    ldr r0,=0xffffffff
    adds r1,r0,#1
    ldr r0,=0x7fffffff
    adds r1,r0,#1
    bx lr

asm_test_ldrd:
    push {r4,r5}
    //ldrd r2,r3,[r0]
    //ldrd r4,r5,[r1]
    adds r0,r2,r4
    //adc  r1,r3,r5
    pop  {r4,r5}
    bx lr

asm_test_sub:
    push {r4,r5}
    ldr r0, =p_uint64
//    ldr r2,[r0],#0x04
    ldr r3,[r0]
    ldr r4,=0x01
    ldr r5,=0x00
    subs r0,r2,r4
//    sbc  r1,r3,r5
    ldr r3,=p_uint64
//    strd r0,r1,[r3]
    pop  {r4,r5}
    bx lr

asm_test_ldr:
    ldr r0,=0x00
//    ldr r1,[r0,#4]! ;r0 = r0 + 4 then r1 = [r0]
    ldr r0,=0x00
//    ldr r1,[r0],#4 ;r1 = [r0] then r0 = r0+4
    ldr r0,=0x00
    ldr r1,=0x02
//    ldr r2,[r0,r1,lsl #2] ;r1 = [r0] then r0 = r0+4
    ldr r0,=0x00
    ldmia r0!,{r1-r3}

    bx lr

asm_test_strcpy:
//    ldrb r2,[r1],#1
//    strb r2,[r0],#1
    cmp  r2,#0
    bne asm_test_strcpy

    bx lr

asm_test_nop:
    nop
    nop
    nop
    ldr r0,=0xfffffff4
    lsls r0,r0,#29
    ldr r0,=0xfffffff4
    lsls r0,r0,#28
    ldr r0,=0xfffffff4
    lsls r0,r0,#30
    ldr r0,=0xfffffff4
    lsls r0,r0,#1

    bLx lr

asm_test_nzcv:
    ldr r0,=0xa0000000
    ldr r1,=0xa0000000
    subs r2,r1,r0 ;A-B = A+(~B+1)
                  ;= 0b1010 0000 0000 0000 0000 0000 0000 0000 +
                  ;  0b0101 1111 1111 1111 1111 1111 1111 1111 + 1
                  ;= 0b0110 0000 0000 0000 0000 0000 0000 0000
                  ;= 0b0000 0000 0000 0000 0000 0000 0000 0000
                  ;N = 0,Z = 1,C = 1,V = 0
    ldr r0,=0x80000005
    ldr r1,=0x80000004
    subs r2,r0,r1 ;A-B = A+(~B+1)
                  ;= 0b1000 0000 0000 0000 0000 0000 0000 0101 +
                  ;  0b0111 1111 1111 1111 1111 1111 1111 1011 + 1
                  ;= 0b0111 1111 1111 1111 1111 1111 1111 1100
                  ;= 0b0000 0000 0000 0000 0000 0000 0000 0001
                  ;N = 0,Z = 0,C = 1,V = 0
    ldr r0,=0x70000000
    ldr r1,=0xf0000000
    subs r2,r0,r1 ;A-B = A+(~B+1)
                  ;= 0b0111 0000 0000 0000 0000 0000 0000 0000 +
                  ;  0b0000 1111 1111 1111 1111 1111 1111 1111 + 1
                  ;= 0b0001 0000 0000 0000 0000 0000 0000 0000
                  ;= 0b1000 0000 0000 0000 0000 0000 0000 0000
                  ;N = 1,Z = 0,C = 0,V = 1
    ldr r0,=0x80000000
    ldr r1,=0x40000000
    subs r2,r0,r1;A-B = A+(~B+1)
                ;=0b1000 0000 0000 0000 0000 0000 0000 0000 +
                ; 0b1011 1111 1111 1111 1111 1111 1111 1111 + 1
                ;=0b1100 0000 0000 0000 0000 0000 0000 0000
                ;=0b0100 0000 0000 0000 0000 0000 0000 0000
                ;N = 0,Z = 0,C = 1,V = 1

    bx lr

/*
The CMP instruction subtracts the value of Operand2 from the value in Rn. This is the same as a
SUBS instruction, except that the result is discarded.
The CMN instruction adds the value of Operand2 to the value in Rn. This is the same as an ADDS
instruction, except that the result is discarded.
*/

asm_test_cmn:
    ldr r0,=0xfffffffe
//    cmn r0,#1
//    cmn r0,#2
//    cmn r0,#3

    bx lr

asm_rev:
    mov r1,r0
    rev r0,r1
    bx lr


asm_test_stm:
//    ldmia   r0,  {r1-r3}
//    mov     r1, #1
//    mov     r2, #2
//    mov     r3, #3
//    stmia   r0,  {r1-r3}

    bx lr

asm_test_ldr_adr:
    adr     r0,asm_test_ldr_adr_lable1 - 1
asm_test_ldr_adr_lable1:    
    ldr     r1,=xIsPrivileged  
//    adr.w     r2,xIsPrivileged
    bx lr


xIsPrivileged:
    mrs r0, control     /* r0 = CONTROL. */
//    tst r0, #1          /* Perform r0 & 1 (bitwise AND) and update the conditions flag. */
//    itt ne
//    movne r0, #0        /* CONTROL[0]!=0. Return false to indicate that the processor is not privileged. */
//    movne r0, #1        /* CONTROL[0]==0. Return true to indicate that the processor is privileged. */
    bx lr               /* Return. */

    
    END

