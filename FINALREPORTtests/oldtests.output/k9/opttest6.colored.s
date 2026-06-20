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
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r4, #10
	mov r2, #0
	mov r1, r5
main$L102:
	movw r3, #0
	cmp r4, r3
	bgt main$L103
main$L104:
	mov r0, r1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r6, r2, #1
	mov r5, r0
main$L107:
	movw r0, #0
	cmp r5, r0
	bgt main$L108
main$L109:
	sub r4, r4, #1
	mov r0, r5
	mov r2, r6
	b main$L102
main$L108:
	movw r0, #3
	cmp r6, r0
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
	sub r5, r5, #1
	b main$L107
main$L112:
	movw r0, #3
	mul r1, r0, r6
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
