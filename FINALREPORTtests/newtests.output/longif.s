.balign 4
.global main
.section .text
.arm
main:
main$L402:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r109, #2
	movw r110, #1
	cmp r109, r110
	bgt main$L104
main$L400:
main$L401:
	movw r111, #0
	mov r0, r111
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L104:
	movw r112, #3
	movw r113, #2
	cmp r112, r113
	ble main$L400
main$L107:
	movw r114, #4
	movw r115, #3
	cmp r114, r115
	ble main$L400
main$L110:
	movw r116, #5
	movw r117, #4
	cmp r116, r117
	ble main$L400
main$L113:
	movw r118, #6
	movw r119, #5
	cmp r118, r119
	ble main$L400
main$L116:
	movw r120, #7
	movw r121, #6
	cmp r120, r121
	ble main$L400
main$L119:
	movw r122, #8
	movw r123, #7
	cmp r122, r123
	ble main$L400
main$L122:
	movw r124, #9
	movw r125, #8
	cmp r124, r125
	ble main$L400
main$L125:
	movw r126, #10
	movw r127, #9
	cmp r126, r127
	ble main$L400
main$L128:
	movw r128, #11
	movw r129, #10
	cmp r128, r129
	ble main$L400
main$L131:
	movw r130, #12
	movw r131, #11
	cmp r130, r131
	ble main$L400
main$L134:
	movw r132, #13
	movw r133, #12
	cmp r132, r133
	ble main$L400
main$L137:
	movw r134, #14
	movw r135, #13
	cmp r134, r135
	ble main$L400
main$L140:
	movw r136, #15
	movw r137, #14
	cmp r136, r137
	ble main$L400
main$L143:
	movw r138, #16
	movw r139, #15
	cmp r138, r139
	ble main$L400
main$L146:
	movw r140, #17
	movw r141, #16
	cmp r140, r141
	ble main$L400
main$L149:
	movw r142, #18
	movw r143, #17
	cmp r142, r143
	ble main$L400
main$L152:
	movw r144, #19
	movw r145, #18
	cmp r144, r145
	ble main$L400
main$L155:
	movw r146, #20
	movw r147, #19
	cmp r146, r147
	ble main$L400
main$L158:
	movw r148, #21
	movw r149, #20
	cmp r148, r149
	ble main$L400
main$L161:
	movw r150, #22
	movw r151, #21
	cmp r150, r151
	ble main$L400
main$L164:
	movw r152, #23
	movw r153, #22
	cmp r152, r153
	ble main$L400
main$L165:
	bl getint
	mov r10101, r0
	movw r154, #0
	cmp r10101, r154
	beq main$L400
main$L170:
	movw r155, #24
	movw r156, #23
	cmp r155, r156
	ble main$L400
main$L173:
	movw r157, #25
	movw r158, #24
	cmp r157, r158
	ble main$L400
main$L176:
	movw r159, #26
	movw r160, #25
	cmp r159, r160
	ble main$L400
main$L179:
	movw r161, #27
	movw r162, #26
	cmp r161, r162
	ble main$L400
main$L182:
	movw r163, #28
	movw r164, #27
	cmp r163, r164
	ble main$L400
main$L185:
	movw r165, #29
	movw r166, #28
	cmp r165, r166
	ble main$L400
main$L188:
	movw r167, #30
	movw r168, #29
	cmp r167, r168
	ble main$L400
main$L191:
	movw r169, #31
	movw r170, #30
	cmp r169, r170
	ble main$L400
main$L194:
	movw r171, #32
	movw r172, #31
	cmp r171, r172
	ble main$L400
main$L197:
	movw r173, #33
	movw r174, #32
	cmp r173, r174
	ble main$L400
main$L200:
	movw r175, #34
	movw r176, #33
	cmp r175, r176
	ble main$L400
main$L203:
	movw r177, #35
	movw r178, #34
	cmp r177, r178
	ble main$L400
main$L206:
	movw r179, #36
	movw r180, #35
	cmp r179, r180
	ble main$L400
main$L209:
	movw r181, #37
	movw r182, #36
	cmp r181, r182
	ble main$L400
main$L212:
	movw r183, #38
	movw r184, #37
	cmp r183, r184
	ble main$L400
main$L215:
	movw r185, #39
	movw r186, #38
	cmp r185, r186
	ble main$L400
main$L218:
	movw r187, #40
	movw r188, #39
	cmp r187, r188
	ble main$L400
main$L221:
	movw r189, #41
	movw r190, #40
	cmp r189, r190
	ble main$L400
main$L224:
	movw r191, #42
	movw r192, #41
	cmp r191, r192
	ble main$L400
main$L227:
	movw r193, #43
	movw r194, #42
	cmp r193, r194
	ble main$L400
main$L230:
	movw r195, #44
	movw r196, #43
	cmp r195, r196
	ble main$L400
main$L233:
	movw r197, #45
	movw r198, #44
	cmp r197, r198
	ble main$L400
main$L236:
	movw r199, #46
	movw r200, #45
	cmp r199, r200
	ble main$L400
main$L239:
	movw r201, #47
	movw r202, #46
	cmp r201, r202
	ble main$L400
main$L242:
	movw r203, #48
	movw r204, #47
	cmp r203, r204
	ble main$L400
main$L245:
	movw r205, #49
	movw r206, #48
	cmp r205, r206
	ble main$L400
main$L248:
	movw r207, #50
	movw r208, #49
	cmp r207, r208
	ble main$L400
