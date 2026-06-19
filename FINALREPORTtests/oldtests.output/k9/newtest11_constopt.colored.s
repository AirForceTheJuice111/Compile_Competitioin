.balign 4
.global b1$bubbleSort
.section .text
.arm
b1$bubbleSort:
b1$bubbleSort$L133:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	mov r10, r0
	str r10, [fp, #-40]
	mov r5, r1
	mov r10, r2
	str r10, [fp, #-44]
	movw r0, #1
	ldr r9, [fp, #-44]
	cmp r9, r0
	ble b1$bubbleSort$L102
b1$bubbleSort$L103:
b1$bubbleSort$L104:
	mov r8, #0
b1$bubbleSort$L107:
	ldr r9, [fp, #-44]
	sub r0, r9, #1
	cmp r8, r0
	blt b1$bubbleSort$L108
b1$bubbleSort$L109:
	ldr r9, [fp, #-40]
	ldr r3, [r9, #4]
	ldr r9, [fp, #-40]
	mov r0, r9
	mov r1, r5
	ldr r9, [fp, #-44]
	sub r2, r9, #1
	blx r3
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
b1$bubbleSort$L108:
	mov r4, r5
	ldr r6, [r5]
	movw r0, #0
	cmp r8, r0
	bge b1$bubbleSort$L111
b1$bubbleSort$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L111:
	cmp r8, r6
	bge b1$bubbleSort$L110
b1$bubbleSort$L112:
	add r0, r8, #1
	add r7, r8, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r4, [r4, r0]
	ldr r6, [r5]
	movw r0, #0
	cmp r7, r0
	bge b1$bubbleSort$L114
b1$bubbleSort$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L114:
	cmp r7, r6
	bge b1$bubbleSort$L113
b1$bubbleSort$L115:
	add r0, r7, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r5, r0]
	cmp r4, r0
	bgt b1$bubbleSort$L130
b1$bubbleSort$L131:
b1$bubbleSort$L132:
	add r8, r8, #1
	b b1$bubbleSort$L107
b1$bubbleSort$L130:
	mov r4, r5
	ldr r6, [r5]
	movw r0, #0
	cmp r8, r0
	bge b1$bubbleSort$L119
b1$bubbleSort$L118:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L119:
	cmp r8, r6
	bge b1$bubbleSort$L118
b1$bubbleSort$L120:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	add r0, r8, #1
	mov r6, r5
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	ldr r10, [fp, #-40]
	str r0, [r10]
	ldr r4, [r5]
	movw r0, #0
	cmp r8, r0
	bge b1$bubbleSort$L122
b1$bubbleSort$L121:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L122:
	cmp r8, r4
	bge b1$bubbleSort$L121
b1$bubbleSort$L123:
	add r7, r8, #1
	ldr r4, [r5]
	movw r0, #0
	cmp r7, r0
	bge b1$bubbleSort$L125
b1$bubbleSort$L124:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L125:
	cmp r7, r4
	bge b1$bubbleSort$L124
b1$bubbleSort$L126:
	add r1, r8, #1
	add r0, r7, #1
	add r7, r8, #1
	mov r4, r5
	movw r2, #4
	mul r1, r1, r2
	movw r2, #4
	mul r0, r0, r2
	add r1, r6, r1
	ldr r0, [r5, r0]
	str r0, [r1]
	ldr r6, [r5]
	movw r0, #0
	cmp r7, r0
	bge b1$bubbleSort$L128
b1$bubbleSort$L127:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L128:
	cmp r7, r6
	bge b1$bubbleSort$L127
b1$bubbleSort$L129:
	add r1, r7, #1
	ldr r9, [fp, #-40]
	ldr r0, [r9]
	movw r2, #4
	mul r1, r1, r2
	str r0, [r4, r1]
	b b1$bubbleSort$L132
b1$bubbleSort$L102:
	movw r0, #0
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #32
	bl malloc
	mov r4, r0
	mov r5, #0
	movw r0, #7
	str r0, [r4]
	movw r0, #6
	str r0, [r4, #4]
	movw r0, #3
	str r0, [r4, #8]
	movw r0, #0
	str r0, [r4, #12]
	movw r0, #5
	str r0, [r4, #16]
	movw r0, #9
	str r0, [r4, #20]
	movw r0, #1
	str r0, [r4, #24]
	movw r0, #2
	str r0, [r4, #28]
	movw r0, #8
	bl malloc
	mov r1, r4
	ldr r2, =b1$bubbleSort
	str r2, [r0, #4]
	ldr r3, [r0, #4]
	ldr r2, [r4]
	blx r3
main$L102:
	ldr r0, [r4]
	cmp r5, r0
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
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
	add r5, r5, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	bl putint
	movw r0, #32
	bl putch
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
