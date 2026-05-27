# Quiz 3 备考笔记：HW7/HW8 现场扩展

## 前两次小测风格

两次 quiz 都不是复现作业主算法，而是在某个已有阶段上加一个“额外语义/运行时性质”：

- Quiz 1：在 HW2 语义 AST 上加浅不可变类检查。关键词是“名字规则 + 继承闭包 + 改写点枚举”。
- Quiz 2：在 HW3/4 AST 到 IR+ 转换上加 local int 初始化动态检查。关键词是“shadow temp + 赋值时置 1 + 使用时检查 + 报告讨论为什么 class var 不适用”。

共同模式：

- 题面会把范围收窄到一个文件或一个阶段，但隐藏分数在“情况是否枚举完整”。
- 允许第一个错误就停，但必须知道所有可能错误点。
- 会给测试，但明确说测试不全。
- 报告不是形式主义，通常要写“哪些情况检查/插桩，为什么这样做，边界情况是什么”。

所以这次如果基于 HW7/HW8，最可能不是让你重写 SSA/SCCP，而是加一个检查、统计、保守优化、或限制版插桩。

## 现场优先熟悉的文件

HW7：

- `HW7/lib/quad/quadssa.cc`
  - `placePhi`
  - `renameVariables`
  - `cleanupUnusedPhi`
  - 辅助函数：`renameUses`、`renameDefs`、`collectDefUsePtrs`、`rebuildDefUseSets`
- `HW7/include/quad/quad.hh`
  - 所有 Quad 语句类型字段。
- `HW7/include/quadflow/flowinfo.hh`
  - CFG、支配树、支配边界、liveness 的现成结构。

HW8：

- `HW8/lib/opt/opt.cc`
  - `calculateBT`
  - `modifyFunc`
  - `optProg`
  - 辅助函数：`evalTerm`、`evalTermChecked`、`evalBinop`、`evalRelop`、`rewriteTerm`、`isEdgeExecutable`、`evalPhi`
- `HW8/include/opt/opt.hh`
  - `RtValue` 三值格、`block_executable`、`temp_value`。
- `HW8/include/quad/quad.hh`
  - 语句字段、clone/print 约定。

测试命令：

```sh
cd HW7 && make build && make run
cd HW8 && make build && make run
cd HW7 && make run-one FILE=quadtest7
cd HW8 && make run-one FILE=opttest7
```

## 90 分钟现场打法

前 10 分钟：

- 读题时把任务拆成三类：需要维护的新状态、哪些语句更新状态、哪些语句需要检查或改写。
- 立刻确认限制：只能改哪个文件、输入输出文件名是否变化、错误是 `cerr` 还是插入 `exit(-xxx)`，是否继续优化。

中间 55 分钟：

- 先实现最小闭环：初始化状态、遍历函数/块/语句、处理 `MOVE`、`MOVE_BINOP`、`PHI`、`CJUMP`、`RETURN`。
- 再补齐副作用语句：`LOAD`、`STORE`、`CALL`、`MOVE_CALL`、`EXTCALL`、`MOVE_EXTCALL`、`PTR_CALC`。
- 每加一种语句立刻写/改一个小 `.quad` 测试，而不是最后一起调。

最后 25 分钟：

- 用 3 个自制边界测试兜底：分支、phi、循环或不可达块。
- 写报告时按“状态、初始化、更新规则、检查点、边界情况、局限性”六段写。
- 如果有 bug，优先保证保守正确：宁可少优化、保留语句，也不要误删可执行块或误判常量。

## HW7 可能题目预测

### 1. SSA 正确性检查器

要求：在 HW7 输出 SSA 后检查是否真满足 SSA。

要检查：

- 每个非参数 temp 在同一函数内最多被定义一次。
- 每个 use 都有定义，或是函数参数。
- PHI 参数的 label 必须是当前块的前驱。
- 每个有多个前驱且变量 live-in 的 join 点应有必要的 PHI。
- PHI 必须在 LABEL 后、普通语句前。

实现位置：

- 最容易在 `quad2ssa()` 中 `cleanupUnusedPhi(funcdecl)` 后加 `checkSSA(funcdecl, domInfo, liveness)`。
- 遍历 `func->params` 建参数集合，遍历所有 statement 的 `def/use`。

报告重点：

- 参数不算函数体内定义。
- PHI 的 use 是来自边的 use，不是普通顺序 use。
- XML 中 def/use 可能不可靠时，最好直接从语句字段结构收集。

### 2. 只对“整数变量”进入 SSA，指针不版本化

题目可能说：为简化后续分析，PTR 类型 temp 不做 SSA 重命名或不放 PHI。

改法：

- 在 `placePhi` 中已有 `varTypes`，只给 `QuadType::INT` 放 PHI。
- 在 `renameDefs` / `renameUses` 或 `vars` 收集阶段过滤类型。
- 注意 `PTR_CALC`、`LOAD`、`STORE` 里的 ptr temp 不要被误版本化。

坑：

- 如果只过滤 def，不过滤 use，会出现 use 改名而 def 不改名，或相反。
- 参数本来就不版本化，别重复处理。

### 3. Phi 参数顺序或缺失修复

题目可能要求：每个 PHI 的参数按前驱 label 升序，且必须恰好覆盖所有前驱。

改法：

- `placePhi` 里创建 args 时已经使用 `set<int>`，天然升序。
- 可加检查/修复：对 `domInfo->predecessors[blockLabel]` 建集合，对 phi args 的 label 建集合，对比。

报告重点：

- PHI 参数和 CFG 边一一对应。
- 删除不可达块后前驱集合会变，HW8 中尤其要同步删 PHI 参数。

### 4. 删除无用 MOVE / 拷贝传播到 SSA

要求：在 HW7 输出 SSA 后，把 `MOVE t2 <- t1` 的使用替换为 `t1` 并删掉 MOVE。

改法：

- 只在 SSA 上做，比较容易，因为每个 temp 单定义。
- 建 `alias[dst]=src`，使用时递归找最终 temp。
- 保守处理：只处理 `MOVE dst <- TEMP`，且 dst 不是参数，不处理 CALL/LOAD/PTR。
- 最后 rebuild def/use。

坑：

- PHI 参数也要替换。
- 如果 dst 在 PHI 自引用循环里，要防止 alias 死循环。

### 5. 检测未初始化 SSA use

与 Quiz 2 呼应。SSA 中某 temp 如果被使用但没有定义且不是参数，则报错。

实现：

- 参数集合 `params`。
- 遍历 def 集合收所有定义。
- 遍历 use 集合查 `!params.count(t) && !defs.count(t)`。
- 对 PHI 参数仍然要查。

## HW8 可能题目预测

### 1. 加“除零运行时检查”插桩

很符合 Quiz 2 风格。要求：对 `MOVE_BINOP` 的 `/` 或 `%`，如果右操作数运行时可能为 0，则插入检查；若必为非 0 不插；若必为 0 可直接 `exit(-102)`。

实现位置：

- 在 `modifyFunc()` 处理 `MOVE_BINOP` 处。
- 用 `getRtValue` 判断 right：
  - `ONE_VALUE(0)`：在该语句前插入 `EXTCALL exit(Const:-102)` 或按题面要求。
  - `ONE_VALUE(nonzero)`：不用插。
  - `MANY_VALUES/NO_VALUE`：保守插检查。如果题目要求 IR 支持有限，可能只能保留原语句并不优化。

