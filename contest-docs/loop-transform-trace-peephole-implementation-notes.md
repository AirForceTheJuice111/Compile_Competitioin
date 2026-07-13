# Loop Simplify、Loop Unroll、Trace Block 与 Peephole 实现说明

本文记录 Compiler-H 在完成 AArch64-only 迁移后，四项循环与后端局部优化的当前实现状态：

- Loop Simplify（循环结构规范化）；
- Loop Unroll（受限完全展开）；
- Trace Block（AArch64 基本块布局）；
- Peephole（AArch64 文本级窥孔优化）。

本文描述的是截至 2026 年 7 月 13 日合并后的实际代码状态。四项优化均已接入主编译器，但当前仍属于实验优化，尚未默认进入比赛 `-O1` 流水线。

## 状态摘要

| 优化项 | 实现状态 | 当前开关状态 | 已确认效果 | 当前主要边界 |
|---|---|---|---|---|
| Loop Simplify | 已实现，带 CFG/SSA 校验和失败回退 | 实验开关 | 专项用例可创建 preheader、latch 和专用 exit | 只处理可规约自然循环，历史性能测试出现回退 |
| Loop Unroll | 已实现受限完全展开，带精确 trip count 与溢出检查 | 实验开关 | 专项用例成功展开循环并重建 SSA | 仅支持小型、常量次数、两块规范循环，公开性能集命中率低 |
| Trace Block | 已实现确定性的静态 trace 布局 | 实验开关 | 可减少到下一基本块的跳转，单例静态 A/B 为正向 | 没有真实分支概率和 PGO，尚缺完整平台级逐项数据 |
| Peephole | 已实现保守文本规则，危险规则已移除 | 实验开关 | 部分历史程序获得收益 | 收益不稳定，部分程序明显回退，当前规则命中范围较窄 |

当前结论不是“四项优化无效”，而是：

1. 四项实现均存在并能被主编译器执行；
2. 正确性设计采用保守识别、结构校验和失败回退；
3. 专项用例与静态 A/B 已证明部分优化确实改变代码；
4. 公开性能程序上的收益尚不稳定，因此暂不适合作为默认比赛配置整体启用。

## 共享 CFG 与 SSA 变换基础设施

Loop Simplify 和 Loop Unroll 共用以下实现：

- `include/opt/cfg_transform.hh`
- `lib/opt/cfg_transform.cc`
- `include/opt/quad_metadata.hh`
- `lib/opt/quad_metadata.cc`

### 自然循环识别

`findNaturalLoopInfo` 从控制流图和支配关系中识别回边，并将共享同一 header 的回边合并成一个自然循环描述：

```text
NaturalLoopInfo
├── header
├── blocks
├── outsidePredecessors
├── backedgePredecessors
└── exitEdges
```

只有当目标 header 支配回边源块时，该边才被视为自然循环回边。自循环会被单独处理，避免错误地将循环入口侧前驱吸收到循环体中。

### CFG 边与 PHI 同步维护

`retargetQuadEdge` 同时修改：

- 块末尾真实 `JUMP/CJUMP` 目标；
- `QuadBlock::exit_labels` 中缓存的后继标签。

PHI 前驱标签由 `replacePhiPredecessor` 单独维护。这样可以避免只修改 terminator、却留下过期 CFG 缓存或 PHI 标签的情况。

### 结构验证

`verifyQuadSsaCfg` 在提交变换前后检查：

- 基本块标签存在且不重复；
- 块首语句与入口标签一致；
- PHI 必须位于普通语句之前；
- terminator 必须是基本块最后一条语句；
- `RETURN` 块不能残留后继；
- 跳转目标必须存在；
- terminator 目标与 `exit_labels` 一致；
- PHI 前驱集合与 CFG 前驱集合一致；
- SSA 定义支配其使用。

### 事务式变换与失败回退

两项循环变换都先克隆函数，在克隆体上执行变换和验证。出现以下情况时保留原函数：

- 输入 CFG/SSA 已经不合法；
- 循环结构不在支持范围内；
- PHI 无法安全重写；
- 变换后的 CFG/SSA 验证失败。

这使实验 pass 的失败不会把半完成的 CFG 写回主程序，也是当前实现保持可扩展性和可调试性的核心设计。

## Loop Simplify

实现位置：

- `include/opt/loopsimplify.hh`
- `lib/opt/loopsimplify.cc`

