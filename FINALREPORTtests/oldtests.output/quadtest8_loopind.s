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
	mov r1030101, r0
	ldr r126, =A$f
	str r126, [r1030101, #8]
	mov r1000101, r1030101
	movw r127, #4
	str r127, [r1000101]
	movw r128, #16
	mov r0, r128
	bl malloc
	mov r1040101, r0
	ldr r129, =B$f
	str r129, [r1040101, #8]
	ldr r130, =B$g
	str r130, [r1040101, #12]
	mov r1010101, r1040101
	movw r131, #2
	str r131, [r1010101, #4]
	mov r1000201, r1010101
	add r1210101, r1010101, #4
	ldr r1070101, [r1000201, #8]
	mov r0, r1000201
	blx r1070101
	mov r1080101, r0
	ldr r1220101, [r1210101]
	add r1090101, r1080101, r1220101
	mov r0, r1090101
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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
