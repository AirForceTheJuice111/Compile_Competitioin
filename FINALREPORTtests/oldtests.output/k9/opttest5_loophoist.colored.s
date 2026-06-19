.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r1, #10
	mov r2, #0
main$L102:
	movw r3, #0
	cmp r1, r3
	bgt main$L103
main$L104:
	mov r0, r4
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r2, r2, #1
	movw r3, #3
	mul r3, r3, r2
main$L107:
	movw r5, #0
	cmp r0, r5
	bgt main$L108
main$L109:
	sub r1, r1, #1
	b main$L102
main$L108:
	movw r4, #3
	cmp r2, r4
	bgt main$L112
main$L113:
	mov r4, r0
main$L114:
	sub r0, r0, #1
	b main$L107
main$L112:
	mov r4, r3
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