报告重点：

- 编译期常量 0、常量非 0、未知三种。
- 不可达块不插。
- 除零不能被常量折叠。

### 2. SCCP 后输出优化统计

要求：统计删掉了多少不可达块、折叠了多少常量、化简了多少 CJUMP、删了多少 PHI。

实现：

- 在 `Opt` 加计数成员，或在 `modifyFunc()` 用局部变量。
- 跳过不可达块时计数。
- `MOVE/MOVE_BINOP/PHI` 因 `ONE_VALUE` 删除时计数。
- `CJUMP -> JUMP` 时计数。
- 输出到 `cerr` 或报告，按题面。

坑：

- 不要改变正常 `.quad` 输出格式，除非题目要求。
- 统计函数级还是程序级要看题面。

### 3. 只传播“安全常量”：CALL 之后内存相关值失效

要求可能是：考虑 `LOAD` 的常量值，若有 `STORE` 或 CALL 就失效。

保守解法：

- 如果要给 `LOAD Mem(ConstAddr)` 做常量传播，用 `mem_value[address]`。
- `STORE Const -> Mem(ConstAddr)` 更新。
- 任意 `CALL/EXTCALL/MOVE_CALL/MOVE_EXTCALL` 清空内存映射。
- 地址不是确定常量则清空或标 MANY。

报告重点：

- 别名不明确时必须保守。
- CALL 可能改全局/对象内存。

### 4. 不可达块中的错误不报警

这个你 HW8 已经实现了类似逻辑。题目可能把它反过来问：只对可达路径上的 `NO_VALUE` use 报错。

检查点：

- `calculateBT()` 只处理 executable block。
- PHI 只看 executable edge。
- `modifyFunc()` 的 warning pass 要用 live_temps 过滤死 phi。

### 5. 常量分支后 PHI 同步裁剪

要求：当 `CJUMP` 被化简成 `JUMP` 后，后继 PHI 删除被裁掉边的参数；如果只剩一个参数，PHI 改 MOVE；如果值已知常量，删 PHI 并替换 use。

这就是 HW8 里的高危点。现场如果出，优先复用：

- `isEdgeExecutable`
- `phi_substitute`
- `fresh_moves`

报告重点：

- CFG 边变化和 PHI 参数必须同步。
- 常量定义被删后，如果 PHI 仍需一个 temp 输入，要插 fresh temp，否则 PHI 引用悬空。

### 6. 限制优化范围：不优化有副作用的表达式

要求可能说：不要删除会产生副作用的语句，即使返回值常量/不用。

规则：

- 可删除：纯 `MOVE`、纯 `MOVE_BINOP` 的常量结果。
- 不可删除：`CALL`、`EXTCALL`、`MOVE_CALL`、`MOVE_EXTCALL`、`STORE`。
- `LOAD` 是否可删看题面；通常保守不删，因为可能触发非法内存访问或读取 volatile。

## 最值得提前背下来的语句处理表

| 语句 | def | use | 保守值 |
| --- | --- | --- | --- |
| `MOVE dst <- src` | dst | src 中 temp | src 的值 |
| `MOVE_BINOP dst <- left op right` | dst | left/right | 两边常量才折叠，除零不折叠 |
| `LOAD dst <- Mem(src)` | dst | src | MANY |
| `STORE src -> Mem(dst)` | 无 | src/dst | 有副作用，不能删 |
| `CALL` | 无 | obj/args | 有副作用 |
| `MOVE_CALL` | dst | obj/args | dst 为 MANY，有副作用不能删 |
| `EXTCALL` | 无 | args | 有副作用 |
| `MOVE_EXTCALL` | dst | args | dst 为 MANY，有副作用不能删 |
| `CJUMP` | 无 | left/right | 常量条件可裁边 |
| `PHI` | dst | args temps | 只合并可达边 |
| `PTR_CALC` | dst | ptr/offset | ptr 值通常 MANY |
| `RETURN` | 无 | exp | exp 可替换常量 |

## 报告模板

```text
本次扩展目标：

新增状态：
- ...

初始化：
- ...

逐语句处理：
- MOVE: ...
- MOVE_BINOP: ...
- PHI: ...
- CJUMP: ...
- CALL/STORE/LOAD: ...

边界情况：
- 不可达块是否处理。
- PHI 的不可达前驱是否参与。
- 函数参数如何处理。
- 指针/内存/CALL 如何保守处理。
- 常量 0、未知值、未定义值如何处理。

正确性说明：
- 为什么不会漏报/误删。
- 为什么在不知道时退化为 MANY_VALUES 或保留原语句。

局限：
- 不做跨函数。
- 不精确处理别名/内存。
```

## 最可能的三道题

第一可能：HW8 对“对象有时不需存到内存中，直接用 temp”做局部优化。老师上课随口提过，且它非常适合作为 HW8 后续扩展：本质是非常保守的对象标量替换。

第二可能：HW8 加运行时安全检查，例如除零、空指针、数组/指针访问前检查。原因是 Quiz 2 已经是 shadow temp 插桩，这类题最像老师风格。

第三可能：HW8 在 SCCP 基础上加一个“只在可达路径上”的错误检测或统计。原因是 HW8 天然有 `block_executable` 和 `temp_value`，很适合出一题考边界枚举。

第四可能：HW7 加 SSA 输出检查器。原因是 HW7 的主算法难现场重写，但“检查 SSA 合法性/PHI 合法性”很适合 90 分钟，也能考对 SSA 细节的理解。

## 预测题题目与解答

以下按“题目”和“解答”拆开写。现场不一定原题命中，但这些解法里的代码骨架、状态设计、边界说明可以直接迁移。

### 预测 1：HW8 对不逃逸对象做 temp 替换

#### 题目

在 HW8 的 SSA Quad 优化阶段中，某些对象只在当前函数内创建、写字段、读字段，并没有真正传给函数调用或返回。对这种对象，不必把字段存到内存中，可以用 SSA temp 保存字段值。请在 `opt.cc` 中实现一个保守优化：

- 对 `malloc` 创建出的对象，如果该对象不逃逸，则可以删除对应的 `STORE field -> Mem(obj + offset)`。
- 后续 `LOAD dst <- Mem(obj + offset)` 可以改写为 `MOVE dst <- field_temp_or_const`。
- 如果对象作为参数传给 `CALL/EXTCALL`、被 `RETURN`、被 `STORE` 到其他内存位置，或地址计算/别名无法判断，则不要优化该对象。
- 只要求在一个函数内部做，不要求跨函数。

#### 解答

这是“对象标量替换”（scalar replacement of aggregates）的保守版。核心思路：识别 `malloc` 得到的对象基址 temp，追踪这个对象的字段地址 `PTR_CALC addr <- obj + Const:offset`，如果对象没有逃逸，就用一个映射 `(obj_temp, offset) -> 当前字段值 QuadTerm` 记录字段值。

最保守、最容易现场实现的范围：