### 优化目标

Loop Simplify 将可规约 SSA 循环逐步规范为更适合 LICM、归纳变量分析和循环展开的形态：

1. 唯一、专用的 preheader；
2. 唯一、专用的 latch；
3. 与循环外前驱隔离的专用 exit edge。

### Preheader 规范化

当循环 header 存在多个循环外前驱，或者唯一外部前驱还有其他后继时，pass 新建 preheader，并把所有循环外进入边汇入该块。

如果 header PHI 的多个外部输入值相同，直接复用该值；如果不同，则在新 preheader 中建立 join PHI，再让 header PHI 只接收一个来自 preheader 的值。

### Latch 规范化

当循环存在多个回边前驱，或者回边前驱不是 header 的专用前驱时，pass 新建 latch，将回边统一汇入该块。

PHI 合并策略与 preheader 相同，从而在不改变 SSA 值语义的前提下形成单一回边入口。

### Exit edge 拆分

如果循环退出目标同时还有循环外前驱，pass 会拆分循环内到该目标的退出边，并同步更新目标块 PHI 前驱标签。

当前实现完成的是专用退出边规范化，不等同于完整 LCSSA；循环外使用值的 LCSSA 构造仍属于后续工作。

### 执行策略与统计

每个函数最多迭代 64 次，每次只完成一个结构变换，然后重新计算 CFG 和支配信息。可观测统计包括：

- `functionsVisited`；
- `functionsChanged`；
- `functionsSkipped`；
- `preheadersCreated`；
- `latchesCreated`；
- `exitEdgesSplit`。

例如：

```bash
BACKEND_PROFILE=1 \
SYSY_EXPERIMENTAL_PASSES='loopsimplify' \
build/compiler test/optimization/loop_simplify_continue.sy \
  -S -O1 --no-parallel-native -o /tmp/loopsimplify.s
```

专项用例应至少出现一次 `latches > 0` 或其他结构修改。

### 当前边界

- 只处理由支配关系识别出的可规约自然循环；
- 不负责完整 LCSSA；
- 不主动进行循环旋转、unswitch 或 interchange；
- 每次 CFG 修改后重新分析，优先保证正确性而非编译时间；
- 历史控制实验中，`2025-EB0-36` 的中位时间从 2.39 秒变为 2.71 秒，约回退 13.4%；
- 当次 60 个性能程序中只有 6 个触发 Loop Simplify，Loop Unroll 没有触发，因此目前保持 opt-in。

## Loop Unroll

实现位置：

- `include/opt/loopunroll.hh`
- `lib/opt/loopunroll.cc`

### 优化目标

当前实现只做小型循环的完全展开。它通过删除循环控制分支、暴露跨迭代常量传播机会，为 SCCP、AlgebraSimp、GVN 和 CopyProp 提供更直的 IR。

### 支持的循环形态

候选循环必须满足：

- 已形成唯一 preheader、单一 header、单一 body/backedge 和单一 exit；
- 循环由两个块组成：header 与 body；
- header 以 `CJUMP` 结束；
- body 无条件跳回 header；
- header PHI 恰好具有 preheader 与 body 两个输入；
- 整数归纳变量初值、边界和步长都能解析为常量；
- 步长非零，比较关系可精确计算；
- 模拟迭代过程中不发生 32 位有符号范围溢出；
- body 中不包含 PHI、分支、return、普通调用或外部调用。

当前比较关系支持：

```text
<  <=  >  >=  ==  !=
```

只要常量模拟能够证明循环退出，就可以支持正向、倒序和非单位步长循环。

### 规模限制

默认限制为：

| 参数 | 默认值 |
|---|---:|
| 最大精确迭代次数 | 8 |
| 最大 body 指令数 | 24 |
| 最大展开后指令数 | 96 |
| 最大新增 SSA temp 数 | 64 |

少于 2 次迭代不会展开；超过限制的候选直接保留原循环。例如 `loop_unroll_guard.sy` 的 12 次循环用于验证展开上限能够阻止代码膨胀。

### SSA 重写过程

完全展开时，pass：

1. 将 preheader 的边改到新展开块；
2. 为每次迭代的定义分配新 SSA temp；
3. 将本轮使用重写到 PHI 当前值或本轮新定义；
4. 计算下一轮 PHI 对应值；
5. 生成直接跳转到 exit 的展开块；
6. 更新 exit PHI 前驱；
7. 将原 header/body 的循环外使用改写到最后一轮值；
8. 删除原 header 和 body；
9. 清理无用纯定义并重新验证 CFG/SSA。

