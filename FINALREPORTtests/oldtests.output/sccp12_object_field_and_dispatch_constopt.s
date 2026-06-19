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
	add r1030101, r101, #2
	mov r0, r1030101
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
	movw r117, #8
	mov r0, r117
	bl malloc
	mov r1030101, r0
	ldr r118, =Child$value
	str r118, [r1030101, #4]
	mov r1000101, r1030101
	ldr r1050101, [r1000101, #4]
	movw r119, #10
	mov r0, r1000101
	mov r1, r119
	blx r1050101
	mov r1010101, r0
main$L102:
	str r1010101, [r1000101]
main$L104:
	ldr r1140101, [r1000101]
	mov r0, r1140101
	sub sp, fp, #36
	add sp, sp, #4
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
	mov r100, r0
	mov r101, r1
	add r1030101, r101, #1
	mov r0, r1030101
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
