<ScheduleProgram program_last_label_num="115" program_last_temp_num="10100" function_count="2">
    <Function index="0" name="__$main__^main" last_label_num="115" last_temp_num="10100" instruction_count="6">
        <Instruction index="0" kind="I_LABEL" assem="L115:">
            <Label num="115" name="L115"/>
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
                <Temp index="0" num="10100" name="t10100"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
    </Function>
    <Function index="1" name="C^m" last_label_num="105" last_temp_num="105" instruction_count="6">
        <Instruction index="0" kind="I_LABEL" assem="L105:">
            <Label num="105" name="L105"/>
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
        <Instruction index="4" kind="I_OPER" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="102" name="t102"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="mov `d0, r1">
            <Dst>
                <Temp index="0" num="101" name="t101"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
    </Function>
</ScheduleProgram>
