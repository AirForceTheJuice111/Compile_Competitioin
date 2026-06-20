.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r120, #16
	mov r0, r120
	bl malloc
	mov r10001, r0
	movw r121, #3
	str r121, [r10001]
	movw r122, #1
	str r122, [r10001, #4]
	movw r123, #2
	str r123, [r10001, #8]
	movw r124, #3
	str r124, [r10001, #12]
	movw r125, #4
	mov r0, r125
	bl malloc
	mov r10301, r0
	ldr r126, =C$m
	str r126, [r10301]
	mov r10101, r10301
	ldr r10601, [r10101]
	mov r0, r10101
	mov r1, r10001
	blx r10601
	mov r10701, r0
	mov r0, r10701
	bl putint
	movw r127, #10
	mov r0, r127
	bl putch
main$L101:
	movw r128, #0
	mov r0, r128
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

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
	mov r10501, r101
	ldr r10301, [r101]
	movw r114, #1
	movw r115, #0
	cmp r114, r115
	bge C$m$L101
C$m$L100:
	movw r116, #65535
	movt r116, #65535
	mov r0, r116
	bl exit
C$m$L101:
	movw r117, #1
	cmp r117, r10301
	bge C$m$L100
C$m$L102:
	movw r111, #1
	movw r112, #1
	add r10701, r111, r112
	movw r113, #4
	mul r10801, r10701, r113
	ldr r11001, [r10501, r10801]
	mov r0, r11001
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
