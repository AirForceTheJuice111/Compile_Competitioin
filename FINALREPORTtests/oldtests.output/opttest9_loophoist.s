.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10501, #10
	mov r10001, #0
	mov r10401, #100
	mov r10002, #10
	bl getint
	mov r10301, r0
	add r10101, r10002, #1
	add r10201, r10002, r10101
	movw r109, #3
	mul r10404, r109, r10201
	mov r10502, r10501
	mov r10402, r10401
	mov r10302, r10301
main$L102:
	movw r110, #0
	cmp r10502, r110
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
	movw r111, #0
	cmp r10303, r111
	bgt main$L108
main$L109:
	sub r10503, r10502, #1
	mov r10502, r10503
	mov r10402, r10403
	mov r10302, r10303
	b main$L102
main$L108:
	sub r10304, r10303, #1
	mov r10403, r10404
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
