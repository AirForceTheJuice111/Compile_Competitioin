.balign 4
.global main
.section .text
.arm
main:
main$L595:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
main$L102:
	movw r107, #2
	movw r108, #1
	cmp r107, r108
	bgt main$L103
main$L104:
	movw r105, #100
	mov r0, r105
	bl putint
	movw r106, #10
	mov r0, r106
	bl putch
	movw r109, #0
	mov r0, r109
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
main$L107:
	movw r110, #3
	movw r111, #2
	cmp r110, r111
	bgt main$L108
main$L109:
	b main$L104
main$L108:
main$L112:
	movw r112, #4
	movw r113, #3
	cmp r112, r113
	bgt main$L113
main$L114:
	b main$L109
main$L113:
main$L117:
	movw r114, #5
	movw r115, #4
	cmp r114, r115
	bgt main$L118
main$L119:
	b main$L114
main$L118:
main$L122:
	movw r116, #6
	movw r117, #5
	cmp r116, r117
	bgt main$L123
main$L124:
	b main$L119
main$L123:
main$L127:
	movw r118, #7
	movw r119, #6
	cmp r118, r119
	bgt main$L128
main$L129:
	b main$L124
main$L128:
main$L132:
	movw r120, #8
	movw r121, #7
	cmp r120, r121
	bgt main$L133
main$L134:
	b main$L129
main$L133:
main$L137:
	movw r122, #9
	movw r123, #8
	cmp r122, r123
	bgt main$L138
main$L139:
	b main$L134
main$L138:
main$L142:
	movw r124, #10
	movw r125, #9
	cmp r124, r125
	bgt main$L143
main$L144:
	b main$L139
main$L143:
main$L147:
	movw r126, #11
	movw r127, #10
	cmp r126, r127
	bgt main$L148
main$L149:
	b main$L144
main$L148:
main$L152:
	movw r128, #12
	movw r129, #11
	cmp r128, r129
	bgt main$L153
main$L154:
	b main$L149
main$L153:
main$L157:
	movw r130, #13
	movw r131, #12
	cmp r130, r131
	bgt main$L158
main$L159:
	b main$L154
main$L158:
main$L162:
	movw r132, #14
	movw r133, #13
	cmp r132, r133
	bgt main$L163
main$L164:
	b main$L159
main$L163:
main$L167:
	movw r134, #15
	movw r135, #14
	cmp r134, r135
	bgt main$L168
main$L169:
	b main$L164
main$L168:
main$L172:
	movw r136, #16
	movw r137, #15
	cmp r136, r137
	bgt main$L173
main$L174:
	b main$L169
main$L173:
main$L177:
	movw r138, #17
	movw r139, #16
	cmp r138, r139
	bgt main$L178
main$L179:
	b main$L174
main$L178:
main$L182:
	movw r140, #18
	movw r141, #17
	cmp r140, r141
	bgt main$L183
main$L184:
	b main$L179
main$L183:
main$L187:
	movw r142, #19
	movw r143, #18
	cmp r142, r143
	bgt main$L188
main$L189:
	b main$L184
main$L188:
main$L192:
	movw r144, #20
	movw r145, #19
	cmp r144, r145
	bgt main$L193
main$L194:
	b main$L189
main$L193:
main$L197:
	movw r146, #21
	movw r147, #20
	cmp r146, r147
	bgt main$L198
main$L199:
	b main$L194
main$L198:
main$L202:
	movw r148, #22
	movw r149, #21
	cmp r148, r149
	bgt main$L203
main$L204:
	b main$L199
main$L203:
main$L207:
	movw r150, #23
	movw r151, #22
	cmp r150, r151
	bgt main$L208
main$L209:
	b main$L204
main$L208:
main$L212:
	movw r152, #24
	movw r153, #23
	cmp r152, r153
	bgt main$L213
main$L214:
	b main$L209
main$L213:
main$L217:
	movw r154, #25
	movw r155, #24
	cmp r154, r155
	bgt main$L218
