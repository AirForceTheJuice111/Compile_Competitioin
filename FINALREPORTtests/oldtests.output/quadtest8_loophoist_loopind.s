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
	movw r102, #300
	mov r0, r102
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
	movw r102, #200
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
	movw r125, #16
	mov r0, r125
	bl malloc
	mov r103010101, r0
	ldr r126, =A$f
	str r126, [r103010101, #8]
	mov r100010101, r103010101
	movw r127, #4
	str r127, [r100010101]
	movw r128, #16
	mov r0, r128
	bl malloc
	mov r104010101, r0
	ldr r129, =B$f
	str r129, [r104010101, #8]
	ldr r130, =B$g
	str r130, [r104010101, #12]
	mov r101010101, r104010101
	add r121010101, r101010101, #4
	movw r131, #2
	str r131, [r101010101, #4]
	mov r100020101, r101010101
	ldr r107010101, [r100020101, #8]
	mov r0, r100020101
	blx r107010101
	mov r108010101, r0
	ldr r122010101, [r121010101]
	add r109010101, r108010101, r122010101
	mov r0, r109010101
	bl putint
	movw r132, #10
	mov r0, r132
	bl putch
	movw r133, #0
	mov r0, r133
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
