# 参考项目优化研究与可迁移性能方向

参考项目：<https://gitlab.eduxiji.net/educg-group-36291-2935673/T202510246206554-2456>

分析时间：2026-07-05。仓库以浅克隆方式只读分析，未复制其代码进入本项目。

## 1. 总体结构

参考项目同样采用虎书风格流水线：

```text
SysY source
  -> AST
  -> IR tree
  -> canonical IR
  -> Quad
  -> blocked Quad
  -> SSA Quad
  -> Quad optimizer
  -> AArch64 instruction selection
  -> peephole optimizer
  -> register allocation
  -> assembly
```

它的关键目录：

- `lib/frontend`：lexer/parser
- `lib/ast`：AST 与 AST 级常量传播
- `lib/ir`：AST 到 IR
- `lib/quad`、`lib/quadflow`：Quad、SSA、CFG、支配树、循环分析、Scalar Evolution
- `lib/optimizer`：主要优化 pass
- `lib/aarch64`、`lib/backend`：AArch64 后端与寄存器分配
- `vendor/libsysy`：SysY 运行时库
- `vendor/libmemoize`：memoize 运行时支持

## 2. 参考项目优化管线

参考项目 `tools/main.cpp` 中 `-O1` 管线大致如下：

1. `astConstPropagation`
2. `ast2tree_tailopt`
3. `UnifyFunctionExitNodesPass`
4. `LoopSimplifyPass`
5. `quad2ssa`
6. `constantPropagation`
7. `AlgebraSimpPass`
8. `GVNPass`
9. `PhiSimplifyPass`
10. `MemSSAPass`
11. `MemDCEPass`
12. `CopyPropagationPass`
13. 第二轮 `GVNPass`
14. `inlining`
15. `Global2TempPass`
16. `LoopInterchangePass`
17. `FunctionSpecializationPass`
18. `LoopUnrollPass`
19. 第二轮 copy/const/algebra/instcombine/GVN
20. `StaticInitDCEPass`
21. `MoveLoopInvariantPass`
22. 第二轮 `MemSSA` + `MemDCE`
23. `MemoizePass`
24. `TransMinMaxPass`
25. 第三/四轮 GVN、常量传播、trace block
26. `DeadCodeEliminatePass`
27. `InstrRearrangePass`
28. `CompileTimeEvaluationPass`
29. AArch64 peephole：`ConstPattern`、`NegAddPattern`、`AddLSLPattern`、`MemOffsetPattern`

这个顺序体现了一个重要经验：单个 pass 往往会创造另一个 pass 的机会，所以它反复穿插 `copy propagation`、`constant propagation`、`GVN` 和 `DCE`，而不是只跑一轮。

## 3. 已有 final 与参考项目的主要差距

当前 final 已有：

- SSA 变换
- 常量传播
- 循环不变量外提
- induction variable / strength reduction
- 基本 DCE/cleanup
- ARM 指令选择、调度、寄存器分配、部分后端修复

参考项目额外值得迁移的优化：

