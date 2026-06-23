# BigInt 工业级改造方案

## 当前代码状态

**已有修复**：LLONG_MIN 溢出 | absSub 无符号回绕 | 复合赋值运算符
**测试**：126 个测试用例全部通过（构造、比较、四则运算、代数性质、输入输出）

## 第一阶段：修复与夯实（P0 优先）

| 任务 | 优先级 | 关键提示 |
|------|--------|----------|
| `isZero()` 加固 | P0 | 改为 `digit_.empty() \|\| (size==1 && digit_[0]==0)`，消除"忘调 trim 导致不一致"的隐患 |
| 一元 `operator-` | P0 | `BigInt r = *this; if (!r.isZero()) r.negative_ = !r.negative_; return r;` |
| `abs()` | P0 | 返回拷贝并将 `negative_` 置 false |
| `toLongLong()` / `toDouble()` | P0 | 用 `unsigned long long` 中间累加避免溢出；超范围抛 `overflow_error` |
| 字符串构造校验 | P1 | 显式遍历检查字符合法性，拒绝空串/纯符号，抛 `invalid_argument` 带描述信息 |
| `gcd()` / `lcm()` | P1 | 先用欧几里得算法（依赖取余）；lcm 先除后乘防溢出 |
| `pow()` | P1 | 快速幂（binary exponentiation），指数用 `unsigned long long` |
| `absMulUint` 改为 static | P2 | 不访问 `this` 的方法应该是静态的 |
| 清理 operator- 冗余行 | P2 | 删除 `ans.negative_ ^= 1` 这行无用代码 |

## 第二阶段：性能优化（核心瓶颈）

### 2.1 Knuth Algorithm D 除法 — P0（最大瓶颈）

**当前问题**：二分查找试商（log base ≈ 30 次迭代/位）+ `vector::insert(begin())` O(n) 移动 → O(n² log B)

**改造要点**：
- 实现 TAOCP Vol 2 的 Algorithm D
- **规范化**：除数和被除数同乘以 `d = base / (除数最高位 + 1)`，使除数最高位 ≥ base/2
- **试商**：`q_hat = (a_hi * base + a_lo) / b_hi`，由于已规范化，q_hat 至多比真实商大 2
- **校正**：最多减 2 次即可得到正确商
- **反规范化**：最终余数除以 d
- 需要辅助函数 `absDivUint`（大数除以单 digit，用于反规范化）
- 不再需要 `insert(begin())`——改为从高位向低位处理

**收益**：O(n² log B) → O(n²)，常数约为原来的 1/30

### 2.2 Karatsuba 乘法 — P1

Z0 = X0*Y0, Z2 = X1*Y1, Z1 = (X0+X1)*(Y0+Y1) - Z0 - Z2，结果 = Z2*B^{2m} + Z1*B^m + Z0

- 递归阈值：~50-80 个 digit（需实际 benchmark）
- 先实现 2.4 移动语义，否则递归中大量临时对象拷贝会抵消收益

### 2.3 reserve 预分配 — P1

`absAdd` reserve `max(lena, lenb) + 1`，`absSub` reserve `max(lena, lenb)`

### 2.4 移动语义 — P1

```cpp
BigInt(BigInt&& other) noexcept : negative_(other.negative_), digit_(std::move(other.digit_)) {
    other.negative_ = false; // 源对象置为零
}
```

**注意**：移动后源对象必须处于合法（可析构）状态。Karatsuba 递归受益最大。

### 2.5 digit 改为 uint64_t — P2

- `using digit_t = uint64_t`，`base = 10^18`
- 需要 `__uint128_t`（GCC/Clang）或 `_umul128`（MSVC）处理两个 digit 相乘的 128 位结果
- 条件编译隔离平台差异，写清楚 fallback 路径
- **风险较高**，改动面广，建议 Phase 2 其他项稳定后再做

## 第三阶段：高级算法

| 任务 | 优先级 | 关键提示 |
|------|--------|----------|
| 位移运算 `<<` `>>` | P1 | 按 digit 整移 + 单 digit 余数处理 |
| Toom-Cook 3-way 乘法 | P2 | 5 估值点求值→逐点乘→插值；需精确除以小整数（2,3,6）；适合数千~数万 digit |
| FFT/NTT 乘法 | P2 | 数论变换，选友好模数（如 998244353, g=3）；双/三模 NTT + CRT 合并避免溢出 |
| Burnikel-Ziegler 除法 | P2 | 分治除法 O(n log n)；需先完成 2.1 后再做 |
| Montgomery 模乘 | P2 | 用于 `powMod(base, exp, mod)`；将取模的除法替换为移位+乘法 |

## 第四阶段：内存管理（按需）

- **Small Buffer Optimization**（P2）：对象内嵌 2-3 个 digit 的小数组，小数值时避免堆分配。**注意**：需从 `vector` 改为原始指针管理，失去 RAII 便利性——建议仅在 profiler 证明分配开销是瓶颈后才实施
- **自定义 Allocator**（P2）：模板参数化，嵌入式/内存受限场景有价值

## 第五阶段：测试与基准

| 任务 | 优先级 |
|------|--------|
| 迁移到 Catch2/Google Test 框架 | P0 |
| 回归测试矩阵（边界值、符号组合、逆运算、代数性质、Python 交叉验证） | P0 |
| 性能基准框架（不同规模梯度，记录 ops/sec，CI 回归检测） | P1 |
| Python 脚本生成随机测试向量 | P1 |
| libFuzzer / AFL 模糊测试 | P2 |

## 实施顺序

```
Phase 1 (Fix & Solidify) → Phase 5 (Testing) → Phase 2 (Performance Core)
                                                    ↓
先做 2.4 (移动语义) → 2.1 (Knuth 除法) → 2.3 (reserve) → 2.2 (Karatsuba) → 2.5 (uint64_t)
                                                    ↓
                                              Phase 3 (Advanced)
                                                    ↓
                                         Phase 4 (Memory, 按需)
```

**关键路径**：2.1（除法）是当前最大瓶颈，也是 gcd、pow 等所有依赖取余的操作的基础。2.4（移动语义）应在 2.2（Karatsuba）之前完成。

## 验证方式

- 每个 Phase 完成后运行完整测试套件（126+ 用例）
- Phase 2 每步用基准框架对比性能数据
- Phase 3 用 Python `int` 生成参考值做交叉验证
