.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1, #1
	mov r0, #2
	mov r2, #100
	mov r2, #0
	add r0, r1, r0
	movw r1, #1
	movw r0, #0
	cmp r1, r0
	beq main$L102
main$L103:
main$L104:
main$L106:
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
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