| 优化 | 参考项目位置 | 作用 | 对 final 的迁移价值 |
| --- | --- | --- | --- |
| GVN | `lib/optimizer/gvn.cc` | 基于支配树的全局值编号，消除重复计算 | 高。SysY 数组下标和循环中重复表达式很多。 |
| Copy Propagation | `copyPropagation.cpp` | 消除 SSA copy、简化 PHI/copy 链 | 高。可降低寄存器压力。 |
| Algebra Simplification / InstCombine | `algebraSimp.cpp`、`instCombine.cpp` | `x+0`、`x*1`、比较/算术合并 | 高。实现成本适中。 |
| Function Inlining | `inline.cpp` | 减少调用开销并暴露跨过程优化机会 | 高。SysY 小函数很多。需控制代码膨胀。 |
| Function Specialization | `functionSpecialization.cpp` | 按常量参数克隆函数 | 中高。性能测试常出现常量维度/参数。 |
| MemSSA | `memssa/memSSA.cpp` | 将内存访问纳入 SSA 分析 | 高但复杂。数组和全局变量优化的基础。 |
| MemDCE / Load Forwarding | `memssa/memdce.cpp` | 用支配路径上的 store/load 替换后续 load | 高。SysY 数组访问频繁。 |
| Global2Temp | `memssa/global2Temp.cpp` | 将仅局部使用的全局标量提升为 SSA temp | 高。竞赛样例常有全局变量。 |
| Loop Simplify | `loopSimplify.cpp` | 建 preheader、latch、规范化循环结构 | 高。当前 final 已有部分 loop header 逻辑，但 SysY 迁移后应更系统。 |
| Loop Interchange | `loopinterchange.cpp` | 调整嵌套循环顺序改善内存局部性 | 中高。多维数组行优先场景收益明显。 |
| Loop Unroll | `loopUnroll.cpp` | 全展开/部分展开，减少分支和归纳变量开销 | 高。性能样例常有小常数循环。 |
| Trace Block | `traceblock.cpp` | 重排基本块，减少跳转，改善 fall-through | 中。后端层收益稳定。 |
| Peephole | `aarch64/peepHoleOpt.cpp` | 常量构造、地址偏移、`add lsl` 等机器级合并 | 高。后端见效快。 |
| Compile-time Evaluation | `compileTimeEvaluation.cpp` | 编译期解释纯函数/常量计算 | 中。需注意不能针对测例投机。 |
| Memoize | `memoize.cpp` + `vendor/libmemoize` | 对纯函数结果缓存 | 中低。实现和合规风险较高，需要严格纯度分析。 |

## 4. 并行化实现情况

定向搜索结果：

- 未发现 `pthread`、`std::thread`、`OpenMP`、`omp`、`async`、`future` 等实际并行运行时或编译器并行化代码。
- 仓库 `todo` 中有“并行化”条目，但没有对应 pass 实现。
- 源码中的 `parallelCopies` 出现在 AArch64/SSA PHI 消解中，含义是“并行复制语义 sequentialize”，不是程序并行化。

因此，参考项目不能直接提供可搬运的并行化实现。它提供的是并行化前置基础：

- Loop Simplify：形成规范循环。
- LoopNestInfo / FunctionLoopAnalysis：识别循环嵌套和 trip count。
- Scalar Evolution：分析归纳变量、地址表达式和步长。
- Alias / MemSSA：分析内存访问是否相互依赖。

这些基础可以用于我们实现真正的 loop parallelization。

## 5. 可实现的并行化方向

竞赛 SysY 本身没有线程语法。若要做并行化，编译器只能在后端或运行时自动改写安全循环。建议分三档。

### 5.1 编译器自身并行编译

目的：降低编译时间，不改变程序运行性能。

可并行的粒度：

- 每个函数独立做 CFG、SSA、局部优化。
- 每个函数独立做指令选择和寄存器分配。
- 测试脚本并行编译多个 `.sy` 文件。

优点：安全、容易验证。缺点：评测通常主要看运行时间，编译时间权重若不高则收益有限。

### 5.2 自动循环并行化

目的：让生成程序在多核目标机上更快。

适用循环形态：

```c
for/while i from lb to ub step const:
    body uses only:
      - private scalar temporaries
      - read-only arrays/global values
      - writes to A[affine(i)] with no cross-iteration overlap
      - reductions: sum += expr, min/max, product, bitwise ops
```

SysY 没有 `for`，但 `while` 可规范化成 canonical loop：

```c
i = init;
while (i < limit) {
    body;
    i = i + step;
}
```

需要分析：

1. 识别单入口单回边循环。
2. 识别 basic induction variable 和 trip count。
3. 对每个 load/store 的地址做 affine/SCEV 分析。
4. 证明不同迭代写集合不重叠，或者识别 reduction。
5. 证明循环内无不可并行副作用：
   - 不能调用未知函数。
   - 不能调用 I/O 和计时函数。
   - 不能写未知别名指针。
6. 生成 worker 函数和运行时调度代码。

运行时方案：

