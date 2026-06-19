.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
B$f$L101:
	movw r102, #0
	mov r0, r102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global A$f
.section .text
.arm
A$f:
A$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
A$f$L101:
	movw r102, #0
	mov r0, r102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r113, #12
	mov r0, r113
	bl malloc
	mov r1030101, r0
	ldr r114, =A$f
	str r114, [r1030101, #8]
	mov r1000101, r1030101
	movw r115, #1
	str r115, [r1000101]
	movw r116, #12
	mov r0, r116
	bl malloc
	mov r1040101, r0
	ldr r117, =B$f
	str r117, [r1040101, #8]
	mov r1010101, r1040101
	movw r118, #1
	str r118, [r1010101, #4]
main$L101:
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
