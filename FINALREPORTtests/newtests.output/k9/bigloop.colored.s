.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r3, #3000
	mov r0, #0
	mov r2, #0
main$L102:
	movw r1, #2
	mul r1, r3, r1
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
	movw r4, #2000
	mul r4, r2, r4
	cmp r1, r4
	blt main$L108
main$L109:
	b main$L102
main$L108:
	add r1, r1, #1
	movw r4, #3
	mul r4, r2, r4
	mul r5, r1, r4
	movw r4, #17
	sdiv r4, r1, r4
	add r0, r0, r5
	add r0, r0, r4
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
