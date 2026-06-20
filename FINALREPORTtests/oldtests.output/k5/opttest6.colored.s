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
C$m$L102:
	movw r2, #0
	cmp r1, r2
	bne C$m$L103
C$m$L104:
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$m$L103:
	mov r0, #3
	sub r1, r1, #1
	b C$m$L102

.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	bl getint
	mov r1, r0
	mov r10, #10
	str r10, [fp, #-44]
	mov r2, #0
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	mov r0, r4
main$L102:
	movw r3, #0
	ldr r9, [fp, #-44]
	cmp r9, r3
	bgt main$L103
main$L104:
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r2, #1
	str r10, [fp, #-40]
	mov r4, r1
main$L107:
	movw r1, #0
	cmp r4, r1
	bgt main$L108
main$L109:
	ldr r9, [fp, #-44]
	sub r10, r9, #1
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	mov r1, r4
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
	sub r4, r4, #1
	b main$L107
main$L112:
	movw r0, #3
	ldr r10, [fp, #-40]
	mul r0, r0, r10
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