- 只处理 `MOVE_EXTCALL dst <- malloc(Const:n)` 创建出来的对象。
- 只处理 `PTR_CALC addr <- obj + Const:offset`，offset 必须是常量。
- 只处理 `STORE value -> Mem(addr)` 和 `LOAD dst <- Mem(addr)`，其中 `addr` 必须能反查到 `(obj, offset)`。
- 如果 `obj` 或字段地址 `addr` 出现在 `CALL` 参数、`EXTCALL` 参数、`RETURN`、普通 `STORE` 的 src、非字段用途里，认为逃逸，不优化。
- 跨基本块传播字段值：如果某字段在所有可达前驱出口都有同一个值，则后继入口可继续使用该值；如果前驱值不同、某前驱没有该字段值、或存在循环导致不确定，则该字段值在交汇点退化为未知，保留原 `LOAD`。

建议现场实现顺序：

1. 先在 `HW8/lib/opt/opt.cc` 增加 helper，提取 temp 编号。

```cpp
static int termTempNum(QuadTerm *term) {
    if (term == nullptr || term->kind != QuadTermKind::TEMP) return -1;
    return term->get_temp()->temp->num;
}

static bool isConstTerm(QuadTerm *term, int *value = nullptr) {
    if (term == nullptr || term->kind != QuadTermKind::CONST) return false;
    if (value) *value = term->get_const();
    return true;
}
```

2. 在 `Opt` 中新增一个 pass，建议放在 `modifyFunc()` 之后调用，或在 `optFunc()` 中 `modifyFunc(); scalarReplaceObjects();`。

需要在 `HW8/include/opt/opt.hh` 加声明：

```cpp
void scalarReplaceObjects();
```

`optFunc()` 改为：

```cpp
calculateBT();
modifyFunc();
scalarReplaceObjects();
return func;
```

3. 在 `opt.cc` 中实现两阶段 pass。

第一阶段收集：

- `object_temps`：所有 `malloc` 结果 temp。
- `addr_to_field`：字段地址 temp -> `(obj, offset)`。
- `escaped`：逃逸对象集合。

伪代码：

```cpp
void Opt::scalarReplaceObjects() {
    set<int> object_temps;
    map<int, pair<int,int>> addr_to_field;
    set<int> escaped;

    for (auto *block : *func->quadblocklist) {
        for (auto *stm : *block->quadlist) {
            if (stm->kind == QuadKind::MOVE_EXTCALL) {
                auto *s = static_cast<QuadMoveExtCall*>(stm);
                if (s->extcall && s->extcall->extfun == "malloc")
                    object_temps.insert(s->dst->temp->num);
            } else if (stm->kind == QuadKind::PTR_CALC) {
                auto *s = static_cast<QuadPtrCalc*>(stm);
                int dst = termTempNum(s->dst);
                int base = termTempNum(s->ptr);
                int off = 0;
                if (dst >= 0 && object_temps.count(base) && isConstTerm(s->offset, &off))
                    addr_to_field[dst] = {base, off};
            }
        }
    }
```

第二阶段做逃逸分析：

```cpp
auto markEscapedTerm = [&](QuadTerm *term) {
    int t = termTempNum(term);
    if (object_temps.count(t)) escaped.insert(t);
    if (addr_to_field.count(t)) escaped.insert(addr_to_field[t].first);
};
```

遍历语句：

- `CALL/MOVE_CALL`：对象参数、字段地址参数都逃逸；`obj_term` 也逃逸。
- `EXTCALL/MOVE_EXTCALL`：除了 `malloc` 自身，参数中的对象/字段地址逃逸。
- `RETURN`：返回对象或字段地址，逃逸。
- `STORE src -> Mem(dst)`：
  - 如果 `dst` 是已知字段地址，`src` 是普通 int/ptr 值，不一定逃逸。
  - 如果 `src` 是对象 temp 或字段地址 temp，说明对象被存到内存，逃逸。
  - 如果 `dst` 不是已知字段地址，但 `dst/src` 涉及对象或字段地址，保守逃逸。
- `PTR_CALC`：如果不是 `obj + Const` 的字段地址形式，且用到了对象，保守逃逸。

伪代码：

```cpp
for (auto *block : *func->quadblocklist) {
    for (auto *stm : *block->quadlist) {
        switch (stm->kind) {
            case QuadKind::CALL: {
                auto *s = static_cast<QuadCall*>(stm);
                markEscapedTerm(s->obj_term);
                if (s->args) for (auto *a : *s->args) markEscapedTerm(a);
                break;
            }
            case QuadKind::MOVE_CALL: {
                auto *s = static_cast<QuadMoveCall*>(stm);
                if (s->call) {
                    markEscapedTerm(s->call->obj_term);
                    if (s->call->args) for (auto *a : *s->call->args) markEscapedTerm(a);
                }
                break;
            }
            case QuadKind::EXTCALL: {
                auto *s = static_cast<QuadExtCall*>(stm);
                if (s->args) for (auto *a : *s->args) markEscapedTerm(a);
                break;
            }
            case QuadKind::MOVE_EXTCALL: {
                auto *s = static_cast<QuadMoveExtCall*>(stm);
                if (s->extcall && s->extcall->extfun != "malloc" && s->extcall->args)
                    for (auto *a : *s->extcall->args) markEscapedTerm(a);
                break;
            }
            case QuadKind::RETURN: {
                markEscapedTerm(static_cast<QuadReturn*>(stm)->exp);
                break;
            }
            case QuadKind::STORE: {
                auto *s = static_cast<QuadStore*>(stm);
                int dst = termTempNum(s->dst);
                int src = termTempNum(s->src);
                if (object_temps.count(src)) escaped.insert(src);
                if (addr_to_field.count(src)) escaped.insert(addr_to_field[src].first);
                if (!addr_to_field.count(dst)) markEscapedTerm(s->dst);
                break;
            }
            case QuadKind::PTR_CALC: {
                auto *s = static_cast<QuadPtrCalc*>(stm);
                int base = termTempNum(s->ptr);
                int off = 0;
                if (object_temps.count(base) && !isConstTerm(s->offset, &off))
                    escaped.insert(base);
                break;
            }
            default:
                break;
        }
    }
}
```

第三阶段做跨块字段值传播和重写。核心是给每个基本块维护两个状态：

- `in_state[label]`：进入该块时，每个对象字段 `(obj, offset)` 当前可用的值。
- `out_state[label]`：执行完该块后，每个对象字段当前可用的值。

状态类型可以直接用 `map<pair<int,int>, QuadTerm*>`。交汇规则必须保守：

- 入口块 `in_state` 为空。
- 非入口块的 `in_state` 是所有前驱 `out_state` 的交集。
- 只有当所有前驱都记录了同一字段、且字段值完全相同，才保留这个字段值。
- 只要某个前驱没有这个字段，或者不同前驱给出的值不同，就删除该字段，表示未知。

先加几个 helper：

