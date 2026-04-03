---
title: "HW5 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW5 实验报告

本次作业实现了 Tree IR+ 到 Quad IR 的翻译，即编译器pipeline中的指令选择阶段。

## 参考材料

- 虎书第8章 Basic Blocks and Traces，介绍了如何用模式匹配将树形 IR 平坦化为三地址码。
- 课程 PPT 关于 Quad IR 设计的部分。

## 关键技术实现

### 整体架构

翻译的核心是 Tree2Quad 类，它继承 tree::Visitor，遍历 Tree IR+ 的每个节点，将其转换为 Quad 指令流。

翻译过程分为两层：

1. Stm节点的 visit 方法：将翻译出的 QuadStm* 追加到 visit_result 向量中。
2. Exp节点的 visit 方法：将翻译结果设置在 output_term（一个 QuadTerm*），同时可能向 visit_result 插入辅助指令（如中间 temp 赋值）。

### 入口函数 tree2quad

tree2quad(Program\*) 是翻译入口。它调用 visit(Program\*)，后者对每个 FuncDecl 做如下操作：

1. 为该函数创建新的 Temp_map，从函数的 last_temp_num+1 和 last_label_num+1 开始分配新编号。
2. 初始化 visit_result，遍历函数体，收集所有生成的 Quad 语句。
3. 调用 splitBlocks() 将平坦的语句流切分为基本块。
4. 组装 QuadFuncDecl。

### 指令选择（模式匹配）

翻译的核心在 visit(Move*) 中，根据 dst 和 src 的组合做模式匹配：

| 模式                       | 生成的 Quad 指令     |
| -------------------------- | -------------------- |
| Move(Mem, src)           | STORE              |
| Move(Temp, Call)         | MOVE_CALL          |
| Move(Temp, ExtCall)      | MOVE_EXTCALL       |
| Move(Temp, Mem)          | LOAD               |
| Move(Temp, Binop(PTR,+))| PTR_CALC           |
| Move(Temp, Binop)        | MOVE_BINOP         |
| Move(Temp, other)        | MOVE               |

对于表达式出现在非 Move 上下文（如 Return(Binop(...))）中时，visit(Binop*) 等方法会自动生成中间 temp，保证三地址码的约束。

### 基本块切分 splitBlocks

splitBlocks 将平坦语句流按以下规则切分为基本块：

- 遇到 LABEL 则关闭上一个块（fall-through 到当前 label），开启新块。
- 遇到 JUMP 则关闭当前块，exit 为跳转目标。
- 遇到 CJUMP 则关闭当前块，exit 为 true/false 两个标签。
- 遇到 RETURN 则关闭当前块，exit 为空。
- 若首条语句不是 LABEL，自动插入一个新标签。

### def/use 集合计算

每条 Quad 指令在创建时同步计算 def/use 集合：

- def：指令写入的 temp（如 MOVE t100 ← ... 的 def 为 {t100}；STORE 的 def 为空）。
- use：指令读取的 temp（从 QuadTerm 中提取，常量和名称不产生 use）。

通过辅助函数 addTermUse(set, term) 统一处理。

### 其他 visit 方法简述

- visit(Seq)：遍历子语句列表，每个递归 accept。
- visit(Jump)：直接生成 QuadJump。
- visit(Cjump)：visit 左右操作数得到 term，生成 QuadCJump。
- visit(Return)：visit 表达式得到 term，生成 QuadReturn。
- visit(ExpStm)：若内部是 Call/ExtCall，生成 QuadCall/QuadExtCall（忽略返回值）。
- visit(Const)：设 output_term = QuadTerm(int)。
- visit(Name)：设 output_term = QuadTerm(string)。
- visit(TempExp)：设 output_term = QuadTerm(QuadTemp)。
- visit(Eseq)：先 accept 语句部分，再 accept 表达式部分取 output\_term。
- visit(Binop)：若为 PTR+，生成 PTR_CALC；否则生成 MOVE_BINOP，结果放入新 temp。
- visit(Mem)：生成 LOAD，结果放入新 temp。
- visit(Call)：作表达式时生成 MOVE_CALL，结果放入新 temp。
- visit(ExtCall)：作表达式时生成 MOVE_EXTCALL，结果放入新 temp。

## 额外测试用例

