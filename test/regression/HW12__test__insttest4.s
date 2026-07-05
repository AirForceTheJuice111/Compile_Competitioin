.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L104:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1, r0
	mov r2, r1
	movw r0, #0
	cmp r2, r0
	bgt C$m$L102
C$m$L103:
	ldr r0, [r1, #4]
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$m$L102:
	ldr r0, [r1]
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
	movw r5, #12
	push {r5}
	pop {r0}
	bl malloc
	mov r1, r0
	adr r0, C$m
	str r0, [r1, #8]
	ldr r0, [r1, #8]
	mov r4, r0
	push {r5}
	pop {r0}
	bl malloc
	mov r1, r0
	adr r0, C$m
	str r0, [r1, #8]
	mov r5, r1
	bl getint
	mov r0, r0
	push {r4}
	push {r5}
	push {r0}
	pop {r1}
	pop {r0}
	pop {ip}
	blx ip
	mov r0, r0
	ldr r0, [r0]

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
