# HW3-HW4 小测题目

**形式**：每道题要求在 ast2tree.cc 中某个 `visit` 函数内添加一段运行时检查代码（用 CJump + exit(-1) 的模式，和 ArrayExp 的越界检查一样）。

---

## Q1：除零检查（BinaryOp, `/` 运算）

**题目**：`a / b` 翻译时没有检查除数是否为 0。请在 `visit(fdmj::BinaryOp *node)` 的算术运算分支中，对 `/` 运算添加运行时检查：若右操作数为 0，调用 `exit(-1)` 终止。

**提示**：右操作数翻译完毕后，可能是任意复杂表达式，需要先物化到 temp（参考 ArrayExp 的 `idx_pre_stm` 模式）。生成的 IR 结构形如：

```
MOVE(t_rhs, rhs_exp)         // 物化
CJUMP(t_rhs == 0, div_exit, div_ok)
LABEL div_exit
EXTCALL exit(-1)
LABEL div_ok
BINOP(/, lhs, t_rhs)
```

最终结果仍以 `Tr_ex(Eseq(..., Binop))` 的形式返回（因为除法是表达式，有值）。

---

### 解答 Q1

找到 `BinaryOp` 中处理算术运算的分支（文件末尾 `// Arithmetic operators` 处），在计算 `binop` 之前插入：

```cpp
// Arithmetic operators: +, -, *, /
node->left->accept(*this);
auto left = visit_exp_result;
auto left_exp = left->unEx(method_temp_map)->exp;

node->right->accept(*this);
auto right = visit_exp_result;
auto right_exp = right->unEx(method_temp_map)->exp;

if (op == "/") {
    // 物化右操作数到 temp，避免重复求值副作用
    auto rhs_temp = method_temp_map->newtemp();
    auto rhs_t_exp = new tree::TempExp(tree::Type::INT, new tree::Temp(rhs_temp->num));
    auto div_exit = method_temp_map->newlabel();
    auto div_ok   = method_temp_map->newlabel();

    auto pre_sl = new vector<tree::Stm *>();
    pre_sl->push_back(new tree::Move(rhs_t_exp, right_exp));         // t = rhs
    pre_sl->push_back(new tree::Cjump("==",                          // if t==0 goto exit
        new tree::TempExp(tree::Type::INT, new tree::Temp(rhs_temp->num)),
        new tree::Const(0), div_exit, div_ok));
    pre_sl->push_back(new tree::LabelStm(div_exit));
    pre_sl->push_back(new tree::ExpStm(                               // exit(-1)
        new tree::ExtCall(tree::Type::INT, "exit",
            new vector<tree::Exp *>({new tree::Const(-1)}))));
    pre_sl->push_back(new tree::LabelStm(div_ok));

    // ESeq: 先做检查，再返回除法结果
    auto result = new tree::Eseq(tree::Type::INT,
        new tree::Seq(pre_sl),
        new tree::Binop(tree::Type::INT, "/", left_exp,
            new tree::TempExp(tree::Type::INT, new tree::Temp(rhs_temp->num))));
    visit_exp_result = new Tr_ex(result);
    return;
}

auto binop = new tree::Binop(tree::Type::INT, op, left_exp, right_exp);
visit_exp_result = new Tr_ex(binop);
```

**关键点**：
- 为什么要物化 `right_exp`：因为 `right_exp` 可能是一个带副作用的表达式（如 `getint()`），直接重复用两次会导致副作用执行两次（一次在 `CJUMP` 里，一次在 `BINOP` 里）。
- 为什么结果是 `Eseq` 而不是 `Tr_nx + Tr_ex`：除法是表达式，有返回值，必须用 `Tr_ex`，而检查是 side-effect，用 `Eseq` 把它放在值前面。

---

## Q2：`new int[n]` 非正大小检查（NewArray）

**题目**：`new int[n]` 若 `n <= 0` 则分配无意义甚至非法的内存。请在 `visit(fdmj::NewArray *node)` 中，在 `malloc` 之前添加运行时检查：若 `n <= 0`，调用 `exit(-1)`。

