.balign 4
.global main
.section .text
.arm
main:
main$L106:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r124, #20
	mov r0, r124
	bl malloc
	mov r100010101, r0
	movw r125, #1
	movw r126, #1
	add r101010101, r125, r126
	movw r127, #4
	str r127, [r100010101]
	movw r128, #1
	str r128, [r100010101, #4]
	movw r129, #2
	str r129, [r100010101, #8]
	movw r130, #3
	str r130, [r100010101, #12]
	movw r131, #4
	str r131, [r100010101, #16]
	mov r107010101, r100010101
	ldr r103010101, [r100010101]
	movw r137, #0
	cmp r101010101, r137
	bge main$L101
main$L100:
	movw r138, #65535
	movt r138, #65535
	mov r0, r138
	bl exit
main$L101:
	cmp r101010101, r103010101
	bge main$L100
main$L102:
	add r116010101, r101010101, #1
	mov r109010101, r100010101
	movw r132, #4
	mul r117010101, r116010101, r132
	movw r133, #7
	str r133, [r107010101, r117010101]
	ldr r104010101, [r100010101]
	movw r139, #2
	movw r140, #0
	cmp r139, r140
	bge main$L104
main$L103:
	movw r141, #65535
	movt r141, #65535
	mov r0, r141
	bl exit
main$L104:
	movw r142, #2
	cmp r142, r104010101
	bge main$L103
main$L105:
	movw r134, #2
	movw r135, #1
	add r120010101, r134, r135
	movw r136, #4
	mul r121010101, r120010101, r136
	ldr r123010101, [r109010101, r121010101]
	mov r0, r123010101
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
