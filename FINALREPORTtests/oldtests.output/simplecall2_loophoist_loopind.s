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
	mov r105010101, r101
	ldr r103010101, [r101]
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
	cmp r117, r103010101
	bge C$m$L100
C$m$L102:
	movw r111, #1
	movw r112, #1
	add r107010101, r111, r112
	movw r113, #4
	mul r108010101, r107010101, r113
	ldr r110010101, [r105010101, r108010101]
	mov r0, r110010101
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
	mov r100010101, r0
	movw r121, #3
	str r121, [r100010101]
	movw r122, #1
	str r122, [r100010101, #4]
	movw r123, #2
	str r123, [r100010101, #8]
	movw r124, #3
	str r124, [r100010101, #12]
	movw r125, #4
	mov r0, r125
	bl malloc
	mov r103010101, r0
	ldr r126, =C$m
	str r126, [r103010101]
	mov r101010101, r103010101
	ldr r106010101, [r101010101]
	mov r0, r101010101
	mov r1, r100010101
	blx r106010101
	mov r107010101, r0
	mov r0, r107010101
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
