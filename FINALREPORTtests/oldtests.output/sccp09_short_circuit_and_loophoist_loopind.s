.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10001, #0
	movw r105, #0
	movw r106, #0
	cmp r105, r106
	bne main$L100
main$L106:
	mov r10102, #2
	mov r10103, r10102
main$L107:
	mov r0, r10103
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L100:
	movw r104, #10
	sdiv r10301, r104, r10001
	movw r107, #0
	cmp r10301, r107
	beq main$L106
main$L105:
	mov r10101, #1
	mov r10103, r10101
	b main$L107

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
