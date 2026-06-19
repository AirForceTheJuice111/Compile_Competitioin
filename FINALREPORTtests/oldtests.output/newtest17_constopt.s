.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1000101, r101
	ldr r1030101, [r1000101, #4]
	mov r1060101, r1030101
	ldr r1040101, [r1030101]
main$L101:
	movw r115, #1
	cmp r115, r1040101
	bge main$L100
main$L102:
	movw r114, #1
	str r114, [r1060101, #8]
main$L104:
	movw r116, #0
	mov r0, r116
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L100:
	movw r117, #65535
	movt r117, #65535
	mov r0, r117
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
