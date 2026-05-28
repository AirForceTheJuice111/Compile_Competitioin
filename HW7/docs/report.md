---
title: "Quiz3报告 + HW7 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# Quiz3 报告

## Quiz实现思路

**有一些测试用例中（如insttest3），预期输出没有无变量的函数的diag信息，我的输出有。读了读quadssa_diag.cc，是预期输出错了，无论如何函数都需打印一下。和老师确认过了这无妨。**

1. 定义全局变量 SsaDiagState diag;
2. quad2ssa 中，每进入一个函数，对diag进行初始化，clear掉所有东西，并把funcName设成funcdecl->funcname。
3. **在此处定义candidatePhiBlocks为不考虑live-in，递归插入phi时被插入的所有block**，actualPhiBlocks为以DF为起点，考虑live-in递归插入phi时被插入的所有block。找phiBlocks的时候顺便记录一下即可。

```cpp
        set<int> candidatePhiBlocks;
        while (!worklist.empty()) {
            int X = worklist.front(); worklist.pop();
            if (!domInfo->dominanceFrontiers.count(X)) continue;
            for (int Y : domInfo->dominanceFrontiers[X]) {
                if (phiBlocks.count(Y)) continue;
                // Pruned SSA: only place PHI if v is live-in at block Y
                auto* yBlock = domInfo->labelToBlock[Y];
                auto* labelStm = yBlock->quadlist->front();
                
                candidatePhiBlocks.insert(Y);
                
                if (liveness->livein->count(labelStm) && (*liveness->livein)[labelStm].count(v)) {
                    phiBlocks.insert(Y);
                    if (!processed.count(Y)) { // Inserting PHI means creation of new defs in Y, which may require more PHIs in Y's dominance frontier
                        processed.insert(Y);
                        worklist.push(Y);
                    }
                }
            }
        }
        
        diag.candidatePhiBlocksByVar[v] = candidatePhiBlocks;
        diag.actualPhiBlocksByVar[v] = phiBlocks;
```


4. 至于createdVersionBlocks，因为我有一个函数versionDef专门处理一个变量被重复定义时增加版本的问题，所以只需在这个函数中加一行代码即可：diag.createdVersionBlocksByVar[num][ver].insert(currentBlock); （currentBlock也为全局变量，懒得传参了）
5. 关于eliminatedVersionBlocksByVar，我想应该是cleanupUnusedPhi时顺便记录一下吧。在其中做了实现，虽然cleanupUnusedPhi也基本上永远不会被调用就是了。

```cpp
                        int vnum = phi->temp_exp->temp->num;
                        int origVar = VersionedTemp::origTempNum(vnum);
                        int ver = vnum - origVar * 100;
                        diag.eliminatedVersionBlocksByVar[origVar][ver].insert(block->entry_label->num);
```

6. 最后，在quad2ssa的每个func最后加入printSsaDiagSummary(funcdecl, diag); 即实现完毕。

## 报告要求中的问题

我们看到quadtest6.4-block.quad:

```
Function __$main__^main() last_label=108 last_temp=123:
  Block: Entry Label: L108
    Exit labels: L102 
    LABEL L108; def: use: 
    MOVE_EXTCALL t100:ptr <- malloc(Const:24); def: t100 use: 
    STORE Const:5 -> Mem(t100:ptr); def: use: t100 
    PTR_CALC t113:ptr <- t100:ptr + Const:4; def: t113 use: t100 
    STORE Const:1 -> Mem(t113:ptr); def: use: t113 
    PTR_CALC t114:ptr <- t100:ptr + Const:8; def: t114 use: t100 
    STORE Const:2 -> Mem(t114:ptr); def: use: t114 
    PTR_CALC t115:ptr <- t100:ptr + Const:12; def: t115 use: t100 
    STORE Const:3 -> Mem(t115:ptr); def: use: t115 
    PTR_CALC t116:ptr <- t100:ptr + Const:16; def: t116 use: t100 
    STORE Const:4 -> Mem(t116:ptr); def: use: t116 
    PTR_CALC t117:ptr <- t100:ptr + Const:20; def: t117 use: t100 
    STORE Const:5 -> Mem(t117:ptr); def: use: t117 
    MOVE t101:int <- Const:0; def: t101 use: 
    JUMP L102; def: use: 
  Block: Entry Label: L102
    Exit labels: L103 L104 
    LABEL L102; def: use: 
    MOVE t106:int <- t101:int; def: t106 use: t101 
    LOAD t103:int <- Mem(t100:ptr); def: t103 use: t100 
    CJUMP < t106:int t103:int? L103 : L104; def: use: t106 t103 
  Block: Entry Label: L103
    Exit labels: L106 L105 
    LABEL L103; def: use: 
    MOVE t108:ptr <- t100:ptr; def: t108 use: t100 
    LOAD t104:int <- Mem(t100:ptr); def: t104 use: t100 
    CJUMP >= t101:int Const:0? L106 : L105; def: use: t101 
  Block: Entry Label: L106
    Exit labels: L105 L107 
    LABEL L106; def: use: 
    CJUMP >= t101:int t104:int? L105 : L107; def: use: t101 t104 
  Block: Entry Label: L105
    Exit labels: 
    LABEL L105; def: use: 
    EXTCALL exit(Const:-1); def: use: 
  Block: Entry Label: L107
    Exit labels: L102 
    LABEL L107; def: use: 
    MOVE_BINOP t119:int <- (+, t101:int, Const:1); def: t119 use: t101 
    MOVE_BINOP t120:int <- (*, t119:int, Const:4); def: t120 use: t119 
    PTR_CALC t121:ptr <- t108:ptr + t120:int; def: t121 use: t108 t120 
    LOAD t109:int <- Mem(t121:ptr); def: t109 use: t121 
    EXTCALL putint(t109:int); def: use: t109 
    EXTCALL putch(Const:10); def: use: 
    MOVE_BINOP t101:int <- (+, t101:int, Const:1); def: t101 use: t101 
    JUMP L102; def: use: 
  Block: Entry Label: L104
    Exit labels: 
    LABEL L104; def: use: 
    RETURN Const:1; def: use: 
```

