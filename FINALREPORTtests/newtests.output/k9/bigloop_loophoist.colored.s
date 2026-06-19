.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r1, #3000
	mov r0, #0
	mov r6, #0
	mov r7, #0
	movw r2, #2
	mul r4, r1, r2
main$L102:
	cmp r7, r4
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
	add r7, r7, #1
	movw r1, #2000
	mul r3, r7, r1
	movw r1, #3
	mul r8, r7, r1
	mov r5, r6
main$L107:
	cmp r5, r3
	blt main$L108
main$L109:
	b main$L102
main$L108:
	add r5, r5, #1
	mul r2, r5, r8
	movw r1, #17
	sdiv r1, r5, r1
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
