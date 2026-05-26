.global extra^ne
extra^ne:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
L120:
movw t121, #3
mov t120, t121
movw t122, #4
cmp t120, t122
bne L121
L122:
movw t123, #0
mov r0, t123
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L121:
movw t124, #1
mov r0, t124
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global extra^gt
extra^gt:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
L130:
movw t131, #5
mov t130, t131
movw t132, #4
cmp t130, t132
bgt L131
L132:
movw t133, #0
mov r0, t133
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L131:
movw t134, #1
mov r0, t134
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global extra^le
extra^le:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
L100:
movw t101, #1
mov t100, t101
movw t102, #1
cmp t100, t102
ble L101
L102:
movw t103, #0
mov r0, t103
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L101:
movw t104, #1
mov r0, t104
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global extra^eq
extra^eq:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
L110:
movw t111, #2
mov t110, t111
movw t112, #2
cmp t110, t112
beq L111
L112:
movw t113, #0
mov r0, t113
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L111:
movw t114, #1
mov r0, t114
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

