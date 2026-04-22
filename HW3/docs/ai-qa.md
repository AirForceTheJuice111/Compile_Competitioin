- Q1那里直接用auto cjump = new tree::Cjump(string("=="), right_exp, new tree::Const(0), exit, ok);为什么会导致right_exp求值两次？没理解

这里有一个关于 IR **树结构**的重要细节，跟 C++ 指针语义有关。

`right_exp` 是一个 `tree::Exp*` 指针，指向一个 IR 树节点。如果你写：

```cpp
auto cjump = new tree::Cjump("==", right_exp, new tree::Const(0), exit, ok);
// ...后续还要用 right_exp 做除法：
new tree::Binop("/", left_exp, right_exp)
```

你的 IR 树结构变成了这样（**DAG，而不是树**）：

```
CJump("==", ──► right_exp ◄──  Binop("/", left_exp, ...)
              [同一个对象]
```

**同一个 `tree::Exp*` 对象被两个父节点共享**。

后端（代码生成、线性化）遍历 IR 树时，**遇到哪个节点就为它生成一次代码**。它不知道这个节点被共享了，于是：

- 遍历 `CJump` 时 → 生成一次 `right_exp` 的代码
- 遍历 `Binop` 时 → 再生成一次 `right_exp` 的代码

**具体例子**：

```java
putint(getint() / getint());   // 假设除数是 getint()
```

若 `right_exp = ExtCall("getint", {})` 且没有物化，生成的汇编大致是：

```asm
call getint   ; CJump 用到，读了第1个输入
cmp result, 0
je exit
call getint   ; Binop 又用到，读了第2个输入！
div ...
```

实际上用了**两个不同的输入值**来做比较和除法，行为完全错误。

---

**物化之后**，IR 变成真正的树：

```
Move(t, right_exp)            ← right_exp 只在这里出现一次
CJump("==", TempExp(t), 0)    ← 读寄存器 t，幂等操作
Binop("/", left_exp, TempExp(t))  ← 再读寄存器 t，幂等操作
```

`TempExp(t)` 虽然也出现了两次，但读寄存器没有副作用，读多少次结果都一样。

---

**总结**：Tiger IR+ 的节点在逻辑上必须构成**树**（每个节点只有一个父节点）。用同一个指针放到两处就变成了 DAG，后端遍历时会为共享节点生成两份代码，对含副作用的表达式（`getint`、`ArrayExp` 的越界检查、`ExtCall` 等）产生错误。`TempExp`/`Const` 可以安全复用是因为读寄存器/常量没有副作用。






- 帮我理解Tr_ex, Tr_nx, Tr_cx，尤其是Tr_cx，我对cx的理解很不到位。以及，它们是不是只会在传递visit_exp_result的时候用到？

---

## 三种包装类的本质

Tiger IR 里，**表达式和条件跳转在语义上是不同的东西**，但翻译时上层并不知道子树会产生哪种形式。`Tr_Exp` 就是中间层，用来延迟这个判断。

### `Tr_ex` — 普通表达式

包着一个 `tree::Exp*`，有具体的值。

```
Tr_ex(Const(42))          // 整数 42
Tr_ex(TempExp(t100))      // 变量 a
Tr_ex(Mem(...))           // 内存读
```

### `Tr_nx` — 语句（无值）

包着一个 `tree::Stm*`，只有副作用，没有值。

```
Tr_nx(Move(t, Const(1)))  // 赋值语句
Tr_nx(Jump(label))        // 跳转
Tr_nx(Seq([...]))         // 语句块
```

### `Tr_cx` — 条件（最难理解）

**不**包含一个完整的跳转指令。它包含三件事：

```cpp
Patch_list *true_list;   // 一组"待填充"的标签指针，目标：条件为真时跳哪里
Patch_list *false_list;  // 同上，条件为假时跳哪里
tree::Stm  *stm;         // 已经生成好的 CJump 节点，但其 true/false 标签号暂时是占位符
```

**关键：CJump 已经被 new 出来了，但里面的标签编号还没有最终值。** `Patch_list` 里存的就是指向那些标签对象的指针，当上层决定好跳往哪个标签后，调用 `patch(label)` 把编号写进去。

---

## 为什么比较运算要返回 `Tr_cx`？

看 `BinaryOp` 中比较的翻译：

```cpp
auto tl = method_temp_map->newlabel();   // 占位标签 L_true
auto fl = method_temp_map->newlabel();   // 占位标签 L_false
auto cjump = new tree::Cjump(op, left_exp, right_exp, tl, fl);
// true_list 里存着 &tl，false_list 里存着 &fl
visit_exp_result = new Tr_cx(true_list, false_list, cjump);
```

此时生成了 `CJump(a < b, L_true, L_false)`，但 `L_true` 和 `L_false` 填的是什么？是**还没有被使用的标签号**。真正的跳转目标由上层（`If`/`While`）决定。

上层的 `visit(fdmj::If *)` 这样做：

