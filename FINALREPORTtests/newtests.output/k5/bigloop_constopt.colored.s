.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, #0
	mov r4, #0
main$L102:
	movw r1, #6000
	cmp r4, r1
	blt main$L103
main$L104:
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r4, r4, #1
	mov r3, #0
main$L107:
	movw r1, #2000
	mul r1, r4, r1
	cmp r3, r1
	blt main$L108
main$L109:
	b main$L102
main$L108:
	add r3, r3, #1
	movw r1, #3
	mul r1, r4, r1
	mul r2, r3, r1
	movw r1, #17
	sdiv r1, r3, r1
	add r0, r0, r2
	add r0, r0, r1
	b main$L107

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
