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

.balign 4
.global TestClass$test0
.section .text
.arm
TestClass$test0:
TestClass$test0$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r101, r0
	mov r100, r1
	movw r115, #8
	mov r0, r115
	bl malloc
	mov r1050101, r0
	mov r1100101, r101
	mov r1110101, r101
	movw r116, #1
	str r116, [r1050101]
	str r1050101, [r1100101]
	ldr r1120101, [r1110101]
	mov r0, r1120101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global TestClass$test1
.section .text
.arm
TestClass$test1:
TestClass$test1$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r101, r0
	mov r100, r1
	movw r123, #8
	mov r0, r123
	bl malloc
	mov r1050101, r0
	mov r1140101, r101
	mov r1150101, r101
	movw r124, #1
	str r124, [r1050101]
	str r1050101, [r1140101]
	ldr r1060101, [r1150101]
	ldr r1070101, [r1060101]
	mov r1110101, r1060101
TestClass$test1$L101:
	movw r125, #0
	cmp r125, r1070101
	bge TestClass$test1$L100
TestClass$test1$L102:
	ldr r1200101, [r1110101, #4]
	mov r0, r1200101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
TestClass$test1$L100:
	movw r126, #65535
	movt r126, #65535
	mov r0, r126
	bl exit

.balign 4
.global TestClass$test2
.section .text
.arm
TestClass$test2:
TestClass$test2$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r101, r0
	mov r100, r1
	movw r122, #8
	mov r0, r122
	bl malloc
	mov r1050101, r0
	movw r123, #1
	str r123, [r1050101]
	mov r1060101, r1050101
	ldr r1070101, [r1060101]
	mov r1110101, r1060101
TestClass$test2$L101:
	movw r124, #0
	cmp r124, r1070101
	bge TestClass$test2$L100
TestClass$test2$L102:
	str r100, [r1110101, #4]
	ldr r1190101, [r101]
	mov r0, r1190101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
TestClass$test2$L100:
	movw r125, #65535
	movt r125, #65535
	mov r0, r125
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
