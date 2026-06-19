.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10101, #3
	movw r130, #4
	mov r0, r130
	bl malloc
	mov r10301, r0
	movw r131, #20
	mov r0, r131
	bl malloc
	mov r10401, r0
	movw r132, #4
	str r132, [r10401]
	mov r12101, r10301
	mov r10001, r10301
	movw r133, #1
	str r133, [r10401, #4]
	movw r134, #2
	str r134, [r10401, #8]
	movw r135, #3
	str r135, [r10401, #12]
	movw r136, #4
	str r136, [r10401, #16]
	str r10401, [r12101]
	mov r10102, r10101
main$L102:
	movw r140, #0
	cmp r10102, r140
	bge main$L103
main$L104:
	movw r139, #10
	mov r0, r139
	bl putch
	movw r141, #2
	mov r0, r141
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10103, r10102, #1
	ldr r10501, [r10001]
	mov r11001, r10501
	ldr r10601, [r10501]
	movw r142, #0
	cmp r10103, r142
	bge main$L106
main$L105:
	movw r143, #65535
	movt r143, #65535
	mov r0, r143
	bl exit
main$L106:
	cmp r10103, r10601
	bge main$L105
main$L107:
	add r12401, r10103, #1
	movw r137, #4
	mul r12501, r12401, r137
	ldr r11101, [r11001, r12501]
	mov r0, r11101
	bl putint
	movw r138, #32
	mov r0, r138
	bl putch
	mov r10102, r10103
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