main$L251:
	movw r209, #51
	movw r210, #50
	cmp r209, r210
	ble main$L400
main$L254:
	movw r211, #52
	movw r212, #51
	cmp r211, r212
	ble main$L400
main$L257:
	movw r213, #53
	movw r214, #52
	cmp r213, r214
	ble main$L400
main$L260:
	movw r215, #54
	movw r216, #53
	cmp r215, r216
	ble main$L400
main$L263:
	movw r217, #55
	movw r218, #54
	cmp r217, r218
	ble main$L400
main$L266:
	movw r219, #56
	movw r220, #55
	cmp r219, r220
	ble main$L400
main$L269:
	movw r221, #57
	movw r222, #56
	cmp r221, r222
	ble main$L400
main$L272:
	movw r223, #58
	movw r224, #57
	cmp r223, r224
	ble main$L400
main$L275:
	movw r225, #59
	movw r226, #58
	cmp r225, r226
	ble main$L400
main$L278:
	movw r227, #60
	movw r228, #59
	cmp r227, r228
	ble main$L400
main$L281:
	movw r229, #61
	movw r230, #60
	cmp r229, r230
	ble main$L400
main$L284:
	movw r231, #62
	movw r232, #61
	cmp r231, r232
	ble main$L400
main$L287:
	movw r233, #63
	movw r234, #62
	cmp r233, r234
	ble main$L400
main$L290:
	movw r235, #64
	movw r236, #63
	cmp r235, r236
	ble main$L400
main$L293:
	movw r237, #65
	movw r238, #64
	cmp r237, r238
	ble main$L400
main$L296:
	movw r239, #66
	movw r240, #65
	cmp r239, r240
	ble main$L400
main$L299:
	movw r241, #67
	movw r242, #66
	cmp r241, r242
	ble main$L400
main$L302:
	movw r243, #68
	movw r244, #67
	cmp r243, r244
	ble main$L400
main$L305:
	movw r245, #69
	movw r246, #68
	cmp r245, r246
	ble main$L400
main$L308:
	movw r247, #70
	movw r248, #69
	cmp r247, r248
	ble main$L400
main$L311:
	movw r249, #71
	movw r250, #70
	cmp r249, r250
	ble main$L400
main$L314:
	movw r251, #72
	movw r252, #71
	cmp r251, r252
	ble main$L400
main$L317:
	movw r253, #73
	movw r254, #72
	cmp r253, r254
	ble main$L400
main$L320:
	movw r255, #74
	movw r256, #73
	cmp r255, r256
	ble main$L400
main$L323:
	movw r257, #75
	movw r258, #74
	cmp r257, r258
	ble main$L400
main$L326:
	movw r259, #76
	movw r260, #75
	cmp r259, r260
	ble main$L400
main$L329:
	movw r261, #77
	movw r262, #76
	cmp r261, r262
	ble main$L400
main$L332:
	movw r263, #78
	movw r264, #77
	cmp r263, r264
	ble main$L400
main$L335:
	movw r265, #79
	movw r266, #78
	cmp r265, r266
	ble main$L400
main$L338:
	movw r267, #80
	movw r268, #79
	cmp r267, r268
	ble main$L400
main$L341:
	movw r269, #81
	movw r270, #80
	cmp r269, r270
	ble main$L400
main$L344:
	movw r271, #82
	movw r272, #81
	cmp r271, r272
	ble main$L400
main$L347:
	movw r273, #83
	movw r274, #82
	cmp r273, r274
	ble main$L400
main$L350:
	movw r275, #84
	movw r276, #83
	cmp r275, r276
	ble main$L400
main$L353:
	movw r277, #85
	movw r278, #84
	cmp r277, r278
	ble main$L400
main$L356:
	movw r279, #86
	movw r280, #85
	cmp r279, r280
	ble main$L400
main$L359:
	movw r281, #87
	movw r282, #86
	cmp r281, r282
	ble main$L400
main$L362:
	movw r283, #88
	movw r284, #87
	cmp r283, r284
	ble main$L400
main$L365:
	movw r285, #89
	movw r286, #88
	cmp r285, r286
	ble main$L400
main$L368:
	movw r287, #90
	movw r288, #89
	cmp r287, r288
	ble main$L400
main$L371:
	movw r289, #91
	movw r290, #90
	cmp r289, r290
	ble main$L400
main$L374:
	movw r291, #92
	movw r292, #91
	cmp r291, r292
	ble main$L400
main$L377:
	movw r293, #93
	movw r294, #92
	cmp r293, r294
	ble main$L400
main$L380:
	movw r295, #94
	movw r296, #93
	cmp r295, r296
	ble main$L400
main$L383:
	movw r297, #95
	movw r298, #94
	cmp r297, r298
	ble main$L400
main$L386:
	movw r299, #96
	movw r300, #95
	cmp r299, r300
	ble main$L400
main$L389:
	movw r301, #97
	movw r302, #96
	cmp r301, r302
	ble main$L400
main$L392:
	movw r303, #98
	movw r304, #97
	cmp r303, r304
	ble main$L400
main$L395:
	movw r305, #99
	movw r306, #98
	cmp r305, r306
	ble main$L400
main$L398:
	movw r307, #100
	movw r308, #99
	cmp r307, r308
	ble main$L400
main$L399:
	movw r107, #100
	mov r0, r107
	bl putint
	movw r108, #10
	mov r0, r108
	bl putch
	b main$L401

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
