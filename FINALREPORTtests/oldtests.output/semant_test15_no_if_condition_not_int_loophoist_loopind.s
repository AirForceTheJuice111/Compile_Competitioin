.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r102010101, #2
	add r109010101, r102010101, #1
	movw r116, #4
	mul r105010101, r109010101, r116
	mov r0, r105010101
	bl malloc
	mov r103010101, r0
	str r102010101, [r103010101]
	mov r100010101, r103010101
	ldr r104010101, [r100010101]
	mov r108010101, r100010101
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
	cmp r123, r104010101
	bge main$L100
main$L102:
	movw r117, #0
	movw r118, #1
	add r112010101, r117, r118
	movw r119, #4
	mul r113010101, r112010101, r119
	ldr r115010101, [r108010101, r113010101]
	movw r124, #0
	cmp r115010101, r124
	bne main$L105
main$L106:
main$L107:
	movw r125, #0
	mov r0, r125
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L105:
	movw r126, #1
	mov r0, r126
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
