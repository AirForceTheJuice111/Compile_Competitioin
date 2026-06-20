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
	movw r104, #0
	mov r0, r104
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
