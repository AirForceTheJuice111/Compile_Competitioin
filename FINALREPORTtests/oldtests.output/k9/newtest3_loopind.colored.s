.balign 4
.global TestClass$test1
.section .text
.arm
TestClass$test1:
TestClass$test1$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
TestClass$test1$L101:
	movw r0, #0
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
	movw r0, #16
	bl malloc
	mov r5, r0
	mov r6, #10
	movw r0, #3
	str r0, [r5]
	movw r0, #1
	str r0, [r5, #4]
	movw r0, #2
	str r0, [r5, #8]
	movw r0, #3
	str r0, [r5, #12]
	add r0, r6, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	str r6, [r0]
	str r0, [r4]
	ldr r4, [r5]
	movw r1, #16960
	movt r1, #15
	movw r0, #0
	cmp r1, r0
	bge main$L101
main$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L101:
	movw r0, #16960
	movt r0, #15
	cmp r0, r4
	bge main$L100
main$L102:
	movw r1, #16960
	movt r1, #15
	movw r0, #1
	add r0, r1, r0
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r5, r0]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
