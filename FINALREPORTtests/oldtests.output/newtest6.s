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
	mov r10401, #1
	mov r11401, r101
	mov r11501, r101
	add r11201, r10401, #1
	movw r121, #4
	mul r10801, r11201, r121
	mov r0, r10801
	bl malloc
	mov r10501, r0
	str r10401, [r10501]
	str r10501, [r11401]
	ldr r10601, [r11501]
	ldr r10701, [r10601]
	mov r11101, r10601
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
	cmp r128, r10701
	bge TestClass$test1$L100
TestClass$test1$L102:
	movw r122, #0
	movw r123, #1
	add r11701, r122, r123
	movw r124, #4
	mul r11801, r11701, r124
	ldr r12001, [r11101, r11801]
	mov r0, r12001
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
	mov r10001, r0
	mov r10301, #10
	mov r11701, r101
	movw r124, #3
	str r124, [r10001]
	movw r125, #1
	str r125, [r10001, #4]
	movw r126, #2
	str r126, [r10001, #8]
	movw r127, #3
	str r127, [r10001, #12]
	mov r11001, r10001
	add r11501, r10301, #1
	movw r128, #4
	mul r10701, r11501, r128
	mov r0, r10701
	bl malloc
	mov r10401, r0
	str r10301, [r10401]
	str r10401, [r11701]
	ldr r10501, [r10001]
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
	cmp r135, r10501
	bge main$L100
main$L102:
	movw r129, #16960
	movt r129, #15
	movw r130, #1
	add r11901, r129, r130
	movw r131, #4
	mul r12001, r11901, r131
	ldr r12201, [r11001, r12001]
	mov r0, r12201
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
	mov r10401, #1
	mov r11001, r101
	mov r11101, r101
	add r10801, r10401, #1
	movw r113, #4
	mul r10601, r10801, r113
	mov r0, r10601
	bl malloc
	mov r10501, r0
	str r10401, [r10501]
	str r10501, [r11001]
	ldr r11201, [r11101]
	mov r0, r11201
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
	mov r10401, #1
	add r11201, r10401, #1
	movw r120, #4
	mul r10801, r11201, r120
	mov r0, r10801
	bl malloc
	mov r10501, r0
	str r10401, [r10501]
	mov r10601, r10501
	ldr r10701, [r10601]
	mov r11101, r10601
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
	cmp r127, r10701
	bge TestClass$test2$L100
TestClass$test2$L102:
	movw r121, #0
	movw r122, #1
	add r11501, r121, r122
	mov r11801, r101
	movw r123, #4
	mul r11601, r11501, r123
	str r100, [r11101, r11601]
	ldr r11901, [r11801]
	mov r0, r11901
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
