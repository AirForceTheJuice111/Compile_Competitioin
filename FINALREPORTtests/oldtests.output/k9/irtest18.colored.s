.balign 4
.global B$g
.section .text
.arm
B$g:
B$g$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	ldr r1, [r0, #12]
	blx r1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r7, r0
	ldr r1, [r0]
	ldr r0, [r7, #8]
	ldr r5, [r7, #4]
	add r4, r1, r0
	ldr r6, [r5]
	movw r1, #0
	movw r0, #0
	cmp r1, r0
	bge B$f$L101
B$f$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit
B$f$L101:
	movw r0, #0
	cmp r0, r6
	bge B$f$L100
B$f$L102:
	movw r1, #0
	movw r0, #1
	add r1, r1, r0
	add r0, r7, #16
	movw r2, #4
	mul r1, r1, r2
	ldr r1, [r5, r1]
	ldr r2, [r0]
	add r4, r4, r1
	mov r0, r7
	blx r2
	add r0, r4, r0
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
	movw r0, #20
	bl malloc
	ldr r1, =A$f
	str r1, [r0, #12]
	mov r6, r0
	movw r0, #1
	str r0, [r6]
	add r4, r6, #12
	movw r0, #20
	bl malloc
	mov r5, r0
	movw r0, #16
	bl malloc
	movw r1, #3
	str r1, [r0]
	add r3, r5, #4
	add r2, r5, #12
	add r1, r5, #16
	movw r7, #1
	str r7, [r0, #4]
	movw r7, #2
	str r7, [r0, #8]
	movw r7, #3
	str r7, [r0, #12]
	str r0, [r3]
	ldr r0, =B$f
	str r0, [r2]
	ldr r0, =B$g
	str r0, [r1]
	movw r0, #1
	str r0, [r5, #8]
	ldr r1, [r4]
	mov r0, r6
	blx r1
	bl putint
	ldr r1, [r5, #12]
	mov r0, r5
	blx r1
	bl putint
main$L101:
	movw r0, #0
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
	movw r0, #0
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