**提示**：`size_exp` 是已翻译好的表达式，如果它是复杂表达式同样要先物化。检查结构形如：

```
CJUMP(size <= 0, size_exit, size_ok)
LABEL size_exit
EXTCALL exit(-1)
LABEL size_ok
... malloc ...
```

由于 `size_exp` 在检查和 `malloc` 两处都用到（`malloc((size+1)*4)` 和 `Mem[ptr]=size`），所以**必须物化**。

---

### 解答 Q2

在 `visit(fdmj::NewArray *node)` 的 `auto sl = new vector<tree::Stm *>();` 之前插入物化和检查：

```cpp
void ASTToTreeVisitor::visit(fdmj::NewArray *node) {
    int addr_len = compiler_config.at("address_length");
    node->size->accept(*this);
    auto size_raw = visit_exp_result->unEx(method_temp_map)->exp;

    // 物化 size（因为要多次使用）
    auto size_temp = method_temp_map->newtemp();
    auto size_exp = new tree::TempExp(tree::Type::INT, new tree::Temp(size_temp->num));

    auto ptr_temp = method_temp_map->newtemp();
    auto sl = new vector<tree::Stm *>();

    // 先物化 size
    sl->push_back(new tree::Move(size_exp, size_raw));

    // 检查 size <= 0
    auto size_exit = method_temp_map->newlabel();
    auto size_ok   = method_temp_map->newlabel();
    sl->push_back(new tree::Cjump("<=",
        new tree::TempExp(tree::Type::INT, new tree::Temp(size_temp->num)),
        new tree::Const(0), size_exit, size_ok));
    sl->push_back(new tree::LabelStm(size_exit));
    sl->push_back(new tree::ExpStm(
        new tree::ExtCall(tree::Type::INT, "exit",
            new vector<tree::Exp *>({new tree::Const(-1)}))));
    sl->push_back(new tree::LabelStm(size_ok));

    // ptr = malloc((size + 1) * addr_len)
    sl->push_back(new tree::Move(
        new tree::TempExp(tree::Type::PTR, new tree::Temp(ptr_temp->num)),
        new tree::ExtCall(tree::Type::PTR, "malloc",
            new vector<tree::Exp *>({
                new tree::Binop(tree::Type::INT, "*",
                    new tree::Binop(tree::Type::INT, "+",
                        new tree::TempExp(tree::Type::INT, new tree::Temp(size_temp->num)),
                        new tree::Const(1)),
                    new tree::Const(addr_len))}))));

    // Mem[ptr] = size
    sl->push_back(new tree::Move(
        new tree::Mem(tree::Type::INT,
            new tree::TempExp(tree::Type::PTR, new tree::Temp(ptr_temp->num))),
        new tree::TempExp(tree::Type::INT, new tree::Temp(size_temp->num))));

    visit_exp_result = new Tr_ex(new tree::Eseq(tree::Type::PTR,
        new tree::Seq(sl),
        new tree::TempExp(tree::Type::PTR, new tree::Temp(ptr_temp->num))));
}
```

**关键点**：原始代码 `size_exp` 直接用了两次（malloc 的参数和 `Mem[ptr]=size`），如果 `size` 是 `getint()` 这样的外部调用，在没有物化的情况下会执行两次。这道题的核心是**物化的必要性**。

---

## Q3：`.length` 的空指针检查（Length）

**题目**：`arr.length` 翻译为 `Mem[arr]`，但若 `arr == null (0)`，则访问地址 0 会段错误。请在 `visit(fdmj::Length *node)` 中添加空指针检查：若 `arr == 0`，调用 `exit(-1)`。

**提示**：`Length` 目前返回的是 `Tr_ex(Mem[arr])`，需要改成带前置检查的 `Tr_ex(Eseq(check_stms, Mem[arr]))`。

---

### 解答 Q3