```cpp
using FieldKey = pair<int,int>; // (object temp, field offset)
using FieldState = map<FieldKey, QuadTerm*>;

static string termKey(QuadTerm *term) {
    if (term == nullptr) return "<null>";
    return term->print();
}

static bool sameTerm(QuadTerm *a, QuadTerm *b) {
    return termKey(a) == termKey(b);
}

static bool sameState(const FieldState &a, const FieldState &b) {
    if (a.size() != b.size()) return false;
    for (auto &[k, av] : a) {
        auto it = b.find(k);
        if (it == b.end()) return false;
        if (!sameTerm(av, it->second)) return false;
    }
    return true;
}

static FieldState cloneState(const FieldState &src) {
    FieldState dst;
    for (auto &[k, v] : src) dst[k] = v->clone();
    return dst;
}

static FieldState meetPredStates(const vector<int> &preds,
                                 const map<int, FieldState> &out_state,
                                 int entry_label) {
    FieldState result;
    bool first = true;
    for (int pred : preds) {
        auto it = out_state.find(pred);
        if (it == out_state.end()) return FieldState(); // 前驱还没有信息，保守未知

        if (first) {
            result = cloneState(it->second);
            first = false;
            continue;
        }

        for (auto rit = result.begin(); rit != result.end(); ) {
            auto jt = it->second.find(rit->first);
            if (jt == it->second.end() || !sameTerm(rit->second, jt->second))
                rit = result.erase(rit);
            else
                ++rit;
        }
    }
    return result;
}
```

再准备 CFG 的前驱表。HW8 的 `QuadBlock` 里有 `exit_labels`，足够现场使用：

```cpp
map<int, QuadBlock*> label2block_local;
map<int, vector<int>> preds;

for (auto *block : *func->quadblocklist) {
    int label = block->entry_label->num;
    label2block_local[label] = block;
}

for (auto *block : *func->quadblocklist) {
    int from = block->entry_label->num;
    if (!block->exit_labels) continue;
    for (auto *lab : *block->exit_labels) {
        if (lab && label2block_local.count(lab->num))
            preds[lab->num].push_back(from);
    }
}
```

定义一个“模拟执行 block 更新字段状态”的函数。它不改语句，只计算该块出口的字段值：

```cpp
auto transferBlock = [&](QuadBlock *block, const FieldState &input) {
    FieldState state = cloneState(input);

    for (auto *stm : *block->quadlist) {
        if (stm->kind == QuadKind::STORE) {
            auto *s = static_cast<QuadStore*>(stm);
            int addr = termTempNum(s->dst);
            if (addr_to_field.count(addr)) {
                FieldKey key = addr_to_field[addr];
                if (!escaped.count(key.first)) {
                    state[key] = s->src->clone();
                    continue;
                }
            }

            // 任何不能理解的内存写都可能改对象字段，保守清空。
            int dst = termTempNum(s->dst);
            if (!addr_to_field.count(dst))
                state.clear();
        } else if (stm->kind == QuadKind::CALL || stm->kind == QuadKind::MOVE_CALL ||
                   stm->kind == QuadKind::EXTCALL || stm->kind == QuadKind::MOVE_EXTCALL) {
            // 如果逃逸分析足够强，这里可以不清空；现场保守清空，避免调用修改内存。
            state.clear();
        }
    }

    return state;
};
```

然后做不动点迭代。注意这只是函数内跨块，不跨函数：

```cpp
map<int, FieldState> in_state;
map<int, FieldState> out_state;

if (!func->quadblocklist->empty()) {
    int entry = func->quadblocklist->front()->entry_label->num;
    for (auto *block : *func->quadblocklist) {
        int label = block->entry_label->num;
        in_state[label] = FieldState();
        out_state[label] = FieldState();
    }

    bool changed = true;
    while (changed) {
        changed = false;

        for (auto *block : *func->quadblocklist) {
            int label = block->entry_label->num;

            FieldState new_in;
            if (label == entry) {
                new_in = FieldState();
            } else {
                new_in = meetPredStates(preds[label], out_state, entry);
            }

            FieldState new_out = transferBlock(block, new_in);

            if (!sameState(in_state[label], new_in)) {
                in_state[label] = cloneState(new_in);
                changed = true;
            }
            if (!sameState(out_state[label], new_out)) {
                out_state[label] = cloneState(new_out);
                changed = true;
            }
        }
    }
}
```

最后按 `in_state` 重写每个块。这里才真正删除 `STORE`、把 `LOAD` 改成 `MOVE`：

```cpp
for (auto *block : *func->quadblocklist) {
    int label = block->entry_label->num;
    FieldState field_value = cloneState(in_state[label]);
    auto *new_list = new vector<QuadStm*>();

    for (auto *stm : *block->quadlist) {
        if (stm->kind == QuadKind::STORE) {
            auto *s = static_cast<QuadStore*>(stm);
            int addr = termTempNum(s->dst);
            if (addr_to_field.count(addr)) {
                auto key = addr_to_field[addr];
                if (!escaped.count(key.first)) {
                    field_value[key] = s->src->clone();
                    continue; // 删除 STORE
                }
            }
        }

        if (stm->kind == QuadKind::LOAD) {
            auto *s = static_cast<QuadLoad*>(stm);
            int addr = termTempNum(s->src);
            if (addr_to_field.count(addr)) {
                auto key = addr_to_field[addr];
                if (!escaped.count(key.first) && field_value.count(key)) {
                    auto *def = new set<Temp*>();
                    def->insert(new Temp(s->dst->temp->num));
                    auto *use = new set<Temp*>();
                    if (field_value[key]->kind == QuadTermKind::TEMP)
                        use->insert(new Temp(field_value[key]->get_temp()->temp->num));
                    new_list->push_back(new QuadMove(s->dst->clone(), field_value[key]->clone(), def, use));
                    continue;
                }
            }
        }

        if (stm->kind == QuadKind::CALL || stm->kind == QuadKind::MOVE_CALL ||
            stm->kind == QuadKind::EXTCALL || stm->kind == QuadKind::MOVE_EXTCALL) {
            field_value.clear();
            new_list->push_back(static_cast<QuadStm*>(stm->clone()));
            continue;
        }

        new_list->push_back(static_cast<QuadStm*>(stm->clone()));
    }
    block->quadlist = new_list;
}
```

这个跨块版本能处理常见情况：

```text
L1:
  obj = malloc(...)
  addr = obj + 0
  STORE 7 -> Mem(addr)
  JUMP L2
L2:
  LOAD t <- Mem(addr)
```

`L2` 的 `in_state` 会从 `L1` 的 `out_state` 得到 `(obj,0)=Const:7`，所以 `LOAD` 可以改成 `MOVE t <- Const:7`。

也能保守处理分支：

```text
L1 -> L2, L3
L2: STORE 7 -> field; JUMP L4
L3: STORE 8 -> field; JUMP L4
L4: LOAD t <- field
```

`L4` 两个前驱给出的字段值不同，meet 后删除该字段，`LOAD` 保留不改。如果两个前驱都 STORE 7，则可改成 `MOVE t <- Const:7`。

如果题目要求把对象“完全不存内存”，还可以删除：

- 不逃逸对象的 `MOVE_EXTCALL malloc`；
- 只为该对象字段服务的 `PTR_CALC`。

但这一步更危险，因为对象可能用于虚函数表、数组长度、运行时布局。现场除非题目明确“只考虑无方法调用、无返回、无别名的普通字段读写对象”，否则建议只删 `STORE`、把 `LOAD` 改 `MOVE`，保留 `malloc/PTR_CALC`，这样正确性更稳。

