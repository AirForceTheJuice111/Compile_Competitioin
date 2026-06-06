.balign 4
.global C$max
.section .text
.arm
C$max:
C$max$L104:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, r0
	mov r1, r1
	mov r0, r2
	cmp r1, r0
	bgt C$max$L102
C$max$L103:
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$max$L102:
	mov r0, r1
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
	movw r4, #8
	push {r4}
	pop {r0}
	bl malloc
	mov r1, r0
	adr r0, C$max
	str r0, [r1, #4]
	ldr r0, [r1, #4]
	mov r5, r0
	push {r4}
	pop {r0}
	bl malloc
	mov r2, r0
	adr r0, C$max
	str r0, [r2, #4]
	push {r5}
	movw r1, #100
	movw r0, #200
	push {r2}
	push {r1}
	push {r0}
	pop {r2}
	pop {r1}
	pop {r0}
	pop {ip}
	blx ip
	mov r0, r0
	push {r0}
	pop {r0}
	bl putint
	movw r0, #1
	mov r0, r0
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
