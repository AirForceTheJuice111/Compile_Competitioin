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
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	movw r0, #20
	bl malloc
	ldr r1, =A$f
	str r1, [r0, #12]
	mov r10, r0
	str r10, [fp, #-40]
	movw r0, #1
	ldr r10, [fp, #-40]
	str r0, [r10]
	ldr r9, [fp, #-40]
	add r10, r9, #12
	str r10, [fp, #-52]
	movw r0, #20
	bl malloc
	mov r10, r0
	str r10, [fp, #-48]
	movw r0, #16
	bl malloc
	mov r4, r0
	movw r0, #3
	str r0, [r4]
	ldr r9, [fp, #-48]
	add r2, r9, #4
	ldr r9, [fp, #-48]
	add r1, r9, #12
	ldr r9, [fp, #-48]
	add r0, r9, #16
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-44]
	movw r3, #1
	str r3, [r4, #4]
	movw r3, #2
	str r3, [r4, #8]
	movw r3, #3
	str r3, [r4, #12]
	str r4, [r2]
	ldr r2, =B$f
	str r2, [r1]
	ldr r1, =B$g
	str r1, [r0]
	movw r0, #1
	ldr r10, [fp, #-44]
	str r0, [r10, #8]
	ldr r9, [fp, #-44]
	mov r4, r9
	ldr r9, [fp, #-52]
	ldr r1, [r9]
	ldr r9, [fp, #-40]
	mov r0, r9
	blx r1
	bl putint
	ldr r1, [r4, #12]
	mov r0, r4
	blx r1
	bl putint
main$L101:
	movw r0, #0
	sub sp, fp, #52
	add sp, sp, #20
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

.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	mov r10, r0
	str r10, [fp, #-40]
	ldr r1, [r0]
	ldr r9, [fp, #-40]
	ldr r0, [r9, #8]
	ldr r9, [fp, #-40]
	ldr r10, [r9, #4]
	str r10, [fp, #-48]
	add r4, r1, r0
	ldr r9, [fp, #-48]
	ldr r10, [r9]
	str r10, [fp, #-44]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
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
	ldr r10, [fp, #-44]
	cmp r0, r10
	bge B$f$L100
B$f$L102:
	movw r1, #0
	movw r0, #1
	add r1, r1, r0
	ldr r9, [fp, #-40]
	add r0, r9, #16
	movw r2, #4
	mul r1, r1, r2
	ldr r9, [fp, #-48]
	ldr r2, [r9, r1]
	ldr r1, [r0]
	add r4, r4, r2
	ldr r9, [fp, #-40]
	mov r0, r9
	blx r1
	add r0, r4, r0
	sub sp, fp, #48
	add sp, sp, #16
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
