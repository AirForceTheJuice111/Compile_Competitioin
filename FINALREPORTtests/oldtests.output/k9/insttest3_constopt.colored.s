.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #8
	bl malloc
	mov r4, r0
	mov r5, #3
	movw r0, #20
	bl malloc
	movw r1, #4
	str r1, [r0]
	mov r1, r4
	add r2, r4, #4
	movw r3, #1
	str r3, [r0, #4]
	movw r3, #2
	str r3, [r0, #8]
	movw r3, #3
	str r3, [r0, #12]
	movw r3, #4
	str r3, [r0, #16]
	str r0, [r1]
	ldr r0, =C$m
	str r0, [r2]
	ldr r1, [r4, #4]
	mov r0, r4
	blx r1
	mov r4, r0
main$L102:
	movw r0, #0
	cmp r5, r0
	bge main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r5, r5, #1
	ldr r6, [r4]
	movw r0, #0
	cmp r5, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	cmp r5, r6
	bge main$L105
main$L107:
	add r0, r5, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	bl putint
	movw r0, #32
	bl putch
	b main$L102

.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	ldr r0, [r0]
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
