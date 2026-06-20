.balign 4
.global List$add
.section .text
.arm
List$add:
List$add$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #24
	add fp, sp, #56
	mov r10, r0
	str r10, [fp, #-40]
	mov r10, r1
	str r10, [fp, #-44]
	ldr r9, [fp, #-40]
	ldr r0, [r9, #4]
	movw r1, #0
	cmp r0, r1
	bne List$add$L102
List$add$L103:
	ldr r9, [fp, #-40]
	ldr r0, [r9, #8]
	ldr r10, [r0, #12]
	str r10, [fp, #-48]
	ldr r9, [fp, #-44]
	mov r1, r9
	ldr r9, [fp, #-48]
	blx r9
	sub sp, fp, #56
	add sp, sp, #24
	pop {r4-r10, fp, lr}
	bx lr
List$add$L102:
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-40]
	str r9, [r10]
	movw r0, #0
	ldr r10, [fp, #-40]
	str r0, [r10, #4]
	ldr r9, [fp, #-40]
	add r10, r9, #8
	str r10, [fp, #-52]
	ldr r9, [fp, #-40]
	add r10, r9, #8
	str r10, [fp, #-56]
	movw r0, #24
	bl malloc
	mov r1, r0
	ldr r0, =List$add
	str r0, [r1, #12]
	ldr r0, =List$init
	str r0, [r1, #16]
	ldr r0, =List$next
	str r0, [r1, #20]
	mov r0, r1
	ldr r1, [r0, #16]
	blx r1
	ldr r10, [fp, #-52]
	str r0, [r10]
	ldr r9, [fp, #-56]
	ldr r0, [r9]
	sub sp, fp, #56
	add sp, sp, #24
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L120:
	push {r4-r10, fp, lr}
	sub sp, sp, #32
	add fp, sp, #64
	movw r10, #50048
	str r10, [fp, #-60]
	movt r10, #457
	str r10, [fp, #-60]
	mov r10, #0
	str r10, [fp, #-52]
	movw r0, #24
	bl malloc
	mov r1, r0
	ldr r0, =List$add
	str r0, [r1, #12]
	ldr r0, =List$init
	str r0, [r1, #16]
	ldr r0, =List$next
	str r0, [r1, #20]
	mov r0, r1
	ldr r1, [r0, #16]
	blx r1
	mov r10, r0
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-48]
	mov r0, r9
main$L102:
	ldr r9, [fp, #-56]
	ldr r10, [fp, #-60]
	cmp r9, r10
	blt main$L103
main$L104:
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L107:
	movw r1, #1
	movw r0, #0
	cmp r1, r0
	bne main$L108
main$L109:
	movw r0, #10
	bl putch
main$L122:
	movw r0, #0
	sub sp, fp, #64
	add sp, sp, #32
	pop {r4-r10, fp, lr}
	bx lr
main$L108:
	ldr r9, [fp, #-40]
	ldr r0, [r9, #4]
	movw r1, #0
	cmp r0, r1
	bne main$L112
main$L113:
main$L114:
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-44]
	movw r0, #34464
	movt r0, #1
	ldr r9, [fp, #-44]
	sdiv r0, r9, r0
	movw r1, #34464
	movt r1, #1
	mul r0, r0, r1
	ldr r10, [fp, #-44]
	cmp r0, r10
	beq main$L117
main$L118:
main$L119:
	ldr r9, [fp, #-40]
	ldr r1, [r9, #20]
	ldr r9, [fp, #-40]
	mov r0, r9
	blx r1
	mov r10, r0
	str r10, [fp, #-40]
	b main$L107
main$L117:
	movw r0, #34464
	movt r0, #1
	ldr r9, [fp, #-44]
	sdiv r0, r9, r0
	bl putint
	movw r0, #32
	bl putch
	b main$L119
main$L112:
	b main$L109
main$L103:
	ldr r9, [fp, #-56]
	add r10, r9, #1
	str r10, [fp, #-56]
	ldr r10, [r0, #12]
	str r10, [fp, #-64]
	ldr r9, [fp, #-56]
	mov r1, r9
	ldr r9, [fp, #-64]
	blx r9
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	b main$L102

.balign 4
.global List$next
.section .text
.arm
List$next:
List$next$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r10, r0
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r0, [r9, #4]
	movw r1, #0
	cmp r0, r1
	beq List$next$L102
List$next$L103:
List$next$L104:
	ldr r9, [fp, #-40]
	mov r0, r9
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
List$next$L102:
	ldr r9, [fp, #-40]
	ldr r0, [r9, #8]
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global List$init
.section .text
.arm
List$init:
List$init$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1, r0
	movw r0, #1
	str r0, [r1, #4]
	movw r0, #0
	str r0, [r1]
	mov r0, r1
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
