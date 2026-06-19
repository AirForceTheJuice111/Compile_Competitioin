.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10401, #47
	mov r10301, #0
	mov r10001, #0
	mov r10101, #1
	mov r10302, r10301
	mov r10102, r10101
	mov r10002, r10001
main$L102:
	cmp r10302, r10401
	blt main$L103
main$L104:
	movw r117, #10
	mov r0, r117
	bl putch
	movw r118, #0
	mov r0, r118
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r119, #0
	cmp r10302, r119
	beq main$L112
main$L113:
	movw r120, #1
	cmp r10302, r120
	beq main$L109
main$L110:
	mov r10201, r10002
	mov r10003, r10102
	add r10103, r10201, r10003
	mov r0, r10103
	bl putint
	mov r10104, r10103
	mov r10004, r10003
main$L111:
	mov r10105, r10104
	mov r10005, r10004
main$L114:
	movw r116, #32
	mov r0, r116
	bl putch
	add r10303, r10302, #1
	mov r10302, r10303
	mov r10102, r10105
	mov r10002, r10005
	b main$L102
main$L109:
	mov r0, r10302
	bl putint
	mov r10104, r10102
	mov r10004, r10002
	b main$L111
main$L112:
	mov r0, r10302
	bl putint
	mov r10105, r10102
	mov r10005, r10002
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