main$L219:
	b main$L214
main$L218:
main$L222:
	movw r156, #26
	movw r157, #25
	cmp r156, r157
	bgt main$L223
main$L224:
	b main$L219
main$L223:
main$L227:
	movw r158, #27
	movw r159, #26
	cmp r158, r159
	bgt main$L228
main$L229:
	b main$L224
main$L228:
main$L232:
	movw r160, #28
	movw r161, #27
	cmp r160, r161
	bgt main$L233
main$L234:
	b main$L229
main$L233:
main$L237:
	movw r162, #29
	movw r163, #28
	cmp r162, r163
	bgt main$L238
main$L239:
	b main$L234
main$L238:
main$L242:
	movw r164, #30
	movw r165, #29
	cmp r164, r165
	bgt main$L243
main$L244:
	b main$L239
main$L243:
main$L247:
	movw r166, #31
	movw r167, #30
	cmp r166, r167
	bgt main$L248
main$L249:
	b main$L244
main$L248:
main$L252:
	movw r168, #32
	movw r169, #31
	cmp r168, r169
	bgt main$L253
main$L254:
	b main$L249
main$L253:
main$L257:
	movw r170, #33
	movw r171, #32
	cmp r170, r171
	bgt main$L258
main$L259:
	b main$L254
main$L258:
main$L262:
	movw r172, #34
	movw r173, #33
	cmp r172, r173
	bgt main$L263
main$L264:
	b main$L259
main$L263:
main$L267:
	movw r174, #35
	movw r175, #34
	cmp r174, r175
	bgt main$L268
main$L269:
	b main$L264
main$L268:
main$L272:
	movw r176, #36
	movw r177, #35
	cmp r176, r177
	bgt main$L273
main$L274:
	b main$L269
main$L273:
main$L277:
	movw r178, #37
	movw r179, #36
	cmp r178, r179
	bgt main$L278
main$L279:
	b main$L274
main$L278:
main$L282:
	movw r180, #38
	movw r181, #37
	cmp r180, r181
	bgt main$L283
main$L284:
	b main$L279
main$L283:
main$L287:
	movw r182, #39
	movw r183, #38
	cmp r182, r183
	bgt main$L288
main$L289:
	b main$L284
main$L288:
main$L292:
	movw r184, #40
	movw r185, #39
	cmp r184, r185
	bgt main$L293
main$L294:
	b main$L289
main$L293:
main$L297:
	movw r186, #41
	movw r187, #40
	cmp r186, r187
	bgt main$L298
main$L299:
	b main$L294
main$L298:
main$L302:
	movw r188, #42
	movw r189, #41
	cmp r188, r189
	bgt main$L303
main$L304:
	b main$L299
main$L303:
main$L307:
	movw r190, #43
	movw r191, #42
	cmp r190, r191
	bgt main$L308
main$L309:
	b main$L304
main$L308:
main$L312:
	movw r192, #44
	movw r193, #43
	cmp r192, r193
	bgt main$L313
main$L314:
	b main$L309
main$L313:
main$L317:
	movw r194, #45
	movw r195, #44
	cmp r194, r195
	bgt main$L318
main$L319:
	b main$L314
main$L318:
main$L322:
	movw r196, #46
	movw r197, #45
	cmp r196, r197
	bgt main$L323
main$L324:
	b main$L319
main$L323:
main$L327:
	movw r198, #47
	movw r199, #46
	cmp r198, r199
	bgt main$L328
main$L329:
	b main$L324
main$L328:
main$L332:
	movw r200, #48
	movw r201, #47
	cmp r200, r201
	bgt main$L333
main$L334:
	b main$L329
main$L333:
main$L337:
	movw r202, #49
	movw r203, #48
	cmp r202, r203
	bgt main$L338
main$L339:
	b main$L334
main$L338:
main$L342:
	movw r204, #50
	movw r205, #49
	cmp r204, r205
	bgt main$L343
