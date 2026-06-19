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
	mov r1040101, #1
	mov r1140101, r101
	mov r1150101, r101
	add r1120101, r1040101, #1
	movw r121, #4
	mul r1080101, r1120101, r121
	mov r0, r1080101
	bl malloc
	mov r1050101, r0
	str r1040101, [r1050101]
	str r1050101, [r1140101]
	ldr r1060101, [r1150101]
	ldr r1070101, [r1060101]
	mov r1110101, r1060101
	movw r125, #0
	movw r126, #0
	cmp r125, r126
	bge TestClass$test1$L101
TestClass$test1$L100:
	movw r127, #65535
	movt r127, #65535
	mov r0, r127
	bl exit
TestClass$test1$L101:
	movw r128, #0
	cmp r128, r1070101
	bge TestClass$test1$L100
TestClass$test1$L102:
	movw r122, #0
	movw r123, #1
	add r1170101, r122, r123
	movw r124, #4
	mul r1180101, r1170101, r124
	ldr r1200101, [r1110101, r1180101]
	mov r0, r1200101
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
	movw r123, #16
	mov r0, r123
	bl malloc
	mov r1000101, r0
	mov r1030101, #10
	mov r1170101, r101
	movw r124, #3
	str r124, [r1000101]
	movw r125, #1
	str r125, [r1000101, #4]
	movw r126, #2
	str r126, [r1000101, #8]
	movw r127, #3
	str r127, [r1000101, #12]
	mov r1100101, r1000101
	add r1150101, r1030101, #1
	movw r128, #4
	mul r1070101, r1150101, r128
	mov r0, r1070101
	bl malloc
	mov r1040101, r0
	str r1030101, [r1040101]
	str r1040101, [r1170101]
	ldr r1050101, [r1000101]
	movw r132, #16960
	movt r132, #15
	movw r133, #0
	cmp r132, r133
	bge main$L101
main$L100:
	movw r134, #65535
	movt r134, #65535
	mov r0, r134
	bl exit
main$L101:
	movw r135, #16960
	movt r135, #15
	cmp r135, r1050101
	bge main$L100
main$L102:
	movw r129, #16960
	movt r129, #15
	movw r130, #1
	add r1190101, r129, r130
	movw r131, #4
	mul r1200101, r1190101, r131
	ldr r1220101, [r1100101, r1200101]
	mov r0, r1220101
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
	mov r101, r0
	mov r100, r1
	mov r1040101, #1
	mov r1100101, r101
	mov r1110101, r101
	add r1080101, r1040101, #1
	movw r113, #4
	mul r1060101, r1080101, r113
	mov r0, r1060101
	bl malloc
	mov r1050101, r0
	str r1040101, [r1050101]
	str r1050101, [r1100101]
	ldr r1120101, [r1110101]
	mov r0, r1120101
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
	mov r101, r0
	mov r100, r1
	mov r1040101, #1
	add r1120101, r1040101, #1
	movw r120, #4
	mul r1080101, r1120101, r120
	mov r0, r1080101
	bl malloc
	mov r1050101, r0
	str r1040101, [r1050101]
	mov r1060101, r1050101
	ldr r1070101, [r1060101]
	mov r1110101, r1060101
	movw r124, #0
	movw r125, #0
	cmp r124, r125
	bge TestClass$test2$L101
TestClass$test2$L100:
	movw r126, #65535
	movt r126, #65535
	mov r0, r126
	bl exit
TestClass$test2$L101:
	movw r127, #0
	cmp r127, r1070101
	bge TestClass$test2$L100
TestClass$test2$L102:
	movw r121, #0
	movw r122, #1
	add r1150101, r121, r122
	mov r1180101, r101
	movw r123, #4
	mul r1160101, r1150101, r123
	str r100, [r1110101, r1160101]
	ldr r1190101, [r1180101]
	mov r0, r1190101
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