```cpp
void ASTToTreeVisitor::visit(fdmj::Length *node) {
    node->exp->accept(*this);
    auto arr_raw = visit_exp_result->unEx(method_temp_map)->exp;

    // 物化 arr（可能是复杂表达式）
    tree::Exp *arr_exp = arr_raw;
    tree::Stm *pre_stm = nullptr;
    if (arr_raw->getTreeKind() != tree::Kind::TEMPEXP && arr_raw->getTreeKind() != tree::Kind::CONST) {
        auto arr_temp = method_temp_map->newtemp();
        pre_stm = new tree::Move(
            new tree::TempExp(tree::Type::PTR, new tree::Temp(arr_temp->num)), arr_raw);
        arr_exp = new tree::TempExp(tree::Type::PTR, new tree::Temp(arr_temp->num));
    }

    // 空指针检查
    auto null_exit = method_temp_map->newlabel();
    auto null_ok   = method_temp_map->newlabel();
    auto check_sl = new vector<tree::Stm *>();
    if (pre_stm != nullptr) check_sl->push_back(pre_stm);
    check_sl->push_back(new tree::Cjump("==", arr_exp, new tree::Const(0), null_exit, null_ok));
    check_sl->push_back(new tree::LabelStm(null_exit));
    check_sl->push_back(new tree::ExpStm(
        new tree::ExtCall(tree::Type::INT, "exit",
            new vector<tree::Exp *>({new tree::Const(-1)}))));
    check_sl->push_back(new tree::LabelStm(null_ok));

    visit_exp_result = new Tr_ex(new tree::Eseq(tree::Type::INT,
        new tree::Seq(check_sl),
        new tree::Mem(tree::Type::INT, arr_exp)));
}
```

**关键点**：`arr_exp` 在检查语句和 `Mem` 两处都要用，若 `arr_exp` 是复杂表达式则**必须物化**，否则会求值两次（在 `Cjump` 一次，在 `Mem` 一次）。

---

## Q4：`arr[idx]` 的空指针检查（ArrayExp）

**题目**：当前的越界检查逻辑是 `Mem[arr]` 读长度，但若 `arr == null`，`Mem[0]` 会先崩溃。请在 `visit(fdmj::ArrayExp *node)` 的 bounds check 之前，在**已物化好 `arr_exp`** 的情况下，添加一个空指针检查。

**注意**：arr 的物化逻辑已经存在，你只需要在 `bounds_sl` 开头插入正确的代码。

---

### 解答 Q4

在 `bounds_sl` 的 `// len = Mem[arr]` 之前插入：

```cpp
// 在 bounds_sl 的最前面插入空指针检查
auto null_exit  = method_temp_map->newlabel();
auto null_ok    = method_temp_map->newlabel();
bounds_sl->push_back(new tree::Cjump("==", arr_exp, new tree::Const(0), null_exit, null_ok));
bounds_sl->push_back(new tree::LabelStm(null_exit));
bounds_sl->push_back(new tree::ExpStm(
    new tree::ExtCall(tree::Type::INT, "exit",
        new vector<tree::Exp *>({new tree::Const(-1)}))));
bounds_sl->push_back(new tree::LabelStm(null_ok));

// 接下来是原有的越界检查（len = Mem[arr], ...）
bounds_sl->push_back(...);  // 原有代码
```

完整的 `bounds_sl` 顺序变为：

```
CJUMP(arr == 0, null_exit, null_ok)    // 新增：空指针检查
LABEL null_exit
EXTCALL exit(-1)
LABEL null_ok
MOVE(len, Mem[arr])                    // 原有：读长度
CJUMP(idx >= 0, ok, exit)             // 原有：下界检查
LABEL ok
CJUMP(idx >= len, exit, done)         // 原有：上界检查
LABEL exit
EXTCALL exit(-1)
LABEL done
```

**关键点**：`arr_exp` 已经被物化（在物化代码中），所以这里可以**安全地多次使用** `arr_exp`，不会重复求值。理解代码中 `arr_pre_stm` 物化存在的原因——正是为了在多个地方（null 检查、读长度、计算地址）复用而不引入副作用。

---

