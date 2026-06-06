<ScheduleProgram program_last_label_num="105" program_last_temp_num="10300" function_count="1">
    <Function index="0" name="__$main__^main" last_label_num="105" last_temp_num="10302" instruction_count="15">
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
        <Instruction index="4" kind="I_LABEL" assem="L102:">
            <Label num="102" name="L102"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="10301" name="t10301"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="10302" name="t10302"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_OPER" assem="sdiv `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="10101" name="t10101"/>
            </Dst>
            <Src>
                <Temp index="0" num="10301" name="t10301"/>
                <Temp index="1" num="10302" name="t10302"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_LABEL" assem="L103:">
            <Label num="103" name="L103"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10102" name="t10102"/>
            </Dst>
            <Src>
                <Temp index="0" num="10101" name="t10101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="10102" name="t10102"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
    </Function>
</ScheduleProgram>
