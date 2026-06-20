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
	movw r120, #16
	mov r0, r120
	bl malloc
	mov r1030101, r0
	ldr r121, =A$f
	str r121, [r1030101, #8]
	mov r1000101, r1030101
	movw r122, #1
	str r122, [r1000101]
	movw r123, #16
	mov r0, r123
	bl malloc
	mov r1040101, r0
	ldr r124, =B$f
	str r124, [r1040101, #8]
	ldr r125, =B$g
	str r125, [r1040101, #12]
	mov r1010101, r1040101
	movw r126, #1
	str r126, [r1010101, #4]
	mov r1000201, r1010101
	ldr r1070101, [r1000201, #8]
	mov r0, r1000201
	blx r1070101
	mov r1080101, r0
	mov r0, r1080101
	bl putint
main$L101:
	movw r127, #0
	mov r0, r127
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
B$f$L101:
	movw r102, #0
	mov r0, r102
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
