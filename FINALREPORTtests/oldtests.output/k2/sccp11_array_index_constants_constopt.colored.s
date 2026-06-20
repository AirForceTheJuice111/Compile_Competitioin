.balign 4
.global main
.section .text
.arm
main:
main$L106:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	movw r0, #20
	bl malloc
	mov r10, r0
	str r10, [fp, #-44]
	movw r0, #4
	ldr r10, [fp, #-44]
	str r0, [r10]
	movw r0, #1
	ldr r10, [fp, #-44]
	str r0, [r10, #4]
	movw r0, #2
	ldr r10, [fp, #-44]
	str r0, [r10, #8]
	movw r0, #3
	ldr r10, [fp, #-44]
	str r0, [r10, #12]
	movw r0, #4
	ldr r10, [fp, #-44]
	str r0, [r10, #16]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	ldr r0, [r9]
main$L101:
	movw r1, #2
	cmp r1, r0
	bge main$L100
main$L102:
	movw r0, #7
	ldr r10, [fp, #-44]
	str r0, [r10, #12]
	ldr r9, [fp, #-44]
	mov r0, r9
	ldr r9, [fp, #-44]
	ldr r1, [r9]
main$L104:
	movw r10, #2
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	cmp r9, r1
	bge main$L103
main$L105:
	ldr r0, [r0, #12]
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
