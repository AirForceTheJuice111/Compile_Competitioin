.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #24
	add fp, sp, #56
	mov r10, #10
	str r10, [fp, #-56]
	mov r10, #0
	str r10, [fp, #-44]
	bl getint
	mov r10, r0
	str r10, [fp, #-40]
	movw r0, #3
	ldr r9, [fp, #-56]
	mul r0, r9, r0
	add r0, r0, #2
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-48]
	mov r10, r0
	str r10, [fp, #-52]
main$L102:
	movw r0, #2
	ldr r9, [fp, #-52]
	cmp r9, r0
	bgt main$L103
main$L104:
	ldr r9, [fp, #-48]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	ldr r9, [fp, #-48]
	mov r0, r9
	sub sp, fp, #56
	add sp, sp, #24
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r0, #0
	ldr r9, [fp, #-40]
	cmp r9, r0
	bgt main$L107
main$L108:
	mov r10, #4
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
main$L109:
	ldr r9, [fp, #-52]
	sub r10, r9, #3
	str r10, [fp, #-52]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-52]
	b main$L102
main$L107:
	ldr r9, [fp, #-52]
	mov r0, r9
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-48]
	b main$L109

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
