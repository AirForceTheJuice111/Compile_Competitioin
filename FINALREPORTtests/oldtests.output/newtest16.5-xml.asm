<ScheduleProgram program_last_label_num="104" program_last_temp_num="110" function_count="1">
    <Function index="0" name="__$main__^main" last_label_num="104" last_temp_num="119" instruction_count="35">
        <Instruction index="0" kind="I_LABEL" assem="L103:">
            <Label num="103" name="L103"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="1" kind="I_OPER" assem="push {r4-r10, fp, lr}">
            <Dst>
                <Temp index="0" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="4" name="t4"/>
                <Temp index="1" num="5" name="t5"/>
                <Temp index="2" num="6" name="t6"/>
                <Temp index="3" num="7" name="t7"/>
                <Temp index="4" num="8" name="t8"/>
                <Temp index="5" num="9" name="t9"/>
                <Temp index="6" num="10" name="t10"/>
                <Temp index="7" num="11" name="t11"/>
                <Temp index="8" num="14" name="t14"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="2" kind="I_OPER" assem="sub sp, sp, #4">
            <Dst>
                <Temp index="0" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="13" name="t13"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="3" kind="I_OPER" assem="add fp, sp, #36">
            <Dst>
                <Temp index="0" num="11" name="t11"/>
            </Dst>
            <Src>
                <Temp index="0" num="13" name="t13"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="4" kind="I_OPER" assem="ldr `d0, [`s0, #4]">
            <Dst>
                <Temp index="0" num="10201" name="t10201"/>
            </Dst>
            <Src>
                <Temp index="0" num="100" name="t100"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10501" name="t10501"/>
            </Dst>
            <Src>
                <Temp index="0" num="10201" name="t10201"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="ldr `d0, [`s0]">
            <Dst>
                <Temp index="0" num="10301" name="t10301"/>
            </Dst>
            <Src>
                <Temp index="0" num="10201" name="t10201"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_MOVE" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="115" name="t115"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_MOVE" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="116" name="t116"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="115" name="t115"/>
                <Temp index="1" num="116" name="t116"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="bge `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="101" name="L101"/>
            </Jumps>
        </Instruction>
        <Instruction index="11" kind="I_LABEL" assem="L100:">
            <Label num="100" name="L100"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_MOVE" assem="movw `d0, #65535">
            <Dst>
                <Temp index="0" num="117" name="t117"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_MOVE" assem="movt `d0, #65535">
            <Dst>
                <Temp index="0" num="117" name="t117"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="117" name="t117"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="bl exit">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_LABEL" assem="L101:">
            <Label num="101" name="L101"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_MOVE" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="118" name="t118"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="118" name="t118"/>
                <Temp index="1" num="10301" name="t10301"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_OPER" assem="bge `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="100" name="L100"/>
            </Jumps>
        </Instruction>
        <Instruction index="20" kind="I_LABEL" assem="L102:">
            <Label num="102" name="L102"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="21" kind="I_MOVE" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="111" name="t111"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="22" kind="I_MOVE" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="112" name="t112"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="23" kind="I_OPER" assem="add `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="10801" name="t10801"/>
            </Dst>
            <Src>
                <Temp index="0" num="111" name="t111"/>
                <Temp index="1" num="112" name="t112"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="24" kind="I_MOVE" assem="movw `d0, #4">
            <Dst>
                <Temp index="0" num="113" name="t113"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="25" kind="I_OPER" assem="mul `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="10901" name="t10901"/>
            </Dst>
            <Src>
                <Temp index="0" num="10801" name="t10801"/>
                <Temp index="1" num="113" name="t113"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="26" kind="I_MOVE" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="114" name="t114"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="27" kind="I_OPER" assem="str `s0, [`s1, `s2]">
            <Dst/>
            <Src>
                <Temp index="0" num="114" name="t114"/>
                <Temp index="1" num="10501" name="t10501"/>
                <Temp index="2" num="10901" name="t10901"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="28" kind="I_LABEL" assem="L104:">
            <Label num="104" name="L104"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="29" kind="I_MOVE" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="119" name="t119"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="30" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="119" name="t119"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="31" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst>
                <Temp index="0" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="11" name="t11"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="32" kind="I_OPER" assem="add sp, sp, #4">
            <Dst>
                <Temp index="0" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="13" name="t13"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="33" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst>
                <Temp index="0" num="4" name="t4"/>
                <Temp index="1" num="5" name="t5"/>
                <Temp index="2" num="6" name="t6"/>
                <Temp index="3" num="7" name="t7"/>
                <Temp index="4" num="8" name="t8"/>
                <Temp index="5" num="9" name="t9"/>
                <Temp index="6" num="10" name="t10"/>
                <Temp index="7" num="11" name="t11"/>
                <Temp index="8" num="14" name="t14"/>
                <Temp index="9" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="13" name="t13"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="34" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src>
                <Temp index="0" num="14" name="t14"/>
            </Src>
            <Jumps/>
        </Instruction>
    </Function>
</ScheduleProgram>
