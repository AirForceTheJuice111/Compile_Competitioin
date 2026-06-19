<ScheduleProgram program_last_label_num="106" program_last_temp_num="122" function_count="4">
    <Function index="0" name="TestClass^test1" last_label_num="106" last_temp_num="128" instruction_count="45">
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
        <Instruction index="4" kind="I_MOVE" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="101" name="t101"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_MOVE" assem="mov `d0, r1">
            <Dst>
                <Temp index="0" num="100" name="t100"/>
            </Dst>
            <Src>
                <Temp index="0" num="1" name="t1"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_MOVE" assem="mov `d0, #1">
            <Dst>
                <Temp index="0" num="104010101" name="t104010101"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="114010101" name="t114010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="101" name="t101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="115010101" name="t115010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="101" name="t101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="add `d0, `s0, #1">
            <Dst>
                <Temp index="0" num="112010101" name="t112010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="104010101" name="t104010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_MOVE" assem="movw `d0, #4">
            <Dst>
                <Temp index="0" num="121" name="t121"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="mul `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="108010101" name="t108010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="112010101" name="t112010101"/>
                <Temp index="1" num="121" name="t121"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="108010101" name="t108010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="bl malloc">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
                <Temp index="1" num="14" name="t14"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_MOVE" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="105010101" name="t105010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="str `s0, [`s1]">
            <Dst/>
            <Src>
                <Temp index="0" num="104010101" name="t104010101"/>
                <Temp index="1" num="105010101" name="t105010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_OPER" assem="str `s0, [`s1]">
            <Dst/>
            <Src>
                <Temp index="0" num="105010101" name="t105010101"/>
                <Temp index="1" num="114010101" name="t114010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_OPER" assem="ldr `d0, [`s0]">
            <Dst>
                <Temp index="0" num="106010101" name="t106010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="115010101" name="t115010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="ldr `d0, [`s0]">
            <Dst>
                <Temp index="0" num="107010101" name="t107010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="106010101" name="t106010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="111010101" name="t111010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="106010101" name="t106010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="20" kind="I_MOVE" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="125" name="t125"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="21" kind="I_MOVE" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="126" name="t126"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="22" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="125" name="t125"/>
                <Temp index="1" num="126" name="t126"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="23" kind="I_OPER" assem="bge `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="101" name="L101"/>
            </Jumps>
        </Instruction>
        <Instruction index="24" kind="I_LABEL" assem="L100:">
            <Label num="100" name="L100"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="25" kind="I_MOVE" assem="movw `d0, #65535">
            <Dst>
                <Temp index="0" num="127" name="t127"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="26" kind="I_MOVE" assem="movt `d0, #65535">
            <Dst>
                <Temp index="0" num="127" name="t127"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="27" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="127" name="t127"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="28" kind="I_OPER" assem="bl exit">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="29" kind="I_LABEL" assem="L101:">
            <Label num="101" name="L101"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="30" kind="I_MOVE" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="128" name="t128"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="31" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="128" name="t128"/>
                <Temp index="1" num="107010101" name="t107010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="32" kind="I_OPER" assem="bge `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="100" name="L100"/>
            </Jumps>
        </Instruction>
        <Instruction index="33" kind="I_LABEL" assem="L102:">
            <Label num="102" name="L102"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="34" kind="I_MOVE" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="122" name="t122"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="35" kind="I_MOVE" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="123" name="t123"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="36" kind="I_OPER" assem="add `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="117010101" name="t117010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="122" name="t122"/>
                <Temp index="1" num="123" name="t123"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="37" kind="I_MOVE" assem="movw `d0, #4">
            <Dst>
                <Temp index="0" num="124" name="t124"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="38" kind="I_OPER" assem="mul `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="118010101" name="t118010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="117010101" name="t117010101"/>
                <Temp index="1" num="124" name="t124"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="39" kind="I_OPER" assem="ldr `d0, [`s0, `s1]">
            <Dst>
                <Temp index="0" num="120010101" name="t120010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="111010101" name="t111010101"/>
                <Temp index="1" num="118010101" name="t118010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="40" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="120010101" name="t120010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="41" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst>
                <Temp index="0" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="11" name="t11"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="42" kind="I_OPER" assem="add sp, sp, #4">
            <Dst>
                <Temp index="0" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="13" name="t13"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="43" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
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
        <Instruction index="44" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src>
                <Temp index="0" num="14" name="t14"/>
            </Src>
            <Jumps/>
        </Instruction>
    </Function>
    <Function index="1" name="TestClass^test2" last_label_num="106" last_temp_num="127" instruction_count="44">
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
        <Instruction index="4" kind="I_MOVE" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="101" name="t101"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_MOVE" assem="mov `d0, r1">
            <Dst>
                <Temp index="0" num="100" name="t100"/>
            </Dst>
            <Src>
                <Temp index="0" num="1" name="t1"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_MOVE" assem="mov `d0, #1">
            <Dst>
                <Temp index="0" num="104010101" name="t104010101"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_OPER" assem="add `d0, `s0, #1">
            <Dst>
                <Temp index="0" num="112010101" name="t112010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="104010101" name="t104010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_MOVE" assem="movw `d0, #4">
            <Dst>
                <Temp index="0" num="120" name="t120"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="mul `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="108010101" name="t108010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="112010101" name="t112010101"/>
                <Temp index="1" num="120" name="t120"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="108010101" name="t108010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="bl malloc">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
                <Temp index="1" num="14" name="t14"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_MOVE" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="105010101" name="t105010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="str `s0, [`s1]">
            <Dst/>
            <Src>
                <Temp index="0" num="104010101" name="t104010101"/>
                <Temp index="1" num="105010101" name="t105010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="106010101" name="t106010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="105010101" name="t105010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="ldr `d0, [`s0]">
            <Dst>
                <Temp index="0" num="107010101" name="t107010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="106010101" name="t106010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="111010101" name="t111010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="106010101" name="t106010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_MOVE" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="124" name="t124"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_MOVE" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="125" name="t125"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="124" name="t124"/>
                <Temp index="1" num="125" name="t125"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="20" kind="I_OPER" assem="bge `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="101" name="L101"/>
            </Jumps>
        </Instruction>
        <Instruction index="21" kind="I_LABEL" assem="L100:">
            <Label num="100" name="L100"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="22" kind="I_MOVE" assem="movw `d0, #65535">
            <Dst>
                <Temp index="0" num="126" name="t126"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="23" kind="I_MOVE" assem="movt `d0, #65535">
            <Dst>
                <Temp index="0" num="126" name="t126"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="24" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="126" name="t126"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="25" kind="I_OPER" assem="bl exit">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="26" kind="I_LABEL" assem="L101:">
            <Label num="101" name="L101"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="27" kind="I_MOVE" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="127" name="t127"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="28" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="127" name="t127"/>
                <Temp index="1" num="107010101" name="t107010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="29" kind="I_OPER" assem="bge `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="100" name="L100"/>
            </Jumps>
        </Instruction>
        <Instruction index="30" kind="I_LABEL" assem="L102:">
            <Label num="102" name="L102"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="31" kind="I_MOVE" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="121" name="t121"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="32" kind="I_MOVE" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="122" name="t122"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="33" kind="I_OPER" assem="add `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="115010101" name="t115010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="121" name="t121"/>
                <Temp index="1" num="122" name="t122"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="34" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="118010101" name="t118010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="101" name="t101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="35" kind="I_MOVE" assem="movw `d0, #4">
            <Dst>
                <Temp index="0" num="123" name="t123"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="36" kind="I_OPER" assem="mul `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="116010101" name="t116010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="115010101" name="t115010101"/>
                <Temp index="1" num="123" name="t123"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="37" kind="I_OPER" assem="str `s0, [`s1, `s2]">
            <Dst/>
            <Src>
                <Temp index="0" num="100" name="t100"/>
                <Temp index="1" num="111010101" name="t111010101"/>
                <Temp index="2" num="116010101" name="t116010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="38" kind="I_OPER" assem="ldr `d0, [`s0]">
            <Dst>
                <Temp index="0" num="119010101" name="t119010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="118010101" name="t118010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="39" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="119010101" name="t119010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="40" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst>
                <Temp index="0" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="11" name="t11"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="41" kind="I_OPER" assem="add sp, sp, #4">
            <Dst>
                <Temp index="0" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="13" name="t13"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="42" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
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
        <Instruction index="43" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src>
                <Temp index="0" num="14" name="t14"/>
            </Src>
            <Jumps/>
        </Instruction>
    </Function>
    <Function index="2" name="TestClass^test0" last_label_num="103" last_temp_num="113" instruction_count="23">
        <Instruction index="0" kind="I_LABEL" assem="L100:">
            <Label num="100" name="L100"/>
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
        <Instruction index="4" kind="I_MOVE" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="101" name="t101"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_MOVE" assem="mov `d0, r1">
            <Dst>
                <Temp index="0" num="100" name="t100"/>
            </Dst>
            <Src>
                <Temp index="0" num="1" name="t1"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_MOVE" assem="mov `d0, #1">
            <Dst>
                <Temp index="0" num="104010101" name="t104010101"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="110010101" name="t110010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="101" name="t101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="111010101" name="t111010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="101" name="t101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="add `d0, `s0, #1">
            <Dst>
                <Temp index="0" num="108010101" name="t108010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="104010101" name="t104010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_MOVE" assem="movw `d0, #4">
            <Dst>
                <Temp index="0" num="113" name="t113"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="mul `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="106010101" name="t106010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="108010101" name="t108010101"/>
                <Temp index="1" num="113" name="t113"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="106010101" name="t106010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="bl malloc">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
                <Temp index="1" num="14" name="t14"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_MOVE" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="105010101" name="t105010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="str `s0, [`s1]">
            <Dst/>
            <Src>
                <Temp index="0" num="104010101" name="t104010101"/>
                <Temp index="1" num="105010101" name="t105010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_OPER" assem="str `s0, [`s1]">
            <Dst/>
            <Src>
                <Temp index="0" num="105010101" name="t105010101"/>
                <Temp index="1" num="110010101" name="t110010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_OPER" assem="ldr `d0, [`s0]">
            <Dst>
                <Temp index="0" num="112010101" name="t112010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="111010101" name="t111010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="112010101" name="t112010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst>
                <Temp index="0" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="11" name="t11"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="20" kind="I_OPER" assem="add sp, sp, #4">
            <Dst>
                <Temp index="0" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="13" name="t13"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="21" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
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
        <Instruction index="22" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src>
                <Temp index="0" num="14" name="t14"/>
            </Src>
            <Jumps/>
        </Instruction>
    </Function>
    <Function index="3" name="__$main__^main" last_label_num="106" last_temp_num="135" instruction_count="56">
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
        <Instruction index="4" kind="I_MOVE" assem="movw `d0, #16">
            <Dst>
                <Temp index="0" num="123" name="t123"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="123" name="t123"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="bl malloc">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
                <Temp index="1" num="14" name="t14"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_MOVE" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="100010101" name="t100010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_MOVE" assem="mov `d0, #10">
            <Dst>
                <Temp index="0" num="103010101" name="t103010101"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="117010101" name="t117010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="101" name="t101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_MOVE" assem="movw `d0, #3">
            <Dst>
                <Temp index="0" num="124" name="t124"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="str `s0, [`s1]">
            <Dst/>
            <Src>
                <Temp index="0" num="124" name="t124"/>
                <Temp index="1" num="100010101" name="t100010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_MOVE" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="125" name="t125"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="str `s0, [`s1, #4]">
            <Dst/>
            <Src>
                <Temp index="0" num="125" name="t125"/>
                <Temp index="1" num="100010101" name="t100010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_MOVE" assem="movw `d0, #2">
            <Dst>
                <Temp index="0" num="126" name="t126"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="str `s0, [`s1, #8]">
            <Dst/>
            <Src>
                <Temp index="0" num="126" name="t126"/>
                <Temp index="1" num="100010101" name="t100010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_MOVE" assem="movw `d0, #3">
            <Dst>
                <Temp index="0" num="127" name="t127"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_OPER" assem="str `s0, [`s1, #12]">
            <Dst/>
            <Src>
                <Temp index="0" num="127" name="t127"/>
                <Temp index="1" num="100010101" name="t100010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="110010101" name="t110010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="100010101" name="t100010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_OPER" assem="add `d0, `s0, #1">
            <Dst>
                <Temp index="0" num="115010101" name="t115010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="103010101" name="t103010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="20" kind="I_MOVE" assem="movw `d0, #4">
            <Dst>
                <Temp index="0" num="128" name="t128"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="21" kind="I_OPER" assem="mul `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="107010101" name="t107010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="115010101" name="t115010101"/>
                <Temp index="1" num="128" name="t128"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="22" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="107010101" name="t107010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="23" kind="I_OPER" assem="bl malloc">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
                <Temp index="1" num="14" name="t14"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="24" kind="I_MOVE" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="104010101" name="t104010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="25" kind="I_OPER" assem="str `s0, [`s1]">
            <Dst/>
            <Src>
                <Temp index="0" num="103010101" name="t103010101"/>
                <Temp index="1" num="104010101" name="t104010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="26" kind="I_OPER" assem="str `s0, [`s1]">
            <Dst/>
            <Src>
                <Temp index="0" num="104010101" name="t104010101"/>
                <Temp index="1" num="117010101" name="t117010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="27" kind="I_OPER" assem="ldr `d0, [`s0]">
            <Dst>
                <Temp index="0" num="105010101" name="t105010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="100010101" name="t100010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="28" kind="I_MOVE" assem="movw `d0, #16960">
            <Dst>
                <Temp index="0" num="132" name="t132"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="29" kind="I_MOVE" assem="movt `d0, #15">
            <Dst>
                <Temp index="0" num="132" name="t132"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="30" kind="I_MOVE" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="133" name="t133"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="31" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="132" name="t132"/>
                <Temp index="1" num="133" name="t133"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="32" kind="I_OPER" assem="bge `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="101" name="L101"/>
            </Jumps>
        </Instruction>
        <Instruction index="33" kind="I_LABEL" assem="L100:">
            <Label num="100" name="L100"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="34" kind="I_MOVE" assem="movw `d0, #65535">
            <Dst>
                <Temp index="0" num="134" name="t134"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="35" kind="I_MOVE" assem="movt `d0, #65535">
            <Dst>
                <Temp index="0" num="134" name="t134"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="36" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="134" name="t134"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="37" kind="I_OPER" assem="bl exit">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="38" kind="I_LABEL" assem="L101:">
            <Label num="101" name="L101"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="39" kind="I_MOVE" assem="movw `d0, #16960">
            <Dst>
                <Temp index="0" num="135" name="t135"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="40" kind="I_MOVE" assem="movt `d0, #15">
            <Dst>
                <Temp index="0" num="135" name="t135"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="41" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="135" name="t135"/>
                <Temp index="1" num="105010101" name="t105010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="42" kind="I_OPER" assem="bge `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="100" name="L100"/>
            </Jumps>
        </Instruction>
        <Instruction index="43" kind="I_LABEL" assem="L102:">
            <Label num="102" name="L102"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="44" kind="I_MOVE" assem="movw `d0, #16960">
            <Dst>
                <Temp index="0" num="129" name="t129"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="45" kind="I_MOVE" assem="movt `d0, #15">
            <Dst>
                <Temp index="0" num="129" name="t129"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="46" kind="I_MOVE" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="130" name="t130"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="47" kind="I_OPER" assem="add `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="119010101" name="t119010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="129" name="t129"/>
                <Temp index="1" num="130" name="t130"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="48" kind="I_MOVE" assem="movw `d0, #4">
            <Dst>
                <Temp index="0" num="131" name="t131"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="49" kind="I_OPER" assem="mul `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="120010101" name="t120010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="119010101" name="t119010101"/>
                <Temp index="1" num="131" name="t131"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="50" kind="I_OPER" assem="ldr `d0, [`s0, `s1]">
            <Dst>
                <Temp index="0" num="122010101" name="t122010101"/>
            </Dst>
            <Src>
                <Temp index="0" num="110010101" name="t110010101"/>
                <Temp index="1" num="120010101" name="t120010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="51" kind="I_MOVE" assem="mov r0, `s0">
            <Dst>
                <Temp index="0" num="0" name="t0"/>
            </Dst>
            <Src>
                <Temp index="0" num="122010101" name="t122010101"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="52" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst>
                <Temp index="0" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="11" name="t11"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="53" kind="I_OPER" assem="add sp, sp, #4">
            <Dst>
                <Temp index="0" num="13" name="t13"/>
            </Dst>
            <Src>
                <Temp index="0" num="13" name="t13"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="54" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
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
        <Instruction index="55" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src>
                <Temp index="0" num="14" name="t14"/>
            </Src>
            <Jumps/>
        </Instruction>
    </Function>
</ScheduleProgram>
