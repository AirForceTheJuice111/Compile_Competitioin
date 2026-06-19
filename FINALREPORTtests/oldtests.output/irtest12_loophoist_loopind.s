.balign 4
.global main
.section .text
.arm
main:
main$L106:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r122, #16
	mov r0, r122
	bl malloc
	mov r100010101, r0
	movw r123, #0
	movw r124, #1
	sub r101010101, r123, r124
	movw r125, #3
	str r125, [r100010101]
	movw r126, #1
	str r126, [r100010101, #4]
	movw r127, #2
	str r127, [r100010101, #8]
	movw r128, #3
	str r128, [r100010101, #12]
	mov r107010101, r100010101
	ldr r103010101, [r100010101]
	movw r134, #0
	cmp r101010101, r134
	bge main$L101
main$L100:
	movw r135, #65535
	movt r135, #65535
	mov r0, r135
	bl exit
main$L101:
	cmp r101010101, r103010101
	bge main$L100
main$L102:
	add r115010101, r101010101, #1
	mov r109010101, r100010101
	movw r129, #4
	mul r116010101, r115010101, r129
	movw r130, #4
	str r130, [r107010101, r116010101]
	ldr r104010101, [r100010101]
	movw r136, #2
	movw r137, #0
	cmp r136, r137
	bge main$L104
main$L103:
	movw r138, #65535
	movt r138, #65535
	mov r0, r138
	bl exit
main$L104:
	movw r139, #2
	cmp r139, r104010101
	bge main$L103
main$L105:
	movw r131, #2
	movw r132, #1
	add r119010101, r131, r132
	movw r133, #4
	mul r120010101, r119010101, r133
	ldr r101020101, [r109010101, r120010101]
main$L107:
	movw r140, #0
	mov r0, r140
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
