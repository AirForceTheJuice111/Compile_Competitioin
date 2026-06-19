.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #12
	bl malloc
	mov r5, r0
	movw r0, #20
	bl malloc
	movw r1, #4
	str r1, [r0]
	mov r1, r5
	add r6, r5, #4
	add r4, r5, #8
	movw r2, #1
	str r2, [r0, #4]
	movw r2, #2
	str r2, [r0, #8]
	movw r2, #3
	str r2, [r0, #12]
	movw r2, #4
	str r2, [r0, #16]
	str r0, [r1]
	movw r0, #24
	bl malloc
	movw r1, #5
	str r1, [r0]
	add r1, r5, #8
	movw r2, #5
	str r2, [r0, #4]
	movw r2, #6
	str r2, [r0, #8]
	movw r2, #7
	str r2, [r0, #12]
	movw r2, #8
	str r2, [r0, #16]
	movw r2, #9
	str r2, [r0, #20]
	str r0, [r6]
	ldr r0, =C$m
	str r0, [r4]
	ldr r4, [r1]
	bl getint
	mov r1, r0
	mov r0, r5
	blx r4
	mov r4, r0
	ldr r5, [r4]
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
C$m$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r2, #0
	cmp r1, r2
	bgt C$m$L102
C$m$L103:
	ldr r0, [r0, #4]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$m$L102:
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
