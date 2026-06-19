.balign 4
.global main
.section .text
.arm
main:
main$L106:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r124, #16
	mov r0, r124
	bl malloc
	mov r1000101, r0
	movw r125, #3
	str r125, [r1000101]
	movw r126, #1
	str r126, [r1000101, #4]
	movw r127, #2
	str r127, [r1000101, #8]
	movw r128, #3
	str r128, [r1000101, #12]
	mov r1070101, r1000101
	ldr r1030101, [r1000101]
main$L100:
	movw r129, #65535
	movt r129, #65535
	mov r0, r129
	bl exit

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
