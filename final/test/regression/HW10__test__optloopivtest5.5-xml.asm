<ScheduleProgram program_last_label_num="110" program_last_temp_num="10000" function_count="1">
    <Function index="0" name="__$main__^main" last_label_num="110" last_temp_num="10001" instruction_count="11">
        <Instruction index="0" kind="I_LABEL" assem="L110:">
            <Label num="110" name="L110"/>
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
                <Temp index="0" num="10000" name="t10000"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="10001" name="t10001"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="10000" name="t10000"/>
                <Temp index="1" num="10001" name="t10001"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_OPER" assem="bgt `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="107" name="L107"/>
            </Jumps>
        </Instruction>
        <Instruction index="9" kind="I_LABEL" assem="L109:">
            <Label num="109" name="L109"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_LABEL" assem="L107:">
            <Label num="107" name="L107"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
    </Function>
</ScheduleProgram>
