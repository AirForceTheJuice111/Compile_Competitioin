.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r10001, r0
	movw r109, #0
	cmp r10001, r109
	bne main$L102
main$L103:
	mov r106, #2
	mov r10103, r106
main$L104:
	mov r0, r10103
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	mov r105, #1
	mov r10103, r105
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
