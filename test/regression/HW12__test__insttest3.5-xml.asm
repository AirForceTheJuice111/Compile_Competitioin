<ScheduleProgram program_last_label_num="108" program_last_temp_num="11900" function_count="2">
    <Function index="0" name="__$main__^main" last_label_num="108" last_temp_num="11904" instruction_count="25">
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
        <Instruction index="4" kind="I_OPER" assem="movw `d0, #8">
            <Dst>
                <Temp index="0" num="11901" name="t11901"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="push {`s0}">
            <Dst/>
            <Src>
                <Temp index="0" num="11901" name="t11901"/>
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
                <Temp index="0" num="10400" name="t10400"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="adr `d0, C^m">
            <Dst>
                <Temp index="0" num="11902" name="t11902"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="str `s0, [`s1, #4]">
            <Dst/>
            <Src>
                <Temp index="0" num="11902" name="t11902"/>
                <Temp index="1" num="10400" name="t10400"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="ldr `d0, [`s0, #4]">
            <Dst>
                <Temp index="0" num="10700" name="t10700"/>
            </Dst>
            <Src>
                <Temp index="0" num="10400" name="t10400"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10900" name="t10900"/>
            </Dst>
            <Src>
                <Temp index="0" num="10700" name="t10700"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="push {`s0}">
            <Dst/>
            <Src>
                <Temp index="0" num="11901" name="t11901"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_OPER" assem="pop {r0}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_EXTCALL" assem="bl malloc">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_OPER" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="10401" name="t10401"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_OPER" assem="adr `d0, C^m">
            <Dst>
                <Temp index="0" num="11904" name="t11904"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="str `s0, [`s1, #4]">
            <Dst/>
            <Src>
                <Temp index="0" num="11904" name="t11904"/>
                <Temp index="1" num="10401" name="t10401"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_OPER" assem="push {`s0}">
            <Dst/>
            <Src>
                <Temp index="0" num="10900" name="t10900"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="20" kind="I_OPER" assem="push {`s0}">
            <Dst/>
            <Src>
                <Temp index="0" num="10401" name="t10401"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="21" kind="I_OPER" assem="pop {r0}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="22" kind="I_OPER" assem="pop {ip}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="23" kind="I_OPER" assem="blx ip">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="24" kind="I_OPER" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="10000" name="t10000"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
    </Function>
    <Function index="1" name="C^m" last_label_num="100" last_temp_num="10300" instruction_count="11">
        <Instruction index="0" kind="I_LABEL" assem="L100:">
            <Label num="100" name="L100"/>
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
                <Temp index="0" num="100" name="t100"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="ldr `d0, [`s0]">
            <Dst>
                <Temp index="0" num="10300" name="t10300"/>
            </Dst>
            <Src>
                <Temp index="0" num="100" name="t100"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="10300" name="t10300"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
    </Function>
</ScheduleProgram>
