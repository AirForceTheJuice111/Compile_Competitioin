---
title: "HW7 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW7 实验报告

本次作业在已有控制流和数据流分析结果的基础上，将 Quad 中间表示转换为 SSA（静态单赋值）形式。主要实现了三个函数：`placePhi`（放置 PHI 函数）、`renameVariables`（变量重命名）和 `cleanupUnusedPhi`（清除无用 PHI）。

## 参考资料

1. 虎书（Modern Compiler Implementation in C）第 19 章：SSA 形式的构造算法，包括迭代支配边界和变量重命名。
2. Cytron 等人的论文"Efficiently Computing Static Single Assignment Form and the Control Dependence Graph"（1991），提出了经典的 SSA 构造算法。
3. 课程课件中关于 SSA 形式的内容。

## 关键技术实现

### PHI 函数放置 (`placePhi`)

#### 定义位置收集

由于 XML 解析器仅提供了活跃变量信息（`allVars`, `livein`, `liveout`），并未填充 `defs` 和 `uses` 映射，因此需要手动遍历函数体中所有语句的 `def` 集合，收集每个变量在哪些基本块中被定义：

```cpp
map<int, set<int>> defBlocksMap;
for (auto* block : *func->quadblocklist) {
    int blabel = block->entry_label->num;
    for (auto* stm : *block->quadlist) {
        if (!stm->def) continue;
        for (auto* t : *(stm->def))
            defBlocksMap[t->num].insert(blabel);
    }
}
```

同时从定义语句中提取变量类型（`QuadType`），用于后续创建 PHI 节点。

#### 剪枝 SSA（Pruned SSA）

使用迭代支配边界算法确定 PHI 放置位置，并结合活跃性分析进行剪枝——仅在变量在目标块入口处活跃（live-in）时才放置 PHI 函数：

```cpp
while (!worklist.empty()) {
    int X = worklist.front(); worklist.pop();
    for (int Y : domInfo->dominanceFrontiers[X]) {
        if (phiBlocks.count(Y)) continue;
        auto* labelStm = yBlock->quadlist->front();
        if ((*liveness->livein)[labelStm].count(v))
            phiBlocks.insert(Y);
    }
}
```

这一剪枝策略避免了为仅在单一块内定义且使用的变量放置不必要的 PHI 函数（例如循环体内的临时变量）。

#### PHI 节点创建

为每个需要 PHI 的块创建 PHI 节点：参数列表按前驱块标号排序（`set<int>` 自然有序），插入到块的 LABEL 语句之后。

### 变量重命名 (`renameVariables`)

#### 参数识别

函数参数（`func->params`）不参与版本化，保持原始编号不变。仅对函数体中通过语句定义的变量进行版本化。

#### 支配树遍历

采用经典的支配树前序遍历算法进行重命名：

1. 对每条非 PHI 语句，先重命名所有 **use**（从栈顶取当前版本），再重命名 **def**（分配新版本并压栈）。
2. 对 PHI 语句，仅重命名 **def**（参数由前驱块处理）。
3. 遍历 CFG 后继块的 PHI 参数，将对应前驱槽位更新为当前版本。
4. 递归处理支配树子节点。
5. 回溯时弹出本块中压入的版本。

版本编号采用 `VersionedTemp::versionedTempNum(origNum, version)`，即 `原编号 × 100 + 版本号`。

#### 独立 Temp 对象

由于 XML 解析器通过 `treeTempMap` 共享 `Temp` 对象（同一编号在不同函数中共用同一指针），重命名时必须创建新的 `Temp` 对象而非原地修改，否则会影响其他函数的正确性。重命名完成后重建所有语句的 `def/use` 集合。

### 清除无用 PHI (`cleanupUnusedPhi`)

迭代地检查并移除定义值未被任何语句使用的 PHI 节点。特别地，排除 PHI 的自引用——如果 PHI 定义的变量仅被该 PHI 自身使用，视为无用。

```cpp
while (changed) {
    changed = false;
    set<int> usedTemps;
    // 收集所有使用的 temp 编号（排除 PHI 自引用）
    // 移除 def 不在 usedTemps 中的 PHI
}
```

反复迭代直到不动点，因为移除一个 PHI 可能导致另一个 PHI 变为无用。

### 函数排序

`quad2ssa` 的输入是 `set<FuncFlowInfo*>`，迭代顺序依赖指针值。为保持与 XML 中函数声明顺序一致，按 `QuadFuncDecl*` 地址排序（因为解析器按 XML 顺序分配这些对象）。

## Git 提交记录

```
4bcd56e HW7: add experiment report and fix CMakeLists.txt
3279dc2 HW7: implement quad2ssa (placePhi, renameVariables, cleanupUnusedPhi)
```

## 测试结果

所有 9 个测试用例均成功运行（无崩溃/错误），SSA 转换结果（PHI 放置、变量版本号、清理）完全正确。

```
$ make run 2>&1 | grep "Done writing"
Done writing Quad-SSA (text) to: quadtest1.4-ssa.quad
Done writing Quad-SSA (text) to: quadtest2.4-ssa.quad
Done writing Quad-SSA (text) to: quadtest3.4-ssa.quad
Done writing Quad-SSA (text) to: quadtest4.4-ssa.quad
Done writing Quad-SSA (text) to: quadtest5.4-ssa.quad
Done writing Quad-SSA (text) to: quadtest6.4-ssa.quad
Done writing Quad-SSA (text) to: quadtest7.4-ssa.quad
Done writing Quad-SSA (text) to: quadtest8.4-ssa.quad
Done writing Quad-SSA (text) to: quadtest9.4-ssa.quad
```

与期望输出对比（`diff`）：

- **完全匹配**: quadtest3, quadtest4, quadtest5, quadtest9
- **仅 `def/use` 集合内顺序不同**: quadtest1, quadtest2, quadtest6, quadtest7, quadtest8

差异原因：`set<Temp*>` 使用指针值排序，而 `Temp` 对象的分配地址取决于内存分配器行为，不同系统/编译器可能产生不同的迭代顺序。SSA 转换的实际内容（PHI 位置、变量版本号、参数映射）完全一致。
