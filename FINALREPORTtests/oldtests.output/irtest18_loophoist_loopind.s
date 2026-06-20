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
	ldr r102010101, [r100, #12]
	mov r0, r100
	blx r102010101
	mov r103010101, r0
	mov r0, r103010101
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
	movw r133, #20
	mov r0, r133
	bl malloc
	mov r103010101, r0
	ldr r134, =A$f
	str r134, [r103010101, #12]
	mov r100010101, r103010101
	movw r135, #1
	str r135, [r100010101]
	add r127010101, r100010101, #12
	movw r136, #20
	mov r0, r136
	bl malloc
	mov r104010101, r0
	movw r137, #16
	mov r0, r137
	bl malloc
	mov r105010101, r0
	movw r138, #3
	str r138, [r105010101]
	add r123010101, r104010101, #4
	add r124010101, r104010101, #12
	add r125010101, r104010101, #16
	mov r101010101, r104010101
	movw r139, #1
	str r139, [r105010101, #4]
	movw r140, #2
	str r140, [r105010101, #8]
	movw r141, #3
	str r141, [r105010101, #12]
	str r105010101, [r123010101]
	ldr r142, =B$f
	str r142, [r124010101]
	ldr r143, =B$g
	str r143, [r125010101]
	movw r144, #1
	str r144, [r101010101, #8]
	mov r100020101, r101010101
	ldr r109010101, [r127010101]
	mov r0, r100010101
	blx r109010101
	mov r110010101, r0
	mov r0, r110010101
	bl putint
	ldr r112010101, [r100020101, #12]
	mov r0, r100020101
	blx r112010101
	mov r113010101, r0
	mov r0, r113010101
	bl putint
main$L101:
	movw r145, #0
	mov r0, r145
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
	movw r102, #0
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
B$f$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r101, r0
	ldr r112010101, [r100]
	ldr r114010101, [r101, #8]
	ldr r103010101, [r101, #4]
	add r107010101, r112010101, r114010101
	ldr r104010101, [r103010101]
	mov r106010101, r103010101
	movw r127, #0
	movw r128, #0
	cmp r127, r128
	bge B$f$L101
B$f$L100:
	movw r129, #65535
	movt r129, #65535
	mov r0, r129
	bl exit
B$f$L101:
	movw r130, #0
	cmp r130, r104010101
	bge B$f$L100
B$f$L102:
	movw r124, #0
	movw r125, #1
	add r117010101, r124, r125
	add r121010101, r101, #16
	movw r126, #4
	mul r118010101, r117010101, r126
	ldr r120010101, [r106010101, r118010101]
	ldr r108010101, [r121010101]
	add r110010101, r107010101, r120010101
	mov r0, r101
	blx r108010101
	mov r109010101, r0
	add r123010101, r110010101, r109010101
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