主流水线在实际发生展开后，还会追加 SCCP 和 AlgebraSimp 清理，以折叠展开暴露出的常量表达式。

### 已确认命中

`loop_unroll_exact.sy` 的 profile 结果为：

```text
BACKEND_PROFILE loopunroll functions=1 loops=1 cloned=12
BACKEND_PROFILE loopunroll-cleanup 0ms
```

这证明 Loop Unroll 已实际展开一个循环并克隆 12 条 IR，而不仅是被注册到流水线。

### 当前边界

- 没有部分展开、运行时展开、remainder loop 或 peeling；
- 不处理多块循环体、循环内条件分支和调用；
- 不基于真实 Cortex-A53 成本模型选择展开因子；
- 完全依赖常量精确 trip count；
- 历史 60 个性能程序的测量中没有触发展开，当前比赛收益尚未建立。

## Trace Block

实现位置：

- `include/backend/aarch64_block_layout.hh`
- `lib/backend/aarch64_block_layout.cc`
- `lib/backend/backend_driver.cc` 中的 `Aarch64StackEmitter`

### 优化目标

Trace Block 只改变 AArch64 基本块的物理输出顺序，不修改 Quad CFG 和 SSA 图。目标是让更可能连续执行的块相邻，从而：

- 形成 fall-through；
- 删除跳向下一标签的无条件分支；
- 必要时翻转条件分支方向；
- 减少跳转数量并改善前端取指布局。

### 静态布局策略

布局从函数入口块开始构造 trace。选择未放置后继时按以下顺序排序：

1. 优先循环嵌套深度更大的块；
2. 优先仍有后继的非 return 块；
3. 最后按原始块顺序保证确定性。

入口 trace 完成后，再从未放置块中选择最优起点，直到所有基本块均被放置。如果分析失败或结果不完整，则回退到原始顺序。

### 与 PHI lowering 的关系

AArch64 emitter 会根据下一个物理块判断 true/false 分支能否 fall-through。需要改变分支方向时，它仍通过生成 edge label 和执行对应 PHI copy 保持边语义，不直接篡改 SSA CFG。

### 当前静态观察

一次 WSL 单例 A/B 中，四项优化整体产生了以下变化：

| 指标 | 基线 | 四项开启 | 变化 |
|---|---:|---:|---:|
| 静态指令数 | 1010 | 988 | -22，约 -2.18% |
| 静态分支数 | 67 | 45 | -22，约 -32.84% |
| 汇编文本字节数 | 17290 | 16914 | 约 -2.17% |
| llvm-mca Total Cycles | 3263 | 3234 | 约 -0.89% |
| llvm-mca Total uOps | 1029 | 1007 | 约 -2.14% |
| llvm-mca Block RThroughput | 514.5 | 503.5 | 约改善 2.14% |

静态指令和静态分支恰好都减少 22 条，说明该单例收益很可能主要来自分支布局，但仍需使用 `traceblock` 单独 A/B 完成归因。

上述 llvm-mca 输入包含 call 和 return；工具会给 call 假定 100 周期，并忽略真实程序计数器路径。因此该结果只能视为正向静态信号，不能当作 Cortex-A53 实际运行提速 2.14% 的证明。

### 当前边界

- 使用循环深度启发式，不使用真实分支概率；
- 没有 PGO、热冷分区或代码对齐成本模型；
- 静态分支减少不等于动态分支次数同比下降；
- 尚未获得 60 个公开性能程序和官方平台的独立 Trace Block 汇总数据。

## AArch64 Peephole

实现位置：

- `include/backend/aarch64_peephole.hh`
- `lib/backend/aarch64_peephole.cc`

### 当前保留规则

Peephole 将汇编文本解析为指令、标签、伪指令和其他行，最多迭代 4 轮。目前仅保留以下可局部证明安全的规则：

1. 删除 `mov reg, reg`；
2. 将 `add/sub dst, src, #0` 删除或改写为 `mov dst, src`；
3. 删除紧邻目标标签的 `b label`。

可观测统计包括：

- `redundantMoves`；
- `zeroArithmetic`；
- `branchesToNextLabel`。

