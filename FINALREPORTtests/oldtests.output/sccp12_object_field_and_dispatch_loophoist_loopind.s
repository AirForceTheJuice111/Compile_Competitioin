.balign 4
.global Base$value
.section .text
.arm
Base$value:
Base$value$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	mov r101, r1
	add r103010101, r101, #1
	mov r0, r103010101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global Child$value
.section .text
.arm
Child$value:
Child$value$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	mov r101, r1
	add r103010101, r101, #2
	mov r0, r103010101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r115, #8
	mov r0, r115
	bl malloc
	mov r103010101, r0
	ldr r116, =Child$value
	str r116, [r103010101, #4]
	mov r100010101, r103010101
	ldr r105010101, [r100010101, #4]
	movw r117, #10
	mov r0, r100010101
	mov r1, r117
	blx r105010101
	mov r101010101, r0
	movw r119, #1
	movw r120, #0
	cmp r119, r120
	bne main$L102
main$L103:
	movw r118, #0
	str r118, [r100010101]
main$L104:
	ldr r114010101, [r100010101]
	mov r0, r114010101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	str r101010101, [r100010101]
	b main$L104

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
