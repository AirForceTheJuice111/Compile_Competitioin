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
	ldr r10201, [r100, #12]
	mov r0, r100
	blx r10201
	mov r10301, r0
	mov r0, r10301
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
	ldr r11201, [r100]
	ldr r11401, [r101, #8]
	ldr r10301, [r101, #4]
	add r10701, r11201, r11401
	ldr r10401, [r10301]
	mov r10601, r10301
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
	cmp r130, r10401
	bge B$f$L100
B$f$L102:
	movw r124, #0
	movw r125, #1
	add r11701, r124, r125
	add r12101, r101, #16
	movw r126, #4
	mul r11801, r11701, r126
	ldr r12001, [r10601, r11801]
	ldr r10801, [r12101]
	add r11001, r10701, r12001
	mov r0, r101
	blx r10801
	mov r10901, r0
	add r12301, r11001, r10901
	mov r0, r12301
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
	mov r10301, r0
	ldr r134, =A$f
	str r134, [r10301, #12]
	mov r10001, r10301
	movw r135, #1
	str r135, [r10001]
	add r12701, r10001, #12
	movw r136, #20
	mov r0, r136
	bl malloc
	mov r10401, r0
	movw r137, #16
	mov r0, r137
	bl malloc
	mov r10501, r0
	movw r138, #3
	str r138, [r10501]
	add r12301, r10401, #4
	add r12401, r10401, #12
	add r12501, r10401, #16
	mov r10101, r10401
	movw r139, #1
	str r139, [r10501, #4]
	movw r140, #2
	str r140, [r10501, #8]
	movw r141, #3
	str r141, [r10501, #12]
	str r10501, [r12301]
	ldr r142, =B$f
	str r142, [r12401]
	ldr r143, =B$g
	str r143, [r12501]
	movw r144, #1
	str r144, [r10101, #8]
	mov r10002, r10101
	ldr r10901, [r12701]
	mov r0, r10001
	blx r10901
	mov r11001, r0
	mov r0, r11001
	bl putint
	ldr r11201, [r10002, #12]
	mov r0, r10002
	blx r11201
	mov r11301, r0
	mov r0, r11301
	bl putint
main$L101:
	movw r145, #0
	mov r0, r145
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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
