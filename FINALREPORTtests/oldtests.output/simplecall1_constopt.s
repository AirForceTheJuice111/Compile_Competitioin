.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r116, #4
	mov r0, r116
	bl malloc
	mov r1020101, r0
	ldr r117, =C$m
	str r117, [r1020101]
	mov r1000101, r1020101
	ldr r1040101, [r1000101]
	mov r0, r1000101
	blx r1040101
	mov r1050101, r0
	mov r0, r1050101
	bl putint
	movw r118, #10
	mov r0, r118
	bl putch
main$L101:
	movw r119, #0
	mov r0, r119
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	movw r104, #100
	mov r0, r104
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
