.balign 4
.global C$f
.section .text
.arm
C$f:
C$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
C$f$L101:
	movw r102, #0
	mov r0, r102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global A$f
.section .text
.arm
A$f:
A$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
A$f$L101:
	movw r102, #0
	mov r0, r102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$g
.section .text
.arm
B$g:
B$g$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
B$g$L101:
	movw r102, #0
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
main$L101:
	movw r101, #0
	mov r0, r101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global D$f
.section .text
.arm
D$f:
D$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	ldr r1050101, [r100, #16]
	ldr r1070101, [r100, #24]
	add r1110101, r100, #28
	ldr r1090101, [r1070101, #16]
	add r1020101, r1050101, r1090101
	mov r0, r1020101
	bl putint
	ldr r1120101, [r1110101]
	mov r0, r1120101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
B$f$L101:
	movw r102, #0
	mov r0, r102
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