## Q5：对象字段访问空指针检查（ClassVar）

**题目**：`obj.field` 翻译为 `Mem[obj + offset]`，若 `obj == null` 则段错误。请在 `visit(fdmj::ClassVar *node)` 中，在计算 `offset` 之后、生成 `Mem` 之前，添加空指针检查。

**难点**：`obj_exp` 可能是一个复杂表达式（如 `this.c`），需要先物化再检查（否则 `Cjump` 用一次、`Mem` 用一次，共两次求值）。

---

### 解答 Q5

在 `// Memory[obj + offset]` 之前添加：

```cpp
// 物化 obj_exp
tree::Exp *safe_obj_exp = obj_exp;
vector<tree::Stm *> pre_sl;
if (obj_exp->getTreeKind() != tree::Kind::TEMPEXP && obj_exp->getTreeKind() != tree::Kind::CONST) {
    auto obj_temp = method_temp_map->newtemp();
    pre_sl.push_back(new tree::Move(
        new tree::TempExp(tree::Type::PTR, new tree::Temp(obj_temp->num)), obj_exp));
    safe_obj_exp = new tree::TempExp(tree::Type::PTR, new tree::Temp(obj_temp->num));
}

// 空指针检查
auto null_exit = method_temp_map->newlabel();
auto null_ok   = method_temp_map->newlabel();
pre_sl.push_back(new tree::Cjump("==", safe_obj_exp, new tree::Const(0), null_exit, null_ok));
pre_sl.push_back(new tree::LabelStm(null_exit));
pre_sl.push_back(new tree::ExpStm(
    new tree::ExtCall(tree::Type::INT, "exit",
        new vector<tree::Exp *>({new tree::Const(-1)}))));
pre_sl.push_back(new tree::LabelStm(null_ok));

// Memory[obj + offset]
auto mem = new tree::Mem(field_type,
    new tree::Binop(tree::Type::PTR, "+", safe_obj_exp, new tree::Const(offset)));

tree::Exp *result = mem;
if (!pre_sl.empty())
    result = new tree::Eseq(field_type, new tree::Seq(new vector<tree::Stm*>(pre_sl)), mem);
visit_exp_result = new Tr_ex(result);
```

**关键点**：`This` 节点（`this`）翻译出来的是 `TempExp`，不需要物化；但 `this.c` 翻译出来是 `Mem[...]`，需要物化。判断条件 `getTreeKind() != TEMPEXP && != CONST` 和 ArrayExp 一致。

---

## Q6：虚方法调用空指针检查（CallStm + CallExp）

**题目**：`obj.method(args)` 中 `obj` 可能为 null。请在 `visit(fdmj::CallStm *node)` 中添加空指针检查。

**额外难点**：`obj_exp` 被用了两次——一次作为函数指针计算 `Mem[obj + method_pos]`，一次作为第一个参数传入 call。若不物化，会导致 obj 被求值两次（此时已经是 bug，与类型无关）。事实上，原始代码的 `CallStm` 中 `obj_exp` 就直接用了两次，这本身就是一个**潜在 bug**——请一并修复。

---

### 解答 Q6

原始代码中 `obj_exp` 的两次使用已经是问题（如果 `obj` 是一个带副作用的表达式）。完整修复后的 `visit(fdmj::CallStm *node)`：

