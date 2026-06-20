.balign 4
.global TestClass$test1
.section .text
.arm
TestClass$test1:
TestClass$test1$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r6, #1
	mov r5, r0
	mov r4, r0
	add r0, r6, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	str r6, [r0]
	str r0, [r5]
	ldr r4, [r4]
	ldr r5, [r4]
	movw r1, #0
	movw r0, #0
	cmp r1, r0
	bge TestClass$test1$L101
TestClass$test1$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit
TestClass$test1$L101:
	movw r0, #0
	cmp r0, r5
	bge TestClass$test1$L100
TestClass$test1$L102:
	movw r1, #0
	movw r0, #1
	add r0, r1, r0
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global TestClass$test0
.section .text
.arm
TestClass$test0:
TestClass$test0$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r6, #1
	mov r5, r0
	mov r4, r0
	add r0, r6, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	str r6, [r0]
	str r0, [r5]
	ldr r0, [r4]
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

.balign 4
.global TestClass$test2
.section .text
.arm
TestClass$test2:
TestClass$test2$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, r0
	mov r5, r1
	mov r6, #1
	add r0, r6, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	str r6, [r0]
	ldr r7, [r0]
	mov r6, r0
	movw r1, #0
	movw r0, #0
	cmp r1, r0
	bge TestClass$test2$L101
TestClass$test2$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit
TestClass$test2$L101:
	movw r0, #0
	cmp r0, r7
	bge TestClass$test2$L100
TestClass$test2$L102:
	movw r1, #0
	movw r0, #1
	add r0, r1, r0
	movw r1, #4
	mul r0, r0, r1
	str r5, [r6, r0]
	ldr r0, [r4]
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
