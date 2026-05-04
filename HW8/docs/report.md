---
title: "HW8 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW8 实验报告

## 实验目标

做了如下的事：（SCCP算法）

- 在函数内联合求解基本块可执行性与 SSA 临时变量的三值格。
- 对可证明为常量的 SSA temp 做常量传播与常量折叠。
- 根据条件跳转的可判定结果裁剪不可达基本块。
- 正确处理 `phi`、内存读取、函数调用和函数参数等保守边界。
- 在可达块中检测未定义值的使用，报告错误并提升为 `MANY_VALUES` 后继续优化。

## 参考资料

- 课程课件 `LLMweek9.pdf`。
- 关于SCCP的网络资料与AI问答。

## 实现思路

### 状态设计

实现完全沿用骨架里的两类状态：

- `block_executable[label]` —— 该基本块是否已被证明可执行。
- `temp_value[temp]` —— SSA temp 的三值格，取值为 `NO_VALUE`、`ONE_VALUE(c)` 或 `MANY_VALUES`。

### 分析阶段（`calculateBT()`）

核心是一个迭代到不动点的循环，只处理已标记为可执行的块。

初始化时：

- 入口块默认可执行。
- 函数参数初始化为 `MANY_VALUES`（调用方传入的值未知）。
- 其他 temp 通过 `getRtValue()` 惰性初始化为 `NO_VALUE`。

每轮迭代按语句种类更新状态：

- `MOVE` —— 传播源操作数的格值（通过 `evalTermChecked`）。
- `MOVE_BINOP` —— 两侧均为 `ONE_VALUE` 时精确折叠；任一侧为 `MANY_VALUES` 时退化为 `MANY_VALUES`（通过 `evalTermChecked`）。
- `LOAD`、`MOVE_CALL`、`MOVE_EXTCALL`、`PTR_CALC` —— 保守地直接记为 `MANY_VALUES`。
- `JUMP` —— 将目标块置为可执行。
- `CJUMP` —— 两侧都是 `ONE_VALUE` 时只放行唯一确定的后继；否则同时打开两条边。
- `PHI` —— 只读取来自可执行前驱的输入；`NO_VALUE` 输入表示"前驱尚未分析完"，正常跳过；多个可执行前驱给出不同常量时提升为 `MANY_VALUES`。

可达块中的未定义值检测：`evalTermChecked` 在发现某 temp 在可达块被使用时仍为 `NO_VALUE` 时，向 `cerr` 输出警告，并将该 temp 提升为 `MANY_VALUES` 后继续分析——这样 CJUMP 就不会因操作数未知而错误地不标记任何后继为可达。

### 回写阶段（`modifyFunc()`）

回写分三个阶段：

#### 阶段一（Liveness 收集）

扫描所有可达块中语句的 `use` 集合，得到 `live_temps`，用于判断某 phi 的目标是否被后续语句引用。

#### 阶段二（常量 phi 输入的 fresh temp 分配）
  
对于结果为 `MANY_VALUES` 的 phi，如果某个可达边的输入 temp 恰好是 `ONE_VALUE`，那么该输入 temp 的赋值语句会在阶段三被删除（因为结果为常量时跳过赋值）。phi 里的引用就会悬空。对此，在前驱块的出口跳转前插入一条 `MOVE fresh_t <- Const`，并用 `phi_subst` 映射表在 phi 内部将原 temp 替换为 fresh temp。

#### 阶段三（新块构建）

包含如下步骤：

- 跳过不可达块。
- `MOVE` / `MOVE_BINOP`：结果为 `ONE_VALUE` 时直接删去赋值（使用点已经被 `rewriteTerm` 替换成常量）。
- `CJUMP`：用 `isEdgeExec` 判断真/假两条边是否可达，若只有一条可达则改写成 `JUMP`，否则保留 `CJUMP`。
- `PHI`：先用 `isEdgeExec` 过滤不可达边的输入；若只剩一个输入，改写为 `MOVE`；若结果为 `ONE_VALUE` 则删去整个 phi。


## 关于多报warning的问题（请助教读一下）

测试下来，opttest6和9会多报warning（Warning: txx used in reachable block with no determined value (undefined use); promoting to MANY_VALUES），但这应该是SCCP这个算法的局限性/我们SSA这种形式的局限性，以opptest6为例：

```java
public int main() {
    int i;
    int i_shadow;
    i_shadow = 0;

    if (0) {
       i = 9;
       i_shadow = 1;
    }

    if (i_shadow) 
       return i;
    else
       return -101;
}
```

原始SSA版本是：

```
Function __$main__^main() last_label=110 last_temp=103:
  Block: Entry Label: L110
    Exit labels: L102 L103 
    LABEL L110; def: use: 
    MOVE t10100:int <- Const:0; def: t10100 use: 
    CJUMP != Const:0 Const:0? L102 : L103; def: use: 
  Block: Entry Label: L102
    Exit labels: L104 
    LABEL L102; def: use: 
    MOVE t10000:int <- Const:9; def: t10000 use: 
    MOVE t10101:int <- Const:1; def: t10101 use: 
    JUMP L104; def: use: 
  Block: Entry Label: L103
    Exit labels: L104 
    LABEL L103; def: use: 
    JUMP L104; def: use: 
  Block: Entry Label: L104
    Exit labels: L107 L108 
    LABEL L104; def: use: 
    PHI t10102:int <- Phi([t10101, L102], [t10100, L103]); def: t10102 use: t10101 t10100 
    PHI t10001:int <- Phi([t10000, L102], [t100, L103]); def: t10001 use: t10000 t100 
    CJUMP != t10102:int Const:0? L107 : L108; def: use: t10102 
  Block: Entry Label: L107
    Exit labels: 
    LABEL L107; def: use: 
    RETURN t10001:int; def: use: t10001 
  Block: Entry Label: L108
    Exit labels: 
    LABEL L108; def: use: 
    MOVE_BINOP t10300:int <- (-, Const:0, Const:101); def: t10300 use: 
    RETURN t10300:int; def: use: t10300 
```

