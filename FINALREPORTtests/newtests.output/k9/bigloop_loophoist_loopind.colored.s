.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r6, #0
	movw r2, #3000
	mov r0, #0
	mov r5, #0
	add r1, r6, #1
	add r4, r6, #1
	movw r3, #2
	mul r3, r2, r3
	movw r2, #2000
	mul r2, r1, r2
	movw r1, #3
	mul r8, r4, r1
main$L102:
	cmp r6, r3
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
	add r6, r6, #1
	add r1, r5, #1
	mul r1, r1, r8
	mov r4, r5
main$L107:
	cmp r4, r2
	blt main$L108
main$L109:
	add r2, r2, #2000
	add r8, r8, #3
	b main$L102
main$L108:
	add r4, r4, #1
	add r7, r0, r1
	add r1, r1, r8
	movw r0, #17
	sdiv r0, r4, r0
	add r0, r7, r0
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
