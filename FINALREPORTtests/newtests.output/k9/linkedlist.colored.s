.balign 4
.global main
.section .text
.arm
main:
main$L120:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r4, #50048
	movt r4, #457
	mov r6, #0
	movw r0, #24
	bl malloc
	ldr r1, =List$add
	str r1, [r0, #12]
	ldr r1, =List$init
	str r1, [r0, #16]
	ldr r1, =List$next
	str r1, [r0, #20]
	ldr r1, [r0, #16]
	blx r1
	mov r5, r0
	mov r0, r5
main$L102:
	cmp r6, r4
	blt main$L103
main$L104:
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
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L108:
	ldr r0, [r5, #4]
	movw r1, #0
	cmp r0, r1
	bne main$L112
main$L113:
main$L114:
	ldr r1, [r5]
	movw r0, #34464
	movt r0, #1
	sdiv r0, r1, r0
	movw r2, #34464
	movt r2, #1
	mul r0, r0, r2
	cmp r0, r1
	beq main$L117
main$L118:
main$L119:
	ldr r1, [r5, #20]
	mov r0, r5
	blx r1
	mov r5, r0
	b main$L107
main$L117:
	movw r0, #34464
	movt r0, #1
	sdiv r0, r1, r0
	bl putint
	movw r0, #32
	bl putch
	b main$L119
main$L112:
	b main$L109
main$L103:
	add r6, r6, #1
	ldr r2, [r0, #12]
	mov r1, r6
	blx r2
	b main$L102

.balign 4
.global List$next
.section .text
.arm
List$next:
List$next$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	ldr r1, [r0, #4]
	movw r2, #0
	cmp r1, r2
	beq List$next$L102
List$next$L103:
List$next$L104:
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
List$next$L102:
	ldr r0, [r0, #8]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global List$add
.section .text
.arm
List$add:
List$add$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	ldr r2, [r0, #4]
	movw r3, #0
	cmp r2, r3
	bne List$add$L102
List$add$L103:
	ldr r0, [r0, #8]
	ldr r2, [r0, #12]
	blx r2
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
List$add$L102:
	str r1, [r0]
	movw r1, #0
	str r1, [r0, #4]
	add r5, r0, #8
	add r4, r0, #8
	movw r0, #24
	bl malloc
	ldr r1, =List$add
	str r1, [r0, #12]
	ldr r1, =List$init
	str r1, [r0, #16]
	ldr r1, =List$next
	str r1, [r0, #20]
	ldr r1, [r0, #16]
	blx r1
	str r0, [r5]
	ldr r0, [r4]
	sub sp, fp, #36
	add sp, sp, #4
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
	movw r1, #1
	str r1, [r0, #4]
	movw r1, #0
	str r1, [r0]
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
