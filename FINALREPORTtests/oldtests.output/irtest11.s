.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r110, #0
	movw r111, #123
	sub r10201, r110, r111
	mov r10501, r100
	ldr r10301, [r100]
	movw r114, #0
	cmp r10201, r114
	bge main$L101
main$L100:
	movw r115, #65535
	movt r115, #65535
	mov r0, r115
	bl exit
main$L101:
	cmp r10201, r10301
	bge main$L100
main$L102:
	add r10701, r10201, #1
	movw r112, #4
	mul r10801, r10701, r112
	movw r113, #4
	str r113, [r10501, r10801]
main$L104:
	movw r116, #0
	mov r0, r116
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