```cpp
void ASTToTreeVisitor::visit(fdmj::CallStm *node) {
    string method_name = node->name->id;
    int method_pos = class_table->get_method_pos(method_name);

    node->obj->accept(*this);
    auto obj_raw = visit_exp_result->unEx(method_temp_map)->exp;

    // 物化 obj（避免多次求值，且空指针检查和调用都需要用到）
    tree::Exp *obj_exp = obj_raw;
    auto pre_sl = new vector<tree::Stm *>();
    if (obj_raw->getTreeKind() != tree::Kind::TEMPEXP && obj_raw->getTreeKind() != tree::Kind::CONST) {
        auto obj_temp = method_temp_map->newtemp();
        pre_sl->push_back(new tree::Move(
            new tree::TempExp(tree::Type::PTR, new tree::Temp(obj_temp->num)), obj_raw));
        obj_exp = new tree::TempExp(tree::Type::PTR, new tree::Temp(obj_temp->num));
    }

    // 空指针检查
    auto null_exit = method_temp_map->newlabel();
    auto null_ok   = method_temp_map->newlabel();
    pre_sl->push_back(new tree::Cjump("==", obj_exp, new tree::Const(0), null_exit, null_ok));
    pre_sl->push_back(new tree::LabelStm(null_exit));
    pre_sl->push_back(new tree::ExpStm(
        new tree::ExtCall(tree::Type::INT, "exit",
            new vector<tree::Exp *>({new tree::Const(-1)}))));
    pre_sl->push_back(new tree::LabelStm(null_ok));

    // Build args
    auto call_args = new vector<tree::Exp *>({obj_exp});
    if (node->par != nullptr) {
        for (auto p : *node->par) {
            p->accept(*this);
            call_args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
        }
    }

    auto func_ptr = new tree::Mem(tree::Type::PTR,
        new tree::Binop(tree::Type::PTR, "+", obj_exp, new tree::Const(method_pos)));

    auto sem = semant_map->getSemant(node);
    tree::Type ret_type = tree::Type::INT;
    if (sem != nullptr) ret_type = typeKind2TreeType(sem->get_type());

    auto call_stm = new tree::ExpStm(
        new tree::Call(ret_type, method_name, func_ptr, call_args));

    pre_sl->push_back(call_stm);
    visit_exp_result = new Tr_nx(new tree::Seq(pre_sl));
}
```

**关键点**：
- 原始代码中 `obj_exp` 用了两次（`func_ptr` 里一次，`call_args[0]` 里一次），这是原始代码的 latent bug；题目要求你发现并修复它。
- `CallStm` 结果是 `Tr_nx`（语句，无返回值），所以检查代码直接 push 进 `pre_sl`，最后包装成 `Seq`，不需要 `Eseq`。
- `CallExp` 同理但结果需要用 `Tr_ex(Eseq(pre_sl_without_call, Call(...)))` 的形式，因为 `Call` 本身是 `Exp`。

---

## 综合理解题（口头/笔答）

**Q7**：ArrayExp 的空指针检查（Q4）和 ClassVar 的空指针检查（Q5）都需要"物化"对象指针。

请解释：为什么 `arr_exp` 在 ArrayExp 里可能是 `TempExp`（不需物化），也可能需要物化？什么情况下不需要？请举出一个「需要物化」和一个「不需要物化」的 FDMJ 源码例子。

---

### 解答 Q7

**不需要物化**（`arr_exp` 已是 `TempExp`）：
```java
int[] a = new int[5];
putint(a[2]);   // a 是 IdExp -> TempExp，直接是寄存器，不需要物化
```

**需要物化**（`arr_exp` 是复杂表达式，有副作用或需多次求值）：
```java
putint(getarray(b)[2]);  // getarray(b) 返回数组指针，是 ExtCall，不是 TempExp
                          // 若不物化，bounds check 里用一次，Mem 计算里再用一次，getarray() 执行两次
```

或者更隐晦的情况：
```java
putint(a[getint()]);  // idx = getint()，若不物化，bounds check 里用一次（>= 0），再用一次（>= len），getint() 执行两次，读了两个不同的输入！
```

这正是 ArrayExp 代码中同时对 `arr` 和 `idx` 做物化判断的原因：`Kind::TEMPEXP` 和 `Kind::CONST` 是"无副作用、引用稳定"的，其余情况都需要物化。

---

**核心知识点总结**：