报告必须写：

- 只优化不逃逸对象。
- 对象传参、返回、存入内存、地址不确定、字段地址外传时都放弃优化。
- 字段 offset 必须是常量。
- CALL 可能保存对象指针或修改对象字段，因此一律视为逃逸。
- 跨块版本使用 dataflow meet：所有前驱同值才传播，不同值或缺失值则保守未知。
- 循环中如果字段值不断变化，迭代会在 meet 后退化为未知；不要试图现场给字段值造 PHI，除非题目明确要求。

### 预测 2：HW8 加除零检查

#### 题目

在 HW8 优化阶段中，对所有整数除法 `/` 和取模 `%` 增加检查：如果右操作数可能为 0，则运行时调用 `exit(-102)`；如果编译期可知非 0，则不插检查；如果编译期可知为 0，则可以直接插入 `exit(-102)` 并保留/删除后续语句按题面要求。

#### 解答

需要改 `HW8/lib/opt/opt.cc` 的 `modifyFunc()`，在 `MOVE_BINOP` 分支里处理。辅助函数：

```cpp
static QuadExtCall *makeExitCall(int code) {
    auto *args = new vector<QuadTerm*>();
    args->push_back(new QuadTerm(code));
    return new QuadExtCall("exit", args, nullptr, nullptr);
}
```

如果题目只要求“能在 IR 中表达”，最简单处理：

- `right` 是 `ONE_VALUE(nonzero)`：什么都不加。
- `right` 是 `ONE_VALUE(0)`：在原语句前插 `EXTCALL exit(Const:-102)`。
- `right` 是 `MANY_VALUES/NO_VALUE`：严格来说需要条件分支检查，但 `modifyFunc()` 里临时造 CFG 很麻烦。90 分钟保守版本可以不化简这条除法，并报告“未知时应插运行时条件检查；若不允许新增 label，则保留原语句并不折叠”。如果题目明确要求插，就需要造新 label 和 blocks。

可执行的简单代码插入点：

```cpp
case QuadKind::MOVE_BINOP: {
    auto *binop = static_cast<QuadMoveBinop*>(stm);
    bool is_div = binop->binop == "/" || binop->binop == "%";
    if (is_div) {
        RtValue rv = evalTerm(this, binop->right);
        if (rv.getType() == ValueType::ONE_VALUE && rv.getIntValue() == 0) {
            new_quadlist->push_back(makeExitCall(-102));
        }
    }
    if (getRtValue(binop->dst->temp->num).getType() == ValueType::ONE_VALUE) break;
    auto *nb = static_cast<QuadMoveBinop*>(stm->clone());
    nb->left = rewriteTerm(this, nb->left);
    nb->right = rewriteTerm(this, nb->right);
    new_quadlist->push_back(nb);
    break;
}
```

如果要完整支持未知右操作数，需要新增两个 label 和两个 block：

- 当前块中把 `t = a / b` 替换成 `CJUMP == b, Const:0 ? Lexit : Lok`。
- 新增 `Lexit` block：`EXTCALL exit(Const:-102)`，再 `JUMP Lok` 或 `RETURN`，看 IR 是否允许 exit 不返回。
- 新增/继续 `Lok` block：执行原除法。

这会破坏当前 `modifyFunc()` 的单块重写结构，现场风险很高。除非题目明确给了插桩方式，否则优先写编译期可确定除零检查。

报告重点：

- `/` 和 `%` 都要处理。
- 除数常量非 0 不插。
- 除数常量 0 必须报错或插 `exit(-102)`。
- 除数未知时完整方案需要 CFG 拆块；如果本实现没拆块，要说明保守选择。

### 预测 3：HW8 输出优化统计

#### 题目

在 SCCP 优化后输出统计信息：删除了多少不可达块、删除了多少常量赋值、化简了多少条件跳转、删除或简化了多少 PHI。

#### 解答

改 `HW8/include/opt/opt.hh`，给 `Opt` 加成员：

```cpp
int stat_removed_blocks = 0;
int stat_removed_const_defs = 0;
int stat_simplified_cjump = 0;
int stat_removed_phi = 0;
int stat_phi_to_move = 0;
```

在 `modifyFunc()` 中计数：

```cpp
if (!isExecutable(block_executable, curr)) {
    stat_removed_blocks++;
    continue;
}
```

`MOVE` / `MOVE_BINOP`：

```cpp
if (getRtValue(move->dst->temp->num).getType() == ValueType::ONE_VALUE) {
    stat_removed_const_defs++;
    break;
}
```

`CJUMP`：

```cpp
if (t_exec && !f_exec) {
    stat_simplified_cjump++;
    ...
} else if (!t_exec && f_exec) {
    stat_simplified_cjump++;
    ...
}
```

`PHI`：

```cpp
if (phi_val.getType() == ValueType::ONE_VALUE) {
    stat_removed_phi++;
    break;
}
if (new_args->size() == 1) {
    stat_phi_to_move++;
    ...
}
```

在 `optFunc()` 末尾输出：

```cpp
cerr << "Opt stats for " << func->funcname
     << ": removed_blocks=" << stat_removed_blocks
     << ", removed_const_defs=" << stat_removed_const_defs
     << ", simplified_cjump=" << stat_simplified_cjump
     << ", removed_phi=" << stat_removed_phi
     << ", phi_to_move=" << stat_phi_to_move
     << endl;
```

注意：不要输出到 `.4-ssa-opt.quad` 文件，只能 `cerr` 或 `cout`。如果评测严格比对 stdout，最好用 `cerr`。

报告重点：

- 统计点必须和实际修改点一致。
- `PHI` 删除和 `PHI -> MOVE` 分开统计。
- 不可达块删除按 block 计，不按语句计。

### 预测 4：HW8 只对可达路径报警

#### 题目

在 SCCP 中，如果某 temp 在可达路径上被使用时仍然是 `NO_VALUE`，输出 warning 并把它提升为 `MANY_VALUES`；不可达块中的同类 use 不报警。

#### 解答

你的当前 HW8 基本已经做了。现场如果重做，核心是：

1. 在 `calculateBT()` 中只扫描 `block_executable[label] == true` 的 block。
2. 普通 use 使用 `evalTermChecked()`：

```cpp
static RtValue evalTermChecked(Opt *opt, QuadTerm *term, bool &changed) {
    if (term == nullptr || term->kind != QuadTermKind::TEMP) return evalTerm(opt, term);
    int num = term->get_temp()->temp->num;
    RtValue val = opt->getRtValue(num);
    if (val.getType() == ValueType::NO_VALUE) {
        cerr << "Warning: t" << num
             << " used in reachable block with no determined value; promoting to MANY_VALUES"
             << endl;
        changed |= updateRtValue(opt->temp_value, num, RtValue(ValueType::MANY_VALUES));
        return RtValue(ValueType::MANY_VALUES);
    }
    return val;
}
```

3. `PHI` 不能简单把所有参数都算 use，只能看可执行边：

```cpp
if (!isEdgeExecutable(opt, pred_label, curr_label)) continue;
```

4. 如果要避免 dead phi 误报警，在 `modifyFunc()` 先收集可达块中的 `live_temps`，只有 phi 结果 live 时才对其输入报警。

