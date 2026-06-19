.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10001, #0
	movw r104, #0
	movw r105, #0
	cmp r104, r105
	bne main$L102
main$L103:
	mov r10102, #7
	mov r10103, r10102
main$L104:
	mov r0, r10103
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	movw r103, #10
	sdiv r10101, r103, r10001
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
