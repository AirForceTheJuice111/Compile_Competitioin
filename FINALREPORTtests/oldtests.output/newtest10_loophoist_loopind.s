.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100010101, #0
	mov r103010101, #100
	mov r111010101, r100010101
	add r122010101, r100010101, #1
	mul r125010101, r100010101, r100010101
	add r119010101, r103010101, #1
	movw r132, #4
	mul r123010101, r122010101, r132
	movw r133, #4
	mul r109010101, r119010101, r133
	mov r0, r109010101
	bl malloc
	mov r104010101, r0
	str r103010101, [r104010101]
	mov r101010101, r104010101
	mov r113010101, r101010101
	add r124010101, r113010101, r123010101
main$L102:
	ldr r105010101, [r101010101]
	cmp r111010101, r105010101
	blt main$L103
main$L104:
	ldr r107010101, [r101010101]
	movw r138, #99
	cmp r107010101, r138
	bgt main$L113
main$L114:
main$L115:
	movw r137, #10
	mov r0, r137
	bl putch
	movw r139, #1
	mov r0, r139
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L113:
	mov r115010101, r101010101
	ldr r108010101, [r101010101]
	movw r140, #99
	movw r141, #0
	cmp r140, r141
	bge main$L111
main$L110:
	movw r142, #65535
	movt r142, #65535
	mov r0, r142
	bl exit
main$L111:
	movw r143, #99
	cmp r143, r108010101
	bge main$L110
main$L112:
	movw r134, #99
	movw r135, #1
	add r127010101, r134, r135
	movw r136, #4
	mul r128010101, r127010101, r136
	ldr r116010101, [r115010101, r128010101]
	mov r0, r116010101
	bl putint
	b main$L115
main$L103:
	ldr r106010101, [r101010101]
	movw r144, #0
	cmp r100010101, r144
	bge main$L106
main$L105:
	movw r145, #65535
	movt r145, #65535
	mov r0, r145
	bl exit
main$L106:
	cmp r100010101, r106010101
	bge main$L105
main$L107:
	str r125010101, [r124010101]
	b main$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