报告重点：

- 不可达块不执行，所以不报警。
- PHI 的输入是否可达取决于边，不只取决于前驱块。
- 报警后提升为 `MANY_VALUES` 是为了让分析继续，并避免错误折叠。

### 预测 5：HW8 常量分支后同步裁剪 PHI

#### 题目

当 SCCP 判定 `CJUMP` 条件为常量并把它改成 `JUMP` 后，后继块中 PHI 的参数必须删除来自不可达边的输入。如果 PHI 只剩一个有效输入，应改成 `MOVE`；如果 PHI 结果是常量，应直接删除 PHI 并替换使用点。

#### 解答

你的当前 HW8 已经有完整实现。现场要记住三件事：

1. 判断边是否可执行不能只看 predecessor block，要看 predecessor 末尾的 `CJUMP` 是否被折叠：

```cpp
static bool isEdgeExecutable(Opt *opt, int pred, int curr) {
    if (!isExecutable(opt->block_executable, pred)) return false;
    auto *block = opt->label2block[pred];
    for (auto *s : *block->quadlist) {
        if (s->kind == QuadKind::JUMP)
            return static_cast<QuadJump*>(s)->label->num == curr;
        if (s->kind == QuadKind::CJUMP) {
            auto *cj = static_cast<QuadCJump*>(s);
            RtValue lv = evalTerm(opt, cj->left);
            RtValue rv = evalTerm(opt, cj->right);
            if (lv.getType() == ValueType::ONE_VALUE && rv.getType() == ValueType::ONE_VALUE) {
                bool taken = evalRelop(cj->relop, lv.getIntValue(), rv.getIntValue());
                return (taken ? cj->t->num : cj->f->num) == curr;
            }
            return true;
        }
    }
    return true;
}
```

2. 重写 PHI 时过滤参数：

```cpp
auto *new_args = new vector<pair<Temp*, Label*>>();
for (auto &arg : *phi->args) {
    int pred = arg.second->num;
    if (!isEdgeExecutable(this, pred, curr)) continue;
    new_args->push_back({new Temp(arg.first->num), new Label(pred)});
}
```

3. 根据剩余参数数目处理：

```cpp
if (phi_val.getType() == ValueType::ONE_VALUE) break;
if (new_args->empty()) break;
if (new_args->size() == 1) {
    int input_num = new_args->at(0).first->num;
    new_quadlist->push_back(new QuadMove(
        phi->temp_exp->clone(),
        new QuadTerm(new QuadTemp(new Temp(input_num), phi->temp_exp->type)),
        nullptr, nullptr));
} else {
    auto *np = static_cast<QuadPhi*>(stm->clone());
    np->args = new_args;
    new_quadlist->push_back(np);
}
```

重要坑：如果某个 PHI 结果是 `MANY_VALUES`，但其中一个输入 temp 的定义因“常量赋值删除”被删了，PHI 会引用悬空 temp。你的 HW8 用 `fresh_moves` 解决：在前驱跳转前插一个 fresh temp 保存常量，然后 PHI 引用 fresh temp。

报告重点：

- PHI 参数对应 CFG 边。
- 删除边必须同步删 PHI 输入。
- 只剩一个输入时 PHI 退化为 MOVE。
- 常量定义被删后，如果 PHI 还需要 temp 形式的输入，要补 fresh temp。

### 预测 6：HW8 限制优化范围，不删除有副作用语句

#### 题目

修改 SCCP，使其只删除纯计算产生的常量赋值，不删除可能有副作用的语句。特别是 `STORE`、`CALL`、`EXTCALL`、`MOVE_CALL`、`MOVE_EXTCALL` 必须保留。

#### 解答

这个主要是规则题。`modifyFunc()` 中保持：

- 可以删：
  - `MOVE dst <- Const/temp` 且 `dst` 已知常量。
  - `MOVE_BINOP dst <- ...` 且 `dst` 已知常量，并且不是除零风险。
  - `PHI` 结果已知常量。
- 不能删：
  - `STORE`：写内存。
  - `CALL` / `EXTCALL`：有副作用。
  - `MOVE_CALL` / `MOVE_EXTCALL`：即使返回值不用，调用本身有副作用。
  - `LOAD`：保守保留，可能涉及非法地址或 volatile。
  - `PTR_CALC`：一般可保留；如果题面要求死代码删除，再用 use-def 判断。

代码上检查当前 `modifyFunc()`：

```cpp
case QuadKind::MOVE_CALL: {
    auto *mc = static_cast<QuadMoveCall*>(stm->clone());
    rewriteCallLike(this, mc->call);
    new_quadlist->push_back(mc);
    break;
}
```

不要因为 `move_call->dst` 后续 unused 或 `MANY_VALUES` 就删。

报告重点：

- SCCP 的常量传播不等于死代码删除。
- 有副作用语句即使结果不用也要保留。
- 不确定是否有副作用时保守保留。

### 预测 7：HW7 SSA 正确性检查器

#### 题目

在 HW7 `quad2ssa` 生成 SSA 后增加检查器，若输出不满足 SSA 性质则报错。至少检查：

- 非参数 temp 在函数内最多定义一次。
- 每个 use 要么是参数，要么已有定义。
- PHI 节点位于 block 开头的 LABEL 之后、普通语句之前。
- PHI 的每个参数 label 都必须是当前 block 的前驱。

#### 解答

改 `HW7/lib/quad/quadssa.cc`。

新增 helper：

```cpp
static bool checkSSA(QuadFuncDecl *func, ControlFlowInfo *domInfo) {
    bool ok = true;
    set<int> params;
    if (func->params) for (auto *p : *func->params) params.insert(p->num);

    map<int, QuadStm*> defs;
    vector<pair<int, QuadStm*>> uses;

    for (auto *block : *func->quadblocklist) {
        bool seen_non_phi = false;
        int blabel = block->entry_label->num;

        for (auto *stm : *block->quadlist) {
            if (stm->kind == QuadKind::LABEL) continue;

            if (stm->kind == QuadKind::PHI) {
                if (seen_non_phi) {
                    cerr << "SSA error: PHI after non-PHI in block L" << blabel << endl;
                    ok = false;
                }
                auto *phi = static_cast<QuadPhi*>(stm);
                if (phi->args) for (auto &arg : *phi->args) {
                    int pred = arg.second->num;
                    if (!domInfo->predecessors[blabel].count(pred)) {
                        cerr << "SSA error: PHI arg from non-predecessor L" << pred
                             << " in block L" << blabel << endl;
                        ok = false;
                    }
                }
            } else {
                seen_non_phi = true;
            }

            if (stm->def) for (auto *t : *stm->def) {
                if (params.count(t->num)) continue;
                if (defs.count(t->num)) {
                    cerr << "SSA error: t" << t->num << " defined more than once" << endl;
                    ok = false;
                } else {
                    defs[t->num] = stm;
                }
            }
            if (stm->use) for (auto *t : *stm->use)
                uses.push_back({t->num, stm});
        }
    }

    for (auto &[u, stm] : uses) {
        if (!params.count(u) && !defs.count(u)) {
            cerr << "SSA error: t" << u << " used without definition" << endl;
            ok = false;
        }
    }
    return ok;
}
```

