.balign 4
.global main
.section .text
.arm
main:
main$L402:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r1, #2
	movw r0, #1
	cmp r1, r0
	bgt main$L104
main$L400:
main$L401:
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L104:
	movw r1, #3
	movw r0, #2
	cmp r1, r0
	ble main$L400
main$L107:
	movw r1, #4
	movw r0, #3
	cmp r1, r0
	ble main$L400
main$L110:
	movw r1, #5
	movw r0, #4
	cmp r1, r0
	ble main$L400
main$L113:
	movw r1, #6
	movw r0, #5
	cmp r1, r0
	ble main$L400
main$L116:
	movw r1, #7
	movw r0, #6
	cmp r1, r0
	ble main$L400
main$L119:
	movw r1, #8
	movw r0, #7
	cmp r1, r0
	ble main$L400
main$L122:
	movw r1, #9
	movw r0, #8
	cmp r1, r0
	ble main$L400
main$L125:
	movw r1, #10
	movw r0, #9
	cmp r1, r0
	ble main$L400
main$L128:
	movw r1, #11
	movw r0, #10
	cmp r1, r0
	ble main$L400
main$L131:
	movw r1, #12
	movw r0, #11
	cmp r1, r0
	ble main$L400
main$L134:
	movw r1, #13
	movw r0, #12
	cmp r1, r0
	ble main$L400
main$L137:
	movw r1, #14
	movw r0, #13
	cmp r1, r0
	ble main$L400
main$L140:
	movw r1, #15
	movw r0, #14
	cmp r1, r0
	ble main$L400
main$L143:
	movw r1, #16
	movw r0, #15
	cmp r1, r0
	ble main$L400
main$L146:
	movw r1, #17
	movw r0, #16
	cmp r1, r0
	ble main$L400
main$L149:
	movw r1, #18
	movw r0, #17
	cmp r1, r0
	ble main$L400
main$L152:
	movw r1, #19
	movw r0, #18
	cmp r1, r0
	ble main$L400
main$L155:
	movw r1, #20
	movw r0, #19
	cmp r1, r0
	ble main$L400
main$L158:
	movw r1, #21
	movw r0, #20
	cmp r1, r0
	ble main$L400
main$L161:
	movw r1, #22
	movw r0, #21
	cmp r1, r0
	ble main$L400
main$L164:
	movw r1, #23
	movw r0, #22
	cmp r1, r0
	ble main$L400
main$L165:
	bl getint
	movw r1, #0
	cmp r0, r1
	beq main$L400
main$L170:
	movw r1, #24
	movw r0, #23
	cmp r1, r0
	ble main$L400
main$L173:
	movw r1, #25
	movw r0, #24
	cmp r1, r0
	ble main$L400
main$L176:
	movw r1, #26
	movw r0, #25
	cmp r1, r0
	ble main$L400
main$L179:
	movw r1, #27
	movw r0, #26
	cmp r1, r0
	ble main$L400
main$L182:
	movw r1, #28
	movw r0, #27
	cmp r1, r0
	ble main$L400
main$L185:
	movw r1, #29
	movw r0, #28
	cmp r1, r0
	ble main$L400
main$L188:
	movw r1, #30
	movw r0, #29
	cmp r1, r0
	ble main$L400
main$L191:
	movw r1, #31
	movw r0, #30
	cmp r1, r0
	ble main$L400
main$L194:
	movw r1, #32
	movw r0, #31
	cmp r1, r0
	ble main$L400
main$L197:
	movw r1, #33
	movw r0, #32
	cmp r1, r0
	ble main$L400
main$L200:
	movw r1, #34
	movw r0, #33
	cmp r1, r0
	ble main$L400
main$L203:
	movw r1, #35
	movw r0, #34
	cmp r1, r0
	ble main$L400
main$L206:
	movw r1, #36
	movw r0, #35
	cmp r1, r0
	ble main$L400
main$L209:
	movw r1, #37
	movw r0, #36
	cmp r1, r0
	ble main$L400
main$L212:
	movw r1, #38
	movw r0, #37
	cmp r1, r0
	ble main$L400
main$L215:
	movw r1, #39
	movw r0, #38
	cmp r1, r0
	ble main$L400
main$L218:
	movw r1, #40
	movw r0, #39
	cmp r1, r0
	ble main$L400
main$L221:
	movw r1, #41
	movw r0, #40
	cmp r1, r0
	ble main$L400
main$L224:
	movw r1, #42
	movw r0, #41
	cmp r1, r0
	ble main$L400
main$L227:
	movw r1, #43
	movw r0, #42
	cmp r1, r0
	ble main$L400
main$L230:
	movw r1, #44
	movw r0, #43
	cmp r1, r0
	ble main$L400
main$L233:
	movw r1, #45
	movw r0, #44
	cmp r1, r0
	ble main$L400
main$L236:
	movw r1, #46
	movw r0, #45
	cmp r1, r0
	ble main$L400
main$L239:
	movw r1, #47
	movw r0, #46
	cmp r1, r0
	ble main$L400
main$L242:
	movw r1, #48
	movw r0, #47
	cmp r1, r0
	ble main$L400
main$L245:
	movw r1, #49
	movw r0, #48
	cmp r1, r0
	ble main$L400
main$L248:
	movw r1, #50
	movw r0, #49
	cmp r1, r0
	ble main$L400
