.global extra^ne
extra^ne:
L120:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t120, #3
movw t121, #4
cmp t120, t121
bne L121
L122:
movw t122, #0
mov r0, t122
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L121:
movw t123, #1
mov r0, t123
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global extra^gt
extra^gt:
L130:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t130, #5
movw t131, #4
cmp t130, t131
bgt L131
L132:
movw t132, #0
mov r0, t132
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L131:
movw t133, #1
mov r0, t133
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global extra^le
extra^le:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, #1
movw t101, #1
cmp t100, t101
ble L101
L102:
movw t102, #0
mov r0, t102
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L101:
movw t103, #1
mov r0, t103
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global extra^eq
extra^eq:
L110:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t110, #2
movw t111, #2
cmp t110, t111
beq L111
L112:
movw t112, #0
mov r0, t112
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L111:
movw t113, #1
mov r0, t113
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

