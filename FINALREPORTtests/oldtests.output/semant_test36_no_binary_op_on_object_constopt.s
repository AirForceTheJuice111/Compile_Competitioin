.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r118, #4
	mov r0, r118
	bl malloc
	mov r10301, r0
	mov r115, #0
	mov r10001, r10301
	movw r119, #4
	mov r0, r119
	bl malloc
	mov r10401, r0
	ldr r11101, [r10001]
	mov r10101, r10401
	ldr r11301, [r10101]
	cmp r11101, r11301
	beq main$L102
	mov r10503, r115
main$L103:
	mov r0, r10503
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	mov r114, #1
	mov r10503, r114
	b main$L103

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
