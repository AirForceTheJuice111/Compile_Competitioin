.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	movw r0, #4
	bl malloc
	mov r10, #0
	str r10, [fp, #-44]
	mov r10, r0
	str r10, [fp, #-40]
	movw r0, #4
	bl malloc
	ldr r9, [fp, #-40]
	ldr r1, [r9]
	ldr r0, [r0]
	cmp r1, r0
	beq main$L102
	ldr r9, [fp, #-44]
	mov r0, r9
main$L103:
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	mov r0, #1
	b main$L103

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
