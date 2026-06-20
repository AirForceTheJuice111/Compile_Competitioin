.balign 4
.global B$g
.section .text
.arm
B$g:
B$g$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	ldr r1020101, [r100, #12]
	mov r0, r100
	blx r1020101
	mov r1030101, r0
	mov r0, r1030101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global A$f
.section .text
.arm
A$f:
A$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	movw r102, #0
	mov r0, r102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r101, r0
	ldr r1120101, [r100]
	ldr r1140101, [r101, #8]
	ldr r1030101, [r101, #4]
	add r1070101, r1120101, r1140101
	ldr r1040101, [r1030101]
	mov r1060101, r1030101
	movw r127, #0
	movw r128, #0
	cmp r127, r128
	bge B$f$L101
B$f$L100:
	movw r129, #65535
	movt r129, #65535
	mov r0, r129
	bl exit
B$f$L101:
	movw r130, #0
	cmp r130, r1040101
	bge B$f$L100
B$f$L102:
	movw r124, #0
	movw r125, #1
	add r1170101, r124, r125
	add r1210101, r101, #16
	movw r126, #4
	mul r1180101, r1170101, r126
	ldr r1200101, [r1060101, r1180101]
	ldr r1080101, [r1210101]
	add r1100101, r1070101, r1200101
	mov r0, r101
	blx r1080101
	mov r1090101, r0
	add r1230101, r1100101, r1090101
	mov r0, r1230101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r133, #20
	mov r0, r133
	bl malloc
	mov r1030101, r0
	ldr r134, =A$f
	str r134, [r1030101, #12]
	mov r1000101, r1030101
	movw r135, #1
	str r135, [r1000101]
	add r1270101, r1000101, #12
	movw r136, #20
	mov r0, r136
	bl malloc
	mov r1040101, r0
	movw r137, #16
	mov r0, r137
	bl malloc
	mov r1050101, r0
	movw r138, #3
	str r138, [r1050101]
	add r1230101, r1040101, #4
	add r1240101, r1040101, #12
	add r1250101, r1040101, #16
	mov r1010101, r1040101
	movw r139, #1
	str r139, [r1050101, #4]
	movw r140, #2
	str r140, [r1050101, #8]
	movw r141, #3
	str r141, [r1050101, #12]
	str r1050101, [r1230101]
	ldr r142, =B$f
	str r142, [r1240101]
	ldr r143, =B$g
	str r143, [r1250101]
	movw r144, #1
	str r144, [r1010101, #8]
	mov r1000201, r1010101
	ldr r1090101, [r1270101]
	mov r0, r1000101
	blx r1090101
	mov r1100101, r0
	mov r0, r1100101
	bl putint
	ldr r1120101, [r1000201, #12]
	mov r0, r1000201
	blx r1120101
	mov r1130101, r0
	mov r0, r1130101
	bl putint
main$L101:
	movw r145, #0
	mov r0, r145
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
