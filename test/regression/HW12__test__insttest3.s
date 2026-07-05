.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r4, #8
	push {r4}
	pop {r0}
	bl malloc
	mov r1, r0
	adr r0, C$m
	str r0, [r1, #4]
	ldr r0, [r1, #4]
	mov r5, r0
	push {r4}
	pop {r0}
	bl malloc
	mov r1, r0
	adr r0, C$m
	str r0, [r1, #4]
	push {r5}
	push {r1}
	pop {r0}
	pop {ip}
	blx ip
	mov r0, r0

.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, r0
	ldr r0, [r0]
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
