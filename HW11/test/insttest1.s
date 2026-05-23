.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t124, #20
mov r0, t124
bl malloc
mov t10000, r0
movw t125, #4
str t125, [t10000]
movw t126, #1
str t126, [t10000, #4]
movw t127, #2
str t127, [t10000, #8]
movw t128, #3
str t128, [t10000, #12]
str t125, [t10000, #16]
ldr t10300, [t10000]
mov t10100, t10300
mov t10101, t10100
L102:
movw t132, #0
cmp t10101, t132
bgt L103
L104:
movw t131, #10
mov r0, t131
bl putch
movw t133, #2
mov r0, t133
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10102, t10101, #1
mov t10700, t10000
ldr t10400, [t10000]
movw t134, #0
cmp t10102, t134
bge L106
L105:
movw t135, #65535
movt t135, #65535
mov r0, t135
bl exit
L106:
cmp t10102, t10400
bge L105
L107:
add t11800, t10102, #1
movw t129, #4
mul t11900, t11800, t129
ldr t10800, [t10700, t11900]
mov r0, t10800
bl putint
movw t130, #32
mov r0, t130
bl putch
mov t10101, t10102
b L102

