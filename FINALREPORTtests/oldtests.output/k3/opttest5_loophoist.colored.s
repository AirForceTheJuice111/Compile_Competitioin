.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #24
	add fp, sp, #56
	bl getint
	mov r1, r0
	mov r10, #10
	str r10, [fp, #-56]
	mov r0, #0
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-48]
	mov r2, r1
main$L102:
	movw r1, #0
	ldr r9, [fp, #-56]
	cmp r9, r1
	bgt main$L103
main$L104:
	ldr r9, [fp, #-48]
	mov r0, r9
	sub sp, fp, #56
	add sp, sp, #24
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r0, #1
	str r10, [fp, #-44]
	movw r0, #3
	ldr r10, [fp, #-44]
	mul r10, r0, r10
	str r10, [fp, #-52]
	ldr r9, [fp, #-48]
	mov r0, r9
	mov r1, r2
main$L107:
	movw r2, #0
	cmp r1, r2
	bgt main$L108
main$L109:
	ldr r9, [fp, #-56]
	sub r10, r9, #1
	str r10, [fp, #-56]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	mov r10, r0
	str r10, [fp, #-48]
	mov r2, r1
	ldr r9, [fp, #-44]
	mov r0, r9
	b main$L102
main$L108:
	movw r0, #3
	ldr r9, [fp, #-44]
	cmp r9, r0
	bgt main$L112
main$L113:
	mov r0, r1
main$L114:
	sub r1, r1, #1
	b main$L107
main$L112:
	ldr r9, [fp, #-52]
	mov r0, r9
	b main$L114

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
