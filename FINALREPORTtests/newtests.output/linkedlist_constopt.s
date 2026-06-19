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
	movw r106, #1
	str r106, [r100, #4]
	movw r107, #0
	str r107, [r100]
	mov r0, r100
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L120:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r140, #24
	mov r0, r140
	bl malloc
	mov r10601, r0
	mov r137, #0
	ldr r141, =List$add
	str r141, [r10601, #12]
	ldr r142, =List$init
	str r142, [r10601, #16]
	ldr r143, =List$next
	str r143, [r10601, #20]
	mov r10701, r10601
	ldr r10901, [r10701, #16]
	mov r0, r10701
	blx r10901
	mov r11001, r0
	mov r10201, r11001
	mov r10001, r10201
	mov r10302, r137
	mov r10002, r10001
main$L102:
	movw r149, #50048
	movt r149, #457
	cmp r10302, r149
	blt main$L103
main$L104:
	mov r10004, r10201
	mov r10005, r10004
main$L107:
main$L108:
	ldr r12801, [r10005, #4]
	movw r150, #0
	cmp r12801, r150
	bne main$L112
main$L113:
main$L114:
	ldr r10101, [r10005]
	movw r144, #34464
	movt r144, #1
	sdiv r13001, r10101, r144
	movw r145, #34464
	movt r145, #1
	mul r13101, r13001, r145
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
	movw r146, #34464
	movt r146, #1
	sdiv r11301, r10101, r146
	mov r0, r11301
	bl putint
	movw r147, #32
	mov r0, r147
	bl putch
	b main$L119
main$L112:
main$L109:
	movw r148, #10
	mov r0, r148
	bl putch
main$L122:
	movw r151, #0
	mov r0, r151
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
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
	movw r108, #0
	cmp r10301, r108
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
	movw r134, #0
	cmp r11201, r134
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
	movw r129, #0
	str r129, [r100, #4]
	add r12101, r100, #8
	add r12201, r100, #8
	movw r130, #24
	mov r0, r130
	bl malloc
	mov r10301, r0
	ldr r131, =List$add
	str r131, [r10301, #12]
	ldr r132, =List$init
	str r132, [r10301, #16]
	ldr r133, =List$next
	str r133, [r10301, #20]
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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
