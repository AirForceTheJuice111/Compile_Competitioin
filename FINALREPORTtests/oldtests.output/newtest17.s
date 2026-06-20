.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10001, r101
	ldr r10301, [r10001, #4]
	mov r10601, r10301
	ldr r10401, [r10301]
	movw r116, #1
	movw r117, #0
	cmp r116, r117
	bge main$L101
main$L100:
	movw r118, #65535
	movt r118, #65535
	mov r0, r118
	bl exit
main$L101:
	movw r119, #1
	cmp r119, r10401
	bge main$L100
main$L102:
	movw r112, #1
	movw r113, #1
	add r10901, r112, r113
	movw r114, #4
	mul r11001, r10901, r114
	movw r115, #1
	str r115, [r10601, r11001]
main$L104:
	movw r120, #0
	mov r0, r120
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
