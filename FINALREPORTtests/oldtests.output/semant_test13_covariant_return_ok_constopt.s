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
	movw r107, #4
	mov r0, r107
	bl malloc
	mov r1020101, r0
	mov r0, r1020101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

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
	movw r107, #4
	mov r0, r107
	bl malloc
	mov r1020101, r0
	mov r0, r1020101
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
	movw r113, #4
	mov r0, r113
	bl malloc
	mov r1030101, r0
	ldr r114, =A$f
	str r114, [r1030101]
	mov r1000101, r1030101
	movw r115, #4
	mov r0, r115
	bl malloc
	mov r1040101, r0
	ldr r116, =B$f
	str r116, [r1040101]
	mov r1010101, r1040101
	mov r1000201, r1010101
	movw r117, #0
	mov r0, r117
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
