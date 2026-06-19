.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100010101, r101
	ldr r103010101, [r100010101, #4]
	mov r106010101, r103010101
	ldr r104010101, [r103010101]
	movw r116, #1
	movw r117, #0
	cmp r116, r117
	bge main$L101
main$L100:
	movw r118, #65535
	movt r118, #65535
	mov r0, r118
	bl exit
main$L101:
	movw r119, #1
	cmp r119, r104010101
	bge main$L100
main$L102:
	movw r112, #1
	movw r113, #1
	add r109010101, r112, r113
	movw r114, #4
	mul r110010101, r109010101, r114
	movw r115, #1
	str r115, [r106010101, r110010101]
main$L104:
	movw r120, #0
	mov r0, r120
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
