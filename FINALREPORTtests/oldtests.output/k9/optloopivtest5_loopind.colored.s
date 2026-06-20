.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r1, #10
	mov r5, #0
	movw r2, #0
	cmp r0, r2
	bgt main$L107
main$L108:
	mov r5, #4
main$L109:
	mov r0, r5
	bl putint
	movw r0, #10
	bl putch
	mov r0, r5
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	movw r0, #3
	mul r0, r1, r0
	add r4, r0, #2
main$L104:
	movw r0, #2
	cmp r4, r0
	bgt main$L105
main$L106:
	b main$L109
main$L105:
	mov r0, r4
	bl putint
	sub r6, r4, #3
	movw r0, #32
	bl putch
	mov r5, r4
	mov r4, r6
	b main$L104

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