| 题号 | 检查位置 | IR 结构模式 | 关键概念 |
|------|----------|-------------|----------|
| Q1 | BinaryOp `/` | `Eseq(check_seq, Binop)` | 表达式中的 side-effect |
| Q2 | NewArray | `Seq` 开头插入 CJump | 物化的必要性 |
| Q3 | Length | `Eseq(check_seq, Mem)` | 简单空指针 |
| Q4 | ArrayExp | `bounds_sl` 开头插入 | 利用已有物化 |
| Q5 | ClassVar | `Eseq(check_seq, Mem)` | 链式访问的类型跟踪 |
| Q6 | CallStm/CallExp | `Seq` + 物化 | latent bug 修复 |
| Q7 | 口答 | — | 物化的边界条件 |






看了你当前的代码（已经做完了除零、NewArray非负、Length空指针），再给4道新题：

---

## Q_new1：`putarray(n, arr)` 的三重检查（PutArray）

**题目**：`putarray(n, arr)` 会从 `arr` 中打印 `n` 个元素。当前没有任何检查。请在 `visit(fdmj::PutArray *node)` 中，在生成 ExtCall 之前添加三重运行时检查：

1. `n >= 0`（不能打印负数个元素）
2. `arr != null`（不能对空数组指针操作）
3. `n <= arr.length`（不能越过数组实际长度）

**难点**：`n` 和 `arr` 可能是复杂表达式，需要物化；读 `arr.length` 需要 `Mem[arr]`，依赖 arr 已物化；三个检查要串联（ok 了才继续）。

---

### 解答 Q_new1

```cpp
void ASTToTreeVisitor::visit(fdmj::PutArray *node) {
    node->n->accept(*this);
    auto n_raw = visit_exp_result->unEx(method_temp_map)->exp;
    node->arr->accept(*this);
    auto arr_raw = visit_exp_result->unEx(method_temp_map)->exp;

    // 物化 n
    auto n_tmp = method_temp_map->newtemp();
    auto n_exp = [&]{ return new tree::TempExp(tree::Type::INT, new tree::Temp(n_tmp->num)); };

    // 物化 arr
    auto arr_tmp = method_temp_map->newtemp();
    auto arr_exp = [&]{ return new tree::TempExp(tree::Type::PTR, new tree::Temp(arr_tmp->num)); };

    // 读数组长度到 len_tmp
    auto len_tmp = method_temp_map->newtemp();
    auto len_exp = [&]{ return new tree::TempExp(tree::Type::INT, new tree::Temp(len_tmp->num)); };

    auto L_n_ok    = method_temp_map->newlabel();
    auto L_arr_ok  = method_temp_map->newlabel();
    auto L_len_ok  = method_temp_map->newlabel();
    auto L_exit    = method_temp_map->newlabel();

    auto sl = new vector<tree::Stm *>();
    sl->push_back(new tree::Move(n_exp(), n_raw));
    sl->push_back(new tree::Move(arr_exp(), arr_raw));

    // 1. n >= 0
    sl->push_back(new tree::Cjump(">=", n_exp(), new tree::Const(0), L_n_ok, L_exit));
    sl->push_back(new tree::LabelStm(L_exit));
    sl->push_back(new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "exit",
        new vector<tree::Exp *>({new tree::Const(-1)}))));

    // 2. arr != null
    sl->push_back(new tree::LabelStm(L_n_ok));
    sl->push_back(new tree::Cjump("!=", arr_exp(), new tree::Const(0), L_arr_ok, L_exit));

    // 3. n <= arr.length
    sl->push_back(new tree::LabelStm(L_arr_ok));
    sl->push_back(new tree::Move(len_exp(), new tree::Mem(tree::Type::INT, arr_exp())));
    sl->push_back(new tree::Cjump("<=", n_exp(), len_exp(), L_len_ok, L_exit));
    sl->push_back(new tree::LabelStm(L_len_ok));

    // 实际调用
    sl->push_back(new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "putarray",
        new vector<tree::Exp *>({n_exp(), arr_exp()}))));

    visit_exp_result = new Tr_nx(new tree::Seq(sl));
}
```

**关键点**：`L_exit` 被复用了三次（三个 CJump 的 false 分支都跳同一个 exit），这是合法且高效的——不需要为每个检查分配一个独立的 exit label。

---

## Q_new2：MethodDecl 方法入口的 `this` 非空检查