`extendedAdds` 和 `zeroCompares` 是兼容旧 profile 的保留字段；相应文本规则已经移除，当前应保持为零。

### 已移除的危险规则

安全合并阶段移除了：

- 基于相邻 `sxtw` 与 `add` 的文本合并；
- 假设 `w9`—`w17` 临时寄存器活跃性、删除零常量构造的 compare 改写。

这些模式仅凭相邻文本无法证明寄存器生命周期，可能错误删除仍有后续用途的定义。当前安全实现把已知生命周期的地址加法和与零比较直接放到指令选择阶段生成，而不是依赖文本窥孔猜测。

### 历史性能结果

历史重复测量显示 Peephole 收益不稳定：

| 程序 | 中位效果 |
|---|---:|
| `2025-B3Z-8` | 快 7.77% |
| `2025-EB0-36` | 快 2.58% |
| `2025-D6H-55` | 快 3.71% |
| `2025-680-52` | 慢 4.63% |
| `2025-MYO-20` | 慢 5.66% |
| `2025-O30-49` | 噪声较大，中位约快 0.7% |
| `2025-4W1-32` | 低于计时分辨率 |

因此 Peephole 虽然在部分程序上有明显收益，但不能按“只要减少指令就一定提速”的假设默认开启。

### 当前边界

- 文本解析器不是完整 AArch64 汇编语义模型；
- 没有寄存器活跃性、别名和机器数据流信息；
- 当前规则较少，部分程序 profile 会显示 `removed_or_combined=0`；
- profile 为零表示本例未命中，不表示 pass 没有运行；
- 更复杂的机器级合并应优先放到指令选择或寄存器分配后的结构化 Machine IR 中。

## 主流水线接入与开关

四项优化在 `lib/backend/backend_driver.cc` 中接入。

当前默认稳定 pass 白名单不包含这四项。必须显式设置：

```bash
export FOUR_PASSES='loopsimplify,loopunroll,traceblock,peephole'

SYSY_EXPERIMENTAL_PASSES="$FOUR_PASSES" \
build/compiler testcase.sy -S -O1 -o /tmp/testcase.s
```

关闭并行化、隔离四项优化时使用：

```bash
SYSY_EXPERIMENTAL_PASSES="$FOUR_PASSES" \
build/compiler testcase.sy -S -O1 --no-parallel-native \
  -o /tmp/testcase.s
```

`SYSY_DISABLE_PASSES` 的优先级更高，可用于从实验组合中单独关闭某项。

### 流水线顺序

与四项优化相关的顺序为：

```text
前置标量优化
  → Loop Simplify
  → LICM
  → Loop Unroll
  → SCCP/AlgebraSimp 展开后清理
  → 归纳变量优化
  → GVN/CopyProp
  → AArch64 指令选择与 Trace Block 布局
  → AArch64 文本 Peephole
```

Loop Simplify 位于 Loop Unroll 之前，用于提供规范结构；Trace Block 和 Peephole 位于后端，分别负责块顺序和最终文本局部规则。

## 回归测试

### 专项用例

| 用例 | 主要验证目标 |
|---|---|
| `test/optimization/loop_simplify_continue.sy` | continue 相关回边与 latch 规范化 |
| `test/optimization/loop_unroll_exact.sy` | 4 次正向循环完全展开 |
| `test/optimization/loop_unroll_descending.sy` | 常量倒序循环展开 |
| `test/optimization/loop_unroll_guard.sy` | 12 次循环超过阈值，不应展开 |
| `test/optimization/trace_layout_branch.sy` | 条件块布局与 fall-through 正确性 |

专项动态测试：

```bash
export FOUR_PASSES='loopsimplify,loopunroll,traceblock,peephole'

for case in test/optimization/*.sy
do
  SYSY_EXPERIMENTAL_PASSES="$FOUR_PASSES" \
  SYSY_OPT='-O1 --no-parallel-native' \
  make run-one "$case"
done
```

### 全量正确性

解析与语义：

```bash
make sysy-parse-regression SYSY_TEST_ROOT="$PWD/test"
make sysy-semantic-regression SYSY_TEST_ROOT="$PWD/test"
```

默认 O1 基线：

```bash
env -u SYSY_EXPERIMENTAL_PASSES \
make sysy-functional-regression \
  SYSY_TEST_ROOT="$PWD/test" \
  SYSY_OPT='-O1 --no-parallel-native'
```

四项优化开启：

