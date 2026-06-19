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
	sub sp, sp, #24
	add fp, sp, #56
	mov r10, r0
	str r10, [fp, #-40]
	ldr r1, [r0]
	ldr r9, [fp, #-40]
	ldr r0, [r9, #8]
	ldr r9, [fp, #-40]
	ldr r2, [r9, #4]
	add r10, r1, r0
	str r10, [fp, #-52]
	ldr r10, [r2]
	str r10, [fp, #-44]
	mov r10, r2
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
	add r0, r1, r0
	ldr r9, [fp, #-40]
	add r2, r9, #16
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-48]
	ldr r0, [r9, r0]
	ldr r1, [r2]
	ldr r9, [fp, #-52]
	add r10, r9, r0
	str r10, [fp, #-56]
	ldr r9, [fp, #-40]
	mov r0, r9
	blx r1
	ldr r9, [fp, #-56]
	add r0, r9, r0
	sub sp, fp, #56
	add sp, sp, #24
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #32
	add fp, sp, #64
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
	str r10, [fp, #-64]
	movw r0, #20
	bl malloc
	mov r10, r0
	str r10, [fp, #-52]
	movw r0, #16
	bl malloc
	mov r10, r0
	str r10, [fp, #-56]
	movw r0, #3
	ldr r10, [fp, #-56]
	str r0, [r10]
	ldr r9, [fp, #-52]
	add r2, r9, #4
	ldr r9, [fp, #-52]
	add r10, r9, #12
	str r10, [fp, #-60]
	ldr r9, [fp, #-52]
	add r0, r9, #16
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-48]
	movw r1, #1
	ldr r10, [fp, #-56]
	str r1, [r10, #4]
	movw r1, #2
	ldr r10, [fp, #-56]
	str r1, [r10, #8]
	movw r1, #3
	ldr r10, [fp, #-56]
	str r1, [r10, #12]
	ldr r9, [fp, #-56]
	str r9, [r2]
	ldr r1, =B$f
	ldr r10, [fp, #-60]
	str r1, [r10]
	ldr r1, =B$g
	str r1, [r0]
	movw r0, #1
	ldr r10, [fp, #-48]
	str r0, [r10, #8]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-64]
	ldr r1, [r9]
	ldr r9, [fp, #-40]
	mov r0, r9
	blx r1
	bl putint
	ldr r9, [fp, #-44]
	ldr r1, [r9, #12]
	ldr r9, [fp, #-44]
	mov r0, r9
	blx r1
	bl putint
main$L101:
	movw r0, #0
	sub sp, fp, #64
	add sp, sp, #32
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