在已有的 simplecall1 和 simplecall2 之外，又自行增加了 5 个测试用例，覆盖不同的 IR 结构：

| 测试文件 | 覆盖的关键结构 | 描述 |
| --- | --- | --- |
| extratest1 | MOVE, MOVE\_BINOP, EXTCALL, RETURN | 简单算术运算和返回 |
| extratest2 | LABEL, CJUMP, JUMP, MOVE\_BINOP | while 循环 |
| extratest3 | MOVE\_EXTCALL, PTR\_CALC, STORE, LOAD | 数组分配、存取 |
| extratest4 | CJUMP, JUMP, 多个基本块 | if-else 条件分支 |
| extratest5 | MOVE\_CALL, ESEQ, Name, 多函数 | 对象创建 + 方法调用 |

## Git 提交记录

```
ab2fa26 (HEAD -> hw5) test: extra tests for tree2quad
033c792 tree2quad fin
cf885be (master) Merge branch 'master' of gitee.com:fudanCompiler/fducompilerh2026
e9fef6d HW5 Added
```

## 测试结果

make run 输出如下：

```
Reading extratest1.3.irp
Reading IR (XML) from: extratest1.3.irp
Canonicalization...
Writing Canonicalized IR to extratest1.3-canon.irp
Done converting IR to Quad
Writing Quad to: extratest1.4.quad
Reading extratest2.3.irp
Reading IR (XML) from: extratest2.3.irp
Canonicalization...
Writing Canonicalized IR to extratest2.3-canon.irp
Done converting IR to Quad
Writing Quad to: extratest2.4.quad
Reading extratest3.3.irp
Reading IR (XML) from: extratest3.3.irp
Canonicalization...
Writing Canonicalized IR to extratest3.3-canon.irp
Done converting IR to Quad
Writing Quad to: extratest3.4.quad
Reading extratest4.3.irp
Reading IR (XML) from: extratest4.3.irp
Canonicalization...
Writing Canonicalized IR to extratest4.3-canon.irp
Done converting IR to Quad
Writing Quad to: extratest4.4.quad
Reading extratest5.3.irp
Reading IR (XML) from: extratest5.3.irp
Canonicalization...
Writing Canonicalized IR to extratest5.3-canon.irp
Done converting IR to Quad
Writing Quad to: extratest5.4.quad
Reading simplecall1.3.irp
Reading IR (XML) from: simplecall1.3.irp
Canonicalization...
Writing Canonicalized IR to simplecall1.3-canon.irp
Done converting IR to Quad
Writing Quad to: simplecall1.4.quad
Reading simplecall2.3.irp
Reading IR (XML) from: simplecall2.3.irp
Canonicalization...
Writing Canonicalized IR to simplecall2.3-canon.irp
Done converting IR to Quad
Writing Quad to: simplecall2.4.quad
```

测试均通过。示例fmj -> quad代码（simplecall1.fmj）：

```
# fmj
public int main() {
    class C c;
    c = new C();
    putint( c.m() );
    putch( 10 );
}

public class C {
    public int m() {
       return 100;
    }
}

# quad
Function __$main__^main() last_label=100 last_temp=109:
  Block: Entry Label: L100
    Exit labels: 
    LABEL L100; def: use: 
    MOVE t100:ptr <- Const:0; def: t100 use: 
    MOVE_EXTCALL t102:ptr <- malloc(Const:4); def: t102 use: 
    PTR_CALC t108:ptr <- t102:ptr + Const:0; def: t108 use: t102 
    STORE Name:C^m -> Mem(t108:ptr); def: use: t108 
    MOVE t100:ptr <- t102:ptr; def: t100 use: t102 
    PTR_CALC t109:ptr <- t100:ptr + Const:0; def: t109 use: t100 
    LOAD t104:ptr <- Mem(t109:ptr); def: t104 use: t109 
    MOVE_CALL t105:int <- m[t104:ptr] (t100:ptr); def: t105 use: t100 t104 
    EXTCALL putint(t105:int); def: use: t105 
    EXTCALL putch(Const:10); def: use: 
Function C^m(t100) last_label=100 last_temp=101:
  Block: Entry Label: L100
    Exit labels: 
    LABEL L100; def: use: 
    RETURN Const:100; def: use: 
```