**题目**：class method 被调用时，`this`（即 `_^return^_method` 对应的 temp）理论上不应为 null（调用方不会对 null 对象调用方法）。请在 `visit(fdmj::MethodDecl *node)` 中，在处理 `vdl`（局部变量声明）之前，向方法体 `sl` 中插入对 `this` 的非空检查。

**提示**：`this_temp` 就是 `method_var_table->get_var_temp("_^return^_" + current_method)`。它是一个已分配好的 temp，类型已被覆盖为 PTR，直接构造 TempExp 使用即可——**不需要物化**，因为 temp 本身不是复杂表达式。

---

### 解答 Q_new2

在 `auto sl = new vector<tree::Stm *>();` 之后、`if (node->vdl != nullptr)` 之前插入：

```cpp
// this 非空检查（方法入口保证）
auto this_temp = method_var_table->get_var_temp(return_var);
auto this_exp = new tree::TempExp(tree::Type::PTR, new tree::Temp(this_temp->num));
auto L_this_exit = method_temp_map->newlabel();
auto L_this_ok   = method_temp_map->newlabel();
sl->push_back(new tree::Cjump("==", this_exp, new tree::Const(0), L_this_exit, L_this_ok));
sl->push_back(new tree::LabelStm(L_this_exit));
sl->push_back(new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "exit",
    new vector<tree::Exp *>({new tree::Const(-1)}))));
sl->push_back(new tree::LabelStm(L_this_ok));
```

**关键点**：`this_temp` 在 `generate_method_var_table` 里分配，`MethodDecl` 里后来把它的类型改为 PTR。这里可以直接用，**无需物化**，因为 `TempExp(t)` 读取的是一个寄存器，没有副作用，也不会被求值两次的问题。这与 Q3、Q5 的物化规则形成对比：TempExp 和 Const 是天然"安全可复用"的。

---

## Q_new3：虚方法调用函数指针非空检查（CallExp）

**题目**：`obj.method(args)` 会先从对象的 vtable 读出函数指针 `Mem[obj + method_pos]`，再通过它调用。但如果 vtable 位置没有被正确初始化（例如某个类没有 override 某方法），函数指针可能是 0，调用时崩溃。

请在 `visit(fdmj::CallExp *node)` 中，在构造 `Call(...)` 之前，将 `func_ptr` 物化到一个 temp，并检查它不为 0。

**额外挑战**：`func_ptr` 是 `Mem[...]` 类型，不是 TempExp，**必须物化**。物化后，`call_args[0]` 里已经有 `obj_exp`，但 `func_ptr` 用的是另一个 temp。最终结果是 `Tr_ex(Eseq(check_stms, Call(...)))`。

---

### 解答 Q_new3

```cpp
void ASTToTreeVisitor::visit(fdmj::CallExp *node) {
    string method_name = node->name->id;
    int method_pos = class_table->get_method_pos(method_name);

    node->obj->accept(*this);
    auto obj_exp = visit_exp_result->unEx(method_temp_map)->exp;

    auto call_args = new vector<tree::Exp *>({obj_exp});
    if (node->par != nullptr) {
        for (auto p : *node->par) {
            p->accept(*this);
            call_args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
        }
    }

    // 物化函数指针到 temp
    auto fp_tmp = method_temp_map->newtemp();
    auto fp_exp = [&]{ return new tree::TempExp(tree::Type::PTR, new tree::Temp(fp_tmp->num)); };

    auto raw_fp = new tree::Mem(tree::Type::PTR,
        new tree::Binop(tree::Type::PTR, "+", obj_exp, new tree::Const(method_pos)));

    auto L_fp_exit = method_temp_map->newlabel();
    auto L_fp_ok   = method_temp_map->newlabel();

    auto pre_sl = new vector<tree::Stm *>();
    pre_sl->push_back(new tree::Move(fp_exp(), raw_fp));   // fp_tmp = Mem[obj+pos]
    pre_sl->push_back(new tree::Cjump("==", fp_exp(), new tree::Const(0), L_fp_exit, L_fp_ok));
    pre_sl->push_back(new tree::LabelStm(L_fp_exit));
    pre_sl->push_back(new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "exit",
        new vector<tree::Exp *>({new tree::Const(-1)}))));
    pre_sl->push_back(new tree::LabelStm(L_fp_ok));

    auto sem = semant_map->getSemant(node);
    tree::Type ret_type = tree::Type::INT;
    if (sem != nullptr) ret_type = typeKind2TreeType(sem->get_type());

    // Call 用物化后的 fp_exp，而不是原来的 raw_fp
    auto call = new tree::Call(ret_type, method_name, fp_exp(), call_args);

    visit_exp_result = new Tr_ex(
        new tree::Eseq(ret_type, new tree::Seq(pre_sl), call));
}
```

