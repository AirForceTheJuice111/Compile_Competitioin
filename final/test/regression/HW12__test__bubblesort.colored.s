.balign 4
.global b1$bubbleSort
.section .text
.arm
b1$bubbleSort:
b1$bubbleSort$L131:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, r0
	mov r0, r1
	mov r1, r2
	movw r0, #1
	cmp r1, r0
	ble b1$bubbleSort$L102
b1$bubbleSort$L103:
b1$bubbleSort$L102:
	movw r0, #0
	mov r0, r0
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
	movw r0, #32
	push {r0}
	pop {r0}
	bl malloc
	mov r4, r0
	movw r0, #7
	str r0, [r4]
	movw r0, #6
	str r0, [r4, #4]
	movw r0, #3
	str r0, [r4, #8]
	movw r0, #0
	str r0, [r4, #12]
	movw r0, #5
	str r0, [r4, #16]
	movw r0, #9
	str r0, [r4, #20]
	movw r0, #1
	str r0, [r4, #24]
	movw r0, #2
	str r0, [r4, #28]
	movw r0, #8
	push {r0}
	pop {r0}
	bl malloc
	mov r1, r0
	adr r0, b1$bubbleSort
	str r0, [r1, #4]
	ldr r0, [r1, #4]
	mov r2, r4
	ldr r3, [r4]
	push {r0}
	push {r1}
	push {r2}
	push {r3}
	pop {r2}
	pop {r1}
	pop {r0}
	pop {ip}
	blx ip

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
