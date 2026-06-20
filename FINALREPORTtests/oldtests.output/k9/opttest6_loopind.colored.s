.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r1, r0
	mov r4, #10
	mov r2, #0
	add r0, r2, #1
	movw r3, #3
	mul r3, r0, r3
	mov r0, r5
	mov r5, r3
main$L102:
	movw r3, #0
	cmp r4, r3
	bgt main$L103
main$L104:
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r7, r2, #1
	mov r6, r1
main$L107:
	movw r1, #0
	cmp r6, r1
	bgt main$L108
main$L109:
	sub r4, r4, #1
	add r5, r5, #3
	mov r1, r6
	mov r2, r7
	b main$L102
main$L108:
	movw r0, #3
	cmp r7, r0
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
	sub r6, r6, #1
	b main$L107
main$L112:
	mov r0, r5
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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
