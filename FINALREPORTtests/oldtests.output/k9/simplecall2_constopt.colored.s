.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	ldr r0, [r1]
C$m$L101:
	movw r2, #1
	cmp r2, r0
	bge C$m$L100
C$m$L102:
	ldr r0, [r1, #8]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$m$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit

.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #16
	bl malloc
	mov r4, r0
	movw r0, #3
	str r0, [r4]
	movw r0, #1
	str r0, [r4, #4]
	movw r0, #2
	str r0, [r4, #8]
	movw r0, #3
	str r0, [r4, #12]
	movw r0, #4
	bl malloc
	ldr r1, =C$m
	str r1, [r0]
	ldr r2, [r0]
	mov r1, r4
	blx r2
	bl putint
	movw r0, #10
	bl putch
main$L101:
	movw r0, #0
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
