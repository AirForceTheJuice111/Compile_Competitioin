.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	movw r115, #4
	str r115, [r100]
	movw r116, #30
	str r116, [r100, #8]
	movw r117, #100
	str r117, [r100, #12]
	ldr r1060101, [r100]
	ldr r1080101, [r100, #8]
	ldr r1110101, [r100, #12]
	add r1090101, r1060101, r1080101
	add r1120101, r1090101, r1110101
	mov r0, r1120101
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
	movw r154, #20
	mov r0, r154
	bl malloc
	mov r1030101, r0
	ldr r155, =A$f
	str r155, [r1030101, #16]
	mov r1000101, r1030101
	movw r156, #20
	mov r0, r156
	bl malloc
	mov r1040101, r0
	add r1210101, r1000101, #16
	mov r1250101, r1000101
	add r1270101, r1000101, #4
	ldr r157, =B$f
	str r157, [r1040101, #16]
	mov r1010101, r1040101
	ldr r1070101, [r1210101]
	mov r0, r1000101
	blx r1070101
	ldr r1090101, [r1010101, #16]
	mov r0, r1010101
	blx r1090101
	mov r1300101, r1010101
	add r1330101, r1010101, #8
	add r1360101, r1010101, #12
	mov r1000201, r1010101
	mov r1420101, r1010101
	add r1450101, r1010101, #8
	add r1480101, r1010101, #12
	ldr r1260101, [r1250101]
	ldr r1280101, [r1270101]
	ldr r1310101, [r1300101]
	ldr r1340101, [r1330101]
	ldr r1370101, [r1360101]
	mov r1400101, r1000201
	add r1290101, r1260101, r1280101
	add r1320101, r1290101, r1310101
	add r1350101, r1320101, r1340101
	add r1110101, r1350101, r1370101
	mov r0, r1110101
	bl putint
	movw r158, #10
	mov r0, r158
	bl putch
	ldr r1410101, [r1400101]
	ldr r1430101, [r1420101]
	ldr r1460101, [r1450101]
	ldr r1490101, [r1480101]
	add r1440101, r1410101, r1430101
	add r1470101, r1440101, r1460101
	add r1140101, r1470101, r1490101
	mov r0, r1140101
	bl putint
	movw r159, #10
	mov r0, r159
	bl putch
	movw r160, #1
	mov r0, r160
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global A$f
.section .text
.arm
A$f:
A$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	movw r106, #1
	str r106, [r100]
	movw r107, #2
	str r107, [r100, #4]
	movw r108, #0
	mov r0, r108
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
