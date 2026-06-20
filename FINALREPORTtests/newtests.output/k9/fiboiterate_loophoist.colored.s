.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, #47
	mov r7, #0
	mov r6, #0
	mov r5, #1
main$L102:
	cmp r7, r4
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
	movw r0, #0
	cmp r7, r0
	beq main$L112
main$L113:
	movw r0, #1
	cmp r7, r0
	beq main$L109
main$L110:
	mov r0, r6
	mov r6, r5
	add r5, r0, r6
	mov r0, r5
	bl putint
	mov r0, r5
main$L111:
	mov r5, r0
main$L114:
	movw r0, #32
	bl putch
	add r7, r7, #1
	b main$L102
main$L109:
	mov r0, r7
	bl putint
	mov r0, r5
	b main$L111
main$L112:
	mov r0, r7
	bl putint
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