main$L251:
	movw r1, #51
	movw r0, #50
	cmp r1, r0
	ble main$L400
main$L254:
	movw r1, #52
	movw r0, #51
	cmp r1, r0
	ble main$L400
main$L257:
	movw r1, #53
	movw r0, #52
	cmp r1, r0
	ble main$L400
main$L260:
	movw r1, #54
	movw r0, #53
	cmp r1, r0
	ble main$L400
main$L263:
	movw r1, #55
	movw r0, #54
	cmp r1, r0
	ble main$L400
main$L266:
	movw r1, #56
	movw r0, #55
	cmp r1, r0
	ble main$L400
main$L269:
	movw r1, #57
	movw r0, #56
	cmp r1, r0
	ble main$L400
main$L272:
	movw r1, #58
	movw r0, #57
	cmp r1, r0
	ble main$L400
main$L275:
	movw r1, #59
	movw r0, #58
	cmp r1, r0
	ble main$L400
main$L278:
	movw r1, #60
	movw r0, #59
	cmp r1, r0
	ble main$L400
main$L281:
	movw r1, #61
	movw r0, #60
	cmp r1, r0
	ble main$L400
main$L284:
	movw r1, #62
	movw r0, #61
	cmp r1, r0
	ble main$L400
main$L287:
	movw r1, #63
	movw r0, #62
	cmp r1, r0
	ble main$L400
main$L290:
	movw r1, #64
	movw r0, #63
	cmp r1, r0
	ble main$L400
main$L293:
	movw r1, #65
	movw r0, #64
	cmp r1, r0
	ble main$L400
main$L296:
	movw r1, #66
	movw r0, #65
	cmp r1, r0
	ble main$L400
main$L299:
	movw r1, #67
	movw r0, #66
	cmp r1, r0
	ble main$L400
main$L302:
	movw r1, #68
	movw r0, #67
	cmp r1, r0
	ble main$L400
main$L305:
	movw r1, #69
	movw r0, #68
	cmp r1, r0
	ble main$L400
main$L308:
	movw r1, #70
	movw r0, #69
	cmp r1, r0
	ble main$L400
main$L311:
	movw r1, #71
	movw r0, #70
	cmp r1, r0
	ble main$L400
main$L314:
	movw r1, #72
	movw r0, #71
	cmp r1, r0
	ble main$L400
main$L317:
	movw r1, #73
	movw r0, #72
	cmp r1, r0
	ble main$L400
main$L320:
	movw r1, #74
	movw r0, #73
	cmp r1, r0
	ble main$L400
main$L323:
	movw r1, #75
	movw r0, #74
	cmp r1, r0
	ble main$L400
main$L326:
	movw r1, #76
	movw r0, #75
	cmp r1, r0
	ble main$L400
main$L329:
	movw r1, #77
	movw r0, #76
	cmp r1, r0
	ble main$L400
main$L332:
	movw r1, #78
	movw r0, #77
	cmp r1, r0
	ble main$L400
main$L335:
	movw r1, #79
	movw r0, #78
	cmp r1, r0
	ble main$L400
main$L338:
	movw r1, #80
	movw r0, #79
	cmp r1, r0
	ble main$L400
main$L341:
	movw r1, #81
	movw r0, #80
	cmp r1, r0
	ble main$L400
main$L344:
	movw r1, #82
	movw r0, #81
	cmp r1, r0
	ble main$L400
main$L347:
	movw r1, #83
	movw r0, #82
	cmp r1, r0
	ble main$L400
main$L350:
	movw r1, #84
	movw r0, #83
	cmp r1, r0
	ble main$L400
main$L353:
	movw r1, #85
	movw r0, #84
	cmp r1, r0
	ble main$L400
main$L356:
	movw r1, #86
	movw r0, #85
	cmp r1, r0
	ble main$L400
main$L359:
	movw r1, #87
	movw r0, #86
	cmp r1, r0
	ble main$L400
main$L362:
	movw r1, #88
	movw r0, #87
	cmp r1, r0
	ble main$L400
main$L365:
	movw r1, #89
	movw r0, #88
	cmp r1, r0
	ble main$L400
main$L368:
	movw r1, #90
	movw r0, #89
	cmp r1, r0
	ble main$L400
main$L371:
	movw r1, #91
	movw r0, #90
	cmp r1, r0
	ble main$L400
main$L374:
	movw r1, #92
	movw r0, #91
	cmp r1, r0
	ble main$L400
main$L377:
	movw r1, #93
	movw r0, #92
	cmp r1, r0
	ble main$L400
main$L380:
	movw r1, #94
	movw r0, #93
	cmp r1, r0
	ble main$L400
main$L383:
	movw r1, #95
	movw r0, #94
	cmp r1, r0
	ble main$L400
main$L386:
	movw r1, #96
	movw r0, #95
	cmp r1, r0
	ble main$L400
main$L389:
	movw r1, #97
	movw r0, #96
	cmp r1, r0
	ble main$L400
main$L392:
	movw r1, #98
	movw r0, #97
	cmp r1, r0
	ble main$L400
main$L395:
	movw r1, #99
	movw r0, #98
	cmp r1, r0
	ble main$L400
main$L398:
	movw r1, #100
	movw r0, #99
	cmp r1, r0
	ble main$L400
main$L399:
	movw r0, #100
	bl putint
	movw r0, #10
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
