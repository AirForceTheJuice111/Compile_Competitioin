.balign 4
.global main
.section .text
.arm
main:
main$L106:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r126, #20
	mov r0, r126
	bl malloc
	mov r1000101, r0
	movw r127, #4
	str r127, [r1000101]
	movw r128, #1
	str r128, [r1000101, #4]
	movw r129, #2
	str r129, [r1000101, #8]
	movw r130, #3
	str r130, [r1000101, #12]
	movw r131, #4
	str r131, [r1000101, #16]
	mov r1070101, r1000101
	ldr r1030101, [r1000101]
main$L101:
	movw r133, #2
	cmp r133, r1030101
	bge main$L100
main$L102:
	movw r132, #7
	str r132, [r1070101, #12]
	mov r1090101, r1000101
	ldr r1040101, [r1000101]
main$L104:
	movw r134, #2
	cmp r134, r1040101
	bge main$L103
main$L105:
	ldr r1230101, [r1090101, #12]
	mov r0, r1230101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r135, #65535
	movt r135, #65535
	mov r0, r135
	bl exit
main$L100:
	movw r136, #65535
	movt r136, #65535
	mov r0, r136
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