**关键点**：原始代码的 `func_ptr` 是 `Mem[obj + method_pos]`，如果不物化就直接传给 `Call`，那 CJump 和 Call 里会各求值一次 `Mem[...]`（两次内存读，不安全）。物化后用 `fp_exp()` 构造两个不同的 `TempExp` 对象（num 相同），这是因为 IR 树是树结构，不能共享节点——这和上面 `n_exp()`、`arr_exp()` 用 lambda 返回新对象是同一个原因。

---

## Q_new4：BinaryOp `%` 的除零检查

**题目**：模运算 `a % b` 和除法一样，当 `b == 0` 时行为未定义（崩溃）。当前代码对 `%` 没有特殊处理，直接走到最后的通用算术分支。请在 `/` 的检查分支下面，对 `%` 添加相同的检查（复用相同结构）。

**引申问题**：是否可以把 `/` 和 `%` 的检查合并为一个分支？怎么写？

---

### 解答 Q_new4

在 `if (op == "/")` 整个分支之后，复制相同结构：

```cpp
if (op == "/" || op == "%") {
    node->left->accept(*this);
    auto left_exp = visit_exp_result->unEx(method_temp_map)->exp;
    node->right->accept(*this);
    auto right_raw = visit_exp_result->unEx(method_temp_map)->exp;

    auto rhs_tmp = method_temp_map->newtemp();
    auto rhs_exp = [&]{ return new tree::TempExp(tree::Type::INT, new tree::Temp(rhs_tmp->num)); };
    auto L_exit = method_temp_map->newlabel();
    auto L_ok   = method_temp_map->newlabel();

    auto pre_sl = new vector<tree::Stm *>();
    pre_sl->push_back(new tree::Move(rhs_exp(), right_raw));
    pre_sl->push_back(new tree::Cjump("==", rhs_exp(), new tree::Const(0), L_exit, L_ok));
    pre_sl->push_back(new tree::LabelStm(L_exit));
    pre_sl->push_back(new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "exit",
        new vector<tree::Exp *>({new tree::Const(-1)}))));
    pre_sl->push_back(new tree::LabelStm(L_ok));

    visit_exp_result = new Tr_ex(new tree::Eseq(tree::Type::INT,
        new tree::Seq(pre_sl),
        new tree::Binop(tree::Type::INT, op, left_exp, rhs_exp())));
    return;
}
```

**关键点**：合并 `/` 和 `%` 用 `op == "/" || op == "%"`，并用 `op` 变量构造最后的 `Binop`，这样一份代码处理两种操作——运算符由 `op` 决定，检查逻辑完全相同。引申问题的答案就是把两个独立的 `if` 合并为一个 `||` 条件。

---

## 汇总

| 题号 | 位置 | 新增检查 | 核心考点 |
|------|------|----------|----------|
| Q_new1 | PutArray | n≥0, arr≠null, n≤arr.length | 多重检查串联，复用同一 exit label |
| Q_new2 | MethodDecl | this≠null | TempExp 无需物化；方法入口约定 |
| Q_new3 | CallExp | 函数指针≠null | vtable 读取需物化；IR 树节点不共享 |
| Q_new4 | BinaryOp `%` | 除零 | 与 `/` 合并分支，op 变量复用 |
