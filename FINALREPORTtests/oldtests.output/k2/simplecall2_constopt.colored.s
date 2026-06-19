.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r0, r1
	ldr r1, [r1]
C$m$L101:
	movw r10, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	cmp r9, r1
	bge C$m$L100
C$m$L102:
	ldr r0, [r0, #8]
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
C$m$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit

.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	movw r0, #16
	bl malloc
	mov r10, r0
	str r10, [fp, #-40]
	movw r0, #3
	ldr r10, [fp, #-40]
	str r0, [r10]
	movw r0, #1
	ldr r10, [fp, #-40]
	str r0, [r10, #4]
	movw r0, #2
	ldr r10, [fp, #-40]
	str r0, [r10, #8]
	movw r0, #3
	ldr r10, [fp, #-40]
	str r0, [r10, #12]
	movw r0, #4
	bl malloc
	ldr r1, =C$m
	str r1, [r0]
	ldr r10, [r0]
	str r10, [fp, #-44]
	ldr r9, [fp, #-40]
	mov r1, r9
	ldr r9, [fp, #-44]
	blx r9
	bl putint
	movw r0, #10
	bl putch
main$L101:
	movw r0, #0
	sub sp, fp, #44
	add sp, sp, #12
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
