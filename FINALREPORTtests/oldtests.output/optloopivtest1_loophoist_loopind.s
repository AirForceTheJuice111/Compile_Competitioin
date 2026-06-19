.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r10001, r0
	sub r10305, r10001, #1
	movw r112, #4
	mul r10306, r10305, r112
	add r10303, r10306, #2
	mov r10302, r10303
main$L102:
	movw r115, #65534
	movt r115, #65535
	cmp r10302, r115
	bgt main$L103
main$L104:
	movw r114, #10
	mov r0, r114
	bl putch
	movw r116, #2
	mov r0, r116
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r0, r10302
	bl putint
	sub r10304, r10302, #4
	movw r113, #32
	mov r0, r113
	bl putch
	mov r10302, r10304
	b main$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
