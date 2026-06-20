.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r112, #4
	mov r0, r112
	bl malloc
	mov r1010101, r0
	ldr r113, =C$max
	str r113, [r1010101]
	mov r1020101, r1010101
	ldr r1040101, [r1020101]
	movw r114, #100
	movw r115, #200
	mov r0, r1020101
	mov r1, r114
	mov r2, r115
	blx r1040101
	mov r1050101, r0
	mov r0, r1050101
	bl putint
	movw r116, #1
	mov r0, r116
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global C$max
.section .text
.arm
C$max:
C$max$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	mov r101, r1
	mov r102, r2
	cmp r101, r102
	bgt C$max$L102
C$max$L103:
	mov r0, r102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$max$L102:
	mov r0, r101
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
