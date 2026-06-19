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
	mov r5, r0
	mov r6, #0
	movw r0, #7
	str r0, [r5]
	movw r0, #6
	str r0, [r5, #4]
	movw r0, #3
	str r0, [r5, #8]
	movw r0, #0
	str r0, [r5, #12]
	movw r0, #5
	str r0, [r5, #16]
	movw r0, #9
	str r0, [r5, #20]
	movw r0, #1
	str r0, [r5, #24]
	movw r0, #2
	str r0, [r5, #28]
	movw r0, #8
	bl malloc
	mov r1, r5
	movw r2, #4
	mul r2, r6, r2
	ldr r3, =b1$bubbleSort
	str r3, [r0, #4]
	add r4, r2, #4
	ldr r3, [r0, #4]
	ldr r2, [r5]
	blx r3
main$L102:
	ldr r0, [r5]
	cmp r6, r0
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
	ldr r7, [r5]
	movw r0, #0
	cmp r6, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	cmp r6, r7
	bge main$L105
main$L107:
	ldr r0, [r5, r4]
	add r6, r6, #1
	add r4, r4, #4
	bl putint
	movw r0, #32
	bl putch
	b main$L102

.balign 4
.global b1$bubbleSort
.section .text
.arm
b1$bubbleSort:
b1$bubbleSort$L133:
	push {r4-r10, fp, lr}
	sub sp, sp, #76
	add fp, sp, #108
	mov r10, r0
	str r10, [fp, #-92]
	mov r10, r1
	str r10, [fp, #-56]
	mov r10, r2
	str r10, [fp, #-40]
	mov r7, #0
	movw r0, #1
	ldr r9, [fp, #-40]
	cmp r9, r0
	ble b1$bubbleSort$L102
b1$bubbleSort$L103:
b1$bubbleSort$L104:
	ldr r9, [fp, #-40]
	sub r10, r9, #1
	str r10, [fp, #-80]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-64]
	ldr r9, [fp, #-92]
	mov r10, r9
	str r10, [fp, #-92]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-68]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-72]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-76]
	ldr r9, [fp, #-92]
	mov r10, r9
	str r10, [fp, #-108]
	movw r0, #4
	mul r5, r7, r0
	movw r0, #4
	mul r4, r7, r0
	movw r0, #4
	mul r3, r7, r0
	movw r0, #4
	mul r2, r7, r0
	movw r0, #4
	mul r1, r7, r0
	movw r0, #4
	mul r0, r7, r0
	add r6, r7, #1
	add r8, r7, #1
	add r10, r7, #1
	str r10, [fp, #-52]
	add r5, r5, #4
	add r4, r4, #8
	add r3, r3, #4
	add r2, r2, #4
	add r1, r1, #8
	add r0, r0, #8
	mov r10, r7
	str r10, [fp, #-44]
	mov r10, r5
	str r10, [fp, #-84]
	mov r10, r4
	str r10, [fp, #-88]
	mov r10, r3
	str r10, [fp, #-96]
	mov r10, r2
	str r10, [fp, #-100]
	mov r10, r1
	str r10, [fp, #-104]
	mov r4, r0
	mov r10, r6
	str r10, [fp, #-48]
	mov r7, r8
	ldr r9, [fp, #-52]
	mov r6, r9
b1$bubbleSort$L107:
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-80]
	cmp r9, r10
	blt b1$bubbleSort$L108
b1$bubbleSort$L109:
	ldr r9, [fp, #-92]
	ldr r3, [r9, #4]
	ldr r9, [fp, #-92]
	mov r0, r9
	ldr r9, [fp, #-56]
	mov r1, r9
	ldr r9, [fp, #-40]
	sub r2, r9, #1
	blx r3
	sub sp, fp, #108
	add sp, sp, #76
	pop {r4-r10, fp, lr}
	bx lr
b1$bubbleSort$L108:
	ldr r9, [fp, #-56]
	ldr r5, [r9]
	movw r0, #0
	ldr r9, [fp, #-44]
	cmp r9, r0
	bge b1$bubbleSort$L111
b1$bubbleSort$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L111:
	ldr r9, [fp, #-44]
	cmp r9, r5
	bge b1$bubbleSort$L110
b1$bubbleSort$L112:
	ldr r9, [fp, #-56]
	ldr r10, [fp, #-84]
	ldr r5, [r9, r10]
	ldr r9, [fp, #-56]
	ldr r8, [r9]
	movw r0, #0
	ldr r9, [fp, #-48]
	cmp r9, r0
	bge b1$bubbleSort$L114
b1$bubbleSort$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L114:
	ldr r9, [fp, #-48]
	cmp r9, r8
	bge b1$bubbleSort$L113
b1$bubbleSort$L115:
	ldr r9, [fp, #-60]
	ldr r10, [fp, #-88]
	ldr r0, [r9, r10]
	cmp r5, r0
	bgt b1$bubbleSort$L130
b1$bubbleSort$L131:
b1$bubbleSort$L132:
	ldr r9, [fp, #-44]
	add r10, r9, #1
	str r10, [fp, #-44]
	ldr r9, [fp, #-84]
	add r10, r9, #4
	str r10, [fp, #-84]
	ldr r9, [fp, #-88]
	add r10, r9, #4
	str r10, [fp, #-88]
	ldr r9, [fp, #-96]
	add r10, r9, #4
	str r10, [fp, #-96]
	ldr r9, [fp, #-100]
	add r10, r9, #4
	str r10, [fp, #-100]
	ldr r9, [fp, #-104]
	add r10, r9, #4
	str r10, [fp, #-104]
	add r4, r4, #4
	ldr r9, [fp, #-48]
	add r10, r9, #1
	str r10, [fp, #-48]
	add r7, r7, #1
	add r6, r6, #1
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-84]
	mov r10, r9
	str r10, [fp, #-84]
	ldr r9, [fp, #-88]
	mov r10, r9
	str r10, [fp, #-88]
	ldr r9, [fp, #-96]
	mov r10, r9
	str r10, [fp, #-96]
	ldr r9, [fp, #-100]
	mov r10, r9
	str r10, [fp, #-100]
	ldr r9, [fp, #-104]
	mov r10, r9
	str r10, [fp, #-104]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	b b1$bubbleSort$L107
b1$bubbleSort$L130:
	ldr r9, [fp, #-56]
	ldr r5, [r9]
	movw r0, #0
	ldr r9, [fp, #-44]
	cmp r9, r0
	bge b1$bubbleSort$L119
b1$bubbleSort$L118:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L119:
	ldr r9, [fp, #-44]
	cmp r9, r5
	bge b1$bubbleSort$L118
b1$bubbleSort$L120:
	ldr r9, [fp, #-64]
	ldr r10, [fp, #-96]
	ldr r0, [r9, r10]
	ldr r10, [fp, #-92]
	str r0, [r10]
	ldr r9, [fp, #-56]
	ldr r5, [r9]
	movw r0, #0
	ldr r9, [fp, #-44]
	cmp r9, r0
	bge b1$bubbleSort$L122
b1$bubbleSort$L121:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L122:
	ldr r9, [fp, #-44]
	cmp r9, r5
	bge b1$bubbleSort$L121
b1$bubbleSort$L123:
	ldr r9, [fp, #-56]
	ldr r5, [r9]
	movw r0, #0
	cmp r7, r0
	bge b1$bubbleSort$L125
b1$bubbleSort$L124:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L125:
	cmp r7, r5
	bge b1$bubbleSort$L124
b1$bubbleSort$L126:
	ldr r9, [fp, #-68]
	ldr r10, [fp, #-100]
	add r1, r9, r10
	ldr r9, [fp, #-72]
	ldr r10, [fp, #-104]
	ldr r0, [r9, r10]
	str r0, [r1]
	ldr r9, [fp, #-56]
	ldr r5, [r9]
	movw r0, #0
	cmp r6, r0
	bge b1$bubbleSort$L128
b1$bubbleSort$L127:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L128:
	cmp r6, r5
	bge b1$bubbleSort$L127
b1$bubbleSort$L129:
	ldr r9, [fp, #-76]
	add r1, r9, r4
	ldr r9, [fp, #-108]
	ldr r0, [r9]
	str r0, [r1]
	b b1$bubbleSort$L132
b1$bubbleSort$L102:
	movw r0, #0
	sub sp, fp, #108
	add sp, sp, #76
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
