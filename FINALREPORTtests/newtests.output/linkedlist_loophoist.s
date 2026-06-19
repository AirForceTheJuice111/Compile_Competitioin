.balign 4
.global main
.section .text
.arm
main:
main$L120:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r10401, #50048
	movt r10401, #457
	mov r10301, #0
	movw r137, #24
	mov r0, r137
	bl malloc
	mov r10601, r0
	ldr r138, =List$add
	str r138, [r10601, #12]
	ldr r139, =List$init
	str r139, [r10601, #16]
	ldr r140, =List$next
	str r140, [r10601, #20]
	mov r10701, r10601
	ldr r10901, [r10701, #16]
	mov r0, r10701
	blx r10901
	mov r11001, r0
	mov r10201, r11001
	mov r10001, r10201
	mov r10302, r10301
	mov r10002, r10001
main$L102:
	cmp r10302, r10401
	blt main$L103
main$L104:
	mov r10004, r10201
	mov r10005, r10004
main$L107:
	movw r146, #1
	movw r147, #0
	cmp r146, r147
	bne main$L108
main$L109:
	movw r145, #10
	mov r0, r145
	bl putch
main$L122:
	movw r148, #0
	mov r0, r148
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L108:
	ldr r12801, [r10005, #4]
	movw r149, #0
	cmp r12801, r149
	bne main$L112
main$L113:
main$L114:
	ldr r10101, [r10005]
	movw r141, #34464
	movt r141, #1
	sdiv r13001, r10101, r141
	movw r142, #34464
	movt r142, #1
	mul r13101, r13001, r142
	cmp r13101, r10101
	beq main$L117
main$L118:
main$L119:
	ldr r11601, [r10005, #20]
	mov r0, r10005
	blx r11601
	mov r10006, r0
	mov r10005, r10006
	b main$L107
main$L117:
	movw r143, #34464
	movt r143, #1
	sdiv r11301, r10101, r143
	mov r0, r11301
	bl putint
	movw r144, #32
	mov r0, r144
	bl putch
	b main$L119
main$L112:
	b main$L109
main$L103:
	add r10303, r10302, #1
	ldr r11101, [r10002, #12]
	mov r0, r10002
	mov r1, r10303
	blx r11101
	mov r10003, r0
	mov r10302, r10303
	mov r10002, r10003
	b main$L102

.balign 4
.global List$init
.section .text
.arm
List$init:
List$init$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	movw r104, #1
	str r104, [r100, #4]
	movw r105, #0
	str r105, [r100]
	mov r0, r100
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
	mov r100, r0
	mov r101, r1
	ldr r11201, [r100, #4]
	movw r132, #0
	cmp r11201, r132
	bne List$add$L102
List$add$L103:
	ldr r10501, [r100, #8]
	ldr r10901, [r10501, #12]
	mov r0, r10501
	mov r1, r101
	blx r10901
	mov r11001, r0
	mov r0, r11001
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
List$add$L102:
	str r101, [r100]
	movw r127, #0
	str r127, [r100, #4]
	add r12101, r100, #8
	add r12201, r100, #8
	movw r128, #24
	mov r0, r128
	bl malloc
	mov r10301, r0
	ldr r129, =List$add
	str r129, [r10301, #12]
	ldr r130, =List$init
	str r130, [r10301, #16]
	ldr r131, =List$next
	str r131, [r10301, #20]
	mov r10401, r10301
	ldr r10701, [r10401, #16]
	mov r0, r10401
	blx r10701
	mov r10801, r0
	str r10801, [r12101]
	ldr r12301, [r12201]
	mov r0, r12301
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global List$next
.section .text
.arm
List$next:
List$next$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	ldr r10301, [r100, #4]
	movw r106, #0
	cmp r10301, r106
	beq List$next$L102
List$next$L103:
List$next$L104:
	mov r0, r100
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
List$next$L102:
	ldr r10501, [r100, #8]
	mov r0, r10501
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
