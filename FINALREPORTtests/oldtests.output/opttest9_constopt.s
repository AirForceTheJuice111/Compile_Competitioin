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
	mov r10301, r0
	mov r109, #10
	mov r110, #100
	mov r10502, r109
	mov r10402, r110
	mov r10302, r10301
main$L102:
	movw r114, #0
	cmp r10502, r114
	bgt main$L103
main$L104:
	mov r0, r10402
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r10403, r10402
	mov r10303, r10302
main$L107:
	movw r115, #0
	cmp r10303, r115
	bgt main$L108
main$L109:
	sub r10503, r10502, #1
	mov r10502, r10503
	mov r10402, r10403
	mov r10302, r10303
	b main$L102
main$L108:
	sub r10304, r10303, #1
	mov r111, #63
	mov r10403, r111
	mov r10303, r10304
	b main$L107

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
