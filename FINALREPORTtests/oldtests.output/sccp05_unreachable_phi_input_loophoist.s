.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r105, #1
	movw r106, #0
	cmp r105, r106
	bne main$L102
main$L103:
	bl getint
	mov r10002, r0
	mov r10003, r10002
main$L104:
	movw r107, #3
	cmp r10003, r107
	beq main$L107
main$L108:
	mov r10102, #5
	mov r10103, r10102
main$L109:
	mov r0, r10103
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	mov r10101, #4
	mov r10103, r10101
	b main$L109
main$L102:
	mov r10001, #3
	mov r10003, r10001
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
