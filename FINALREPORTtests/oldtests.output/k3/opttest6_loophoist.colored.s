.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r0, r1
	mov r2, #0
	mov r1, #3
C$m$L102:
	movw r10, #0
	str r10, [fp, #-40]
	ldr r10, [fp, #-40]
	cmp r0, r10
	bne C$m$L103
C$m$L104:
	mov r0, r2
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
C$m$L103:
	sub r0, r0, #1
	mov r2, r1
	b C$m$L102

.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #28
	add fp, sp, #60
	bl getint
	mov r10, #10
	str r10, [fp, #-60]
	mov r1, #0
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-52]
main$L102:
	movw r2, #0
	ldr r9, [fp, #-60]
	cmp r9, r2
	bgt main$L103
main$L104:
	ldr r9, [fp, #-52]
	mov r0, r9
	sub sp, fp, #60
	add sp, sp, #28
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r1, #1
	str r10, [fp, #-44]
	movw r1, #3
	ldr r10, [fp, #-44]
	mul r10, r1, r10
	str r10, [fp, #-56]
	ldr r9, [fp, #-52]
	mov r1, r9
	mov r10, r0
	str r10, [fp, #-48]
main$L107:
	movw r0, #0
	ldr r9, [fp, #-48]
	cmp r9, r0
	bgt main$L108
main$L109:
	ldr r9, [fp, #-60]
	sub r10, r9, #1
	str r10, [fp, #-60]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-60]
	mov r10, r1
	str r10, [fp, #-52]
	ldr r9, [fp, #-48]
	mov r0, r9
	ldr r9, [fp, #-44]
	mov r1, r9
	b main$L102
main$L108:
	movw r0, #3
	ldr r9, [fp, #-44]
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
	mov r1, r0
main$L114:
	ldr r9, [fp, #-48]
	sub r10, r9, #1
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	b main$L107
main$L112:
	ldr r9, [fp, #-56]
	mov r1, r9
	b main$L114

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
