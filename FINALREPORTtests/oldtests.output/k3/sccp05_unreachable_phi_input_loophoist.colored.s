.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r1, #1
	movw r0, #0
	cmp r1, r0
	bne main$L102
main$L103:
	bl getint
main$L104:
	movw r1, #3
	cmp r0, r1
	beq main$L107
main$L108:
	mov r0, #5
main$L109:
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	mov r0, #4
	b main$L109
main$L102:
	mov r0, #3
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
