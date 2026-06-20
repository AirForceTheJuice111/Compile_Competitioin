.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	ldr r1020101, [r100, #4]
	mov r1050101, r1020101
	ldr r1030101, [r1020101]
main$L101:
	movw r114, #1
	cmp r114, r1030101
	bge main$L100
main$L102:
	movw r113, #1
	str r113, [r1050101, #8]
main$L104:
	movw r115, #0
	mov r0, r115
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L100:
	movw r116, #65535
	movt r116, #65535
	mov r0, r116
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
