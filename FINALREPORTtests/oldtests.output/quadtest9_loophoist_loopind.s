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
	movw r113, #4
	str r113, [r100]
	movw r114, #30
	str r114, [r100, #8]
	movw r115, #100
	str r115, [r100, #12]
	ldr r106010101, [r100]
	ldr r108010101, [r100, #8]
	ldr r111010101, [r100, #12]
	add r109010101, r106010101, r108010101
	add r112010101, r109010101, r111010101
	mov r0, r112010101
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
	movw r104, #1
	str r104, [r100]
	movw r105, #2
	str r105, [r100, #4]
	movw r106, #0
	mov r0, r106
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
	movw r152, #20
	mov r0, r152
	bl malloc
	mov r103010101, r0
	ldr r153, =A$f
	str r153, [r103010101, #16]
	mov r100010101, r103010101
	movw r154, #20
	mov r0, r154
	bl malloc
	mov r104010101, r0
	add r121010101, r100010101, #16
	mov r125010101, r100010101
	add r127010101, r100010101, #4
	ldr r155, =B$f
	str r155, [r104010101, #16]
	mov r101010101, r104010101
	ldr r107010101, [r121010101]
	mov r0, r100010101
	blx r107010101
	ldr r109010101, [r101010101, #16]
	mov r0, r101010101
	blx r109010101
	mov r130010101, r101010101
	add r133010101, r101010101, #8
	add r136010101, r101010101, #12
	mov r100020101, r101010101
	mov r142010101, r101010101
	add r145010101, r101010101, #8
	add r148010101, r101010101, #12
	ldr r126010101, [r125010101]
	ldr r128010101, [r127010101]
	ldr r131010101, [r130010101]
	ldr r134010101, [r133010101]
	ldr r137010101, [r136010101]
	mov r140010101, r100020101
	add r129010101, r126010101, r128010101
	add r132010101, r129010101, r131010101
	add r135010101, r132010101, r134010101
	add r111010101, r135010101, r137010101
	mov r0, r111010101
	bl putint
	movw r156, #10
	mov r0, r156
	bl putch
	ldr r141010101, [r140010101]
	ldr r143010101, [r142010101]
	ldr r146010101, [r145010101]
	ldr r149010101, [r148010101]
	add r144010101, r141010101, r143010101
	add r147010101, r144010101, r146010101
	add r114010101, r147010101, r149010101
	mov r0, r114010101
	bl putint
	movw r157, #10
	mov r0, r157
	bl putch
	movw r158, #1
	mov r0, r158
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
