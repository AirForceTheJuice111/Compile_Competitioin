.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r117, #8
	mov r0, r117
	bl malloc
	mov r1020101, r0
	movw r118, #1
	str r118, [r1020101]
	mov r1030101, r1020101
	ldr r1040101, [r1030101]
	mov r1080101, r1030101
main$L101:
	movw r120, #0
	cmp r120, r1040101
	bge main$L100
main$L102:
	movw r119, #2
	str r119, [r1080101, #4]
	movw r121, #0
	mov r0, r121
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L100:
	movw r122, #65535
	movt r122, #65535
	mov r0, r122
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
