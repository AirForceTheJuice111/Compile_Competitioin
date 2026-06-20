.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	mov r101, r1
	mov r1050101, r101
	ldr r1030101, [r101]
C$m$L101:
	movw r113, #1
	cmp r113, r1030101
	bge C$m$L100
C$m$L102:
	ldr r1100101, [r1050101, #8]
	mov r0, r1100101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$m$L100:
	movw r114, #65535
	movt r114, #65535
	mov r0, r114
	bl exit

.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r122, #16
	mov r0, r122
	bl malloc
	mov r1000101, r0
	movw r123, #3
	str r123, [r1000101]
	movw r124, #1
	str r124, [r1000101, #4]
	movw r125, #2
	str r125, [r1000101, #8]
	movw r126, #3
	str r126, [r1000101, #12]
	movw r127, #4
	mov r0, r127
	bl malloc
	mov r1030101, r0
	ldr r128, =C$m
	str r128, [r1030101]
	mov r1010101, r1030101
	ldr r1060101, [r1010101]
	mov r0, r1010101
	mov r1, r1000101
	blx r1060101
	mov r1070101, r0
	mov r0, r1070101
	bl putint
	movw r129, #10
	mov r0, r129
	bl putch
main$L101:
	movw r130, #0
	mov r0, r130
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
