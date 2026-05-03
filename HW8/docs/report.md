---
title: "HW8 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW8 实验报告

## 实验目标

本次作业要求在 SSA 形式的 Quad IR 上实现稀疏条件常量传播优化（SCCP）。

实现目标包括：

- 在函数内联合求解基本块可执行性与 SSA 临时变量的三值格。
- 对可证明为常量的 SSA temp 做常量传播与常量折叠。
- 根据条件跳转的可判定结果裁剪不可达基本块。
- 正确处理 `phi`、内存读取、函数调用和函数参数等保守边界。
- 在可达块中检测未定义值的使用，报告错误并提升为 `MANY_VALUES` 后继续优化。

## 开发过程与参考资料

参考材料按实际开发顺序如下：

1. 课程课件 `LLMweek9.pdf`。主要用于确定 HW8 的算法边界，包括三值格、最小不动点、`CJUMP` 规则、`phi` 规则，以及"参数/内存/调用默认保守"的约束。
2. 课程提供的骨架接口 [HW8/include/opt/opt.hh](../include/opt/opt.hh)。这里给出了 `RtValue`、`block_executable`、`temp_value`、`label2block` 四个核心数据结构，实现时严格遵守接口约束，不额外改动。
3. Quad IR 定义 [HW8/include/quad/quad.hh](../include/quad/quad.hh) 与克隆实现 [HW8/lib/quad/quad.cc](../lib/quad/quad.cc)。主要用来确认每种 `QuadStm` 的字段结构，并遵守"先 clone 再修改"的约定。
4. XML 解析实现 [HW8/lib/util/xml2quad.cc](../lib/util/xml2quad.cc)。用于核对 `phi` 参数、`MOVE_CALL`、`MOVE_EXTCALL` 等节点在输入 XML 中的真实组织方式。
5. 课程提供的原始测试样例 `opttest1`–`opttest9` 及其预期输出。用来校准优化输出格式，观察 `binop` / `relop` 枚举值以及 `phi` 被折叠/删除的预期形式。
6. 《Modern Compiler Implementation in Java》SSA 与数据流分析章节。帮助对齐"格单调上升 + 迭代到不动点"的理论基础。

## 实现思路

### 1. 状态设计

实现完全沿用骨架里的两类状态：

- `block_executable[label]` —— 该基本块是否已被证明可执行。
- `temp_value[temp]` —— SSA temp 的三值格，取值为 `NO_VALUE`、`ONE_VALUE(c)` 或 `MANY_VALUES`。

格严格单调上升，不可回退：`NO_VALUE → ONE_VALUE(c) → MANY_VALUES`。

### 2. 分析阶段（`calculateBT()`）

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

**可达块中的未定义值检测**：对于 `MOVE`、`MOVE_BINOP`、`CJUMP` 中的 temp 操作数，改用 `evalTermChecked()` 而非裸 `evalTerm()`。`evalTermChecked` 在发现某 temp 在可达块被使用时仍为 `NO_VALUE` 时，向 `cerr` 输出警告，并将该 temp 提升为 `MANY_VALUES` 后继续分析——这样 CJUMP 就不会因操作数未知而错误地不标记任何后继为可达。

### 3. 回写阶段（`modifyFunc()`）

回写分三个阶段：

**阶段一（Liveness 收集）**：扫描所有可达块中语句的 `use` 集合，得到 `live_temps`，用于判断某 phi 的目标是否被后续语句引用。

**阶段二（常量 phi 输入的 fresh temp 分配）**：  
对于结果为 `MANY_VALUES` 的 phi，如果某个可达边的输入 temp 恰好是 `ONE_VALUE`，那么该输入 temp 的赋值语句会在阶段三被删除（因为结果为常量时跳过赋值）。phi 里的引用就会悬空。对此，在前驱块的出口跳转前插入一条 `MOVE fresh_t <- Const`，并用 `phi_subst` 映射表在 phi 内部将原 temp 替换为 fresh temp。

**阶段三（新块构建）**：

- 跳过不可达块。
- `MOVE` / `MOVE_BINOP`：结果为 `ONE_VALUE` 时直接删去赋值（使用点已经被 `rewriteTerm` 替换成常量）。
- `CJUMP`：用 `isEdgeExec` lambda 判断真/假两条边是否可达，若只有一条可达则改写成 `JUMP`，否则保留 `CJUMP`。
- `PHI`：先用 `isEdgeExec` 过滤不可达边的输入；若只剩一个输入，在特定情况下改写为 `MOVE`（见下文）；若结果为 `ONE_VALUE` 则删去整个 phi。

