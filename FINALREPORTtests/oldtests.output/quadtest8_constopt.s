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
	movw r104, #200
	mov r0, r104
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
	movw r104, #100
	mov r0, r104
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
	movw r104, #300
	mov r0, r104
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
	movw r127, #16
	mov r0, r127
	bl malloc
	mov r1030101, r0
	ldr r128, =A$f
	str r128, [r1030101, #8]
	mov r1000101, r1030101
	movw r129, #4
	str r129, [r1000101]
	movw r130, #16
	mov r0, r130
	bl malloc
	mov r1040101, r0
	ldr r131, =B$f
	str r131, [r1040101, #8]
	ldr r132, =B$g
	str r132, [r1040101, #12]
	mov r1010101, r1040101
	movw r133, #2
	str r133, [r1010101, #4]
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
	movw r134, #10
	mov r0, r134
	bl putch
	movw r135, #0
	mov r0, r135
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
