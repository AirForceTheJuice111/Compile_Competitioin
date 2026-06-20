.balign 4
.global Base$value
.section .text
.arm
Base$value:
Base$value$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	add r0, r1, #1
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
	add r0, r1, #2
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
	movw r0, #8
	bl malloc
	ldr r1, =Child$value
	str r1, [r0, #4]
	mov r4, r0
	ldr r2, [r4, #4]
	movw r1, #10
	mov r0, r4
	blx r2
	movw r2, #1
	movw r1, #0
	cmp r2, r1
	bne main$L102
main$L103:
	movw r0, #0
	str r0, [r4]
main$L104:
	ldr r0, [r4]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	str r0, [r4]
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
