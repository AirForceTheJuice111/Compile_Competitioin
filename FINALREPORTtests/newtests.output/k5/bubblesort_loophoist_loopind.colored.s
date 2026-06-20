.balign 4
.global b1$bubbleSort
.section .text
.arm
b1$bubbleSort:
b1$bubbleSort$L133:
	push {r4-r10, fp, lr}
	sub sp, sp, #124
	add fp, sp, #156
	mov r10, r0
	str r10, [fp, #-40]
	mov r10, r1
	str r10, [fp, #-44]
	mov r10, r2
	str r10, [fp, #-48]
	mov r10, #0
	str r10, [fp, #-52]
	movw r0, #1
	ldr r9, [fp, #-48]
	cmp r9, r0
	ble b1$bubbleSort$L102
b1$bubbleSort$L103:
b1$bubbleSort$L104:
	ldr r9, [fp, #-48]
	sub r10, r9, #1
	str r10, [fp, #-112]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-88]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-92]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-96]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-128]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-100]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-104]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-108]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-156]
	movw r0, #4
	ldr r9, [fp, #-52]
	mul r3, r9, r0
	movw r0, #4
	ldr r9, [fp, #-52]
	mul r2, r9, r0
	movw r0, #4
	ldr r9, [fp, #-52]
	mul r1, r9, r0
	movw r0, #4
	ldr r9, [fp, #-52]
	mul r10, r9, r0
	str r10, [fp, #-140]
	movw r0, #4
	ldr r9, [fp, #-52]
	mul r10, r9, r0
	str r10, [fp, #-148]
	movw r0, #4
	ldr r9, [fp, #-52]
	mul r0, r9, r0
	ldr r9, [fp, #-52]
	add r10, r9, #1
	str r10, [fp, #-64]
	ldr r9, [fp, #-52]
	add r10, r9, #1
	str r10, [fp, #-76]
	ldr r9, [fp, #-52]
	add r10, r9, #1
	str r10, [fp, #-84]
	add r10, r3, #4
	str r10, [fp, #-120]
	add r4, r2, #8
	add r3, r1, #4
	ldr r9, [fp, #-140]
	add r2, r9, #4
	ldr r9, [fp, #-148]
	add r1, r9, #8
	add r0, r0, #8
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-120]
	mov r10, r9
	str r10, [fp, #-116]
	mov r10, r4
	str r10, [fp, #-124]
	mov r10, r3
	str r10, [fp, #-132]
	mov r10, r2
	str r10, [fp, #-136]
	mov r10, r1
	str r10, [fp, #-144]
	mov r10, r0
	str r10, [fp, #-152]
	ldr r9, [fp, #-64]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-76]
	mov r10, r9
	str r10, [fp, #-72]
	ldr r9, [fp, #-84]
	mov r10, r9
	str r10, [fp, #-80]
b1$bubbleSort$L107:
	ldr r9, [fp, #-56]
	ldr r10, [fp, #-112]
	cmp r9, r10
	blt b1$bubbleSort$L108
b1$bubbleSort$L109:
	ldr r9, [fp, #-40]
	ldr r3, [r9, #4]
	ldr r9, [fp, #-40]
	mov r0, r9
	ldr r9, [fp, #-44]
	mov r1, r9
	ldr r9, [fp, #-48]
	sub r2, r9, #1
	blx r3
	sub sp, fp, #156
	add sp, sp, #124
	pop {r4-r10, fp, lr}
	bx lr
b1$bubbleSort$L108:
	ldr r9, [fp, #-44]
	ldr r4, [r9]
	movw r0, #0
	ldr r9, [fp, #-56]
	cmp r9, r0
	bge b1$bubbleSort$L111
b1$bubbleSort$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L111:
	ldr r9, [fp, #-56]
	cmp r9, r4
	bge b1$bubbleSort$L110
b1$bubbleSort$L112:
	ldr r9, [fp, #-88]
	ldr r10, [fp, #-116]
	ldr r4, [r9, r10]
	ldr r9, [fp, #-44]
	ldr r10, [r9]
	str r10, [fp, #-68]
	movw r0, #0
	ldr r9, [fp, #-60]
	cmp r9, r0
	bge b1$bubbleSort$L114
b1$bubbleSort$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L114:
	ldr r9, [fp, #-60]
	ldr r10, [fp, #-68]
	cmp r9, r10
	bge b1$bubbleSort$L113
b1$bubbleSort$L115:
	ldr r9, [fp, #-92]
	ldr r10, [fp, #-124]
	ldr r0, [r9, r10]
	cmp r4, r0
	bgt b1$bubbleSort$L130
b1$bubbleSort$L131:
b1$bubbleSort$L132:
	ldr r9, [fp, #-56]
	add r10, r9, #1
	str r10, [fp, #-56]
	ldr r9, [fp, #-116]
	add r10, r9, #4
	str r10, [fp, #-116]
	ldr r9, [fp, #-124]
	add r10, r9, #4
	str r10, [fp, #-124]
	ldr r9, [fp, #-132]
	add r10, r9, #4
	str r10, [fp, #-132]
	ldr r9, [fp, #-136]
	add r10, r9, #4
	str r10, [fp, #-136]
	ldr r9, [fp, #-144]
	add r10, r9, #4
	str r10, [fp, #-144]
	ldr r9, [fp, #-152]
	add r10, r9, #4
	str r10, [fp, #-152]
	ldr r9, [fp, #-60]
	add r10, r9, #1
	str r10, [fp, #-60]
	ldr r9, [fp, #-72]
	add r10, r9, #1
	str r10, [fp, #-72]
	ldr r9, [fp, #-80]
	add r10, r9, #1
	str r10, [fp, #-80]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-116]
	mov r10, r9
	str r10, [fp, #-116]
	ldr r9, [fp, #-124]
	mov r10, r9
	str r10, [fp, #-124]
	ldr r9, [fp, #-132]
	mov r10, r9
	str r10, [fp, #-132]
	ldr r9, [fp, #-136]
	mov r10, r9
	str r10, [fp, #-136]
	ldr r9, [fp, #-144]
	mov r10, r9
	str r10, [fp, #-144]
	ldr r9, [fp, #-152]
	mov r10, r9
	str r10, [fp, #-152]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-72]
	mov r10, r9
	str r10, [fp, #-72]
	ldr r9, [fp, #-80]
	mov r10, r9
	str r10, [fp, #-80]
	b b1$bubbleSort$L107
b1$bubbleSort$L130:
	ldr r9, [fp, #-44]
	ldr r4, [r9]
	movw r0, #0
	ldr r9, [fp, #-56]
	cmp r9, r0
	bge b1$bubbleSort$L119
b1$bubbleSort$L118:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L119:
	ldr r9, [fp, #-56]
	cmp r9, r4
	bge b1$bubbleSort$L118
b1$bubbleSort$L120:
	ldr r9, [fp, #-96]
	ldr r10, [fp, #-132]
	ldr r0, [r9, r10]
	ldr r10, [fp, #-128]
	str r0, [r10]
	ldr r9, [fp, #-44]
	ldr r4, [r9]
	movw r0, #0
	ldr r9, [fp, #-56]
	cmp r9, r0
	bge b1$bubbleSort$L122
b1$bubbleSort$L121:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L122:
	ldr r9, [fp, #-56]
	cmp r9, r4
	bge b1$bubbleSort$L121
b1$bubbleSort$L123:
	ldr r9, [fp, #-44]
	ldr r4, [r9]
	movw r0, #0
	ldr r9, [fp, #-72]
	cmp r9, r0
	bge b1$bubbleSort$L125
b1$bubbleSort$L124:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L125:
	ldr r9, [fp, #-72]
	cmp r9, r4
	bge b1$bubbleSort$L124
b1$bubbleSort$L126:
	ldr r9, [fp, #-100]
	ldr r10, [fp, #-136]
	add r1, r9, r10
	ldr r9, [fp, #-104]
	ldr r10, [fp, #-144]
	ldr r0, [r9, r10]
	str r0, [r1]
	ldr r9, [fp, #-44]
	ldr r4, [r9]
	movw r0, #0
	ldr r9, [fp, #-80]
	cmp r9, r0
	bge b1$bubbleSort$L128
b1$bubbleSort$L127:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L128:
	ldr r9, [fp, #-80]
	cmp r9, r4
	bge b1$bubbleSort$L127
b1$bubbleSort$L129:
	ldr r9, [fp, #-108]
	ldr r10, [fp, #-152]
	add r1, r9, r10
	ldr r9, [fp, #-156]
	ldr r0, [r9]
	str r0, [r1]
	b b1$bubbleSort$L132
b1$bubbleSort$L102:
	movw r0, #0
	sub sp, fp, #156
	add sp, sp, #124
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #24
	add fp, sp, #56
	movw r0, #32
	bl malloc
	mov r10, r0
	str r10, [fp, #-56]
	mov r10, #0
	str r10, [fp, #-40]
	movw r0, #7
	ldr r10, [fp, #-56]
	str r0, [r10]
	movw r0, #6
	ldr r10, [fp, #-56]
	str r0, [r10, #4]
	movw r0, #3
	ldr r10, [fp, #-56]
	str r0, [r10, #8]
	movw r0, #0
	ldr r10, [fp, #-56]
	str r0, [r10, #12]
	movw r0, #5
	ldr r10, [fp, #-56]
	str r0, [r10, #16]
	movw r0, #9
	ldr r10, [fp, #-56]
	str r0, [r10, #20]
	movw r0, #1
	ldr r10, [fp, #-56]
	str r0, [r10, #24]
	movw r0, #2
	ldr r10, [fp, #-56]
	str r0, [r10, #28]
	movw r0, #8
	bl malloc
	ldr r9, [fp, #-56]
	mov r1, r9
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	movw r2, #4
	ldr r9, [fp, #-40]
	mul r2, r9, r2
	ldr r3, =b1$bubbleSort
	str r3, [r0, #4]
	add r4, r2, #4
	ldr r3, [r0, #4]
	ldr r9, [fp, #-56]
	ldr r2, [r9]
	blx r3
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
main$L102:
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-56]
	ldr r0, [r9]
	ldr r9, [fp, #-44]
	cmp r9, r0
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #56
	add sp, sp, #24
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-56]
	ldr r10, [r9]
	str r10, [fp, #-52]
	movw r0, #0
	ldr r9, [fp, #-44]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-52]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-56]
	ldr r0, [r9, r4]
	ldr r9, [fp, #-44]
	add r10, r9, #1
	str r10, [fp, #-48]
	add r4, r4, #4
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-44]
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
