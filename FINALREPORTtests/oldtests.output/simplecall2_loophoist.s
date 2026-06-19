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
	cmp r117, r1030101
	bge C$m$L100
C$m$L102:
	movw r111, #1
	movw r112, #1
	add r1070101, r111, r112
	movw r113, #4
	mul r1080101, r1070101, r113
	ldr r1100101, [r1050101, r1080101]
	mov r0, r1100101
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
	movw r120, #16
	mov r0, r120
	bl malloc
	mov r1000101, r0
	movw r121, #3
	str r121, [r1000101]
	movw r122, #1
	str r122, [r1000101, #4]
	movw r123, #2
	str r123, [r1000101, #8]
	movw r124, #3
	str r124, [r1000101, #12]
	movw r125, #4
	mov r0, r125
	bl malloc
	mov r1030101, r0
	ldr r126, =C$m
	str r126, [r1030101]
	mov r1010101, r1030101
	ldr r1060101, [r1010101]
	mov r0, r1010101
	mov r1, r1000101
	blx r1060101
	mov r1070101, r0
	mov r0, r1070101
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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
