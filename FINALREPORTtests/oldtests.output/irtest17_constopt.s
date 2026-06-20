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
	movw r104, #0
	mov r0, r104
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$g
.section .text
.arm
B$g:
B$g$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
B$g$L101:
	movw r104, #0
	mov r0, r104
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
	movw r122, #16
	mov r0, r122
	bl malloc
	mov r1030101, r0
	ldr r123, =A$f
	str r123, [r1030101, #8]
	mov r1000101, r1030101
	movw r124, #1
	str r124, [r1000101]
	movw r125, #16
	mov r0, r125
	bl malloc
	mov r1040101, r0
	ldr r126, =B$f
	str r126, [r1040101, #8]
	ldr r127, =B$g
	str r127, [r1040101, #12]
	mov r1010101, r1040101
	movw r128, #1
	str r128, [r1010101, #4]
	mov r1000201, r1010101
	ldr r1070101, [r1000201, #8]
	mov r0, r1000201
	blx r1070101
	mov r1080101, r0
	mov r0, r1080101
	bl putint
main$L101:
	movw r129, #0
	mov r0, r129
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
	movw r104, #0
	mov r0, r104
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
