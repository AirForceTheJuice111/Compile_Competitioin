.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #36
	add fp, sp, #68
	bl getint
	mov r10, r0
	str r10, [fp, #-56]
	mov r0, #10
	mov r10, #0
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	add r1, r9, #1
	movw r2, #3
	mul r1, r1, r2
	mov r10, r0
	str r10, [fp, #-68]
	ldr r9, [fp, #-40]
	mov r0, r9
	ldr r9, [fp, #-56]
	mov r2, r9
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-48]
	mov r10, r1
	str r10, [fp, #-64]
main$L102:
	movw r1, #0
	ldr r9, [fp, #-68]
	cmp r9, r1
	bgt main$L103
main$L104:
	sub sp, fp, #68
	add sp, sp, #36
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-48]
	add r10, r9, #1
	str r10, [fp, #-52]
	mov r10, r2
	str r10, [fp, #-60]
main$L107:
	movw r1, #0
	ldr r9, [fp, #-60]
	cmp r9, r1
	bgt main$L108
main$L109:
	ldr r9, [fp, #-68]
	sub r10, r9, #1
	str r10, [fp, #-68]
	ldr r9, [fp, #-64]
	add r10, r9, #3
	str r10, [fp, #-64]
	ldr r9, [fp, #-68]
	mov r10, r9
	str r10, [fp, #-68]
	ldr r9, [fp, #-60]
	mov r2, r9
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-48]
	ldr r9, [fp, #-64]
	mov r10, r9
	str r10, [fp, #-64]
	b main$L102
main$L108:
	movw r0, #3
	ldr r9, [fp, #-52]
	cmp r9, r0
	bgt main$L112
main$L113:
	movw r0, #4
	bl malloc
	ldr r1, =C$m
	str r1, [r0]
	ldr r2, [r0]
	movw r1, #3
	blx r2
main$L114:
	ldr r9, [fp, #-60]
	sub r10, r9, #1
	str r10, [fp, #-60]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-60]
	b main$L107
main$L112:
	ldr r9, [fp, #-64]
	mov r0, r9
	b main$L114

.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, r1
	mov r1, #0
C$m$L102:
	movw r2, #0
	cmp r0, r2
	bne C$m$L103
C$m$L104:
	mov r0, r1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$m$L103:
	mov r1, #3
	sub r0, r0, #1
	b C$m$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
