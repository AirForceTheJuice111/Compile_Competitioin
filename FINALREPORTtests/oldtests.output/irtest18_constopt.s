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
B$f$L101:
	movw r126, #0
	cmp r126, r1040101
	bge B$f$L100
B$f$L102:
	ldr r1200101, [r1060101, #4]
	ldr r1080101, [r101, #16]
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
B$f$L100:
	movw r127, #65535
	movt r127, #65535
	mov r0, r127
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
	movw r135, #20
	mov r0, r135
	bl malloc
	mov r1030101, r0
	ldr r136, =A$f
	str r136, [r1030101, #12]
	mov r1000101, r1030101
	movw r137, #1
	str r137, [r1000101]
	add r1270101, r1000101, #12
	movw r138, #20
	mov r0, r138
	bl malloc
	mov r1040101, r0
	movw r139, #16
	mov r0, r139
	bl malloc
	mov r1050101, r0
	movw r140, #3
	str r140, [r1050101]
	add r1230101, r1040101, #4
	add r1240101, r1040101, #12
	add r1250101, r1040101, #16
	mov r1010101, r1040101
	movw r141, #1
	str r141, [r1050101, #4]
	movw r142, #2
	str r142, [r1050101, #8]
	movw r143, #3
	str r143, [r1050101, #12]
	str r1050101, [r1230101]
	ldr r144, =B$f
	str r144, [r1240101]
	ldr r145, =B$g
	str r145, [r1250101]
	movw r146, #1
	str r146, [r1010101, #8]
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
	movw r147, #0
	mov r0, r147
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
	movw r104, #0
	mov r0, r104
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
