.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	movw r113, #4
	str r113, [r100]
	movw r114, #30
	str r114, [r100, #8]
	movw r115, #100
	str r115, [r100, #12]
	ldr r10601, [r100]
	ldr r10801, [r100, #8]
	ldr r11101, [r100, #12]
	add r10901, r10601, r10801
	add r11201, r10901, r11101
	mov r0, r11201
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r152, #20
	mov r0, r152
	bl malloc
	mov r10301, r0
	ldr r153, =A$f
	str r153, [r10301, #16]
	mov r10001, r10301
	movw r154, #20
	mov r0, r154
	bl malloc
	mov r10401, r0
	add r12101, r10001, #16
	mov r12501, r10001
	add r12701, r10001, #4
	ldr r155, =B$f
	str r155, [r10401, #16]
	mov r10101, r10401
	ldr r10701, [r12101]
	mov r0, r10001
	blx r10701
	ldr r10901, [r10101, #16]
	mov r0, r10101
	blx r10901
	mov r13001, r10101
	add r13301, r10101, #8
	add r13601, r10101, #12
	mov r10002, r10101
	mov r14201, r10101
	add r14501, r10101, #8
	add r14801, r10101, #12
	ldr r12601, [r12501]
	ldr r12801, [r12701]
	ldr r13101, [r13001]
	ldr r13401, [r13301]
	ldr r13701, [r13601]
	mov r14001, r10002
	add r12901, r12601, r12801
	add r13201, r12901, r13101
	add r13501, r13201, r13401
	add r11101, r13501, r13701
	mov r0, r11101
	bl putint
	movw r156, #10
	mov r0, r156
	bl putch
	ldr r14101, [r14001]
	ldr r14301, [r14201]
	ldr r14601, [r14501]
	ldr r14901, [r14801]
	add r14401, r14101, r14301
	add r14701, r14401, r14601
	add r11401, r14701, r14901
	mov r0, r11401
	bl putint
	movw r157, #10
	mov r0, r157
	bl putch
	movw r158, #1
	mov r0, r158
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global A$f
.section .text
.arm
A$f:
A$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	movw r104, #1
	str r104, [r100]
	movw r105, #2
	str r105, [r100, #4]
	movw r106, #0
	mov r0, r106
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
