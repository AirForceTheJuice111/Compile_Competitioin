.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r118, #12
	mov r0, r118
	bl malloc
	mov r1030101, r0
	movw r119, #2
	str r119, [r1030101]
	mov r1000101, r1030101
	ldr r1040101, [r1000101]
	mov r1080101, r1000101
main$L101:
	movw r120, #0
	cmp r120, r1040101
	bge main$L100
main$L102:
	ldr r1150101, [r1080101, #4]
	movw r121, #0
	cmp r1150101, r121
	bne main$L105
main$L106:
main$L107:
	movw r122, #0
	mov r0, r122
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L105:
	movw r123, #1
	mov r0, r123
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L100:
	movw r124, #65535
	movt r124, #65535
	mov r0, r124
	bl exit

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
