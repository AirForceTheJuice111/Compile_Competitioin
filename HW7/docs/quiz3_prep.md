  # Quiz 3 备考：题目预测与完整答案

  > 基于前两次小测风格分析，针对 HW7（SSA转换）和 HW8（SCCP优化）代码框架的预测题目。
  > 已排除后续作业已覆盖的内容：HW9（LICM）、HW10（Induction Variable + Strength Reduction）、HW11（Out-of-SSA + 指令选择）。
  > 每道题包含：题目描述、核心思路、完整代码、报告要点。

  ---

  ## 概率排序

  | 题目 | 基础HW | 概率 | 理由 |
  |------|--------|------|------|
  | 代数化简 Algebraic Simplification | HW8 | ★★★★★ | 概念简单、case多、有趣边界、SCCP的自然延伸 |
  | 复制传播 Copy Propagation | HW7/HW8 | ★★★★☆ | 经典SSA优化、未被任何HW覆盖 |
  | 全局值编号 GVN/CSE | HW7/HW8 | ★★★☆☆ | 经典SSA优化、难度稍高 |
  | 死代码消除 General DCE | HW7 | ★★★☆☆ | HW8只做了常量DCE，通用DCE未做 |
  | 冗余Load消除 | HW8 | ★★☆☆☆ | 涉及内存别名，报告问题有趣 |

  ---

  ## 前置：必须记住的代码接口

  ```cpp
  // 遍历所有语句
  for (auto* block : *func->quadblocklist) {
      for (auto* stm : *block->quadlist) { ... }
  }

  // 新建 temp
  int fresh = ++func->last_temp_num;

  // 判断语句类型并转型
  if (stm->kind == QuadKind::MOVE_BINOP) {
      auto* s = static_cast<QuadMoveBinop*>(stm);
      // s->dst->temp->num, s->left, s->right, s->binop
  }

  // 判断 QuadTerm 类型
  if (term->kind == QuadTermKind::CONST) { int v = term->get_const(); }
  if (term->kind == QuadTermKind::TEMP)  { int n = term->get_temp()->temp->num; }

  // 删除语句（迭代器写法）
  auto it = block->quadlist->begin();
  while (it != block->quadlist->end()) {
      if (/* 要删除 */) it = block->quadlist->erase(it);
      else ++it;
  }

  // 插入语句（在 pos 之前）
  block->quadlist->insert(pos, new_stm);
  ```

  ---

  ## 题目一：Algebraic Simplification（代数化简）【最高概率】

  ### 题目描述

  > 在 HW8 的 SSA 代码基础上，增加代数化简（Algebraic Simplification）优化 pass。
  > 对 MOVE_BINOP 语句，若操作数中有常量（CONST），则根据代数恒等式化简：
  > 例如 `t = x + 0` 化简为 `t = x`，`t = x * 0` 化简为 `t = 0`，等等。
  > 化简后若结果是一个 MOVE（`t = x` 或 `t = 常量`），则将 MOVE_BINOP 替换为 MOVE。
  > 注意：某些化简在特定条件下不安全（见报告要点）。

  ### 核心思路

  遍历所有 MOVE_BINOP 语句，对每种 binop 检查操作数是否为特定常量，若满足恒等式则替换为 MOVE。
  替换后可能触发新的常量传播机会，因此可以迭代（或单趟即可，视题目要求）。

  ### 完整代码

  ```cpp
  // 返回 nullptr 表示无法化简；否则返回替换用的 QuadMove
  static QuadMove* tryAlgebraicSimplify(QuadMoveBinop* s) {
      QuadTerm* L = s->left;
      QuadTerm* R = s->right;
      const string& op = s->binop;
      bool Lconst = (L->kind == QuadTermKind::CONST);
      bool Rconst = (R->kind == QuadTermKind::CONST);
      int Lv = Lconst ? L->get_const() : 0;
      int Rv = Rconst ? R->get_const() : 0;

      QuadTerm* result = nullptr;

      if (op == "+") {
          if (Rconst && Rv == 0) result = L->clone();       // x + 0 = x
          else if (Lconst && Lv == 0) result = R->clone();  // 0 + x = x
      } else if (op == "-") {
          if (Rconst && Rv == 0) result = L->clone();       // x - 0 = x
          // x - x = 0: 需要两边是同一个 temp
          else if (!Lconst && !Rconst &&
                   L->get_temp()->temp->num == R->get_temp()->temp->num)
              result = new QuadTerm(0);                      // x - x = 0
      } else if (op == "*") {
          if (Rconst && Rv == 1) result = L->clone();       // x * 1 = x
          else if (Lconst && Lv == 1) result = R->clone();  // 1 * x = x
          else if (Rconst && Rv == 0) result = new QuadTerm(0); // x * 0 = 0
          else if (Lconst && Lv == 0) result = new QuadTerm(0); // 0 * x = 0
      } else if (op == "/") {
          if (Rconst && Rv == 1) result = L->clone();       // x / 1 = x
          // 注意：x / x 不能化简为 1（x 可能为 0，导致除零）
          // 注意：0 / x 不能化简为 0（x 可能为 0）
      } else if (op == "%") {
          if (Rconst && Rv == 1) result = new QuadTerm(0);  // x % 1 = 0
          // 注意：x % x 不能化简（x 可能为 0）
      } else if (op == "<<" || op == ">>") {
          if (Rconst && Rv == 0) result = L->clone();       // x << 0 = x, x >> 0 = x
      } else if (op == "&") {
          if (Rconst && Rv == 0) result = new QuadTerm(0);  // x & 0 = 0
          else if (Lconst && Lv == 0) result = new QuadTerm(0);
          // x & -1 (全1) = x: 若需要可加
      } else if (op == "|") {
          if (Rconst && Rv == 0) result = L->clone();       // x | 0 = x
          else if (Lconst && Lv == 0) result = R->clone();
      } else if (op == "^") {
          if (Rconst && Rv == 0) result = L->clone();       // x ^ 0 = x
          else if (Lconst && Lv == 0) result = R->clone();
          else if (!Lconst && !Rconst &&
                   L->get_temp()->temp->num == R->get_temp()->temp->num)
              result = new QuadTerm(0);                      // x ^ x = 0
      }

      if (!result) return nullptr;
      return new QuadMove(s->dst->clone(), result, nullptr, nullptr);
  }

  static void algebraicSimplification(QuadFuncDecl* func) {
      for (auto* block : *func->quadblocklist) {
          auto it = block->quadlist->begin();
          while (it != block->quadlist->end()) {
              if ((*it)->kind == QuadKind::MOVE_BINOP) {
                  auto* s = static_cast<QuadMoveBinop*>(*it);
                  QuadMove* simplified = tryAlgebraicSimplify(s);
                  if (simplified) {
                      // 同步 def/use
                      auto* defSet = new set<Temp*>();
                      defSet->insert(simplified->dst->temp);
                      auto* useSet = new set<Temp*>();
                      if (simplified->src->kind == QuadTermKind::TEMP)
                          useSet->insert(simplified->src->get_temp()->temp);
                      simplified->def = defSet;
                      simplified->use = useSet;
                      it = block->quadlist->erase(it);
                      it = block->quadlist->insert(it, simplified);
                  }
              }
              ++it;
          }
      }
  }
  ```

  ### 调用位置

  在 `optFunc()` 中 `modifyFunc()` 之后调用：

  ```cpp
  calculateBT();
  modifyFunc();
  algebraicSimplification(func);  // 新增
  return func;
  ```

  ### 报告要点

  **需要处理的情况（逐一列出）：**
  - `x + 0 = x`，`0 + x = x`
  - `x - 0 = x`，`x - x = 0`
  - `x * 1 = x`，`1 * x = x`，`x * 0 = 0`，`0 * x = 0`
  - `x / 1 = x`
  - `x % 1 = 0`
  - `x << 0 = x`，`x >> 0 = x`
  - `x & 0 = 0`，`x | 0 = x`，`x ^ 0 = x`，`x ^ x = 0`

  **不能化简的边界情况（报告必答）：**
  - `x / x` **不能**化简为 1：若 x = 0 则除零错误，语义改变
  - `0 / x` **不能**化简为 0：若 x = 0 则除零错误
  - `x % x` **不能**化简为 0：同上
  - `x - x` 可以化简为 0（SSA 下两个 temp 编号相同即同一值，且减法无副作用）

  ---

  ## 题目二：Copy Propagation（复制传播）【高概率】

  ### 题目描述

  > 在 HW7 或 HW8 的 SSA 代码基础上，增加复制传播（Copy Propagation）优化。
  > 对形如 `t_dst = t_src`（MOVE 语句，src 是 TEMP）的语句，
  > 将程序中所有对 `t_dst` 的使用替换为 `t_src`，然后删除该 MOVE 语句。
  > SSA 形式下此操作永远安全。迭代执行直到不动点。

  ### 核心思路

  1. 扫描所有 MOVE，找 src 是 TEMP 的，建立 `copy_map: t_dst -> t_src`
  2. 追踪 copy chain（`t3=t2, t2=t1` → t3 最终替换为 t1）
  3. 遍历所有语句，把 use 里的 temp 按 copy_map 替换
  4. 删掉已传播的 MOVE
  5. 重复直到不动点

  ### 完整代码

  ```cpp
  static int findCopyRoot(const map<int,int>& copy_map, int num) {
      while (copy_map.count(num)) num = copy_map.at(num);
      return num;
  }

  static void propagateTerm(QuadTerm* term, const map<int,int>& copy_map) {
      if (!term || term->kind != QuadTermKind::TEMP) return;
      int root = findCopyRoot(copy_map, term->get_temp()->temp->num);
      if (root != term->get_temp()->temp->num)
          term->get_temp()->temp = new Temp(root);
  }

  static void copyPropagation(QuadFuncDecl* func) {
      bool changed = true;
      while (changed) {
          changed = false;

          // Step 1: 建立 copy_map
          map<int,int> copy_map;
          for (auto* block : *func->quadblocklist)
              for (auto* stm : *block->quadlist) {
                  if (stm->kind != QuadKind::MOVE) continue;
                  auto* mv = static_cast<QuadMove*>(stm);
                  if (mv->src->kind != QuadTermKind::TEMP) continue;
                  copy_map[mv->dst->temp->num] = mv->src->get_temp()->temp->num;
              }
          if (copy_map.empty()) break;

          // Step 2: 替换所有 use
          for (auto* block : *func->quadblocklist) {
              for (auto* stm : *block->quadlist) {
                  switch (stm->kind) {
                      case QuadKind::MOVE: {
                          auto* s = static_cast<QuadMove*>(stm);
                          propagateTerm(s->src, copy_map); break;
                      }
                      case QuadKind::LOAD: {
                          auto* s = static_cast<QuadLoad*>(stm);
                          propagateTerm(s->src, copy_map); break;
                      }
                      case QuadKind::STORE: {
                          auto* s = static_cast<QuadStore*>(stm);
                          propagateTerm(s->src, copy_map);
                          propagateTerm(s->dst, copy_map); break;
                      }
                      case QuadKind::MOVE_BINOP: {
                          auto* s = static_cast<QuadMoveBinop*>(stm);
                          propagateTerm(s->left, copy_map);
                          propagateTerm(s->right, copy_map); break;
                      }
                      case QuadKind::CALL: {
                          auto* s = static_cast<QuadCall*>(stm);
                          propagateTerm(s->obj_term, copy_map);
                          if (s->args) for (auto* a : *s->args) propagateTerm(a, copy_map);
                          break;
                      }
                      case QuadKind::MOVE_CALL: {
                          auto* s = static_cast<QuadMoveCall*>(stm);
                          if (s->call) {
                              propagateTerm(s->call->obj_term, copy_map);
                              if (s->call->args) for (auto* a : *s->call->args) propagateTerm(a, copy_map);
                          }
                          break;
                      }
                      case QuadKind::EXTCALL: {
                          auto* s = static_cast<QuadExtCall*>(stm);
                          if (s->args) for (auto* a : *s->args) propagateTerm(a, copy_map);
                          break;
                      }
                      case QuadKind::MOVE_EXTCALL: {
                          auto* s = static_cast<QuadMoveExtCall*>(stm);
                          if (s->extcall && s->extcall->args)
                              for (auto* a : *s->extcall->args) propagateTerm(a, copy_map);
                          break;
                      }
                      case QuadKind::CJUMP: {
                          auto* s = static_cast<QuadCJump*>(stm);
                          propagateTerm(s->left, copy_map);
                          propagateTerm(s->right, copy_map); break;
                      }
                      case QuadKind::RETURN: {
                          auto* s = static_cast<QuadReturn*>(stm);
                          propagateTerm(s->exp, copy_map); break;
                      }
                      case QuadKind::PHI: {
                          auto* s = static_cast<QuadPhi*>(stm);
                          if (s->args) for (auto& [t, l] : *s->args) {
                              int root = findCopyRoot(copy_map, t->num);
                              if (root != t->num) t = new Temp(root);
                          }
                          break;
                      }
                      case QuadKind::PTR_CALC: {
                          auto* s = static_cast<QuadPtrCalc*>(stm);
                          propagateTerm(s->ptr, copy_map);
                          propagateTerm(s->offset, copy_map); break;
                      }
                      default: break;
                  }
              }
          }

          // Step 3: 删除已传播的 MOVE
          for (auto* block : *func->quadblocklist) {
              auto it = block->quadlist->begin();
              while (it != block->quadlist->end()) {
                  if ((*it)->kind == QuadKind::MOVE) {
                      auto* mv = static_cast<QuadMove*>(*it);
                      if (mv->src->kind == QuadTermKind::TEMP &&
                          copy_map.count(mv->dst->temp->num)) {
                          it = block->quadlist->erase(it);
                          changed = true;
                          continue;
                      }
                  }
                  ++it;
              }
          }
      }
  }
  ```

  ### 报告要点

  **需要处理的情况：**
  - 只对 `MOVE t_dst = t_src`（src 是 TEMP）建立 copy 关系
  - PHI 节点的 args 也需要替换
  - 需要追踪 copy chain（`t3=t2, t2=t1` → t3 替换为 t1）

  **为什么非 SSA 形式下 copy propagation 更复杂？（报告必答）**
  非 SSA 下，`t_src` 在 `t_dst = t_src` 之后可能被重新赋值，此时不能将后续的 `t_dst` 替换为 `t_src`。需要做到达定义分析（reaching
  definitions）确认在每个使用点，`t_src` 的值仍然是 copy 时的值。SSA 形式下每个 temp 只有一个定义，`t_src`
  的值在整个函数内不变，替换永远安全。

  ---

  ## 题目三：Global Value Numbering / CSE（全局值编号/公共子表达式消除）【中等概率】

  ### 题目描述

  > 在 HW7 或 HW8 的 SSA 代码基础上，实现公共子表达式消除（CSE）。
  > 若两条 MOVE_BINOP 语句计算相同的操作（相同 binop、相同操作数 temp 编号），
  > 则第二条是冗余的，将其替换为 `MOVE t2 = t1`（t1 是第一条的结果）。
  > 在 SSA 形式下，操作数 temp 编号相同即值相同，因此此判断是精确的。

  ### 核心思路

  维护一个"值表" `value_table: (binop, left_num, right_num) -> temp_num`。
  按支配树顺序（dominator tree order）遍历基本块，对每条 MOVE_BINOP：
  - 若 key 已在值表中，替换为 MOVE（复制传播）
  - 否则加入值表，继续

  **注意**：必须按支配树顺序遍历，否则可能用到尚未支配当前块的定义。
  简化版：若题目不要求跨块，只做块内 CSE 也可以（局部 CSE）。

  ### 局部 CSE 代码（块内，更容易实现）

  ```cpp
  static void localCSE(QuadFuncDecl* func) {
      for (auto* block : *func->quadblocklist) {
          // key: (binop, left_num_or_const, right_num_or_const) -> dst_temp_num
          // 用 string 编码 key 最简单
          map<string, int> expr_table;

          auto it = block->quadlist->begin();
          while (it != block->quadlist->end()) {
              if ((*it)->kind != QuadKind::MOVE_BINOP) { ++it; continue; }
              auto* s = static_cast<QuadMoveBinop*>(*it);

              // 编码操作数（TEMP用编号，CONST用"c:值"）
              auto termKey = [](QuadTerm* t) -> string {
                  if (t->kind == QuadTermKind::CONST)
                      return "c:" + to_string(t->get_const());
                  return "t:" + to_string(t->get_temp()->temp->num);
              };
              string key = s->binop + "|" + termKey(s->left) + "|" + termKey(s->right);

              if (expr_table.count(key)) {
                  // 替换为 MOVE t_dst = t_prev
                  int prev = expr_table[key];
                  auto* mv = new QuadMove(
                      s->dst->clone(),
                      new QuadTerm(new QuadTemp(new Temp(prev), s->dst->type)),
                      nullptr, nullptr);
                  auto* defSet = new set<Temp*>(); defSet->insert(mv->dst->temp);
                  auto* useSet = new set<Temp*>(); useSet->insert(mv->src->get_temp()->temp);
                  mv->def = defSet; mv->use = useSet;
                  it = block->quadlist->erase(it);
                  it = block->quadlist->insert(it, mv);
              } else {
                  expr_table[key] = s->dst->temp->num;
              }
              ++it;
          }
      }
  }
  ```

  ### 报告要点

  **需要处理的情况：**
  - MOVE_BINOP：相同 (binop, left, right) → 冗余，替换为 MOVE
  - 交换律：`a + b` 和 `b + a` 是否视为相同？（保守做法：不处理，只处理完全相同的）

  **为什么 LOAD 不能做 CSE？（报告必答）**
  `LOAD t = mem(addr)` 即使 addr 相同，两次 LOAD 之间若有 STORE 写入同一地址，则两次 LOAD 的值不同。除非能证明两次 LOAD 之间没有任何
   STORE（需要别名分析），否则不能消除冗余 LOAD。MOVE_BINOP 没有内存副作用，因此可以安全做 CSE。

  ---

  ## 题目四：General DCE（通用死代码消除）【中等概率】

  ### 题目描述

  > 在 HW7 的 SSA 代码基础上，实现通用死代码消除（Dead Code Elimination）。
  > 若一条语句定义的 temp 从未被任何其他语句使用，则该语句为死代码，删除之。
  > 注意：有副作用的语句（CALL、EXTCALL、STORE）即使结果不被使用，也不能删除。
  > 迭代执行直到不动点。

  ### 完整代码

  ```cpp
  static void dce(QuadFuncDecl* func) {
      bool changed = true;
      while (changed) {
          changed = false;

          // 收集所有被使用的 temp 编号（PHI 排除自引用）
          set<int> usedTemps;
          for (auto* block : *func->quadblocklist) {
              for (auto* stm : *block->quadlist) {
                  if (stm->kind == QuadKind::PHI) {
                      auto* phi = static_cast<QuadPhi*>(stm);
                      int defNum = phi->temp_exp->temp->num;
                      if (stm->use)
                          for (auto* t : *stm->use)
                              if (t->num != defNum) usedTemps.insert(t->num);
                  } else {
                      if (stm->use)
                          for (auto* t : *stm->use) usedTemps.insert(t->num);
                  }
              }
          }

          // 删除 def 不被使用的语句
          for (auto* block : *func->quadblocklist) {
              auto it = block->quadlist->begin();
              while (it != block->quadlist->end()) {
                  auto* stm = *it;
                  // 有副作用或控制流：不能删
                  if (stm->kind == QuadKind::CALL   ||
                      stm->kind == QuadKind::EXTCALL ||
                      stm->kind == QuadKind::STORE   ||
                      stm->kind == QuadKind::MOVE_CALL ||
                      stm->kind == QuadKind::MOVE_EXTCALL ||
                      stm->kind == QuadKind::JUMP    ||
                      stm->kind == QuadKind::CJUMP   ||
                      stm->kind == QuadKind::RETURN  ||
                      stm->kind == QuadKind::LABEL) {
                      ++it; continue;
                  }
                  if (stm->def && !stm->def->empty()) {
                      int defNum = (*stm->def->begin())->num;
                      if (!usedTemps.count(defNum)) {
                          it = block->quadlist->erase(it);
                          changed = true;
                          continue;
                      }
                  }
                  ++it;
              }
          }
      }
  }
  ```

  ### 报告要点

  **不能删除的语句：**
  - CALL/EXTCALL/STORE/MOVE_CALL/MOVE_EXTCALL：有副作用
  - JUMP/CJUMP/RETURN/LABEL：控制流

  **为什么 SSA 形式下 DCE 比非 SSA 简单？（报告必答）**
  非 SSA 下一个变量有多个定义点，删除某个定义前必须先做活跃变量分析（liveness
  analysis），确认该定义在其所有可能的使用点之前不会被其他定义覆盖。SSA 形式下每个 temp 只有唯一定义，只需检查该 temp 是否出现在任意
   use 集合中，无需活跃分析。

  ---

  ## 快速参考：各题目对应修改的文件

  | 题目 | 基础 HW | 修改文件 | 新增函数 |
  |------|---------|---------|---------|
  | 代数化简 | HW8 | `opt.cc` | `algebraicSimplification(QuadFuncDecl*)` |
  | Copy Propagation | HW7/HW8 | `quadssa.cc` 或 `opt.cc` | `copyPropagation(QuadFuncDecl*)` |
  | GVN/CSE | HW7/HW8 | `quadssa.cc` 或 `opt.cc` | `localCSE(QuadFuncDecl*)` |
  | General DCE | HW7 | `quadssa.cc` | `dce(QuadFuncDecl*)` |

  ---

  ## 临场检查清单

  - [ ] 读清楚题目：是 HW7 还是 HW8 基础？只能改哪个文件？
  - [ ] 先写报告框架（列出所有需要处理的情况，以及不能处理的边界）
  - [ ] 注意 PHI 节点的特殊处理（几乎每道题都有）
  - [ ] 注意有副作用的语句：CALL、EXTCALL、STORE、MOVE_CALL、MOVE_EXTCALL
  - [ ] 注意空指针：`if (!stm->def) continue`，`if (!phi->args) continue`
  - [ ] 迭代到不动点（while changed）
  - [ ] 修改语句后同步 def/use 集合（或调用 rebuildDefUseSets）

