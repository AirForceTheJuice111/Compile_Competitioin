.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t130, #4
mov r0, t130
bl malloc
mov t10300, r0
mov t10100, #3
movw t131, #20
mov r0, t131
bl malloc
mov t10400, r0
str t130, [t10400]
mov t12100, t10300
mov t10000, t10300
movw t132, #1
str t132, [t10400, #4]
movw t133, #2
str t133, [t10400, #8]
movw t134, #3
str t134, [t10400, #12]
str t130, [t10400, #16]
str t10400, [t12100]
mov t10101, t10100
L102:
movw t138, #0
cmp t10101, t138
bge L103
L104:
movw t137, #10
mov r0, t137
bl putch
movw t139, #2
mov r0, t139
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10102, t10101, #1
ldr t10500, [t10000]
mov t11000, t10500
ldr t10600, [t10500]
movw t140, #0
cmp t10102, t140
bge L106
L105:
movw t141, #65535
movt t141, #65535
mov r0, t141
bl exit
L106:
cmp t10102, t10600
bge L105
L107:
add t12400, t10102, #1
movw t135, #4
mul t12500, t12400, t135
ldr t11100, [t11000, t12500]
mov r0, t11100
bl putint
movw t136, #32
mov r0, t136
bl putch
mov t10101, t10102
b L102