可以看到，
    PHI t10001:int <- Phi([t10000, L102], [t100, L103]); def: t10001 use: t10000 t100 



## 新增测试说明

除了课程原有的 `opttest1`–`opttest9`，我补充了以下额外测试：

- `opttest10`, 常量折叠：`t = 3 + 4` 折叠为 `MOVE t <- 7`，验证 `MOVE_BINOP → ONE_VALUE` 删去赋值并替换使用点。
- `opttest11`, 死块消除：`CJUMP` 条件恒为真，验证假分支块被完全删去。
- `opttest12`, 循环体保留：循环内部的 phi 结果为 `MANY_VALUES`，验证可达循环体即使每轮结果相同也不被误删。
- `opttest13`, 除零保护：`MOVE_BINOP` 除以常量 0，验证不会被折叠为具体值。
- `opttest14`, phi 折叠：两条可执行前驱都给 phi 输入同一个常量，验证 phi 被折叠为 `ONE_VALUE` 并删去。
- `opttest15`, 死前驱冲突隔离：一条前驱因 CJUMP 折叠消失，其上的冲突常量不应污染 phi 结果，phi 最终折叠为常量。
- `opttest16`, 未定义值检测：`CJUMP` 的操作数 temp 从未被赋值（`NO_VALUE`），验证 `evalTermChecked` 报告警告、提升为 `MANY_VALUES`，两条后继均保留。

## git log --oneline

```text
d4febf1 (HEAD -> hw8) feat: implement NO_VALUE undefined-use detection per README
cd8bcd9 HW8: add extra tests 10-15 covering all code paths
82e70a5 (fj/hw8) HW8: fix SCCP optimizer to pass all 9 tests
788d562 Merge branch 'master' of https://gitee.com/fudanCompiler/fducompilerh2026 into hw8
35f39d9 (origin-http/master) Remove useless libsysy
4cd6e47 HW8: README tests
a10a865 HW8 test files added
5b346cc HW8 README.md
0f8b237 HW8 README added
272fd3d (master) Merge branch 'master' of gitee.com:fudanCompiler/fducompilerh2026
dce8e17 (origin/master, origin/HEAD) HW8 added
```

## 测试结果

```text
make build

-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /home/wsy/fducompilerh2026/HW8/build
[2/2] Linking CXX executable tools/main/main

make run

Reading opttest1.4-ssa-xml.quad
Reading Quad (SSA) from xml: opttest1.4-ssa-xml.quad
Writing optimized Quad to file: opttest1.4-ssa-opt.quad
-----Done---
Reading opttest2.4-ssa-xml.quad
Reading Quad (SSA) from xml: opttest2.4-ssa-xml.quad
Writing optimized Quad to file: opttest2.4-ssa-opt.quad
-----Done---
Reading opttest3.4-ssa-xml.quad
Reading Quad (SSA) from xml: opttest3.4-ssa-xml.quad
Writing optimized Quad to file: opttest3.4-ssa-opt.quad
-----Done---
Reading opttest4.4-ssa-xml.quad
Reading Quad (SSA) from xml: opttest4.4-ssa-xml.quad
Writing optimized Quad to file: opttest4.4-ssa-opt.quad
-----Done---
Reading opttest5.4-ssa-xml.quad
Reading Quad (SSA) from xml: opttest5.4-ssa-xml.quad
Writing optimized Quad to file: opttest5.4-ssa-opt.quad
-----Done---
Reading opttest6.4-ssa-xml.quad
Reading Quad (SSA) from xml: opttest6.4-ssa-xml.quad
Writing optimized Quad to file: opttest6.4-ssa-opt.quad
-----Done---
Reading opttest7.4-ssa-xml.quad
Reading Quad (SSA) from xml: opttest7.4-ssa-xml.quad
Warning: t10001 used in reachable block with no determined value (undefined use); promoting to MANY_VALUES
Writing optimized Quad to file: opttest7.4-ssa-opt.quad
-----Done---
Reading opttest8.4-ssa-xml.quad
Reading Quad (SSA) from xml: opttest8.4-ssa-xml.quad
Writing optimized Quad to file: opttest8.4-ssa-opt.quad
-----Done---
Reading opttest9.4-ssa-xml.quad
Reading Quad (SSA) from xml: opttest9.4-ssa-xml.quad
Writing optimized Quad to file: opttest9.4-ssa-opt.quad
-----Done---
Reading opttest10.4-ssa-xml.quad ... -----Done---
Reading opttest11.4-ssa-xml.quad ... -----Done---
Reading opttest12.4-ssa-xml.quad ... -----Done---
Reading opttest13.4-ssa-xml.quad ... -----Done---
Reading opttest14.4-ssa-xml.quad ... -----Done---
Reading opttest15.4-ssa-xml.quad ... -----Done---
Reading opttest16.4-ssa-xml.quad
Warning: t10001 used in reachable block with no determined value (undefined use); promoting to MANY_VALUES
Writing optimized Quad to file: opttest16.4-ssa-opt.quad
-----Done---

diff 对比（与 /tmp/expected/ 下预期输出）：

PASS opttest1 ~ opttest9（全部一致）
```
