.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1, #0
	mov r0, #0
main$L102:
	movw r2, #10
	cmp r1, r2
	blt main$L103
main$L104:
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r2, #5
	cmp r1, r2
	bgt main$L107
main$L108:
main$L109:
	add r1, r1, #1
	b main$L102
main$L107:
	movw r2, #100
	movw r0, #200
	add r0, r2, r0
	b main$L109

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