```cpp
auto cx = visit_exp_result->unCx(method_temp_map);  // 拿到 Tr_cx

auto true_label = method_temp_map->newlabel();   // if-then 入口
auto false_label = method_temp_map->newlabel();  // if-else 入口

cx->true_list->patch(true_label);    // 把 CJump 的 true 目标填成 then 入口
cx->false_list->patch(false_label);  // 把 CJump 的 false 目标填成 else 入口
```

`patch` 的实现是：

```cpp
void patch(tree::Label *label) {
    for (auto l : *patch_list)
        l->num = label->num;   // 原地改写占位标签的编号
}
```

所以 **CJump 对象里的标签指针和 Patch_list 里的标签指针是同一个对象**，写进 `patch_list` 里的标签编号，CJump 里也跟着变了。这就是"回填（backpatch）"。

---

## `unEx`、`unCx`、`unNx` 是强制转换

| 调用 | 含义 |
|------|------|
| `unEx(tm)` | "我需要一个有值的表达式" |
| `unNx(tm)` | "我只需要副作用，不要值" |
| `unCx(tm)` | "我需要条件跳转形式" |

当类型不匹配时，系统会**合理地生成桥接代码**。看 tr_exp.cc 里的实现：

**`Tr_cx::unEx`**（把条件变成 0/1 整数）：
```cpp
// 生成: t = 0; CJump(..., L_true, L_false); L_true: t = 1; L_false:
// 最终 ESeq(seq, t) 返回 t 的值
```

**`Tr_ex::unCx`**（把普通表达式变成条件）：
```cpp
// 生成: CJump(exp != 0, L_true, L_false)
// 非零为真
```

**`Tr_cx::unNx`**（当条件用作语句，比如 `if (f()) {}`）：
```cpp
// true 和 false 都指向同一个出口标签，跳过去啥都不做
```

---

## 逻辑运算如何利用 Tr_cx

`&&` 的翻译展示了 Patch_list 的精髓：

```
a < b && c < d
```

```
left_cx:  CJump(a<b, L1, L2)    true_list=[L1], false_list=[L2]
right_cx: CJump(c<d, L3, L4)    true_list=[L3], false_list=[L4]

left_cx->true_list->patch(middle)   // L1 填成 middle（左真则看右）

结果 Tr_cx:
  stm = Seq([CJump(a<b,middle,L2), Label(middle), CJump(c<d,L3,L4)])
  true_list  = [L3]        // 整体为真 = 右边为真
  false_list = [L2, L4]    // 整体为假 = 左假 或 右假
```

上层 `If` 只需调用 `true_list.patch(then_label)` 和 `false_list.patch(else_label)`，就同时把 `L3→then`、`L2→else`、`L4→else` 全部填好了，**不需要知道有几个 CJump**。

---

## 回答你的问题

**它们是不是只在传递 `visit_exp_result` 时用到？**

是的，`Tr_ex/Tr_nx/Tr_cx` 是 visitor 之间传递翻译结果的**临时容器**，生命周期就在一次 `visit` 到下一次 `unEx/unNx/unCx` 调用之间。调用完 `unXx` 之后，你用的就是里面真正的 `tree::*` 节点了。它们不会出现在最终的 `tree::Program` 里。







- size_tmp_exp被new两次是必要的吗

对这个具体情况是**不必要的**，但写两个是正确的做法，原因层次不同：

---

**`size_exp` 必须物化（new size_tmp），`size_tmp_exp` 不必须 new 两次**

`size_exp` 可能是 `getint()` 等有副作用的表达式，不物化会执行两次——这是**语义 bug**，前面讨论过。

而 `size_tmp_exp1` / `size_tmp_exp2` 只是读同一个寄存器，读寄存器没有副作用。理论上你可以：

```cpp
auto size_tmp_exp = new TempExp(tree::Type::INT, new tree::Temp(size_tmp->num));
// 然后两处都用 size_tmp_exp（同一个指针）
auto move  = new tree::Move(size_tmp_exp, size_exp);   // 写目标
auto cjump = new tree::Cjump("<", size_tmp_exp, ...);  // 读源
```

**不会产生双重求值问题**。

---

**但 new 两次是更好的做法，原因是**：

IR 在语义上要求是**树**而不是 DAG（每个节点只有一个父节点）。用同一个指针放进两个父节点就成了 DAG：

```
Move ──► size_tmp_exp ◄── CJump
         [同一对象]
```

虽然目前代码生成不会因此出错，但后续 pass（canonicalize、线性化、寄存器分配）可能会**原地修改节点**（如加注释、改类型），共享指针会导致一处改动影响另一处。

看代码里 `ArrayExp` 的惯例也是每次 `new tree::TempExp(... new tree::Temp(len_temp->num))`，而不是复用指针。

---

**结论**：`size_tmp_exp` new 两次不是为了防止双重求值（那是物化 `size_exp` 的目的），而是维持 **IR 树结构**的正确性和后续 pass 的安全性。这个写法是正确的。
