main$L344:
	b main$L339
main$L343:
main$L347:
	movw r206, #51
	movw r207, #50
	cmp r206, r207
	bgt main$L348
main$L349:
	b main$L344
main$L348:
main$L352:
	movw r208, #52
	movw r209, #51
	cmp r208, r209
	bgt main$L353
main$L354:
	b main$L349
main$L353:
main$L357:
	movw r210, #53
	movw r211, #52
	cmp r210, r211
	bgt main$L358
main$L359:
	b main$L354
main$L358:
main$L362:
	movw r212, #54
	movw r213, #53
	cmp r212, r213
	bgt main$L363
main$L364:
	b main$L359
main$L363:
main$L367:
	movw r214, #55
	movw r215, #54
	cmp r214, r215
	bgt main$L368
main$L369:
	b main$L364
main$L368:
main$L372:
	movw r216, #56
	movw r217, #55
	cmp r216, r217
	bgt main$L373
main$L374:
	b main$L369
main$L373:
main$L377:
	movw r218, #57
	movw r219, #56
	cmp r218, r219
	bgt main$L378
main$L379:
	b main$L374
main$L378:
main$L382:
	movw r220, #58
	movw r221, #57
	cmp r220, r221
	bgt main$L383
main$L384:
	b main$L379
main$L383:
main$L387:
	movw r222, #59
	movw r223, #58
	cmp r222, r223
	bgt main$L388
main$L389:
	b main$L384
main$L388:
main$L392:
	movw r224, #60
	movw r225, #59
	cmp r224, r225
	bgt main$L393
main$L394:
	b main$L389
main$L393:
main$L397:
	movw r226, #61
	movw r227, #60
	cmp r226, r227
	bgt main$L398
main$L399:
	b main$L394
main$L398:
main$L402:
	movw r228, #62
	movw r229, #61
	cmp r228, r229
	bgt main$L403
main$L404:
	b main$L399
main$L403:
main$L407:
	movw r230, #63
	movw r231, #62
	cmp r230, r231
	bgt main$L408
main$L409:
	b main$L404
main$L408:
main$L412:
	movw r232, #64
	movw r233, #63
	cmp r232, r233
	bgt main$L413
main$L414:
	b main$L409
main$L413:
main$L417:
	movw r234, #65
	movw r235, #64
	cmp r234, r235
	bgt main$L418
main$L419:
	b main$L414
main$L418:
main$L422:
	movw r236, #66
	movw r237, #65
	cmp r236, r237
	bgt main$L423
main$L424:
	b main$L419
main$L423:
main$L427:
	movw r238, #67
	movw r239, #66
	cmp r238, r239
	bgt main$L428
main$L429:
	b main$L424
main$L428:
main$L432:
	movw r240, #68
	movw r241, #67
	cmp r240, r241
	bgt main$L433
main$L434:
	b main$L429
main$L433:
main$L437:
	movw r242, #69
	movw r243, #68
	cmp r242, r243
	bgt main$L438
main$L439:
	b main$L434
main$L438:
main$L442:
	movw r244, #70
	movw r245, #69
	cmp r244, r245
	bgt main$L443
main$L444:
	b main$L439
main$L443:
main$L447:
	movw r246, #71
	movw r247, #70
	cmp r246, r247
	bgt main$L448
main$L449:
	b main$L444
main$L448:
main$L452:
	movw r248, #72
	movw r249, #71
	cmp r248, r249
	bgt main$L453
main$L454:
	b main$L449
main$L453:
main$L457:
	movw r250, #73
	movw r251, #72
	cmp r250, r251
	bgt main$L458
main$L459:
	b main$L454
main$L458:
main$L462:
	movw r252, #74
	movw r253, #73
	cmp r252, r253
	bgt main$L463
main$L464:
	b main$L459
main$L463:
main$L467:
	movw r254, #75
	movw r255, #74
	cmp r254, r255
	bgt main$L468
main$L469:
	b main$L464