- 编译器内置一个轻量线程池 runtime，例如 `__sysy_parallel_for(begin,end,step,fn,ctx)`。
- 将循环体 lift 成 worker 函数，捕获数组基址、标量参数、边界。
- 对 reduction 变量分配 per-thread partial，再在主线程合并。

风险：

- 需要链接 pthread 或平台线程库，必须确认比赛允许。
- 小循环并行开销可能大于收益，需要门槛判断。
- 浮点 reduction 改变结合顺序，可能改变结果；默认不应并行化 float reduction，除非评测容忍或有严格策略。

### 5.3 库函数/模式级并行化

比一般 loop parallelization 更稳的方向是识别固定模式：

- 大数组初始化：`for i: a[i] = c`
- 数组拷贝：`for i: a[i] = b[i]`
- map：`a[i] = f(b[i])`，`f` 无副作用
- int sum reduction：`sum = sum + a[i]`
- 矩阵乘/卷积的固定嵌套循环

这类模式可以先由 loop idiom pass 识别，再生成专门 runtime 调用或展开/分块代码。相较任意循环并行化，更容易做正确性证明。

## 6. 对当前 final 的性能优化路线

### 第一阶段：迁移 SysY 后立即可做

1. **大赛 CLI 与 AArch64/ARM ABI 清理**：保证链接官方 `libsysy`。
2. **AST 常量折叠与 ConstExp 求值**：SysY 语义本身要求。
3. **全局/局部数组布局优化**：行优先线性化，避免运行时长度 word。
4. **代数化简和 copy propagation**：实现简单且收益稳定。
5. **后端窥孔优化**：
   - 合并 `add tmp, base, #imm` + `ldr/str [tmp]`。
   - 用 shift/add 处理乘以 2 的幂或 `a + b << k`。
   - 消除冗余 `mov`、连续 load/store。

### 第二阶段：SSA/循环性能

1. **GVN + DCE 固定点循环**：在每轮结构性优化后清理。
2. **函数内联**：先内联小函数、单调用函数、叶函数。
3. **Loop Simplify**：为所有循环 pass 建 canonical preheader/latch。
4. **Loop Unroll**：
   - 常数小 trip count 全展开。
   - 中等循环部分展开 2/4/8 倍。
   - 控制代码膨胀阈值。
5. **Loop Interchange**：针对多维数组行优先，把 stride 更小的维度放到内层。

### 第三阶段：内存和跨过程

1. **MemSSA / alias analysis**：区分全局、栈数组、参数数组。
2. **Load forwarding**：支配路径上 store/load 替换。
3. **Global scalar promotion**：仅单函数使用、无逃逸的全局标量转 temp。
4. **Function specialization**：按常量参数克隆函数。
5. **Compile-time evaluation**：仅对可证明纯函数启用，避免违反比赛“反投机优化”要求。

### 第四阶段：并行化

1. 先实现编译器内部 per-function 并行，作为低风险工程优化。
2. 再实现 loop idiom 并行化，仅覆盖数组初始化、copy、int reduction。
3. 最后尝试一般 affine loop parallelization。

## 7. 合规注意事项

章程明确反对针对特定测例的投机性、针对性优化。所有性能优化应满足：

- 基于通用语义和程序分析，不根据文件名、哈希、输入规模硬编码。
- 可在报告中解释触发条件、正确性依据和保守跳过条件。
- 遇到无法证明无副作用/无别名/无循环依赖时不优化。
- 浮点优化避免违反顺序语义，尤其是重排、reduction 和 FMA 引入的舍入差异。

## 8. 推荐优先级

最高优先级：

1. SysY 前端和类型系统正确性。
2. 官方运行时库 ABI。
3. GVN、copy propagation、DCE 固定点。
4. 函数内联。
5. 后端窥孔。

中期优先级：

1. MemSSA 与全局标量提升。
2. Loop unroll。
3. Loop interchange。
4. Function specialization。

探索优先级：

1. loop idiom parallelization。
2. memoize。
3. compile-time evaluation of pure functions。
4. 一般 affine loop parallelization。
