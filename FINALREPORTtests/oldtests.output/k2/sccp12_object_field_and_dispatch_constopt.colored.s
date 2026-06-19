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
	sub sp, sp, #12
	add fp, sp, #44
	movw r0, #8
	bl malloc
	ldr r1, =Child$value
	str r1, [r0, #4]
	mov r10, r0
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r10, [r9, #4]
	str r10, [fp, #-44]
	movw r1, #10
	ldr r9, [fp, #-40]
	mov r0, r9
	ldr r9, [fp, #-44]
	blx r9
main$L102:
	ldr r10, [fp, #-40]
	str r0, [r10]
main$L104:
	ldr r9, [fp, #-40]
	ldr r0, [r9]
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr

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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