在 `quad2ssa()` 中调用：

```cpp
cleanupUnusedPhi(funcdecl);
checkSSA(funcdecl, domInfo);
```

如果题目要求发现第一个错误就停止，可以：

```cpp
if (!checkSSA(funcdecl, domInfo)) return nullptr;
```

报告重点：

- 参数是入口已有值，不要求函数内定义。
- SSA 单定义按 temp 编号检查。
- PHI use 虽然语义上来自前驱边，但仍必须有定义或是参数。
- PHI 参数 label 必须匹配 CFG predecessor。

### 预测 8：HW7 只对 int temp 做 SSA，ptr temp 不版本化

#### 题目

修改 HW7，使 SSA 转换只对 `int` 类型 temp 生效，`ptr` 类型 temp 不插 PHI、不重命名版本。

#### 解答

最稳的改法是在“收集待版本化变量”时就过滤类型。

1. `placePhi()` 已经有 `varTypes`：

```cpp
if (varTypes.count(v) && varTypes[v] != QuadType::INT) continue;
```

放在：

```cpp
for (int v : liveness->allVars) {
    if (!defBlocksMap.count(v)) continue;
    if (varTypes.count(v) && varTypes[v] != QuadType::INT) continue;
    ...
}
```

2. `renameVariables()` 中收集 `vars` 时要过滤定义类型。新增 helper：

```cpp
static bool isIntDef(QuadStm *stm, int temp_num) {
    switch (stm->kind) {
        case QuadKind::MOVE:
            return static_cast<QuadMove*>(stm)->dst->type == QuadType::INT;
        case QuadKind::LOAD:
            return static_cast<QuadLoad*>(stm)->dst->type == QuadType::INT;
        case QuadKind::MOVE_BINOP:
            return static_cast<QuadMoveBinop*>(stm)->dst->type == QuadType::INT;
        case QuadKind::MOVE_CALL:
            return static_cast<QuadMoveCall*>(stm)->dst->type == QuadType::INT;
        case QuadKind::MOVE_EXTCALL:
            return static_cast<QuadMoveExtCall*>(stm)->dst->type == QuadType::INT;
        case QuadKind::PHI:
            return static_cast<QuadPhi*>(stm)->temp_exp->type == QuadType::INT;
        case QuadKind::PTR_CALC:
            return false;
        default:
            return false;
    }
}
```

然后：

```cpp
for (auto* t : *(stm->def))
    if (!paramSet.count(t->num) && isIntDef(stm, t->num))
        vars.insert(t->num);
```

只要 `vars` 中没有 ptr temp，`versionUse` 和 `versionDef` 的这句会自然跳过 ptr：

```cpp
if (paramSet.count(num) || !vars.count(num)) return temp;
```

报告重点：

- 插 PHI 和重命名必须用同一套过滤规则。
- 不能只过滤 def 或只过滤 use。
- `PTR_CALC` 的 dst 是 ptr，不进 SSA。

### 预测 9：HW7 拷贝传播，删除 `MOVE t2 <- t1`

#### 题目

在 SSA 输出后做一个简单拷贝传播：对于 `MOVE t2 <- t1`，把后续所有 `t2` 的使用替换成 `t1`，并删除该 MOVE。

#### 解答

改 `HW7/lib/quad/quadssa.cc`，在 `cleanupUnusedPhi(funcdecl)` 后调用 `copyPropSSA(funcdecl)`，然后 `rebuildDefUseSets(funcdecl)`。

核心 helper：

```cpp
static int resolveAlias(int t, map<int,int> &alias) {
    set<int> seen;
    while (alias.count(t) && !seen.count(t)) {
        seen.insert(t);
        t = alias[t];
    }
    return t;
}

static void replaceTermAlias(QuadTerm *term, map<int,int> &alias) {
    if (!term || term->kind != QuadTermKind::TEMP) return;
    int old = term->get_temp()->temp->num;
    int rep = resolveAlias(old, alias);
    if (rep != old) term->get_temp()->temp = new Temp(rep);
}
```

遍历函数：

```cpp
static void copyPropSSA(QuadFuncDecl *func) {
    map<int,int> alias;

    for (auto *block : *func->quadblocklist) {
        for (auto *stm : *block->quadlist) {
            if (stm->kind == QuadKind::MOVE) {
                auto *s = static_cast<QuadMove*>(stm);
                if (s->src && s->src->kind == QuadTermKind::TEMP)
                    alias[s->dst->temp->num] = s->src->get_temp()->temp->num;
            }
        }
    }

    for (auto *block : *func->quadblocklist) {
        auto *new_list = new vector<QuadStm*>();
        for (auto *stm : *block->quadlist) {
            // 先替换各种 use，类似 renameUses 的 switch。
            replaceUsesByAlias(stm, alias);

            if (stm->kind == QuadKind::MOVE) {
                auto *s = static_cast<QuadMove*>(stm);
                if (s->src && s->src->kind == QuadTermKind::TEMP && alias.count(s->dst->temp->num))
                    continue;
            }
            new_list->push_back(stm);
        }
        block->quadlist = new_list;
    }
    rebuildDefUseSets(func);
}
```

`replaceUsesByAlias` 可以复制 `renameUses` 的 switch 结构，把 `renameTermUse(...)` 换成 `replaceTermAlias(...)`。PHI 参数也要处理：

```cpp
case QuadKind::PHI: {
    auto *s = static_cast<QuadPhi*>(stm);
    for (auto &arg : *s->args) {
        int rep = resolveAlias(arg.first->num, alias);
        arg.first = new Temp(rep);
    }
    break;
}
```

报告重点：

- SSA 中每个 temp 单定义，所以 copy propagation 简单。
- 只处理 temp-to-temp MOVE，不处理 LOAD/CALL/PTR。
- PHI 参数也属于 use，必须替换。
- 用 `seen` 防止 alias 链成环。