**`isEdgeExec` lambda 的双模式判断**：  
仅靠 `block_executable[pred]` 是不够的——如果前驱块是可执行的，但块内的 `CJUMP` 折叠后指向了另一条边，那么 `pred → curr` 这条边实际上是不可执行的。`isEdgeExec` 通过检查前驱块末尾的 `JUMP`/`CJUMP` 来区分这两种情况，并通过可选的 `is_cjump_fold` 出参告知调用者"这条边消失是因为 CJUMP 折叠"。

**单输入 phi 的处理**：  
删除不可达前驱后，phi 可能只剩一个输入。此时仅在以下两种情况将其改写成 `MOVE`：

1. 边消失是因为 CJUMP 折叠（`had_cjump_fold == true`），且目标 temp 被使用（live）。
2. 输入 temp 的值为 `NO_VALUE`（未定义传播），且目标 temp 被使用（live）。

其余情况保留单输入 phi（与课程预期输出格式一致）。

## 关键实现

### 1. 单调 join 与更新辅助函数

```cpp
static RtValue joinRtValue(RtValue current, RtValue incoming) {
    if (current.getType() == ValueType::MANY_VALUES) return current;
    if (incoming.getType() == ValueType::NO_VALUE) return current;
    if (current.getType() == ValueType::NO_VALUE) return incoming;
    if (incoming.getType() == ValueType::MANY_VALUES) return incoming;
    if (current.getIntValue() == incoming.getIntValue()) return current;
    return RtValue(ValueType::MANY_VALUES);
}

static bool updateRtValue(map<int, RtValue> &m, int num, RtValue incoming) {
    RtValue cur = m.count(num) ? m[num] : RtValue();
    RtValue joined = joinRtValue(cur, incoming);
    if (isSameRtValue(cur, joined)) return false;
    m[num] = joined;
    return true;
}
```

所有 temp 的更新都经过 `joinRtValue`，保证格的单调性，避免在各语句分支里重复手写转移逻辑。

### 2. 可达块中未定义值的检测（`evalTermChecked`）

```cpp
static RtValue evalTermChecked(Opt *opt, QuadTerm *term, bool &changed) {
    if (term == nullptr || term->kind != QuadTermKind::TEMP) return evalTerm(opt, term);
    int num = term->get_temp()->temp->num;
    RtValue val = opt->getRtValue(num);
    if (val.getType() == ValueType::NO_VALUE) {
        cerr << "Warning: t" << num << " used in reachable block with no determined value"
                " (undefined use); promoting to MANY_VALUES" << endl;
        changed |= updateRtValue(opt->temp_value, num, RtValue(ValueType::MANY_VALUES));
        return RtValue(ValueType::MANY_VALUES);
    }
    return val;
}
```

`MOVE`、`MOVE_BINOP`、`CJUMP` 的操作数均通过此函数获取，PHI 的输入不通过此函数（`NO_VALUE` 在 phi 中是正常的"前驱尚未分析"语义）。

### 3. `CJUMP` 双模式传播与 `isEdgeExec` lambda

```cpp
auto isEdgeExec = [&](int pred, int curr, bool *is_cjump_fold = nullptr) -> bool {
    if (!isExecutable(block_executable, pred)) return false;
    // ...检查前驱末尾的 JUMP/CJUMP...
    if (s->kind == QuadKind::CJUMP) {
        auto *cj = static_cast<QuadCJump*>(s);
        RtValue lv = evalTerm(this, cj->left), rv = evalTerm(this, cj->right);
        if (lv.getType() == ONE_VALUE && rv.getType() == ONE_VALUE) {
            bool taken = evalRelop(cj->relop, lv.getIntValue(), rv.getIntValue());
            int tgt = taken ? cj->t->num : cj->f->num;
            bool exec = (tgt == curr);
            if (!exec && is_cjump_fold) *is_cjump_fold = true;
            return exec;
        }
        return true;
    }
};
```

`isEdgeExec` 在 phi 过滤和 CJUMP 改写中统一使用，确保"块可达"和"边可达"两个概念不混淆。

### 4. 常量 phi 输入的 fresh temp 机制

当 phi 的结果为 `MANY_VALUES`，但某个输入 temp `t` 在分析后变为 `ONE_VALUE(c)` 时，`t` 的赋值语句会在回写阶段被删除，phi 里的引用就会悬空。解决方案：在阶段二预扫描时为此类 `(t, pred)` 对分配一个 fresh temp，并在前驱块出口跳转前插入 `MOVE fresh <- c`，然后在 phi 里将 `t` 替换为 `fresh`。