---

## 重新审视：更符合老师风格的题目

前两次小测的深层模式：
- Quiz 1：新的**静态语义约束检查**（AST级别）
- Quiz 2：新的**动态运行时插桩**（IR生成级别）

纯优化pass（代数化简、copy propagation）缺少"为什么对某类情况失效"的报告维度。
更符合风格的题目是：在HW7/HW8的SSA Quad上，**插入运行时检查** 或 **实现新的静态分析**。

---

## 题目五：整数溢出动态检测【高概率，最符合风格】

### 题目描述

> 在 HW8（SCCP优化后）的 SSA Quad 基础上，对整数运算插入运行时溢出检测。
> 对每个 MOVE_BINOP 语句（加、减、乘），在其后插入检查代码：
> 若运算结果溢出32位有符号整数范围（即结果不等于用64位计算的结果），
> 则调用外部函数 `exit(-102)`。
> 只需修改 opt.cc（或 quadssa.cc，视题目要求）。

### 核心思路

类比 Quiz 2 的 shadow temp 方法，但这里是在 **优化后的 Quad** 上插桩：

对每条 `MOVE_BINOP t_dst = t_left op t_right`（op 为 +/-/*）：
1. 插入 `MOVE_EXTCALL t_check = __check_overflow(t_left, t_right, op_code)`
2. 或者：插入 `CJUMP t_dst == (int)(t_left op t_right) ? ok : overflow`

实际上在 Quad IR 层面，最简单的做法是：
- 对加法：检查 `(t_left > 0 && t_right > INT_MAX - t_left) || (t_left < 0 && t_right < INT_MIN - t_left)`
- 但这太复杂，更实际的做法是调用一个外部检查函数

**最简实现**：在每个 +/-/* 的 MOVE_BINOP 之后，插入一个 `EXTCALL __overflow_check(t_dst, t_left, t_right, op_code)`，由外部函数负责检查并在溢出时 exit(-102)。

### 完整代码

```cpp
static void insertOverflowChecks(QuadFuncDecl* func) {
    set<string> checkOps = {"+", "-", "*"};

    for (auto* block : *func->quadblocklist) {
        auto it = block->quadlist->begin();
        while (it != block->quadlist->end()) {
            if ((*it)->kind != QuadKind::MOVE_BINOP) { ++it; continue; }
            auto* s = static_cast<QuadMoveBinop*>(*it);
            if (!checkOps.count(s->binop)) { ++it; continue; }

            ++it; // 移到 MOVE_BINOP 之后的位置

            // 构造参数列表：(result, left, right)
            auto* args = new vector<QuadTerm*>();
            args->push_back(new QuadTerm(new QuadTemp(new Temp(s->dst->temp->num), s->dst->type)));
            args->push_back(s->left->clone());
            args->push_back(s->right->clone());

            // 插入 EXTCALL __overflow_check(result, left, right)
            // 外部函数负责：若 result 不等于 64 位计算结果，则 exit(-102)
            string checkFuncName = "__overflow_check_" + s->binop;
            // 用 "+" -> "add", "-" -> "sub", "*" -> "mul" 映射函数名
            if (s->binop == "+") checkFuncName = "__overflow_check_add";
            else if (s->binop == "-") checkFuncName = "__overflow_check_sub";
            else checkFuncName = "__overflow_check_mul";

            auto* check = new QuadExtCall(
                checkFuncName, args,
                new set<Temp*>(), new set<Temp*>()
            );
            it = block->quadlist->insert(it, check);
            ++it;
        }
    }
}
```

### 报告要点

**需要插入检查的情况：**
- `+`（加法）：`INT_MAX + 1` 溢出
- `-`（减法）：`INT_MIN - 1` 溢出
- `*`（乘法）：`INT_MAX * 2` 溢出

**不需要/不能插入检查的情况：**
- `/`（除法）：整数除法不会溢出，**除了** `INT_MIN / -1`（结果为 INT_MAX+1）——这是一个特殊边界
- `%`（取模）：同除法，`INT_MIN % -1` 也是特殊边界
- `<<`（左移）：溢出语义在C++中是未定义行为，但在Java/FDMJ中定义为截断，不需要检查
- `>>`（右移）：不会溢出

**为什么 SCCP 已知为常量的运算不需要插入检查？（报告必答）**
若 SCCP 已经确定 `t_dst` 是 ONE_VALUE（常量），说明该运算在编译时已经被求值，且结果已知不溢出（SCCP 的 evalBinop 用 C++ int 计算，若溢出则结果已经是截断后的值）。可以跳过这些语句的检查，减少运行时开销。

**为什么对 class 变量（若有）的溢出检查更复杂？（类比 Quiz 2 的报告问题）**
class 变量通过 LOAD/STORE 访问内存，其值在运行时才能确定，且可能被多个方法修改。若要检查 class 变量参与的运算，需要在每次 LOAD 之后也插入检查，开销更大。

---

## 题目六：Null Pointer 动态检测【高概率，最符合风格】

### 题目描述

> 在 HW7 或 HW8 的 SSA Quad 基础上，对内存访问插入空指针检测。
> 对每个 LOAD 和 STORE 语句，在执行前检查地址是否为 0（null）：
> 若地址为 0，则调用外部函数 `exit(-103)`。
> 类似数组越界检查的插入方式（参考 HW3-4 的 ast2ir.cc）。

### 核心思路

类比 Quiz 2 的动态插桩，但这里是在 **Quad IR 层面** 插入检查：

对每条 `LOAD t_dst = mem(t_addr)` 或 `STORE mem(t_addr) = t_src`：
1. 在该语句之前插入：`CJUMP t_addr == 0 ? null_label : ok_label`
2. 在 `null_label` 块中插入：`EXTCALL exit(-103)`

但在 Quad IR 中更简单的做法是直接插入 EXTCALL：

```
// 在 LOAD/STORE 之前插入：
EXTCALL __null_check(t_addr)   // 外部函数：若 t_addr == 0 则 exit(-103)
```

### 完整代码

```cpp
static void insertNullChecks(QuadFuncDecl* func) {
    for (auto* block : *func->quadblocklist) {
        auto it = block->quadlist->begin();
        while (it != block->quadlist->end()) {
            auto* stm = *it;
            QuadTerm* addrTerm = nullptr;

            if (stm->kind == QuadKind::LOAD) {
                addrTerm = static_cast<QuadLoad*>(stm)->src;
            } else if (stm->kind == QuadKind::STORE) {
                addrTerm = static_cast<QuadStore*>(stm)->dst;
            }

            if (addrTerm && addrTerm->kind == QuadTermKind::TEMP) {
                // 在 LOAD/STORE 之前插入 null check
                auto* args = new vector<QuadTerm*>();
                args->push_back(addrTerm->clone());
                auto* check = new QuadExtCall(
                    "__null_check", args,
                    new set<Temp*>(), new set<Temp*>()
                );
                it = block->quadlist->insert(it, check);
                ++it; // 跳过刚插入的 check，指向原来的 LOAD/STORE
            }
            ++it;
        }
    }
}
```

### 报告要点

**需要插入检查的情况：**
- `LOAD t_dst = mem(t_addr)`：t_addr 可能为 null
- `STORE mem(t_addr) = t_src`：t_addr 可能为 null
- `PTR_CALC t_dst = t_base + offset`：t_base 可能为 null（若要检查数组基地址）

**不需要插入检查的情况：**
- 地址是 NAME（全局变量地址）：全局变量地址不为 null
- SCCP 已知地址为非零常量：可以跳过

**为什么对 PTR_CALC 的结果不需要检查，但对其 base 需要检查？（报告必答）**
`PTR_CALC t_dst = t_base + offset` 计算数组元素地址，若 t_base 为 null，则 t_dst 也是无效地址。应在 PTR_CALC 之前检查 t_base，而不是检查 t_dst（因为 t_dst 是计算结果，即使非零也可能是无效地址）。

**为什么对 class 变量的 null check 比 local 变量更复杂？（类比 Quiz 2）**
class 变量通过 `this` 指针访问，`this` 本身也可能为 null（虽然在正常调用中不会）。若要完整检查，需要在每次方法调用时也检查 `this` 是否为 null，这涉及到 CALL 语句的 obj_term。

---

## 题目七：SSA 正确性验证（静态检查）【中等概率，类比 Quiz 1 风格】

### 题目描述

> 在 HW7 的 SSA 转换结果上，实现一个 SSA 正确性验证 pass。
> 检查生成的 SSA 是否满足以下性质：
> 1. 每个 temp 至多有一个定义（SSA 的核心性质）
> 2. 每个 PHI 节点的参数数量等于该块的前驱块数量
> 3. 每个 temp 的 use 必须被其 def 所支配（def dominates use）
> 发现违反时报错并停止。

### 核心思路

类比 Quiz 1 的语义检查，这是一个**静态验证 pass**：

1. **检查唯一定义**：遍历所有语句，收集每个 temp 的定义次数，若 > 1 则报错
2. **检查 PHI 参数数量**：对每个 PHI，检查 `args->size()` 是否等于该块的前驱数量
3. **检查 def dominates use**：对每个 use，找到其 def 所在块，检查 def 块是否支配 use 块（需要 dominator 信息）

### 完整代码

```cpp
static void verifySSA(QuadFuncDecl* func, ControlFlowInfo* cfi) {
    // Check 1: 每个 temp 至多一个定义
    map<int, int> defCount;
    for (auto* block : *func->quadblocklist) {
        for (auto* stm : *block->quadlist) {
            if (!stm->def) continue;
            for (auto* t : *stm->def) {
                defCount[t->num]++;
                if (defCount[t->num] > 1) {
                    cerr << "SSA violation: t" << t->num << " has multiple definitions" << endl;
                    exit(1);
                }
            }
        }
    }

    // Check 2: PHI 参数数量 == 前驱块数量
    for (auto* block : *func->quadblocklist) {
        int label = block->entry_label->num;
        int predCount = cfi->predecessors.count(label) ? cfi->predecessors[label].size() : 0;
        for (auto* stm : *block->quadlist) {
            if (stm->kind != QuadKind::PHI) continue;
            auto* phi = static_cast<QuadPhi*>(stm);
            int argCount = phi->args ? phi->args->size() : 0;
            if (argCount != predCount) {
                cerr << "SSA violation: PHI for t" << phi->temp_exp->temp->num
                     << " in block " << label << " has " << argCount
                     << " args but " << predCount << " predecessors" << endl;
                exit(1);
            }
        }
    }

    // Check 3: def dominates use（简化版：检查 def 块支配 use 块）
    // 先建立 temp -> def 块的映射
    map<int, int> defBlock; // temp_num -> block label
    for (auto* block : *func->quadblocklist) {
        for (auto* stm : *block->quadlist) {
            if (!stm->def) continue;
            for (auto* t : *stm->def)
                defBlock[t->num] = block->entry_label->num;
        }
    }
    // 检查每个 use
    for (auto* block : *func->quadblocklist) {
        int useLabel = block->entry_label->num;
        for (auto* stm : *block->quadlist) {
            if (stm->kind == QuadKind::PHI) continue; // PHI 的 use 在前驱块，特殊处理
            if (!stm->use) continue;
            for (auto* t : *stm->use) {
                if (!defBlock.count(t->num)) continue; // 参数 temp，跳过
                int dLabel = defBlock[t->num];
                // 检查 dLabel 是否支配 useLabel
                if (!cfi->dominators[useLabel].count(dLabel)) {
                    cerr << "SSA violation: t" << t->num
                         << " used in block " << useLabel
                         << " but defined in block " << dLabel
                         << " which does not dominate it" << endl;
                    exit(1);
                }
            }
        }
    }
}
```

### 报告要点

**需要检查的情况（逐一列出）：**
- 每个 temp 的定义次数（必须 ≤ 1）
- PHI 节点的参数数量与前驱块数量是否一致
- 每个 use 是否被其 def 支配（def dominates use）
- PHI 的 use 特殊：`t = phi(t1 from L1, t2 from L2)` 中，t1 的 def 必须支配 L1，t2 的 def 必须支配 L2

**为什么非 SSA 形式下这个验证没有意义？（报告必答）**
非 SSA 形式下，同一个变量可以有多个定义点，"每个 temp 至多一个定义"这条性质本身就不成立。非 SSA 的正确性验证需要改为：每个 use 都有至少一个到达定义（reaching definition），这需要到达定义分析，比 SSA 验证复杂得多。

---

  Global CSE（跨块公共子表达式消除）

  这是你点名要的，也确实是高概率题。

  核心思路

  SSA 形式下，两个 MOVE_BINOP 如果 (binop, left_temp_num, right_temp_num) 完全相同，那么它们计算的值一定相同（因为 SSA 中同一 temp 编号 = 同一值）。

  跨块 CSE 的关键约束：第一次计算必须支配第二次计算所在的块，否则第二次执行时第一次可能没执行过。

  算法

  按支配树前序遍历（dominator tree preorder）处理块：
```cpp
  // 全局表：(binop, left, right) -> 第一次计算的 dst temp num
  map<string, int> avail_expr;

  void globalCSE(QuadFuncDecl* func, ControlFlowInfo* domInfo) {
      // 按支配树 DFS 序遍历
      function<void(int)> visit = [&](int label) {
          auto* block = domInfo->labelToBlock[label];
          // 记录本块新增的 key，退出时回滚（类似 renameVariables 的 stack pop）
          vector<string> added_keys;

          for (auto it = block->quadlist->begin(); it != block->quadlist->end(); ++it) {
              if ((*it)->kind != QuadKind::MOVE_BINOP) continue;
              auto* s = static_cast<QuadMoveBinop*>(*it);

              string key = s->binop + "|" + termKey(s->left) + "|" + termKey(s->right);

              if (avail_expr.count(key)) {
                  // 冗余！替换为 MOVE dst <- 已有结果
                  int prev_temp = avail_expr[key];
                  *it = new QuadMove(s->dst->clone(),
                      new QuadTerm(new QuadTemp(new Temp(prev_temp), s->dst->type)),
                      nullptr, nullptr);
              } else {
                  avail_expr[key] = s->dst->temp->num;
                  added_keys.push_back(key);
              }
          }

          // 递归处理支配树子节点
          if (domInfo->domTree.count(label))
              for (int child : domInfo->domTree[label]) visit(child);

          // 回滚：退出本块时删除本块加入的表达式
          for (auto& k : added_keys) avail_expr.erase(k);
      };

      visit(domInfo->entryBlock);
  }
```
  为什么必须按支配树遍历（报告必答）

  如果 block A 不支配 block B，那么执行 B 时 A 可能没执行过，A 中计算的 temp 值不存在。按支配树遍历 + 回滚，保证 avail_expr 中的表达式一定在当前块的所有执行路径上都已计算过。

  不能做 CSE 的情况

  - LOAD：两次 LOAD mem(t1) 之间可能有 STORE 修改了该地址，不能消除
  - CALL/EXTCALL：有副作用，不能消除
  - 交换律：a + b 和 b + a 是否视为相同？保守做法不处理；激进做法对 +、*、&、|、^ 排序操作数

  现场实现要点

  - termKey 函数：CONST 编码为 "c:值"，TEMP 编码为 "t:编号"
  - 需要从 HW7 的 ControlFlowInfo 拿 domTree 和 entryBlock
  - 如果是在 HW8 基础上做，HW8 的 Opt 类没有 domInfo，需要自己重建或者题目会给

  ---
  深度预测：最可能的"意想不到"题目

  基于老师的风格——题面引入新概念，要求在现有代码上实现，且需要全面理解代码中所有语句类型——我重新预测：

  预测 A（最高概率）：SSA 上的 Use-Def 链构建 + 活跃范围查询

  题目可能是：

  ▎ 在 HW7 的 SSA 输出上，构建显式的 def-use 链（每个定义点到所有使用点的映射），并回答：给定一个 temp，输出它的"活跃范围"（从定义到最后一次使用经过的所有块）。

  为什么符合风格：
  - SSA 中 def-use 链是 trivial 的（每个 use 只有一个 def），但构建它需要遍历所有语句的所有 use 字段
  - 活跃范围计算需要理解支配树和 CFG 的关系
  - PHI 节点的 use 语义特殊（use 发生在前驱块的末尾，不是当前块）
  - 报告可以问"为什么 PHI 的 use 不在当前块？这对活跃范围有什么影响？"

  预测 B（高概率）：循环检测 + 循环不变量识别

  题目可能是：

  ▎ 在 HW7/HW8 的 SSA 代码上，检测所有自然循环（natural loops），并标记哪些 MOVE_BINOP 是循环不变的（loop-invariant）。输出每个循环的头块、体块集合、以及循环不变语句列表。

  为什么符合风格：
  - 循环检测需要找回边（back edge）：successor 支配 predecessor
  - 自然循环体 = 从回边尾部反向 BFS 到头部能到达的所有块
  - 循环不变判断：操作数要么是常量，要么定义在循环外，要么是另一个循环不变语句的结果
  - 这不是 HW9（HW9 是 LICM，做的是移动代码），这里只是识别和报告
  - 需要用到 domInfo->dominators 和 domInfo->successors/predecessors

  关键代码骨架：
```cpp
  // 找回边：succ dominates pred
  vector<pair<int,int>> back_edges;
  for (auto& [pred, succs] : domInfo->successors)
      for (int succ : succs)
          if (domInfo->dominators[pred].count(succ))
              back_edges.push_back({pred, succ});

  // 对每条回边，计算自然循环体
  for (auto& [tail, header] : back_edges) {
      set<int> loop_body = {header};
      stack<int> worklist;
      if (tail != header) { loop_body.insert(tail); worklist.push(tail); }
      while (!worklist.empty()) {
          int n = worklist.top(); worklist.pop();
          for (int pred : domInfo->predecessors[n])
              if (!loop_body.count(pred)) { loop_body.insert(pred); worklist.push(pred); }
      }
  }

  循环不变判断：
  bool isLoopInvariant(QuadStm* stm, set<int>& loop_body, set<int>& invariant_defs) {
      // 只处理 MOVE, MOVE_BINOP（无副作用的纯计算）
      // 检查每个 use 的 temp：
      //   - 是常量 → OK
      //   - 定义在循环外 → OK
      //   - 定义在循环内但已被标记为 invariant → OK
      //   - 否则 → 不是 invariant
  }

  预测 C（高概率）：Store-Load 转发（Memory Forwarding）

  题目可能是：

  ▎ 在 HW8 的 SCCP 基础上，增加简单的 store-to-load 转发：如果一个 STORE 写入地址 A 值 V，后续 LOAD 从同一地址 A 读取，且中间没有其他可能修改 A 的语句（STORE/CALL/EXTCALL），则 LOAD
  ▎ 可以替换为 MOVE。

  为什么符合风格：
  - 需要理解所有可能修改内存的语句（STORE、CALL、EXTCALL、MOVE_CALL、MOVE_EXTCALL）
  - 地址匹配需要用 SSA 的 temp 编号相等性
  - 保守处理：任何 CALL 都可能修改任意内存，必须 kill 所有已知的 store 信息
  - 报告可以问"为什么不能跨 CALL 转发？为什么不能跨块转发（除非支配）？"

  块内版本（最容易现场实现）：
  for (auto* block : *func->quadblocklist) {
      // store_map: addr_temp_num -> stored_value_term
      map<int, QuadTerm*> store_map;

      for (auto it = block->quadlist->begin(); it != block->quadlist->end(); ++it) {
          auto* stm = *it;
          if (stm->kind == QuadKind::STORE) {
              auto* s = static_cast<QuadStore*>(stm);
              if (s->dst->kind == QuadTermKind::TEMP)
                  store_map[s->dst->get_temp()->temp->num] = s->src->clone();
          } else if (stm->kind == QuadKind::LOAD) {
              auto* s = static_cast<QuadLoad*>(stm);
              if (s->src->kind == QuadTermKind::TEMP) {
                  int addr = s->src->get_temp()->temp->num;
                  if (store_map.count(addr)) {
                      // 替换 LOAD 为 MOVE
                      *it = new QuadMove(s->dst->clone(), store_map[addr]->clone(), nullptr, nullptr);
                  }
              }
          } else if (stm->kind == QuadKind::CALL || stm->kind == QuadKind::EXTCALL ||
                     stm->kind == QuadKind::MOVE_CALL || stm->kind == QuadKind::MOVE_EXTCALL) {
              store_map.clear(); // CALL 可能修改任意内存
          }
      }
  }

  跨块版本（更完整，适合题目明确要求跨 basic block）：

  核心思想：
  - 给每个基本块维护入口/出口的"最近一次确定 STORE"信息。
  - `in_store[label]` 表示进入块 `label` 时，哪些地址 temp 已知最近被写入了什么值。
  - `out_store[label]` 表示执行完块 `label` 后，这些地址 temp 的已知写入值。
  - 多个前驱汇合时，只保留所有前驱都存在、且写入值完全相同的地址；否则该地址退化为未知。
  - 遇到任意可能改内存的语句（未知地址 STORE、CALL、EXTCALL、MOVE_CALL、MOVE_EXTCALL）就清空或 kill 对应信息。

  数据结构：
```cpp
  using StoreState = map<int, QuadTerm*>; // addr_temp_num -> stored_value

  static int termTempNum(QuadTerm *term) {
      if (term == nullptr || term->kind != QuadTermKind::TEMP) return -1;
      return term->get_temp()->temp->num;
  }

  static bool sameTerm(QuadTerm *a, QuadTerm *b) {
      if (a == nullptr || b == nullptr) return a == b;
      return a->print() == b->print();
  }

  static StoreState cloneStoreState(const StoreState &src) {
      StoreState dst;
      for (auto &[addr, val] : src) dst[addr] = val->clone();
      return dst;
  }

  static bool sameStoreState(const StoreState &a, const StoreState &b) {
      if (a.size() != b.size()) return false;
      for (auto &[addr, av] : a) {
          auto it = b.find(addr);
          if (it == b.end()) return false;
          if (!sameTerm(av, it->second)) return false;
      }
      return true;
  }
```

  构造前驱表：
```cpp
  map<int, QuadBlock*> label2block;
  map<int, vector<int>> preds;

  for (auto *block : *func->quadblocklist) {
      int label = block->entry_label->num;
      label2block[label] = block;
  }

  for (auto *block : *func->quadblocklist) {
      int from = block->entry_label->num;
      if (!block->exit_labels) continue;
      for (auto *lab : *block->exit_labels) {
          if (lab && label2block.count(lab->num))
              preds[lab->num].push_back(from);
      }
  }
```

  meet 函数：只保留所有前驱都同意的 store 信息。
```cpp
  static StoreState meetStoreStates(const vector<int> &preds,
                                    const map<int, StoreState> &out_store) {
      StoreState result;
      bool first = true;

      for (int pred : preds) {
          auto it = out_store.find(pred);
          if (it == out_store.end()) return StoreState();

          if (first) {
              result = cloneStoreState(it->second);
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

  transfer 函数：模拟执行一个 block，计算出口状态。
```cpp
  auto transferBlock = [&](QuadBlock *block, const StoreState &input) {
      StoreState state = cloneStoreState(input);

      for (auto *stm : *block->quadlist) {
          if (stm->kind == QuadKind::STORE) {
              auto *s = static_cast<QuadStore*>(stm);
              int addr = termTempNum(s->dst);

              if (addr >= 0) {
                  // 写同一地址后，后续 LOAD 可转发这个值。
                  state[addr] = s->src->clone();
              } else {
                  // STORE 到未知地址，可能改任意内存，保守清空。
                  state.clear();
              }
          } else if (stm->kind == QuadKind::CALL || stm->kind == QuadKind::EXTCALL ||
                     stm->kind == QuadKind::MOVE_CALL || stm->kind == QuadKind::MOVE_EXTCALL) {
              // 调用可能修改任意内存，保守清空。
              state.clear();
          }
      }

      return state;
  };
```

  不动点求解：
```cpp
  map<int, StoreState> in_store;
  map<int, StoreState> out_store;

  if (!func->quadblocklist->empty()) {
      int entry = func->quadblocklist->front()->entry_label->num;

      for (auto *block : *func->quadblocklist) {
          int label = block->entry_label->num;
          in_store[label] = StoreState();
          out_store[label] = StoreState();
      }

      bool changed = true;
      while (changed) {
          changed = false;

          for (auto *block : *func->quadblocklist) {
              int label = block->entry_label->num;

              StoreState new_in;
              if (label == entry)
                  new_in = StoreState();
              else
                  new_in = meetStoreStates(preds[label], out_store);

              StoreState new_out = transferBlock(block, new_in);

              if (!sameStoreState(in_store[label], new_in)) {
                  in_store[label] = cloneStoreState(new_in);
                  changed = true;
              }
              if (!sameStoreState(out_store[label], new_out)) {
                  out_store[label] = cloneStoreState(new_out);
                  changed = true;
              }
          }
      }
  }
```

  根据 `in_store` 真正重写每个 block：
```cpp
  for (auto *block : *func->quadblocklist) {
      int label = block->entry_label->num;
      StoreState store_map = cloneStoreState(in_store[label]);
      auto *new_list = new vector<QuadStm*>();

      for (auto *stm : *block->quadlist) {
          if (stm->kind == QuadKind::STORE) {
              auto *s = static_cast<QuadStore*>(stm);
              int addr = termTempNum(s->dst);
              if (addr >= 0)
                  store_map[addr] = s->src->clone();
              else
                  store_map.clear();

              new_list->push_back(static_cast<QuadStm*>(stm->clone()));
              continue;
          }

          if (stm->kind == QuadKind::LOAD) {
              auto *s = static_cast<QuadLoad*>(stm);
              int addr = termTempNum(s->src);
              if (addr >= 0 && store_map.count(addr)) {
                  auto *def = new set<Temp*>();
                  def->insert(new Temp(s->dst->temp->num));

                  auto *use = new set<Temp*>();
                  if (store_map[addr]->kind == QuadTermKind::TEMP)
                      use->insert(new Temp(store_map[addr]->get_temp()->temp->num));

                  new_list->push_back(new QuadMove(
                      s->dst->clone(),
                      store_map[addr]->clone(),
                      def,
                      use
                  ));
                  continue;
              }
          }

          if (stm->kind == QuadKind::CALL || stm->kind == QuadKind::EXTCALL ||
              stm->kind == QuadKind::MOVE_CALL || stm->kind == QuadKind::MOVE_EXTCALL) {
              store_map.clear();
              new_list->push_back(static_cast<QuadStm*>(stm->clone()));
              continue;
          }

          new_list->push_back(static_cast<QuadStm*>(stm->clone()));
      }

      block->quadlist = new_list;
  }
```

  这个跨块版本能优化：
```text
  L1:
    STORE Const:7 -> Mem(t100)
    JUMP L2

  L2:
    LOAD t101 <- Mem(t100)
```

  因为 `out_store[L1]` 里有 `t100 -> Const:7`，所以 `in_store[L2]` 也有这个信息，`LOAD` 可以改成：
```text
  MOVE t101 <- Const:7
```

  分支汇合时必须保守：
```text
  L2: STORE Const:7 -> Mem(t100); JUMP L4
  L3: STORE Const:8 -> Mem(t100); JUMP L4
  L4: LOAD t101 <- Mem(t100)
```

  `L4` 的两个前驱给同一地址不同值，meet 后删除 `t100` 的 store 信息，所以 `LOAD` 保留。若两个前驱都写 `Const:7`，则可以转发。

  报告要点：
  - 跨块转发不是简单看文本顺序，而是 dataflow：`in = meet(pred.out)`，`out = transfer(in)`。
  - 多前驱汇合时，只有所有前驱同地址同值才保留。
  - 任意 CALL/EXTCALL/MOVE_CALL/MOVE_EXTCALL 必须 kill，因为可能修改内存。
  - 未知地址 STORE 必须 kill，因为可能写到任意已知地址。
  - 这个版本仍然不做别名分析：只在地址 temp 编号完全相等时转发，地址不同但实际别名的情况保守不处理。

  预测 D（中高概率）：SSA 上的类型一致性检查

  题目可能是：

  ▎ 在 HW7 的 SSA 输出上，检查类型一致性：PHI 节点的所有输入 temp 和输出 temp 必须类型相同；MOVE_BINOP 的操作数必须是 INT；PTR_CALC 的 base 必须是 PTR。报告所有类型不一致的位置。

  为什么符合风格：
  - 类比 Quiz 1 的"语义检查"
  - 需要建立 temp_num -> type 的映射（从定义语句推断）
  - PHI 合并不同类型是错误
  - 需要遍历所有语句类型，理解每个字段的类型约束

  预测 E（中等概率）：Strength Reduction（强度削减）

  题目可能是：

  ▎ 在 HW8 的 SCCP 之后，对 MOVE_BINOP 做强度削减：
  ▎ - x * 2^n → x << n
  ▎ - x / 2^n（x ≥ 0 时）→ x >> n
  ▎ - x * (2^n ± 1) → (x << n) ± x

  为什么符合风格：
  - 利用 SCCP 已经算出的常量信息
  - 需要判断一个常量是否是 2 的幂（(v & (v-1)) == 0）
  - 有边界情况：负数除法不能用右移、乘以 0 或 1 是特殊情况
  - 报告可以问"为什么 x / 2^n 对负数 x 不能直接用 x >> n？"（因为整数除法向零取整，右移向负无穷取整）
```
  ---
  我的最终概率排序（重新校准）

  ┌──────┬───────────────────────────┬─────────┬────────────────────────────────────────────────────────────────┐
  │ 排名 │           题目            │ 基础HW  │                              理由                              │
  ├──────┼───────────────────────────┼─────────┼────────────────────────────────────────────────────────────────┤
  │ 1    │ 循环检测 + 循环不变量识别 │ HW7     │ 用到 dominator，算法性强，HW9 只做了 LICM 但没让你自己检测循环 │
  ├──────┼───────────────────────────┼─────────┼────────────────────────────────────────────────────────────────┤
  │ 2    │ Global CSE（支配树遍历）  │ HW7/HW8 │ 经典 SSA 优化，需要理解支配关系，回滚机制类似 rename           │
  ├──────┼───────────────────────────┼─────────┼────────────────────────────────────────────────────────────────┤
  │ 3    │ Store-Load 转发           │ HW8     │ 扩展 SCCP 处理内存，边界情况多（CALL kill），报告维度丰富      │
  ├──────┼───────────────────────────┼─────────┼────────────────────────────────────────────────────────────────┤
  │ 4    │ Def-Use 链构建 + 活跃范围 │ HW7     │ 需要理解 PHI 语义，支配关系                                    │
  ├──────┼───────────────────────────┼─────────┼────────────────────────────────────────────────────────────────┤
  │ 5    │ 强度削减                  │ HW8     │ 利用 SCCP 常量，边界情况有趣                                   │
  ├──────┼───────────────────────────┼─────────┼────────────────────────────────────────────────────────────────┤
  │ 6    │ 类型一致性检查            │ HW7     │ 类比 Quiz 1 风格的静态检查                                     │
  └──────┴───────────────────────────┴─────────┴────────────────────────────────────────────────────────────────┘

  ---
  现场应对策略补充

  不管出什么题，90 分钟内的关键：

  1. 前 5 分钟：确认题目要改哪个文件、输入输出格式、是否需要新建函数
  2. 确认是否需要 domInfo：如果题目基于 HW7，ControlFlowInfo 已经有 dominators、domTree、predecessors、successors、dominanceFrontiers。如果基于 HW8，这些信息不在 Opt
  类里，可能需要自己算或者题目会提供
  3. 所有语句类型的 switch 骨架：直接从 renameUses 或 collectDefUsePtrs 复制改，这是最快的
  4. 回滚模式：如果需要按支配树遍历并维护状态，直接抄 renameBlock 的 pushCnt + pop 模式
