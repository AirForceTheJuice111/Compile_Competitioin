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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
