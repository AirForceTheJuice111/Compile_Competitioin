.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r7, r0
	mov r4, #10
	mov r6, #0
	mov r5, #4
main$L102:
	movw r0, #0
	cmp r4, r0
	bgt main$L103
main$L104:
	mov r0, r6
	bl putint
	movw r0, #10
	bl putch
	mov r0, r6
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r0, #0
	cmp r7, r0
	bgt main$L107
main$L108:
	mov r6, r5
main$L109:
	sub r4, r4, #1
	b main$L102
main$L107:
	movw r0, #3
	mul r0, r0, r4
	add r6, r0, #2
	mov r0, r6
	bl putint
	movw r0, #32
	bl putch
	b main$L109

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
