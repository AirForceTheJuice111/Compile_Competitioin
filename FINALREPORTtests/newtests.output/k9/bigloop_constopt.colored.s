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
	mov r2, #0
main$L102:
	movw r1, #6000
	cmp r2, r1
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
	add r2, r2, #1
	mov r1, #0
main$L107:
	movw r3, #2000
	mul r3, r2, r3
	cmp r1, r3
	blt main$L108
main$L109:
	b main$L102
main$L108:
	add r1, r1, #1
	movw r3, #3
	mul r3, r2, r3
	mul r4, r1, r3
	movw r3, #17
	sdiv r3, r1, r3
	add r0, r0, r4
	add r0, r0, r3
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
