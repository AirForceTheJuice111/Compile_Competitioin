.global extra^phiTrueEdge
extra^phiTrueEdge:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, #5
movw t103, #10
cmp t100, t103
blt L104
L102:
mov t102, #7
mov t101, t102
L101:
mov r0, t101
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L104:
mov t101, t100
b L101