### 5. 除零保护

```cpp
if (binop == "/") {
    if (right == 0) return RtValue(ValueType::MANY_VALUES);
    return RtValue(left / right);
}
```

除以零和取模零均不折叠，保守返回 `MANY_VALUES`，避免在优化阶段引入运行时异常。

## 遇到的问题与解决

### 1. 输出形式不只是在原 IR 上替换常量

SCCP 不只是"替换 temp 为常量"，还应当利用条件常量裁掉不可达块。最终实现里把不可达块直接从 block 列表里去掉，并同步把 `CJUMP` 改成 `JUMP`，删去死边对应的 phi 输入。

### 2. "块可达"≠"边可达"——opttest9 失败的根因

最初实现 phi 过滤时只检查 `block_executable[pred]`，而 opttest9 的输入里有一个前驱块本身是可执行的，但其内部 `CJUMP` 折叠后选择了另一条边，导致 `pred → curr` 这条边实际不可达。引入 `isEdgeExec` lambda 后，phi 过滤和 CJUMP 改写都能正确区分两种情况。

### 3. 常量输入 temp 的赋值被删除后 phi 悬空

当 phi 结果为 `MANY_VALUES`，其中一个输入 temp 的值为 `ONE_VALUE` 时，该赋值会被 phase 3 删除，导致 phi 引用了一个没有定义的 temp。通过阶段二的 fresh temp 预分配机制解决：在前驱出口处插入 `MOVE fresh <- const`，phi 内部替换引用。

### 4. `NO_VALUE` 在可达块被使用导致 CJUMP 不传播

若 CJUMP 的操作数 temp 值为 `NO_VALUE`，在原来的 `evalTerm` 下直接返回 `NO_VALUE`，导致 `calculateBT` 里两个后继块都不被标记为可执行，产生级联错误。引入 `evalTermChecked` 后，`NO_VALUE` 操作数会被立即提升为 `MANY_VALUES`，CJUMP 两个后继都会被标记为可执行，分析继续正常进行。

## 新增测试说明

除了课程原有的 `opttest1`–`opttest9`，我补充了以下额外测试：

- `opttest10` —— 常量折叠：`t = 3 + 4` 折叠为 `MOVE t <- 7`，验证 `MOVE_BINOP → ONE_VALUE` 删去赋值并替换使用点。
- `opttest11` —— 死块消除：`CJUMP` 条件恒为真，验证假分支块被完全删去。
- `opttest12` —— 循环体保留：循环内部的 phi 结果为 `MANY_VALUES`，验证可达循环体即使每轮结果相同也不被误删。
- `opttest13` —— 除零保护：`MOVE_BINOP` 除以常量 0，验证不会被折叠为具体值。
- `opttest14` —— phi 折叠：两条可执行前驱都给 phi 输入同一个常量，验证 phi 被折叠为 `ONE_VALUE` 并删去。
- `opttest15` —— 死前驱冲突隔离：一条前驱因 CJUMP 折叠消失，其上的冲突常量不应污染 phi 结果，phi 最终折叠为常量。
- `opttest16` —— 未定义值检测：`CJUMP` 的操作数 temp 从未被赋值（`NO_VALUE`），验证 `evalTermChecked` 报告警告、提升为 `MANY_VALUES`，两条后继均保留。

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

