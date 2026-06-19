.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	ldr r102010101, [r100, #4]
	mov r105010101, r102010101
	ldr r103010101, [r102010101]
	movw r115, #1
	movw r116, #0
	cmp r115, r116
	bge main$L101
main$L100:
	movw r117, #65535
	movt r117, #65535
	mov r0, r117
	bl exit
main$L101:
	movw r118, #1
	cmp r118, r103010101
	bge main$L100
main$L102:
	movw r111, #1
	movw r112, #1
	add r108010101, r111, r112
	movw r113, #4
	mul r109010101, r108010101, r113
	movw r114, #1
	str r114, [r105010101, r109010101]
main$L104:
	movw r119, #0
	mov r0, r119
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
