.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r4, #4
	push {r4}
	pop {r0}
	bl malloc
	mov r5, r0
	movw r0, #0
	str r0, [r5]
	movw r0, #20
	push {r0}
	pop {r0}
	bl malloc
	mov r1, r0
	str r4, [r1]
	movw r0, #1
	str r0, [r1, #4]
	movw r0, #2
	str r0, [r1, #8]
	movw r0, #3
	str r0, [r1, #12]
	str r4, [r1, #16]
	mov r0, r5
	str r1, [r0]
	mov r0, r5
	mov r2, #3
	mov r5, r0
	sub r1, r2, #1
	movw r0, #4
	mul r0, r1, r0
	add r0, r0, #4
	mov r6, r2
	mov r4, r0
main$L102:
	movw r0, #0
	cmp r4, r0
	bge main$L103
main$L104:
	movw r0, #10
	push {r0}
	pop {r0}
	bl putch
	movw r0, #2
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r8, r6, #1
	ldr r0, [r5]
	mov r6, r0
	ldr r7, [r0]
	movw r0, #0
	cmp r8, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	push {r0}
	pop {r0}
	bl exit
main$L106:
	cmp r8, r7
	bge main$L105
main$L107:
	ldr r0, [r6, r4]
	push {r0}
	pop {r0}
	bl putint
	movw r0, #32
	push {r0}
	pop {r0}
	bl putch
	sub r0, r4, #4
	mov r6, r8
	mov r4, r0
	b main$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
