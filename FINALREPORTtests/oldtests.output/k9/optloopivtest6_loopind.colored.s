.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, #10
	mov r5, #0
	bl getint
	mov r6, r0
	movw r0, #3
	mul r0, r4, r0
	add r4, r0, #2
main$L102:
	movw r0, #2
	cmp r4, r0
	bgt main$L103
main$L104:
	mov r0, r5
	bl putint
	movw r0, #10
	bl putch
	mov r0, r5
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r0, #0
	cmp r6, r0
	bgt main$L107
main$L108:
	mov r5, #4
main$L109:
	sub r4, r4, #3
	b main$L102
main$L107:
	mov r0, r4
	bl putint
	movw r0, #32
	bl putch
	mov r5, r4
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