main$L468:
main$L472:
	movw r256, #76
	movw r257, #75
	cmp r256, r257
	bgt main$L473
main$L474:
	b main$L469
main$L473:
main$L477:
	movw r258, #77
	movw r259, #76
	cmp r258, r259
	bgt main$L478
main$L479:
	b main$L474
main$L478:
main$L482:
	movw r260, #78
	movw r261, #77
	cmp r260, r261
	bgt main$L483
main$L484:
	b main$L479
main$L483:
main$L487:
	movw r262, #79
	movw r263, #78
	cmp r262, r263
	bgt main$L488
main$L489:
	b main$L484
main$L488:
main$L492:
	movw r264, #80
	movw r265, #79
	cmp r264, r265
	bgt main$L493
main$L494:
	b main$L489
main$L493:
main$L497:
	movw r266, #81
	movw r267, #80
	cmp r266, r267
	bgt main$L498
main$L499:
	b main$L494
main$L498:
main$L502:
	movw r268, #82
	movw r269, #81
	cmp r268, r269
	bgt main$L503
main$L504:
	b main$L499
main$L503:
main$L507:
	movw r270, #83
	movw r271, #82
	cmp r270, r271
	bgt main$L508
main$L509:
	b main$L504
main$L508:
main$L512:
	movw r272, #84
	movw r273, #83
	cmp r272, r273
	bgt main$L513
main$L514:
	b main$L509
main$L513:
main$L517:
	movw r274, #85
	movw r275, #84
	cmp r274, r275
	bgt main$L518
main$L519:
	b main$L514
main$L518:
main$L522:
	movw r276, #86
	movw r277, #85
	cmp r276, r277
	bgt main$L523
main$L524:
	b main$L519
main$L523:
main$L527:
	movw r278, #87
	movw r279, #86
	cmp r278, r279
	bgt main$L528
main$L529:
	b main$L524
main$L528:
main$L532:
	movw r280, #88
	movw r281, #87
	cmp r280, r281
	bgt main$L533
main$L534:
	b main$L529
main$L533:
main$L537:
	movw r282, #89
	movw r283, #88
	cmp r282, r283
	bgt main$L538
main$L539:
	b main$L534
main$L538:
main$L542:
	movw r284, #90
	movw r285, #89
	cmp r284, r285
	bgt main$L543
main$L544:
	b main$L539
main$L543:
main$L547:
	movw r286, #91
	movw r287, #90
	cmp r286, r287
	bgt main$L548
main$L549:
	b main$L544
main$L548:
main$L552:
	movw r288, #92
	movw r289, #91
	cmp r288, r289
	bgt main$L553
main$L554:
	b main$L549
main$L553:
main$L557:
	movw r290, #93
	movw r291, #92
	cmp r290, r291
	bgt main$L558
main$L559:
	b main$L554
main$L558:
main$L562:
	movw r292, #94
	movw r293, #93
	cmp r292, r293
	bgt main$L563
main$L564:
	b main$L559
main$L563:
main$L567:
	movw r294, #95
	movw r295, #94
	cmp r294, r295
	bgt main$L568
main$L569:
	b main$L564
main$L568:
main$L572:
	movw r296, #96
	movw r297, #95
	cmp r296, r297
	bgt main$L573
main$L574:
	b main$L569
main$L573:
main$L577:
	movw r298, #97
	movw r299, #96
	cmp r298, r299
	bgt main$L578
main$L579:
	b main$L574
main$L578:
main$L582:
	movw r300, #98
	movw r301, #97
	cmp r300, r301
	bgt main$L583
main$L584:
	b main$L579
main$L583:
main$L587:
	movw r302, #99
	movw r303, #98
	cmp r302, r303
	bgt main$L588
main$L589:
	b main$L584
main$L588:
main$L592:
	movw r304, #100
	movw r305, #99
	cmp r304, r305
	bgt main$L593
main$L594:
	b main$L589
main$L593:
	b main$L594

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
