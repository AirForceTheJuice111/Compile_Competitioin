.balign 4
.global TestClass$test1
.section .text
.arm
TestClass$test1:
TestClass$test1$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r101, r0
	mov r100, r1
TestClass$test1$L101:
	movw r106, #0
	mov r0, r106
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r125, #16
	mov r0, r125
	bl malloc
	mov r1000101, r0
	mov r1170101, r101
	movw r126, #3
	str r126, [r1000101]
	movw r127, #1
	str r127, [r1000101, #4]
	movw r128, #2
	str r128, [r1000101, #8]
	movw r129, #3
	str r129, [r1000101, #12]
	movw r130, #44
	mov r0, r130
	bl malloc
	mov r1040101, r0
	movw r131, #10
	str r131, [r1040101]
	str r1040101, [r1170101]
	mov r1100101, r1000101
	ldr r1050101, [r1000101]
main$L101:
	movw r133, #16960
	movt r133, #15
	cmp r133, r1050101
	bge main$L100
main$L102:
	movw r132, #2308
	movt r132, #61
	ldr r1220101, [r1100101, r132]
	mov r0, r1220101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L100:
	movw r134, #65535
	movt r134, #65535
	mov r0, r134
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
