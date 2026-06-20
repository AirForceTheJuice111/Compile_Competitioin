.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r105, #2
	movw r106, #3
	add r10001, r105, r106
	movw r107, #1
	movw r108, #0
	cmp r107, r108
	bne main$L102
main$L103:
	bl getint
	mov r10102, r0
	mov r10103, r10102
main$L104:
	mov r0, r10103
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	mul r10101, r10001, r10001
	mov r10103, r10101
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
