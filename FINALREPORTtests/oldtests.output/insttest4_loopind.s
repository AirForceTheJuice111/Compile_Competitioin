.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	mov r101, r1
	movw r107, #0
	cmp r101, r107
	bgt C$m$L102
C$m$L103:
	ldr r10601, [r100, #4]
	mov r0, r10601
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$m$L102:
	ldr r10401, [r100]
	mov r0, r10401
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r148, #12
	mov r0, r148
	bl malloc
	mov r10301, r0
	movw r149, #20
	mov r0, r149
	bl malloc
	mov r10401, r0
	movw r150, #4
	str r150, [r10401]
	mov r12901, r10301
	add r13601, r10301, #4
	add r13701, r10301, #8
	mov r10601, r10301
	movw r151, #1
	str r151, [r10401, #4]
	movw r152, #2
	str r152, [r10401, #8]
	movw r153, #3
	str r153, [r10401, #12]
	movw r154, #4
	str r154, [r10401, #16]
	str r10401, [r12901]
	movw r155, #24
	mov r0, r155
	bl malloc
	mov r10501, r0
	movw r156, #5
	str r156, [r10501]
	add r13801, r10601, #8
	mov r11401, r10601
	movw r157, #5
	str r157, [r10501, #4]
	movw r158, #6
	str r158, [r10501, #8]
	movw r159, #7
	str r159, [r10501, #12]
	movw r160, #8
	str r160, [r10501, #16]
	movw r161, #9
	str r161, [r10501, #20]
	str r10501, [r13601]
	ldr r162, =C$m
	str r162, [r13701]
	ldr r11201, [r13801]
	bl getint
	mov r11301, r0
	mov r11501, r11201
	mov r0, r11401
	mov r1, r11301
	blx r11501
	mov r11601, r0
	mov r10001, r11601
	ldr r10701, [r10001]
	mov r10101, r10701
	sub r14305, r10101, #1
	movw r163, #4
	mul r14306, r14305, r163
	add r14303, r14306, #4
	mov r10102, r10101
	mov r14302, r14303
main$L102:
	movw r166, #0
	cmp r14302, r166
	bge main$L103
main$L104:
	movw r165, #10
	mov r0, r165
	bl putch
	movw r167, #2
	mov r0, r167
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10103, r10102, #1
	mov r11801, r10001
	ldr r10801, [r10001]
	movw r168, #0
	cmp r10103, r168
	bge main$L106
main$L105:
	movw r169, #65535
	movt r169, #65535
	mov r0, r169
	bl exit
main$L106:
	cmp r10103, r10801
	bge main$L105
main$L107:
	ldr r11901, [r11801, r14302]
	sub r14304, r14302, #4
	mov r0, r11901
	bl putint
	movw r164, #32
	mov r0, r164
	bl putch
	mov r10102, r10103
	mov r14302, r14304
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
