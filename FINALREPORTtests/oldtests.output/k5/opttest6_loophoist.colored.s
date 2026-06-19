.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, #0
	mov r2, #3
C$m$L102:
	movw r3, #0
	cmp r1, r3
	bne C$m$L103
C$m$L104:
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$m$L103:
	sub r1, r1, #1
	mov r0, r2
	b C$m$L102

.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	bl getint
	mov r1, r0
	mov r10, #10
	str r10, [fp, #-48]
	mov r2, #0
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	mov r0, r4
main$L102:
	movw r3, #0
	ldr r9, [fp, #-48]
	cmp r9, r3
	bgt main$L103
main$L104:
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r2, #1
	str r10, [fp, #-40]
	movw r2, #3
	ldr r10, [fp, #-40]
	mul r4, r2, r10
	mov r10, r1
	str r10, [fp, #-44]
main$L107:
	movw r1, #0
	ldr r9, [fp, #-44]
	cmp r9, r1
	bgt main$L108
main$L109:
	ldr r9, [fp, #-48]
	sub r10, r9, #1
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	ldr r9, [fp, #-44]
	mov r1, r9
	ldr r9, [fp, #-40]
	mov r2, r9
	b main$L102
main$L108:
	movw r0, #3
	ldr r9, [fp, #-40]
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
	ldr r9, [fp, #-44]
	sub r10, r9, #1
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L107
main$L112:
	mov r0, r4
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
