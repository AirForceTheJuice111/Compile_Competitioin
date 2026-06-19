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
	sub sp, sp, #20
	add fp, sp, #52
	bl getint
	mov r2, r0
	mov r0, #10
	mov r4, #0
	add r1, r4, #1
	movw r3, #3
	mul r1, r1, r3
	mov r10, r0
	str r10, [fp, #-52]
	ldr r9, [fp, #-40]
	mov r0, r9
	mov r3, r4
	mov r10, r1
	str r10, [fp, #-48]
main$L102:
	movw r1, #0
	ldr r9, [fp, #-52]
	cmp r9, r1
	bgt main$L103
main$L104:
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r3, #1
	str r10, [fp, #-44]
	mov r4, r2
main$L107:
	movw r1, #0
	cmp r4, r1
	bgt main$L108
main$L109:
	ldr r9, [fp, #-52]
	sub r10, r9, #1
	str r10, [fp, #-52]
	ldr r9, [fp, #-48]
	add r10, r9, #3
	str r10, [fp, #-48]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-52]
	mov r2, r4
	ldr r9, [fp, #-44]
	mov r3, r9
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
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
main$L114:
	sub r4, r4, #1
	b main$L107
main$L112:
	ldr r9, [fp, #-48]
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