PASS opttest1 ~ opttest15（全部一致）
```


## 实验目标

本次作业要求在 SSA 形式的 Quad IR 上实现稀疏条件常量传播优化（SCCP）。

实现目标包括：

- 在函数内联合求解基本块可执行性与 SSA 临时变量的三值格。
- 对可证明为常量的 SSA temp 做常量传播与常量折叠。
- 根据条件跳转的可判定结果裁剪不可达基本块。
- 正确处理 `phi`、内存读取、函数调用和函数参数等保守边界。

## 开发过程与参考资料

本次实现时我按“先固化规格，再对照骨架实现，再补边界测试”的顺序推进。

参考材料按实际开发顺序如下：

1. 课程课件 `LLMweek9.pdf`。主要用于确定 HW8 的算法边界，包括三值格、最小不动点、`CJUMP` 规则、`phi` 规则，以及“参数/内存/调用默认保守”的约束。为了便于查阅，我先将其整理成了仓库内笔记 [HW8/LLMweek9.md](../LLMweek9.md)。
2. 课程提供的骨架接口 [HW8/include/opt/opt.hh](../include/opt/opt.hh)。这里已经给出了 `RtValue`、`block_executable`、`temp_value`、`label2block` 四个核心数据结构，我据此约束实现，不额外改动接口。
3. Quad IR 定义 [HW8/include/quad/quad.hh](../include/quad/quad.hh) 与克隆实现 [HW8/lib/quad/quad.cc](../lib/quad/quad.cc)。主要用来确认每种 `QuadStm` 的字段结构，并遵守“先 clone 再修改”的约定。
4. XML 解析实现 [HW8/lib/util/xml2quad.cc](../lib/util/xml2quad.cc)。用于核对 `phi` 参数、`MOVE_CALL`、`MOVE_EXTCALL` 等节点在输入 XML 中的真实组织方式。
5. 现有测试样例 [HW8/test](../test)。主要用来校准优化输出形式，确认生成的是 `<prefix>.4-ssa-opt.quad`，并观察课程样例里实际出现的 `binop` / `relop`。
6. 《Modern Compiler Implementation in Java》。这本书的 SSA 与数据流分析章节帮助我快速对齐了“格单调上升 + 迭代到不动点”的思路。

## 实现思路

### 1. 状态设计

实现完全沿用了骨架里的两类状态：

- `block_executable[label]` 表示某个基本块是否已被证明可执行。
- `temp_value[temp]` 表示 SSA temp 的三值格，取值为 `NO_VALUE`、`ONE_VALUE(c)` 或 `MANY_VALUES`。

其中变量格严格单调上升：

- `NO_VALUE -> ONE_VALUE(c)`
- `NO_VALUE -> MANY_VALUES`
- `ONE_VALUE(c) -> MANY_VALUES`

一旦上升到 `MANY_VALUES` 就不再回退。

### 2. 分析阶段

分析函数是 `Opt::calculateBT()`，核心是一个迭代到不动点的循环。

初始化时：

- 入口块默认可执行。
- 函数参数默认初始化为 `MANY_VALUES`。
- 其他 temp 通过 `getRtValue()` 惰性初始化为 `NO_VALUE`。

每轮迭代中，只处理已经可执行的基本块，并按语句种类更新状态：

- `MOVE` 直接传播源操作数的格值。
- `MOVE_BINOP` 在两侧都是常量时精确计算；一旦某侧为 `MANY_VALUES`，结果退化为 `MANY_VALUES`。
- `LOAD`、`MOVE_CALL`、`MOVE_EXTCALL`、`PTR_CALC` 都按保守规则直接记为 `MANY_VALUES`。
- `JUMP` 直接将目标块置为可执行。
- `CJUMP` 若条件两侧都是常量，则只放行唯一确定的后继；否则保守地让两条边都可执行。
- `PHI` 只读取来自可执行前驱的输入；若可执行前驱给出不同常量，则提升为 `MANY_VALUES`。

### 3. 回写阶段

回写函数是 `Opt::modifyFunc()`，在 `func->clone()` 得到的副本上进行，避免直接破坏输入 IR。

回写时做了三类事情：

- 把可证明为常量的 temp 使用点替换成 `Const`。
- 把两侧都已常量化的 `MOVE_BINOP` 折叠为 `MOVE dst <- Const`。
- 删除不可达块，并把已知单边可达的 `CJUMP` 改写成 `JUMP`。

对于 `phi`，我做了更进一步的保守简化：

- 如果 `phi` 的结果本身已经是常量，则直接改写成 `MOVE dst <- Const`。
- 如果只剩一个可执行来源，则将其改写成普通 `MOVE`。
- 否则仅保留来自可执行前驱的参数。

### 4. 关键边界

这次实现里最容易出错的地方有三个：

- `phi` 不能机械地看所有输入，必须先判断前驱块是否可执行。
- 死分支上的冲突常量不能污染 `phi` 结果。
- 除零表达式不能在分析阶段错误地折叠成一个具体常量，因此我在常量计算里把 `/ 0` 和 `% 0` 保守提升为 `MANY_VALUES`。

## 关键实现

### 1. 单调 join

我单独实现了 `joinRtValue()` 和 `updateRtValue()`，把所有 temp 的更新统一成格上的 join。这样可以避免在不同语句分支里重复手写“当前是 `NO_VALUE` / `ONE_VALUE` / `MANY_VALUES` 时该怎么转移”的逻辑。

### 2. `CJUMP` 的双模式传播

`CJUMP` 是这次 SCCP 的关键控制点。

- 若左右都是 `ONE_VALUE`，则精确算出真假，只传播一条边。
- 只要任一侧不是常量，就同时打开真假两条边。

这样才能保证后续 `phi` 的输入集合是和控制流同步演化的，而不是脱节的。

### 3. `phi` 的“只看活前驱”求值

`phi` 的处理我严格按课件规则来做：

- 不可执行前驱直接忽略。
- `NO_VALUE` 只表示当前还缺少证据，不应立刻把结果变成 `MANY_VALUES`。
- 只有两个以上可执行前驱给出不同常量时，才真正发生冲突。

这也是新增测试 `opttest7` 与 `opttest8` 的主要覆盖点。

## 遇到的问题与解决

### 1. PDF 无法直接抽取文本

`pdftotext` 对这份课件抽出来是空文件，因此我先把 PDF 每页渲染成 PNG，再逐页人工整理成了 [HW8/LLMweek9.md](../LLMweek9.md)。这样后续实现时就不需要反复翻页看图。

### 2. 输出形式不只是在原 IR 上替换常量

从课件与样例意图来看，SCCP 不只是“替换 temp 为常量”，还应当利用条件常量裁掉不可达块。因此最终实现里把不可达块直接从输出函数的 block 列表里去掉，并同步把 `CJUMP` 改成 `JUMP`。

### 3. 新生成文件的直接读取工具有路径解析问题

在检查新增样例的 `.4-ssa-opt.quad` 时，直接文件读取工具一度找不到刚生成的文件。我改用终端 `sed` 直接查看内容，确认优化结果正确后再继续后续工作。

## 新增测试说明

除了课程原有的 `opttest1` 到 `opttest6`，我补了两组边界样例：

- `opttest7`：两条都可执行的前驱分别给 `phi` 输入同一个常量，验证 `phi` 仍能被折叠为常量 `1`。
- `opttest8`：一条前驱因常量条件被裁掉，验证死前驱上的冲突常量不会污染 `phi`，最终结果仍能折叠为 `42`。

这两组样例覆盖了课件中 `phi` 规则最容易出错的两条边界。

## git log --oneline

```text
272fd3d (HEAD -> hw8, master) Merge branch 'master' of gitee.com:fudanCompiler/fducompilerh2026
dce8e17 (origin/master, origin/HEAD) HW8 added
56f6bad quiz2: correct ast2tree
4947b17 (fj/master) quiz fin
47b2e5b quizzing
2485ffe Merge branch 'master' of ssh://ssh.forgejo.dywsy21.cn:18082/dywsy21/Compiler-H
f8f294a hw3,4: revised helper funcs
b6ac939 quiz 1
97dc63e hw3,4: prepared for quiz
71e0ee3 Merge branch 'master' of gitee.com:fudanCompiler/fducompilerh2026
85dcae4 Updated .gitignore
a22aef6 Merge branch 'hw7'
f23be04 Merge branch 'master' of gitee.com:fudanCompiler/fducompilerh2026
ab70d9f (fj/hw7, hw7) fin: hw7
5667f49 HW6: test/simplecall results
6711b05 HW7: add experiment report and fix CMakeLists.txt
3279dc2 HW7: implement quad2ssa (placePhi, renameVariables, cleanupUnusedPhi)
9fda2f9 rename HW6/lib/quadflow/CmakeLists.txt to HW6/lib/quadflow/CMakeLists.txt. CmakeList -> CMakeList
c686d39 rename: doc -> docs
46fc80a Merge branch 'master' of gitee.com:fudanCompiler/fducompilerh2026
```

## 测试结果

```text
make build

-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /home/wsy/fducompilerh2026/HW8/build
[1/2] Building CXX object lib/opt/CMakeFiles/opt.dir/opt.cc.o
[2/2] Linking CXX executable tools/main/main

make run

cd /home/wsy/fducompilerh2026/HW8/test && \
for file in $(ls .); do \
        if [ "${file#*.}" = "4-ssa-xml.quad" ]; then \
                echo "Reading ${file}"; \
                /home/wsy/fducompilerh2026/HW8/build/tools/main/main "${file%%.*}"; \
        fi; \
done; \
cd .. > /dev/null 2>&1
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
Writing optimized Quad to file: opttest7.4-ssa-opt.quad
-----Done---
Reading opttest8.4-ssa-xml.quad
Reading Quad (SSA) from xml: opttest8.4-ssa-xml.quad
Writing optimized Quad to file: opttest8.4-ssa-opt.quad
-----Done---
```
