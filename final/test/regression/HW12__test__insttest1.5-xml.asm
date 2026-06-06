<ScheduleProgram program_last_label_num="108" program_last_temp_num="11400" function_count="1">
    <Function index="0" name="__$main__^main" last_label_num="108" last_temp_num="11406" instruction_count="19">
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
        <Instruction index="4" kind="I_OPER" assem="movw `d0, #20">
            <Dst>
                <Temp index="0" num="11401" name="t11401"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="push {`s0}">
            <Dst/>
            <Src>
                <Temp index="0" num="11401" name="t11401"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="pop {r0}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_EXTCALL" assem="bl malloc">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_OPER" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="10000" name="t10000"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="movw `d0, #4">
            <Dst>
                <Temp index="0" num="11402" name="t11402"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="str `s0, [`s1]">
            <Dst/>
            <Src>
                <Temp index="0" num="11402" name="t11402"/>
                <Temp index="1" num="10000" name="t10000"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="11403" name="t11403"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_OPER" assem="str `s0, [`s1, #4]">
            <Dst/>
            <Src>
                <Temp index="0" num="11403" name="t11403"/>
                <Temp index="1" num="10000" name="t10000"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="movw `d0, #2">
            <Dst>
                <Temp index="0" num="11404" name="t11404"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_OPER" assem="str `s0, [`s1, #8]">
            <Dst/>
            <Src>
                <Temp index="0" num="11404" name="t11404"/>
                <Temp index="1" num="10000" name="t10000"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="movw `d0, #3">
            <Dst>
                <Temp index="0" num="11405" name="t11405"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_OPER" assem="str `s0, [`s1, #12]">
            <Dst/>
            <Src>
                <Temp index="0" num="11405" name="t11405"/>
                <Temp index="1" num="10000" name="t10000"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_OPER" assem="str `s0, [`s1, #16]">
            <Dst/>
            <Src>
                <Temp index="0" num="11402" name="t11402"/>
                <Temp index="1" num="10000" name="t10000"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="ldr `d0, [`s0]">
            <Dst>
                <Temp index="0" num="10100" name="t10100"/>
            </Dst>
            <Src>
                <Temp index="0" num="10000" name="t10000"/>
            </Src>
            <Jumps/>
        </Instruction>
    </Function>
</ScheduleProgram>