其中 t103出现了：SSA_TEMP_DIAG temp=t103 candidate_phi=[L102]；有candidate却无actual。

这是因为L107到L102有一条backedge，使得L102为其自身的dominance frontier，因此candidate中有L102。

然而，L107或L108到L102的边上t103都没有live-in；事实上，t103仅在L102中被load定义了一次，不存在因控制路径不同而值不同的情况，因此不需要实际增加phi。

# HW7 实验报告

本次作业在已有控制流和数据流分析结果的基础上，将 Quad 中间表示转换为 SSA（静态单赋值）形式。主要实现了三个函数：`placePhi`（放置 PHI 函数）、`renameVariables`（变量重命名）和 `cleanupUnusedPhi`（清除无用 PHI）。

## 参考资料

1. 虎书（Modern Compiler Implementation in C）第 19 章：SSA 形式的构造算法，包括迭代支配边界和变量重命名。
2. Cytron 等人的论文"Efficiently Computing Static Single Assignment Form and the Control Dependence Graph"（1991），提出了经典的 SSA 构造算法。
3. 课程课件中关于 SSA 形式的内容。

## PHI 函数放置 (`placePhi`)

总之就是把PHI函数放在定义了某个变量的节点的Dominance Frontier节点上。这些节点是定义节点第一个没有完全支配的节点，从其之前有其他的路径到达这些Dominance Frontier，因此作为分支节点，有且仅有它们需要被加上PHI。

### 定义位置收集

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

### 剪枝 SSA（Pruned SSA）

使用迭代Dominance Frontier算法确定 PHI 放置位置，并结合活跃性分析进行剪枝——仅在变量在目标块入口处活跃（live-in）时才放置 PHI 函数：

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

### PHI 节点创建

为每个需要 PHI 的块创建 PHI 节点：参数列表按前驱块标号排序（`set<int>` 自然有序），插入到块的 LABEL 语句之后。

## 变量重命名 (`renameVariables`)

### 参数识别

函数参数（`func->params`）不参与版本化，保持原始编号不变。仅对函数体中通过语句定义的变量进行版本化。

### 支配树遍历

采用经典的支配树前序遍历算法进行重命名：

1. 对每条非 PHI 语句，先重命名所有 **use**（从栈顶取当前版本），再重命名 **def**（分配新版本并压栈）。
2. 对 PHI 语句，仅重命名 **def**（参数由前驱块处理）。
3. 遍历 CFG 后继块的 PHI 参数，将对应前驱槽位更新为当前版本。
4. 递归处理支配树子节点。
5. 回溯时弹出本块中压入的版本。

版本编号采用 `VersionedTemp::versionedTempNum(origNum, version)`，即 `原编号 × 100 + 版本号`。

## 清除无用 PHI (`cleanupUnusedPhi`)

迭代地检查并移除定义值未被任何语句使用的 PHI 节点。特别地，排除 PHI 的自引用——如果 PHI 定义的变量仅被该 PHI 自身使用，视为无用。

```cpp
while (changed) {
    changed = false;
    set<int> usedTemps;
    // 收集所有使用的 temp 编号，汇总成 usedTemps
    // 移除 def 不在 usedTemps 中的 PHI（排除 PHI 自引用）
}
```

反复迭代直到不动点，因为移除一个 PHI 可能导致另一个 PHI 变为无用。

## 函数排序

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

与期望输出对比，完全匹配的有: quadtest3, quadtest4, quadtest5, quadtest9；仅 `def/use` 集合内顺序不同: quadtest1, quadtest2, quadtest6, quadtest7, quadtest8。

差异原因：`set<Temp*>` 使用指针值排序，而 `Temp` 对象的分配地址取决于内存分配器行为，不同系统/编译器可能产生不同的迭代顺序。SSA 转换的实际内容（PHI 位置、变量版本号、参数映射）完全一致。