```bash
SYSY_EXPERIMENTAL_PASSES="$FOUR_PASSES" \
make sysy-functional-regression \
  SYSY_TEST_ROOT="$PWD/test" \
  SYSY_OPT='-O1 --no-parallel-native'
```

与并行化组合：

```bash
SYSY_EXPERIMENTAL_PASSES="$FOUR_PASSES" \
make sysy-functional-regression \
  SYSY_TEST_ROOT="$PWD/test" \
  SYSY_OPT='-O1'
```

全部动态回归必须满足：

```text
compile_fail=0
link_fail=0
run_fail=0
wrong=0
```

### 当前测试证据

合并后的代码已完成以下检查：

- Windows Release 干净构建成功，44 个编译单元全部完成；
- 307 个带 `.out` 的正向用例在 O0、默认 O1 和显式开启四项优化的配置下均成功生成 AArch64 汇编；
- WSL profile 已确认 Loop Unroll 在专项用例中实际命中并完成清理；
- 当前仍应以 WSL/QEMU 的 307 个动态输出回归和 60 个公开性能程序动态回归作为合入默认流水线前的强制验收项。

## 性能验证方法与判定标准

仓库当前的 `sysy-performance-regression` 负责性能用例的编译、链接、运行和输出比对，并不测量真实执行时间。

性能验证应分三层：

1. `BACKEND_PROFILE=1`：确认 pass 实际命中；
2. 汇编、真实 `.text` 大小与 `llvm-mca-18 -mcpu=cortex-a53`：筛查静态收益和代码膨胀；
3. 官方平台或真实 Cortex-A53：确认最终性能分数。

公平 A/B 必须使用同一个 Git SHA，并只改变 `SYSY_EXPERIMENTAL_PASSES`。推荐分别比较：

```text
默认 O1
仅 traceblock
仅 peephole
仅 loopsimplify
loopsimplify + loopunroll
四项全部开启
```

只有同时满足以下条件，某项优化才适合加入默认比赛流水线：

- 全量功能、语义和 reject 测试通过；
- 60 个 `test/performance` 程序输出全部正确；
- 与默认并行化组合后仍全部正确；
- profile 证明在公开性能集上实际命中；
- 官方平台总性能分数稳定提高；
- 没有不可接受的单项严重回退或代码体积膨胀。

WSL/QEMU 运行时间受 x86 到 AArch64 动态翻译、QEMU JIT、WSL2 和宿主调度影响，只能作为筛查信号，不能单独证明 Cortex-A53 比赛性能收益。

## 当前边界与后续路线

### Loop Simplify

- 增加完整 LCSSA；
- 避免对不需要后续循环变换的函数无收益规范化；
- 增加结构成本与后续 pass 需求判断；
- 对历史回退用例定位额外跳转、PHI 和寄存器压力来源。

### Loop Unroll

- 扩展到多块但可安全复制的循环体；
- 支持受控部分展开和 remainder loop；
- 使用 Cortex-A53 代码体积、分支和寄存器压力成本模型；
- 在 Loop Simplify/LCSSA 完善后扩大命中范围；
- 增加展开因子而不是只有完全展开。

### Trace Block

- 引入静态分支概率或 profile 信息；
- 增加热冷块分离和对齐策略；
- 统计实际删除、翻转的分支数量；
- 对 60 个公开性能程序完成独立 A/B，而不是只观察组合结果。

### Peephole

- 将规则迁移到结构化 Machine IR；
- 使用寄存器活跃性保证跨指令合并安全；
- 在寄存器分配后实现地址、移位和 load/store offset 合并；
- 删除已经永远为零的兼容统计字段，或恢复为结构化安全规则；
- 逐规则建立正确性与性能回归，避免多条规则共同开启后无法归因。

### 推荐启用顺序

考虑当前收益和风险，后续验证建议按以下顺序推进：

1. Trace Block 单独 A/B；
2. 当前保守 Peephole 单独 A/B；
3. Loop Simplify 单独定位回退；
4. Loop Simplify 与 Loop Unroll 联合扩大命中；
5. 四项与默认并行化、寄存器分配联合测试；
6. 只将真实平台上稳定获益的子集加入默认 `-O1`。

在完成上述验证前，保持四项优化通过 `SYSY_EXPERIMENTAL_PASSES` 显式启用，是当前最符合比赛正确性和可维护性要求的配置。
