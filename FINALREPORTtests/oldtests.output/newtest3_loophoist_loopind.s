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
	mov r100010101, r0
	mov r103010101, #10
	mov r117010101, r101
	movw r124, #3
	str r124, [r100010101]
	movw r125, #1
	str r125, [r100010101, #4]
	movw r126, #2
	str r126, [r100010101, #8]
	movw r127, #3
	str r127, [r100010101, #12]
	mov r110010101, r100010101
	add r115010101, r103010101, #1
	movw r128, #4
	mul r107010101, r115010101, r128
	mov r0, r107010101
	bl malloc
	mov r104010101, r0
	str r103010101, [r104010101]
	str r104010101, [r117010101]
	ldr r105010101, [r100010101]
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
	cmp r135, r105010101
	bge main$L100
main$L102:
	movw r129, #16960
	movt r129, #15
	movw r130, #1
	add r119010101, r129, r130
	movw r131, #4
	mul r120010101, r119010101, r131
	ldr r122010101, [r110010101, r120010101]
	mov r0, r122010101
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
