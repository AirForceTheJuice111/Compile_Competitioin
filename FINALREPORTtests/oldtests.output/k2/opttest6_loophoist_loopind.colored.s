.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	mov r0, r1
	mov r10, #0
	str r10, [fp, #-44]
	mov r1, #3
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
C$m$L102:
	movw r10, #0
	str r10, [fp, #-40]
	ldr r10, [fp, #-40]
	cmp r0, r10
	bne C$m$L103
C$m$L104:
	ldr r9, [fp, #-44]
	mov r0, r9
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
C$m$L103:
	sub r0, r0, #1
	mov r10, r1
	str r10, [fp, #-44]
	b C$m$L102

.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #48
	add fp, sp, #80
	bl getint
	mov r10, r0
	str r10, [fp, #-56]
	mov r10, #10
	str r10, [fp, #-72]
	mov r10, #0
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	add r0, r9, #1
	movw r1, #3
	mul r0, r0, r1
	ldr r9, [fp, #-72]
	mov r10, r9
	str r10, [fp, #-76]
	ldr r9, [fp, #-40]
	mov r1, r9
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-48]
	mov r10, r0
	str r10, [fp, #-68]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-76]
	cmp r9, r0
	bgt main$L103
main$L104:
	mov r0, r1
	sub sp, fp, #80
	add sp, sp, #48
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-48]
	add r10, r9, #1
	str r10, [fp, #-52]
	mov r0, r1
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-64]
main$L107:
	movw r1, #0
	ldr r9, [fp, #-64]
	cmp r9, r1
	bgt main$L108
main$L109:
	ldr r9, [fp, #-76]
	sub r10, r9, #1
	str r10, [fp, #-76]
	ldr r9, [fp, #-68]
	add r10, r9, #3
	str r10, [fp, #-68]
	ldr r9, [fp, #-76]
	mov r10, r9
	str r10, [fp, #-76]
	mov r1, r0
	ldr r9, [fp, #-64]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-48]
	ldr r9, [fp, #-68]
	mov r10, r9
	str r10, [fp, #-68]
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
	ldr r10, [r0]
	str r10, [fp, #-80]
	movw r1, #3
	ldr r9, [fp, #-80]
	blx r9
main$L114:
	ldr r9, [fp, #-64]
	sub r10, r9, #1
	str r10, [fp, #-64]
	ldr r9, [fp, #-64]
	mov r10, r9
	str r10, [fp, #-64]
	b main$L107
main$L112:
	ldr r9, [fp, #-68]
	mov r0, r9
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
