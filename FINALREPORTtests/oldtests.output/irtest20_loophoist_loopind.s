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
	add r104010101, r100, #8
	add r107010101, r100, #12
	ldr r103010101, [r100]
	ldr r105010101, [r104010101]
	ldr r108010101, [r107010101]
	add r106010101, r103010101, r105010101
	add r109010101, r106010101, r108010101
	mov r0, r109010101
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
	ldr r108010101, [r100]
	ldr r110010101, [r100, #4]
	ldr r113010101, [r101]
	ldr r116010101, [r101, #8]
	ldr r119010101, [r101, #12]
	mov r100010101, r101
	mov r126010101, r101
	add r129010101, r101, #8
	add r132010101, r101, #12
	add r111010101, r108010101, r110010101
	add r114010101, r111010101, r113010101
	add r117010101, r114010101, r116010101
	add r103010101, r117010101, r119010101
	mov r0, r103010101
	bl putint
	ldr r122010101, [r100010101]
	ldr r124010101, [r100010101, #4]
	ldr r127010101, [r126010101]
	ldr r130010101, [r129010101]
	ldr r133010101, [r132010101]
	add r125010101, r122010101, r124010101
	add r128010101, r125010101, r127010101
	add r131010101, r128010101, r130010101
	add r105010101, r131010101, r133010101
	mov r0, r105010101
	bl putint
main$L101:
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
