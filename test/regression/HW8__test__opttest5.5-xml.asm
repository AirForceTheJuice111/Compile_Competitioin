<ScheduleProgram program_last_label_num="108" program_last_temp_num="10400" function_count="1">
    <Function index="0" name="__$main__^main" last_label_num="108" last_temp_num="10403" instruction_count="36">
        <Instruction index="0" kind="I_LABEL" assem="L108:">
            <Label num="108" name="L108"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="1" kind="I_OPER" assem="push {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="2" kind="I_OPER" assem="sub sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="3" kind="I_OPER" assem="add fp, sp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="4" kind="I_EXTCALL" assem="bl getint">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="10300" name="t10300"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_MOVE" assem="mov `d0, #0">
            <Dst>
                <Temp index="0" num="106" name="t106"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="10401" name="t10401"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="10300" name="t10300"/>
                <Temp index="1" num="10401" name="t10401"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="bne `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="102" name="L102"/>
            </Jumps>
        </Instruction>
        <Instruction index="10" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10102" name="t10102"/>
            </Dst>
            <Src>
                <Temp index="0" num="106" name="t106"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10001" name="t10001"/>
            </Dst>
            <Src>
                <Temp index="0" num="100" name="t100"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_LABEL" assem="L103:">
            <Label num="103" name="L103"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="10402" name="t10402"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="10102" name="t10102"/>
                <Temp index="1" num="10402" name="t10402"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="bne `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="106" name="L106"/>
            </Jumps>
        </Instruction>
        <Instruction index="16" kind="I_LABEL" assem="L107:">
            <Label num="107" name="L107"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_OPER" assem="movw `d0, #65435">
            <Dst>
                <Temp index="0" num="10403" name="t10403"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="movt `d0, #65535">
            <Dst>
                <Temp index="0" num="10403" name="t10403"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="10403" name="t10403"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="20" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="21" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="22" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="23" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="24" kind="I_LABEL" assem="L106:">
            <Label num="106" name="L106"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="25" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="10001" name="t10001"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="26" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="27" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="28" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="29" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="30" kind="I_LABEL" assem="L102:">
            <Label num="102" name="L102"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="31" kind="I_MOVE" assem="mov `d0, #1">
            <Dst>
                <Temp index="0" num="105" name="t105"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="32" kind="I_MOVE" assem="mov `d0, #9">
            <Dst>
                <Temp index="0" num="107" name="t107"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="33" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10102" name="t10102"/>
            </Dst>
            <Src>
                <Temp index="0" num="105" name="t105"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="34" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10001" name="t10001"/>
            </Dst>
            <Src>
                <Temp index="0" num="107" name="t107"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="35" kind="I_OPER" assem="b `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="103" name="L103"/>
            </Jumps>
        </Instruction>
    </Function>
</ScheduleProgram>
