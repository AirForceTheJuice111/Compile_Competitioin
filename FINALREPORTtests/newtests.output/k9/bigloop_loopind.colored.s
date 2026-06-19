.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r5, #0
	movw r3, #3000
	mov r0, #0
	add r1, r5, #1
	add r4, r5, #1
	movw r2, #2000
	mul r2, r1, r2
	movw r1, #3
	mul r6, r4, r1
main$L102:
	movw r1, #2
	mul r1, r3, r1
	cmp r5, r1
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
	add r5, r5, #1
	mov r4, #0
	add r1, r4, #1
	mul r1, r1, r6
main$L107:
	cmp r4, r2
	blt main$L108
main$L109:
	add r2, r2, #2000
	add r6, r6, #3
	b main$L102
main$L108:
	add r4, r4, #1
	add r0, r0, r1
	add r1, r1, r6
	movw r7, #17
	sdiv r7, r4, r7
	add r0, r0, r7
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
