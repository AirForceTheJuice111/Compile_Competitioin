.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1010101, #1
	add r1090101, r1010101, #1
	movw r115, #4
	mul r1050101, r1090101, r115
	mov r0, r1050101
	bl malloc
	mov r1020101, r0
	str r1010101, [r1020101]
	mov r1030101, r1020101
	ldr r1040101, [r1030101]
	mov r1080101, r1030101
	movw r120, #0
	movw r121, #0
	cmp r120, r121
	bge main$L101
main$L100:
	movw r122, #65535
	movt r122, #65535
	mov r0, r122
	bl exit
main$L101:
	movw r123, #0
	cmp r123, r1040101
	bge main$L100
main$L102:
	movw r116, #0
	movw r117, #1
	add r1120101, r116, r117
	movw r118, #4
	mul r1130101, r1120101, r118
	movw r119, #0
	str r119, [r1080101, r1130101]
main$L104:
	movw r124, #0
	mov r0, r124
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
