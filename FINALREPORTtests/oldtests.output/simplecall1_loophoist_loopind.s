.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	movw r102, #100
	mov r0, r102
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
	movw r114, #4
	mov r0, r114
	bl malloc
	mov r102010101, r0
	ldr r115, =C$m
	str r115, [r102010101]
	mov r100010101, r102010101
	ldr r104010101, [r100010101]
	mov r0, r100010101
	blx r104010101
	mov r105010101, r0
	mov r0, r105010101
	bl putint
	movw r116, #10
	mov r0, r116
	bl putch
main$L101:
	movw r117, #0
	mov r0, r117
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
