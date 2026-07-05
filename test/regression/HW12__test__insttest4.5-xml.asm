<ScheduleProgram program_last_label_num="108" program_last_temp_num="12000" function_count="2">
    <Function index="0" name="C^m" last_label_num="104" last_temp_num="10601" instruction_count="23">
        <Instruction index="0" kind="I_LABEL" assem="L104:">
            <Label num="104" name="L104"/>
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
                <Temp index="0" num="101" name="t101"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="mov `d0, r1">
            <Dst>
                <Temp index="0" num="100" name="t100"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="10601" name="t10601"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="100" name="t100"/>
                <Temp index="1" num="10601" name="t10601"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_OPER" assem="bgt `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="102" name="L102"/>
            </Jumps>
        </Instruction>
        <Instruction index="9" kind="I_LABEL" assem="L103:">
            <Label num="103" name="L103"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="ldr `d0, [`s0, #4]">
            <Dst>
                <Temp index="0" num="10600" name="t10600"/>
            </Dst>
            <Src>
                <Temp index="0" num="101" name="t101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="10600" name="t10600"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_LABEL" assem="L102:">
            <Label num="102" name="L102"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_OPER" assem="ldr `d0, [`s0]">
            <Dst>
                <Temp index="0" num="10400" name="t10400"/>
            </Dst>
            <Src>
                <Temp index="0" num="101" name="t101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="10400" name="t10400"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="20" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="21" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="22" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
    </Function>
    <Function index="1" name="__$main__^main" last_label_num="108" last_temp_num="12004" instruction_count="31">
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
        <Instruction index="4" kind="I_OPER" assem="movw `d0, #12">
            <Dst>
                <Temp index="0" num="12001" name="t12001"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="push {`s0}">
            <Dst/>
            <Src>
                <Temp index="0" num="12001" name="t12001"/>
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
                <Temp index="0" num="10300" name="t10300"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="adr `d0, C^m">
            <Dst>
                <Temp index="0" num="12002" name="t12002"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="str `s0, [`s1, #8]">
            <Dst/>
            <Src>
                <Temp index="0" num="12002" name="t12002"/>
                <Temp index="1" num="10300" name="t10300"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="ldr `d0, [`s0, #8]">
            <Dst>
                <Temp index="0" num="10600" name="t10600"/>
            </Dst>
            <Src>
                <Temp index="0" num="10300" name="t10300"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="11000" name="t11000"/>
            </Dst>
            <Src>
                <Temp index="0" num="10600" name="t10600"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="push {`s0}">
            <Dst/>
            <Src>
                <Temp index="0" num="12001" name="t12001"/>
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
                <Temp index="0" num="10301" name="t10301"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_OPER" assem="adr `d0, C^m">
            <Dst>
                <Temp index="0" num="12004" name="t12004"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="str `s0, [`s1, #8]">
            <Dst/>
            <Src>
                <Temp index="0" num="12004" name="t12004"/>
                <Temp index="1" num="10301" name="t10301"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10900" name="t10900"/>
            </Dst>
            <Src>
                <Temp index="0" num="10301" name="t10301"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="20" kind="I_EXTCALL" assem="bl getint">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="21" kind="I_OPER" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="10800" name="t10800"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="22" kind="I_OPER" assem="push {`s0}">
            <Dst/>
            <Src>
                <Temp index="0" num="11000" name="t11000"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="23" kind="I_OPER" assem="push {`s0}">
            <Dst/>
            <Src>
                <Temp index="0" num="10900" name="t10900"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="24" kind="I_OPER" assem="push {`s0}">
            <Dst/>
            <Src>
                <Temp index="0" num="10800" name="t10800"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="25" kind="I_OPER" assem="pop {r1}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="26" kind="I_OPER" assem="pop {r0}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="27" kind="I_OPER" assem="pop {ip}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="28" kind="I_OPER" assem="blx ip">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="29" kind="I_OPER" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="10000" name="t10000"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="30" kind="I_OPER" assem="ldr `d0, [`s0]">
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
