.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, #10
	mov r6, #0
	bl getint
main$L102:
	movw r1, #0
	cmp r4, r1
	bgt main$L103
main$L104:
	mov r0, r5
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r6, r6, #1
main$L107:
	movw r1, #0
	cmp r0, r1
	bgt main$L108
main$L109:
	sub r4, r4, #1
	b main$L102
main$L108:
	movw r1, #3
	cmp r6, r1
	bgt main$L112
main$L113:
	mov r5, r0
main$L114:
	sub r0, r0, #1
	b main$L107
main$L112:
	movw r1, #3
	mul r5, r1, r6
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