• 这题本质上是：对象的某个字段，如果我们能一直知道它当前等于什么，就不必真的从内存里读写它。

  比如原来 IR 是：

  t1 = malloc(...)
  t2 = t1 + 0
  STORE 7 -> Mem(t2)
  ...
  LOAD t3 <- Mem(t2)

  如果 t1 这个对象没有被传出去、没有返回、没有被存到别的内存里，那中间没人能偷偷改它的字段。所以我们可以认为：

  (obj=t1, offset=0) 当前值是 Const:7

  后面的 LOAD t3 <- Mem(t2) 就能改成：

  MOVE t3 <- Const:7

  ———

  几个名字的含义

  object_temps

  表示哪些 temp 是对象本体，也就是 malloc 的返回值。

  例如：

  MOVE_EXTCALL t10300:ptr <- malloc(Const:12)

  那么 t10300 就放进 object_temps。

  ———

  addr_to_field

  表示某个地址 temp 对应对象的哪个字段。

  例如：

  PTR_CALC t11100:ptr <- t10300:ptr + Const:8

  意思是 t11100 是 t10300 这个对象偏移量为 8 的字段地址。

  所以记录：

  addr_to_field[t11100] = (t10300, 8)

  后面看到：

  STORE Const:5 -> Mem(t11100)

  就知道这是在写：

  对象 t10300 的 offset=8 字段

  ———

  FieldKey

  就是字段的唯一名字：

  using FieldKey = pair<int,int>;

  第一个 int 是对象 temp 编号，第二个 int 是字段偏移量。

  例如：

  (t10300, 8)

  表示：

  对象 t10300 的 offset 8 字段

  ———

  FieldState

  表示“当前已经知道哪些字段的值”。

  using FieldState = map<FieldKey, QuadTerm*>;

  例如：

  {
    (t10300, 0) -> Const:1,
    (t10300, 4) -> Const:10
  }

  意思是当前程序点上，我们知道：

  对象 t10300 的 offset 0 字段是 1
  对象 t10300 的 offset 4 字段是 10

  ———

  escaped

  表示“对象逃逸了”。

  对象逃逸就是：对象指针跑到当前函数控制不了的地方去了。比如：

  CALL f(obj)
  RETURN obj
  STORE obj -> Mem(x)

  一旦对象逃逸，我们就不能保证没人改它的字段，所以不优化它。

  所以：

  if (escaped.count(obj)) {
      不删 STORE，不改 LOAD
  }

  ———

  为什么要有 in_state 和 out_state

  因为你要求改成跨块版本，所以不能只在一个 basic block 里看。

  我们要知道：

  - 进入某个 block 时，字段值是什么；
  - 执行完某个 block 后，字段值变成什么。

  所以：

  in_state[label]

  表示进入 label 这个 block 时，已知字段值。

  out_state[label]

  表示执行完 label 这个 block 后，已知字段值。

  例子：

  L1:
    STORE 7 -> obj.field
    JUMP L2

  L2:
    LOAD t <- obj.field

  那么：

  out_state[L1] = { obj.field -> 7 }
  in_state[L2] = { obj.field -> 7 }

  所以 L2 里的 LOAD 可以改成 MOVE。

  ———

  meet 是什么

  meet 就是多个前驱 block 汇合时，决定后继入口还能知道什么。

  例子 1：

  L2:
    STORE 7 -> obj.field
    JUMP L4

  L3:
    STORE 7 -> obj.field
    JUMP L4

  L4:
    LOAD t <- obj.field

  L4 有两个前驱 L2 和 L3。

  两个前驱都说：

  obj.field = 7

  所以：

  in_state[L4] = { obj.field -> 7 }

  LOAD 可以改。

  例子 2：

  L2:
    STORE 7 -> obj.field
    JUMP L4

  L3:
    STORE 8 -> obj.field
    JUMP L4

  L4:
    LOAD t <- obj.field

  两个前驱给的值不同：

  L2: obj.field = 7
  L3: obj.field = 8

  那 L4 入口到底是多少取决于运行走哪条路，不能确定。

  所以 meet 后删掉这个字段：

  in_state[L4] = {}

  LOAD 保留。

  例子 3：

  L2:
    STORE 7 -> obj.field
    JUMP L4

  L3:
    JUMP L4

  L4:
    LOAD t <- obj.field

  L3 没写过这个字段。也不能确定。

  所以：

  in_state[L4] = {}

  LOAD 保留。

  总结：所有前驱都有同一个字段值，才传播；否则未知。

  ———

  transferBlock 是什么

  transferBlock 是“模拟执行一个 block”。

  它输入这个 block 入口的 FieldState，输出执行完整个 block 后的 FieldState。

  比如输入：

  state = {}

  block 里有：

  STORE 7 -> obj.field
  STORE 9 -> obj.field2

  执行后：

  state = {
    obj.field -> 7,
    obj.field2 -> 9
  }

  如果看到：

  LOAD t <- obj.field

  它不改变字段状态。

  如果看到未知内存写：

  STORE x -> Mem(unknown_addr)

  保守起见，可能会清空状态：

  state = {}

  因为这个 unknown address 可能刚好指向我们追踪的对象字段。

  如果看到函数调用：

  CALL f(...)

  也保守清空，因为函数可能改内存。

  ———

  整体流程

  实现分四步。

  第一步：找对象。

  MOVE_EXTCALL t <- malloc(...)

  把 t 记为一个对象。

  第二步：找字段地址。

  PTR_CALC addr <- obj + Const:offset

  记录：

  addr_to_field[addr] = (obj, offset)

  第三步：逃逸分析。

  如果对象或字段地址被传出去，就标记逃逸：

  CALL f(obj)
  RETURN obj
  STORE obj -> Mem(x)
  CALL f(field_addr)

  逃逸对象不优化。

  第四步：跨块字段值分析。

  反复计算：

  in_state[block] = meet(所有前驱的 out_state)
  out_state[block] = transferBlock(block, in_state[block])

  直到不再变化。

  第五步：真正改代码。

  进入每个 block 时，用算好的：

  field_value = in_state[block]

  然后顺序扫描语句。

  看到：

  STORE value -> Mem(field_addr)

  如果这个字段属于不逃逸对象：

  field_value[(obj, offset)] = value
  删除这个 STORE

  看到：

  LOAD dst <- Mem(field_addr)

  如果当前 field_value 里有这个字段值：

  MOVE dst <- value

  否则保留 LOAD。

  ———

  为什么不直接删 malloc 和 PTR_CALC

  因为这更危险。

  对象布局里可能有：

  - vtable 指针；
  - 数组长度；
  - 方法调用需要对象地址；
  - 其他没有识别出来的内存行为。

  所以我建议现场保守做：

  删 STORE
  LOAD 改 MOVE
  保留 malloc
  保留 PTR_CALC

  这样即使优化不彻底，也更不容易错。

  ———

  一句话总结

  这个优化就是维护一张表：

  当前程序点：对象的某个字段 = 某个值

  跨 basic block 时，只在所有前驱都同意这个值的情况下继续传播。对象一旦逃逸，或者内存行为看不懂，就放弃优化。










› 如何新分配一个temp编号


• 在这个仓库里，新 temp 编号通常用函数级的：

  int fresh = ++func->last_temp_num;

  然后创建 temp：

  new Temp(fresh)

  如果要作为 QuadTemp：

  auto *qt = new QuadTemp(new Temp(fresh), QuadType::INT);

  如果要作为 QuadTerm：

  auto *term = new QuadTerm(new QuadTemp(new Temp(fresh), QuadType::INT));

  比如插入一条：

  MOVE fresh <- Const:7

  代码是：

  int fresh = ++func->last_temp_num;

  auto *def = new set<Temp*>();
  def->insert(new Temp(fresh));

  auto *move = new QuadMove(
      new QuadTemp(new Temp(fresh), QuadType::INT),
      new QuadTerm(7),
      def,
      new set<Temp*>()
  );

  如果你在 optProg() 里还维护了程序级 last_temp_num，最后也要更新：

  newProg->last_temp_num = max(newProg->last_temp_num, opt_func->last_temp_num);

  不过你当前 HW8 的 optProg() 是直接用原 prog->last_temp_num 初始化 newProg，然后 push 每个优化后的函数。如果现场只在函数内部新增 temp，最关键的是更新：

  func->last_temp_num

  你的 HW8 里已经有类似写法：

  int fresh = ++func->last_temp_num;

  就在 modifyFunc() 处理 PHI 常量输入的 fresh_moves 那段。
