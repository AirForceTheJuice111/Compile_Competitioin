.global extra^phiTrueEdge
extra^phiTrueEdge:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
L100:
movw t103, #5
mov t100, t103
movw t105, #10
cmp t100, t105
blt L104
L102:
movw t104, #7
mov t102, t104
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

