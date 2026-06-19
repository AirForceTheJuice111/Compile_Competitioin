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
	movw r106, #0
	cmp r10001, r106
	bne main$L102
main$L103:
	mov r10102, #42
	mov r10103, r10102
main$L104:
	add r10501, r10103, #1
	mov r0, r10501
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	mov r10101, #42
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
