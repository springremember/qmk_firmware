# QK61 Vim 方案（keymaps/vim）

参考 NUT65 Vim 固件，在 QK61 上以 `qmk-vim` 引擎实现 Vim 绝大多数功能，纯固件实现、无需系统层软件。目标系统：Windows / Linux（Ctrl 方案，非 Mac）。

硬件：CIDOO QK61（VID `0x36B0` / PID `0x3035`，矩阵 6×16，RGB Matrix 64 灯，VIA，FS026）。

引擎 `qmk-vim` 为独立仓库，通过 **git 子模块**引入；「新 Fn 层」遵循 `qmk-vim-fn/fn` **约定文档**（仅约定、无代码），在 `keymap.c` 内联实现：

- `keyboards/qk61/keymaps/vim/qmk-vim-fn/` → `git@github.com:springremember/qmk-vim-fn.git`（子模块）
- myfn 约定 → `git@github.com:springremember/qmk-vim-fn.git（`fn/` 目录）`（**文档**；本 keymap 内联实现，不引入其代码）

> **版本 V1.0（冻结）**：引擎锁定 `qmk-vim` `v1.0`（子模块 commit `62bb338`），约定锁定 `qmk-vim-fn/fn` `v1.0`。踩坑/问题记录见 **第十二节**。
> **版本 V2.35（当前）**：共享层 7be99d1 → c18917b（与 NUT65 V2.42 同源）。**首个真实源码级体积收益**：环形下标 `% EMIT_CAP` 改 `& EMIT_MASK`（热路径）。Cortex-M0 上 `int % 256` 要生成符号修正，`& 255` 只需一条 `uxtb`；被除数恒非负 ⇒ 严格等价，编译期静态断言锁死 2 的幂前提。实测：QK61 **77548 → 77508 B（−40 B）**（NUT65 −16 B）。**更正**：早先「LTO 下源码级收益为零」的结论被独立审计推翻，已在 `qmk/engineering-spec.md` §5.1/§5.2 更正。门禁不变：engine 778 断言、10 个 glue 套件、矩阵 exit 0。**刷机请用 V2.35**。
> **版本 V2.34（上一版）**：共享层 b84d09d → 7be99d1（与 NUT65 V2.41 同源）。把带计数命令的键码预算从「**静默丢键**」改为「**确定性截断**」：队列 256 格满格时 `emit.c` 直接 `return`，丢的是**命令尾部** ⇒ 半截执行（范围已选中却发不出 `Ctrl+X`），文档被以难以复现的方式改错；实测 `99dd`+`.`×3 应发 412 实发 256（丢 156）。现所有带计数发射器按 `kv_emit_room() = 250 − pending` 与自身每次重复成本截断计数，保证命令发完且队列不到顶。**`pending=0` 时逐键不变**：前后探针在 446 条命令上比对 ⇒ 445 条逐字节相同，唯一变化 `v99w`（257 → 250，本就超封顶在丢键）。测试红→绿 `715/63` → **`778/0`**；变异 31 处 CAUGHT 29。**刷机请用 V2.34**。
> **版本 V2.33（上一版）**：共享层 826eadf → f249503（与 NUT65 V2.39 同源）。修 **D24**：宿主 +1 列偏移被 `k` 保持 ⇒ 字符级可视选区**左端比 Vim 右一列**，`vky`/`vkd` 内容不符（**缓冲区/寄存器可见**）。现 `k` 前先 `Esc, Shift+Left` 翻锚点到右端、光标落到 Vim 列，再 `Shift+Up×n`（`j` 不需要）。前后对照 255 个 `v*k*`/`v*j*` 用例 ⇒ **44 FIXED / 0 REGRESSED**，变异 CAUGHT。`TOTAL 598 PASS 405 XFAIL 193 KNOWN-FAIL 0 NEW-FAIL 0`。**仍开放**：`VPASTE`（Vim 可视 `p`/`P` 会把被替换文本写进无名寄存器）。**刷机请用 V2.33**。
> **版本 V2.32（上一版）**：共享层 c54f693 → 826eadf（与 NUT65 V2.38 同源）。修 **D23**：行可视（`V`）的 Esc 落点 —— Vim 落到**选区起点行首**（最上行、列 0），宿主停在活动端行行尾 ⇒ 紧接 `x`/`s` 会删掉换行并**并行**（`V<Esc>x`@L1 模型 `L1L2\n…`、Vim `1\nL2…`，**数据损坏**）。现 DOWN 态补 `Up×off`、两态都补 `Home`。对照 56 个 `V*` 用例 ⇒ **10 FIXED / 0 REGRESSED**。并修正 XFAIL 引用语义 3 处、登记 2 个未修缺陷（`VBLOCK` 纵向选区宽一列、`VPASTE` 可视 `p` 的寄存器语义）。`TOTAL 598 PASS 403 XFAIL 195 KNOWN-FAIL 0 NEW-FAIL 0`。**刷机请用 V2.32**。
> **版本 V2.31（上一版）**：共享层 853e04d → c54f693（与 NUT65 V2.37 同源）。修 **D22**：纵向/词动作之后的 Esc 落点 —— `j`/`k`/`w`/`e`/`$` 只让 lo/hi 偏移失效，而 +1 列偏移仍成立；Esc 补偿此前依赖偏移有效，于这类动作后不补 `←`，`vj<Esc>x` 会删到下一列（**缓冲区可见**）。现拆出独立比特 `s_v_rt1`（`v` 置位；`0`/`^`/`G`/`gg` 与 `h`/`l` 清零；纵向与词动作保持）。对照 106 个 `v*<Esc>*`/`vy*` 用例 ⇒ **22 FIXED / 0 REGRESSED**，变异均 CAUGHT，`vis-jEsc-x` 已完全通过。`TOTAL 598 PASS 400 XFAIL 198 KNOWN-FAIL 0 NEW-FAIL 0`。**刷机请用 V2.31**。
> **版本 V2.30（上一版）**：共享层 69e9011 → 853e04d（与 NUT65 V2.36 同源）。修两个**缓冲区可见**的光标缺陷：**D21** `y` + 带计数的字符级动作（`3yl` 的动作是 `3l`，回位只发 1 个 `←` ⇒ `3ylp` 粘到错位置）；现改 `←×n`。**D20** 可视 `y` 落在**选区起点**、`Esc` 落在**活动端**，宿主光标在其后 ⇒ `y` 补 `←×w`、`Esc` 补 1 个 `←`（否则 `vlyp`/`vll<Esc>x` 缓冲区可见）。对照 60 个 `vy*`/`v<Esc>*` 用例 ⇒ **40 FIXED / 0 REGRESSED**，3 条变异全部 CAUGHT。棘轮基线剪 6 条、XFAIL 摘 6 条：`TOTAL 599 PASS 399 XFAIL 180 KNOWN-FAIL 20 NEW-FAIL 0`。**刷机请用 V2.30**。
> **版本 V2.29（上一版）**：共享层 cc94d68 → 69e9011（与 NUT65 V2.35 同源）。修复 `cc`/`S` 的**无名寄存器类型**（独立审查 P1-1）：旧实现只把行内容（不含行尾换行）纳入选区再 `Ctrl+X`，寄存器因此是**字符级**，随后的 `p` 会当字符级往**行内**粘 —— `cc<Esc>p`@L2 模型得 `L1\n\nL2L3\nL4`、Vim 得 `L1\n\nL2\nL3\nL4`（**缓冲区可见**）。现与 `dd` 一样先把**行尾换行**纳入选区（寄存器行级），再补 `Shift+Enter` 造出空行、补 `←` 退回空行。前/后探针对照 30 个 `cc`/`S` 用例：**21 修好、0 回归**。矩阵棘轮基线剪掉 11 个已修用例：`TOTAL 599 PASS 387 XFAIL 48 KNOWN-FAIL 164 NEW-FAIL 0`。**刷机请用 V2.29**。
> **版本 V2.28（上一版）**：共享层 82098e5 → cc94d68。修复**字符级可视模式的后向词动作**（D19，数据损坏）：按 `v` 后立刻按 `b`/`B` 时，宿主光标在 `hi`、Vim 光标在 `hi−1`，当 Vim 光标恰在**词首**时 `Ctrl+Shift+Left` 会回到锚点 → 半开选区塌成空 → 随后的 `d`/`x` **剪切整行**（`vbd`@列4 模型删掉整行，Vim 只删 `one t`）。现先 `Esc, Shift+Left` 把锚点挪到右端、宿主光标落到 Vim 光标，再 `Ctrl+Shift+Left×n`：`vbd`/`vby`/`vbbd`/`v2bd`/`vbBd` 在全部列的**缓冲区与寄存器**均与真实 Vim 一致（前/后探针对照 85 用例：44 修好、0 回归）。残留：先向右扩选再按 `b`（`vlbd`/`vwbd`）Vim 会**缩小**选区，本实现以右端为锚重建，方向正确但边界不精确（读不到词边界，固有，已文档化）。矩阵棘轮基线相应剪掉 6 个已修用例：`TOTAL 599 PASS 376 XFAIL 48 KNOWN-FAIL 175 NEW-FAIL 0`。**刷机请用 V2.28**。
> **版本 V2.27（上一版）**：共享层 4d94173 → 82098e5，与 NUT65 V2.33 同源。一次**独立对抗审查**（隔离 worktree 重建探针 + 宿主模型 + 真实 `vim.tiny`，20 个变异）驱动的大批修复，其中两项是早前修复引入的**回归**：
> ①**计数命令撑爆发送队列（回归，数据损坏）** —— `X`/行级 `p` 由 1 键变 3 键后，`99X`/`yy99p`/`dd99p`/`99J` 达 297/294 键、封顶 256 并**静默丢键**（命令只执行一部分）。现 `x`/`X`/`s` 一次选中 N 个字符（N+1 键）、`p`/`P` 只定位一次、`NJ` 超预算截断：`99X`=101、`yy99p`=107、`99J`=248。
> ②**`Nx`/`NX` 寄存器只剩最后一个字符**（`3x` 得 `e`、Vim 得 `ree`）；③计数 `p`/`P` 曾把副本交错插入；④**`y`+动作的光标前提是错的**（实测 Vim 留在被复制区间起点，`yhp`/`ybp` 曾粘错位置）；⑤可视模式**大写 `P`** 曾被吞；⑥`J` 光标差一导致 `Jx` 删错字符。
> 另含 D1/D12 **字符级可视模式偏移状态机**（越过锚点重锚，`vhd`/`vwd` 不再塌成空选区剪整行）、缩进恒为整行（D4）、缩进后取消残留选区并回位光标（D6）、`x`/`X`/`s` 写剪贴板与 `p`/`P` 按寄存器类型定位（D7）、`dd` 纳入行尾换行（**首行不再留空行**）、`c`+行选动作光标退回空行；并把 599 用例矩阵固化为仓库内常驻测试。
> 测试：engine 621 断言、10 个 glue 套件全绿。**刷机请用 V2.27**。
> **版本 V2.26（上一版）**：`task` 传 `timer_read32`（审核 P2-5）；V2.24/V2.25 为子模块指针同步与小修。
> **版本 V2.23（上一版）**：修正 `V gg j d` 把两行拼接的**数据损坏** —— `abs`（`G`/`gg` 之后）状态按**活动端所在边界**收边：`gg` 之后活动端是上边界用 `Shift+Home`（旧版误用 `Shift+End`，`V gg j y` 只得到 `\nL3\n`、`V gg j d` 会把两行拼接）。反向越过锚行仍是固有限制（已文档化）。
> **版本 V2.22（上一版）**：可视行模式重写为方向无关的按行语义（修复 `V k d` 拼接两行、`V gg y` 丢首尾正文、`V d` 只清正文、`V s` 只删 1 字符、`V y` 非 linewise、`V j` 漏长行末字符、`h`/`l` 改行范围、`V y` 后残留选区、`v`/`V` 不能互切）；并加行选跨度上限 100 行。
> **版本 V2.21（上一版，行选缺陷，建议升级）**：可视行模式修复 —— ①动作锚点改 `Shift+Home`；②行选移动补 `Shift+End`。
> **版本 V2.20**（行选缺陷，建议升级）
> **版本 V2.19**（含 P0，勿刷）
> **版本 V2.17**：第 2/3 轮审计修复 —— 可视输入作废统一（透传/CAG/myfn 三路径）、`gG` 不再误判为 `gg`、`g` 后接非法键按规格吞掉、Caps 重入/溢出/物理 Ctrl 三项 P0 修复。
> **版本 V2.16（上一版）**：审计修复 —— Caps 卡键（重入/溢出）与发错键（物理 Ctrl/层键）已修；可视计数上限 2 位、`0` 续接、`gg` 已实现。
> **版本 V2.15（上一版）**：`Caps` 触发改为 —— **按下即进入 Caps 模式**（不等 200ms）；**裸 `Caps` 单击无任何效果**；**`Fn`+`Caps` 单击开关 Vim**。
> **版本 V2.14（上一版）**：`Visual-Line`（`V`）—— 灯色改为**洋红 rose** `#FF0080`（与 Visual 紫 `#800080` 区分）；进入即选中**整行**（`Home`→`Shift+End`）；`y`/`d` 等动作后**退出可视**。
> **版本 V2.13（上一版）**：`Visual-Line`（`V`）灯色独立为**紫红** `#FF00FF`（与 Visual 的紫 `#800080` 区分）；见第六节。
> **版本 V2.11（上一版）**：`Caps` 长按改为 **Caps 模式**（规格见 qmk-vim-fn `caps/` 模块）：`1`–`0`/`-`/`=` = `F1`–`F12`（不带 Ctrl），其余键 = `Ctrl+键`，松开退出；不再临时进入 Normal（见第二节）。
> **版本 V2.10（上一版）**：引擎 `qmk-vim-fn`（子模块 `9fa2a24`，含共享层 `vim_insert_flash()`）。有线 USB 枚举问题见 **第十三节**（P1″ 真因=镜像体积/布局，V2.8 起用 LTO 缩体修复）。**V2.10 行为变更**：`Normal` 空闲按 `Esc` 回到 `Insert` 后 **Esc 灯与 logo 电量灯转橙 `#FF8000` 3s**（与 Esc 宽限窗口同一计时），随后自动回 Insert 绿（见第六节）。

## 一、键位与层

共 **5 层**：

| 层 | 含义 |
| :--- | :--- |
| 0 | Win Base |
| 1 | Mac Base（开机 Mac 模式时自动开启） |
| 2 | Win Fn（厂商原厂层，**保留但不再触发**） |
| 3 | Mac Fn（厂商原厂层，**保留但不再触发**） |
| 4 | `_FN` **新 Fn 层**（自定义，见下） |

Base 层 Esc 由 `QK_GESC` 改为 **`KC_ESC`**（供 vim 拦截）。

### 新 Fn 层 `_FN`（layer 4）

底排 **Fn 键 = `MO(4)`**（物理右 Alt 位）。`_FN` 目前**只保留**：

| 键位 | 功能 |
| :--- | :--- |
| `1` / `2` / … / `0` | F1 / F2 / … / F10（`KC_F1`…`KC_F10`） |
| `-` / `=` | F11 / F12（`KC_F11` / `KC_F12`） |
| `[` / `]` | 音量减 / 增（`KC_VOLD` / `KC_VOLU`） |
| `Q` / `W` / `E` / `R` | 蓝牙 1 / 2 / 3 / 2.4G（`MD_BLE1/2/3`、`MD_24G`） |
| `T` | 切有线：QK61 **有物理开关** → 按 myfn 约定**空跑**（吞键，不输出 `t`） |
| `Space` | **Fn+Space = 电量提示**（用数字键 LED 1–10 显示；按 myfn 约定，QK61 在 `keymap.c` 内联实现） |
| 其余 | 透明（`KC_TRNS`，即照常输出） |

> **注意**：原厂 Fn 层（layer 2/3）不再有按键指向，因此 **F1–F12、`~`、媒体键、RGB 亮度/速度/色相、`QK_BAT`、`QK_WLO`（Win-Lock 开关）等原厂 Fn 功能不再提供入口**。这是刻意的精简。原厂 `_WIN_FN` 已恢复为厂商原样（`-`/`=` = `F11`/`F12`），仅作保留。

### 底排方案（Base 层，Win/Mac 同步）

| 物理位置 | 当前键位 | 说明 |
| :--- | :--- | :--- |
| 右 Alt | `VIM_MOUSE` | **短按**进出鼠标模式（共享层 FSM）；不是 `MO(4)` |
| 右 Ctrl | `KC_RCTL` | 普通右 Ctrl |
| 原 Fn 位 | `MO(_FN)` | 按住进入 myfn 层（F 区/音量/无线等） |
| 原 Menu | `KC_APP` | 普通 Menu 键 |
| 右 Shift | `KC_RSFT` | 普通右 Shift（vim 开启时走右 Shift 懒发送） |

> **注意**：本表按 `keymap.c` 的底排实际取值（`… KC_SPC, VIM_MOUSE, MO(_FN), KC_APP, KC_RCTL`）修订。
> 旧文档曾写「Normal 下底排方向键移动指针」，该功能**不存在**；鼠标指针只在鼠标模式内由 `hjkl` 控制。

- 判定阈值 = QMK `TAPPING_TERM`（默认 **200ms**）。
- **运行时直接用编译键位**：`keymap.c` 用强符号覆盖 `keymap_key_to_keycode()` → `keycode_at_keymap_location_raw()`，忽略 EEPROM 里的 VIA 动态键位，直接从固件 `keymaps` 取键。
- **不再在启动时重灌动态键位**：V2.0 曾在 `keyboard_post_init_user()` 用 `QMK_BUILDDATE` 哈希判断并调用 `dynamic_keymap_reset()`，结果首次刷入后**有线 USB 无法枚举**（设备管理器显示“未知设备”，按键无反应；蓝牙/2.4G 正常）。原因是该调用把约 960 字节逐字节写入 QK61 的模拟 flash（`common/user_eeprom.c`，每次 program 都 `__disable_irq()`），正好压在 USB 枚举窗口上。由于 `keymap_key_to_keycode()` 已是强覆盖、EEPROM 动态键位根本不会被读取，这段重灌是多余的，V2.1 起**彻底移除**（详见第十三节）。
- `DYNAMIC_KEYMAP_LAYER_COUNT = 5`（第 5 层必须同步调大，否则触发 QMK 静态断言）。
- **`Fn` + `Esc` 长按 3 秒 = 重置 EEPROM**（`eeconfig_init()`）并重启；短按 `Fn`+`Esc` 不输出任何键（先松 Fn 也不会漏出真 Esc）。
- **休眠/唤醒/重连**：保留 `DISABLE_CUSTOM_SLEEP`（厂商深睡会挂死本 MCU），改由 `qk61.c` 的 **C1 RF 状态机**管理：无线空闲 5 分钟 → 拉低 SDB 断电 RF；任意按键 / 插入 USB → `Init_Gpio_Infomation()` 恢复 SDB → 重握手 → 重发当前模式。另含**插线自动切 USB 并重新枚举**。**无 Fn+Enter 手动睡眠**。

## 二、Vim 模式与开关

Vim 模式**默认开启**，开机即处于 **Insert（打字）模式**。

| 操作 | 效果 |
| :--- | :--- |
| `Fn` + `Caps` **单击** | **开关 Vim 模式**（开=回到 Insert；关=进入透传，底灯变红，其余键位按厂商行为） |
| `Caps` **单击**（裸按） | **无任何效果**（不开关 Vim，也不进入 Normal） |
| `Caps` **长按** | **按下即进入 Caps 模式**（momentary，规格 `qmk-vim-fn/caps/`）：`1`–`0`/`-`/`=` = `F1`–`F12`（不带 Ctrl）；**其余键** = `Ctrl+键`（修饰键即 `Ctrl+修饰`）；松开 `Caps` 退出。**不改变 Vim 开关与模式**，Vim 关闭时同样可用 |
| `Esc`（Insert，非宽限） | **进入 Normal（不发送 Esc）** |
| `Esc`（Insert，3s 宽限内） | 发送真实 Esc，留在 Insert，并**重置 3s 宽限** |
| `Esc`（Normal 空闲） | 发送真实 Esc，**回到 Insert**，并**开启 3s 宽限**；同时 Esc 灯与 logo 电量灯转**橙** `#FF8000` 提示「已回到打字」，3s 后自动回 Insert 绿（窗口内再按 `Esc` 续期） |
| Visual / Visual Line 中 `Esc` | 真正退出可视并回 Normal（不发送 Esc） |
| 多键 pending 时 `Esc` | 仅取消 pending，不发送键 |
| `Tab` | **任何模式都直接透传**（真实按下/抬起/重复），`Alt+Tab` 正常 |

> **Esc 宽限（3s）**：只由「Normal 空闲按 Esc 回到 Insert」开启，窗口内再按 `Esc` 会重置计时；
> 其余进入 Insert 的路径（开机、`Fn+Caps` 开启 Vim、`i/a/o` 等）**没有宽限**，也**不亮橙**。
> 进入 Normal 现在**只由 `Esc`** 负责（`Caps` 长按是 Caps 模式）；**Vim 开关的唯一入口是 `Fn`+`Caps`**（裸 `Caps` 单击无效果）。

### Caps 长按模式（`F1`–`F12` + `Ctrl`）

**按下 `Caps` 即进入**（无需等待 200ms；若按下期间未按其它键就抬起，则撤销本次进入），松开退出；模式内按键**不经过 Vim 引擎**（Vim 开关/模式不变，Vim 关闭亦可用）。
规格与用例见 qmk-vim-fn 的 [`caps/readme.md`](qmk-vim-fn/caps/readme.md)、[`caps/testcase.md`](qmk-vim-fn/caps/testcase.md)。

| 模式内按键 | 发出 |
| :--- | :--- |
| `1` `2` `3` `4` `5` `6` `7` `8` `9` `0` `-` `=` | `F1`…`F12`（**不带 Ctrl**，Ctrl 也不按住） |
| **其余全部键**（含 `Enter`/`Space`/`Tab`/`Backspace`/方向键/字母/符号/修饰键） | **`Ctrl` + 该键**（`Shift` 按下即 `Ctrl+Shift`）；Ctrl 在第一个非 F 键按下时按住、最后一个松开时释放 |

> 别名冲突自担：`Ctrl+M`=Enter、`Ctrl+I`=Tab、`Ctrl+H`=Backspace、`Ctrl+[`=Esc。
> 模式内 `Esc` 发的是 `Ctrl+Esc`，不会触发 Vim 的 Esc 切换；**裸 `Caps` 单击无任何效果**，`Fn`+`Caps` 单击才是 Vim 开关。

## 三、Vim 功能

### 模式切换

`i` `a` `A` `I` `o` `O` 进入插入模式；`v` / `V` 进入可视 / 可视行模式。

### 移动（motions）

- `h` / `j` / `k` / `l` — 左 / 下 / 上 / 右
- `w` / `b` — 跳词首（系统 Ctrl+→ / Ctrl+←）
- `e` — 与 `w` 近似
- `0` / `$` — 行首 / 行尾（Home / End）
- `gg` / `G` — 文档首 / 文档末（Ctrl+Home / Ctrl+End）
- `Ctrl+F` / `Ctrl+B` — 下翻页 / 上翻页（PageDown / PageUp）
- `-` / `+` — 上一行行首 / 下一行行首
- `Backspace` — 左移一格；`Space` — 右移一格（仅在非鼠标上下文时，见第五节）

### 编辑（actions）

- `x` / `X` — 删除光标处 / 光标前字符（Del / Backspace）
- `r` — 替换单个字符；`R` — **不提供**（共享引擎未实现替换模式，见 qmk-vim-fn `vim/readme.md` §11；按下即普通透传）
- `s` / `S` — 改写当前字符 / 整行
- `c` / `d` / `y` + motion — 改写 / 删除 / 复制并移动（`cw`、`d$`、`yw`…）
- `cc` / `dd` / `yy` — 改写 / 删除 / 复制整行（`dd`：选中本行文本 + `Ctrl+X` 剪切 + `Backspace`，**末行可删**；`yy` 用整行选区复制。注意：`dd` 是「选中本行 + 剪切」两次主机编辑，**一次 `u` 只恢复一半**（共享层已取消自动双撤销））
- `C` / `D` / `Y` — 改写到行尾 / 删到行尾 / 复制到行尾
- `p` / `P` — 粘贴到光标后 / 前（Ctrl+V / Ctrl+C 方案）
- `u` / `Ctrl+R` — 撤销 / 重做
- `J`（`Shift+J`）— 合并下一行到当前行（End + Delete）
- `.` — 重复上一次操作（回放结束自动回到 Normal）

### 数字前缀

Normal 模式数字键作计数器（上限 2 位），可配合行操作（如 `3dd`）。进入 Visual / Visual-Line 会丢弃未消费计数。

### 查找

`/` / `?` — 调用宿主搜索（Ctrl+F）。

## 四、Shift + Esc 组合键

按住 `Shift` 再按 `Esc`（其它组合下 `Shift` 仍是普通 Shift）：

| 组合 | 输出 |
| :--- | :--- |
| 左 `Shift` + `Esc` | `~` |
| 右 `Shift` + `Esc` | `` ` `` |

> **右 `Shift` 特例（仅 Vim 开启时）**：为避免孤立 Shift 触发宿主输入法切换，右 `Shift` **单独按下/抬起不发送任何键**；
> 当它按住期间有别的键时才**临时补上左 Shift**（`右Shift+a` = `A`，`右Shift+Ctrl+C` = `Ctrl+Shift+C`），松开右 Shift 即撤下。
> Vim 关闭时右 `Shift` 与普通修饰键无异。
> 原「右 `Shift` + `1`..`0`/`-`/`=` = F1..F12」**已移除**（F 区改由 `_FN` 层提供）。
> Insert 模式下 `Ctrl`+数字按宿主的普通 `Ctrl+数字` 发送（共享层**没有**「右 Ctrl+数字 = F 区」的特殊处理；F 区只在 `Caps` 模式内由 `1`–`0` 触发）。

## 五、鼠标功能

由**底排 `VIM_MOUSE` 键（原右 Alt 位）短按切换**鼠标模式（`Insert` / `Normal` / `Visual` 均可进出；退出后回到进入前的模式）。

| 按键 | 行为 |
| :--- | :--- |
| `VIM_MOUSE` 短按 | 进 / 出鼠标模式（不输出键）；**长按**（≥200ms）= 普通右 Alt / Mac 右 GUI 修饰 |
| `h` / `j` / `k` / `l` | 按住移动鼠标指针：左 / 下 / 上 / 右（QMK mousekey 连续移动并加速），松开停止 |
| `Shift+J` / `Shift+K` | 滚轮向下 / 向上 |
| `Space` | 短按 = 鼠标左键单击；长按（≥200ms）= 按住左键拖动，松开释放 |
| `Enter` | 鼠标右键 |
| `Shift` | **不退出**鼠标模式（供 `Shift+J`/`Shift+K` 滚轮组合） |
| `Ctrl` / `Alt` / `Win`（左/右） | **按下即退出**鼠标模式，并**重新识别**该修饰键（回到进入前模式后正常生效） |
| 其它任意键 / `Esc` | 退出鼠标模式（**自动松开**所有按住的鼠标键/指针/滚轮），回进入前模式并**重新识别**该键 |

- 鼠标报告通过 QMK `mousekey` 产生，由 qk61 的 `es_send_mouse` 按当前模式（USB / BLE / 2.4G）发送。
- 组合键：**左Ctrl + 左Alt + 退格 = Ctrl+Alt+Delete**（拦截并吞掉原退格；左右修饰键需为左侧）。

## 六、灯效

RGB Matrix 64 灯：键位 0–60，logo 三灯 61–63。qk61.c 原有的 Caps / Win-Lock / 无线模式 / 电量数字等指示逻辑保持不变。

| 范围 | 效果 |
| :--- | :--- |
| 全局（除下述特定灯、以及 qk61.c 指示占用的灯） | `cycle_out_in_dual`，亮度 **53%**（106/200），速度 **11%**（≈28/255） |
| **按键短亮集合** | 26 字母 + `Backspace`(13) `Tab`(14) `Enter`(40) `LShift`(41) `RShift`(52) `LCtrl`(53) `LAlt`(55) `Space`(56) `Menu`(59) `RCtrl`(60)：按下跟随全局色相短暂亮灯（约 200ms），平时熄灭 |
| `Fn`(58) | **常亮**，跟随全局色相（底排定位灯） |
| `Esc`(0) | 显示当前 vim 模式色，亮度 **66%**（132/200） |
| logo 61–63 | 颜色 = 模式色，**亮灯个数 = 电量** |

**模式色**：Vim 关闭 = 红；Normal = 蓝；Insert = 绿；**Visual = 紫 `#800080`；Visual-Line（`V`）= 洋红 rose `#FF0080`**；鼠标模式 = 青。

**回到打字提示（橙）**：`Normal --Esc--> Insert` 后 **3s 内**，Esc 灯与 logo 电量灯改显**橙** `#FF8000`（替换 Insert 绿），之后自动恢复。判据与 `0`=不覆盖 的裁决都在共享层 `vim_insert_flash_color()`（vim 开 + 模式 Insert + Esc 宽限窗口未过期；窗口用 32 位计时，不会因 16 位回绕复活）；只有这一条路径触发，开机 / `Caps` 开启 Vim / `i`/`a`/`o`/`s`/`c` 等进入 Insert 的方式**不亮橙**。亮度与灯数与模式色一致（Esc 66%、logo 按电量）。

**logo 电量个数**：`lit = (User_Batt_BaiFen * 3 + 99) / 100`；USB 有线 / 充电时 `User_Batt_BaiFen = 100` → 3 颗全亮。

**让位规则**（避免破坏 qk61.c 既有指示）：
- `Esc` / logo / Win 常亮：`User_Power_Low` 或 `Test_Led` 激活时让位。
- 按键短亮：`Key_Fn_Status`、`Led_Rf_Pair_Flg`、`User_Power_Low`、`Test_Led` 任一激活时跳过。

> 本键位下 logo 三灯被模式/电量占用，覆盖 VIA 的 logo 灯效；`keymaps/default` 仍保留 VIA logo 控制。

## 七、与 NUT65 的差异（未移植项）

- **鼠标模式**：由共享层 FSM 提供（`VIM_MOUSE` 键短按进出；模式内 `hjkl`=指针、`Shift+J`/`Shift+K`=滚轮、`Space`=左键、`Enter`=右键）。**没有**「Normal 下底排方向键移动指针」这类实现（旧文档曾如此描述，实际不存在）。
- **休眠与电源管理**：QK61 使用自研 **C1 RF 状态机**（仅在 `qk61.c`，保留 `DISABLE_CUSTOM_SLEEP`）；NUT65 仍为厂商原厂方案。
- NUT65 keymap 的 `rgb_record` 灯效录制、编码器音量等 QK61 不存在的部分未引入。

## 八、文件结构

- `keyboards/qk61/keymaps/vim/keymap.c` — 键位、vim 交互、鼠标、灯效
- `keyboards/qk61/keymaps/vim/config.h` — 灯位/亮度等键盘侧配置、`MYFN_LAYER`、`DYNAMIC_KEYMAP_LAYER_COUNT`
- `keyboards/qk61/keymaps/vim/rules.mk` — 引入 `qmk-vim-fn` 共享层源文件
- `keyboards/qk61/keymaps/vim/qmk-vim-fn/` — **子模块**：vim 引擎
- 「新 Fn 层」myfn 约定见 `qmk-vim-fn/fn` 仓库文档（本 keymap 内联实现，无子模块）
- `keyboards/qk61/qk61.c` — 厂商代码；本方案改动：指示灯钩子转调 keymap、`KC_SPC`/`KC_LGUI`/`KC_RGUI` 转交 `process_record_user`

## 九、编译与刷写

```bash
export PATH=<项目根>/toolchain/usr/bin:$HOME/.local/bin:$PATH
export QMK_HOME=<qmk_firmware 目录>

make qk61:vim
make qk61:vim:flash
```

产物：`qk61_vim.bin` / `qk61_vim.hex`（留档于仓库 `output/`，按版本另存为
`output/qk61_vim_vX.Y.{bin,hex}`，配套 VIA 定义另存 `output/qk61_vim_vX.Y_via.json`）。

## 十、已知局限

与 NUT65 Vim 一致：`e` 近似 `w`；`^`、`W`/`B`/`E`、`f`/`t`/`;`/`,`、`%`、mark、正则搜索、文本对象/块选/寄存器/宏、`~`、`zz` 等无法通过键码实现（**缩进 `<`/`>` 已实现**：`>>`/`<<`/`>motion`/`>0`）；`R`/`r` 在中文输入法激活时需先切英文；`.` 重复仅覆盖 vim 引擎录制范围。`dd` 用「选中本行文本 + `Ctrl+X` + `Backspace`」：**末行可删**、光标停在上一行行尾；已知取舍是**首行会留一个空行**。`dd` 由多次主机编辑组成，**一次 `u` 只恢复一半**（共享层已取消自动双撤销，见 `qmk-vim-fn/vim/readme.md` §3）。

## 十一、参考项目地址

- QMK：https://github.com/qmk/qmk_firmware
- 社区 QMK-VIM（上游引擎）：https://github.com/andrewjrae/qmk-vim
- 本方案共享层 `qmk-vim-fn`（fork）：https://github.com/springremember/qmk-vim-fn
- 「新 Fn 层」myfn 约定：https://github.com/springremember/qmk-vim-fn/fn
- QK61 官方移植参考：https://github.com/springremember/qmk_firmware_QK61

## 十二、问题记录（V1.0）

> 为 V1.0 冻结整理的踩坑记录，供日后重构参考。引擎级细节见 `qmk-vim-fn/vim/changes.md` 的「V1.0 问题记录」。
> **V1.0 固件另存改名**：`output/qk61_vim_v1.0.bin` / `.hex`（与 `output/qk61_vim.bin` 内容一致，仅作版本留档，避免被后续重构覆盖）。

### 通用（引擎，两键盘共有）
- **E1 dd**：C1 的裸 `Ctrl+X` 只在 VSCode 类有效（Notepad 无效）；末行曾是难点。V1.0 定为 `Home×2 + Shift+End + Ctrl+X + Backspace`：**末行可删**、`p` 可粘；代价是**首行留空行**。
- **E2 卡 Shift**：引擎无条件回写修饰键会放大一次丢失的释放；触发点 `pr_boot_combo` 带 Shift 跳 bootloader → 修复为跳转前清报告。
- **E3 Alt+Tab 卡 Tab**：只处理按下、先松 Alt 时 Tab 释放被引擎吞 → 对称透传 + 取消半途操作符。
- **E4 双撤销**：dd 是两次编辑 → `u`/重做自动双步（间隔 50ms）。
- **E5 子模块错配**：bump 后必须重编并校验产物哈希。
- **E6 集成**：`layer_count` 变更需重导 VIA layout、重编、注意 EEPROM。

### QK61 特有
- Tab 用**无条件透传**（`if (KC_TAB) return true;`），因此**没有** NUT65 的卡 Tab 问题。
- `_FN`：`-`/`=`=F11/F12、`[`/`]`=音量（V1.0）；`Fn+1..0`=F1..F10。
- 移除「右 `Shift`+数字/`-`/`=` 出 F 区」（保留 `Shift+Esc`）。
- Insert 模式 `Ctrl`+数字 = 宿主的 `Ctrl+数字`（无 F 区特殊处理）。
- 原厂 Fn 层（layer 2/3）逐键保留；原厂刷机 = 按住 `Esc` 插线 / PCB Reset（无 Fn 组合）。
- 与 NUT65 的专属差异（Enter 长按右键 / Ctrl+Alt+Del / 亮度）**不互相**同步。

## 十三、问题记录（V2.0 → V2.1）

### P1 有线 USB 无法枚举（“未知设备”）
- **现象**：刷入 V2.0 后，**有线 USB** 模式亮灯但主机识别为“未知设备”、按键无反应；**蓝牙 / 2.4G 完全正常**；原厂固件正常；重刷 V1.0 立即恢复。
- **根因**：V2.0 的 `keyboard_post_init_user()` 在 `QMK_BUILDDATE` 哈希与 EEPROM 记录不一致时调用 `dynamic_keymap_reset()`（`5 层 × 6 行 × 16 列 × 2 字节 ≈ 960 字节`）。QK61 用 `EEPROM_DRIVER = custom`（`common/user_eeprom.c` 的模拟 flash），`eeprom_write_block()` 对每个变化字节调用 `ee_write_variable()` 做一次 flash program 并 `__disable_irq()`，必要时还会整页（8KB）搬移。该过程发生在 `protocol_pre_init()`（USB 已 connect、枚举进行中）之后、`protocol_post_init()` 之前，长时间关中断使 USB 控制传输超时，枚举失败。
- **为何与引擎无关**：引擎/共享层代码在三种模式下运行路径一致；只有 USB 走枚举，故只有 USB 坏。
- **修复（V2.1）**：删除该 build-id 判断与 `dynamic_keymap_reset()`（及随之无用的 `version.h` / `dynamic_keymap.h`）。`keymap_key_to_keycode()` 强覆盖已保证运行时始终用编译键位，EEPROM 动态键位不会被读，无需重灌。
- **验证**：诊断固件 D1（仅删该段）刷入后 USB 立即恢复 → 定位确认；正式 V2.1 归档 `output/qk61_vim_v2.1.{bin,hex}`。
- **教训**：启动期（尤其枚举窗口内）避免大批量 EEPROM/flash 写；需要刷新动态键位时应延后到枚举完成之后，或依赖 VIA 原生机制。

### P1′ 有线 USB 再度无法枚举（V2.5 → V2.6 → V2.7，同一机制的另一触发源）
- **现象**：刷入 V2.5（09-24 构建）后，与 V2.0 完全相同的症状复现（有线亮灯但“未知设备”，蓝牙/2.4G 正常）。
- **根因**：**不是**本次共享层改动（`keymap.c`/`qk61.c`/`config.h`/`rules.mk` 与 V2.4 逐字节相同；共享层 diff 仅引擎逻辑，不写 flash）。真正的触发源是 **QMK/VIA 原生的 `via_init()`**：`quantum/via.c` 的 `via_eeprom_is_valid()` 用 **`QMK_BUILDDATE` 的年月日** 生成 magic，与 EEPROM 内 magic 比对；构建日期一变即判无效 → `via_init()` 调用 `eeconfig_init_via()` → `dynamic_keymap_reset()`（≈960 字节）+ 宏复位。QK61 的模拟 flash 逐字节 program、整页 **8KB**（`common/user_eeprom.c` `PAGE_SIZE`），一次写还可能触发关中断的整页 `ee_format`，长时间关中断压垮枚举。
- **`via_init()` 执行时机**：`quantum/main.c` 顺序为 `keyboard_setup()`（内含 `eeprom_driver_init()` → `keyboard_pre_init_quantum()` → `keyboard_pre_init_kb()` → `keyboard_pre_init_user()`）→ `protocol_pre_init()`（USB connect，**枚举开始**）→ `keyboard_init()`（**`via_init()` 在此**）。故 `via_init()` 的重灌落在枚举窗口内。
- **修复（V2.7，定稿）**：在 `keymap.c` 用 `keyboard_pre_init_user()` 提前刷新 magic —— 它运行在 `keyboard_setup()` 内、**USB connect 之前**，任何 flash 写 / 整页格式化都在枚举窗口之外；随后 `via_init()` 见 magic 已有效即跳过重灌。`keymap_key_to_keycode()` 强覆盖使 VIA 动态键位永不被读，故只刷 3 字节 magic 即可：
  ```c
  void keyboard_pre_init_user(void) {
      if (!via_eeprom_is_valid()) via_eeprom_set_valid(true);
  }
  ```
  > 注：V2.6 曾在 `via_init_kb()` 做同样的事，但它运行在 `keyboard_init()` 内、**枚举进行中**，3 字节写仍可能触发整页 `ee_format` 而压垮枚举，故 V2.6 无效；V2.7 把写入前移到 `keyboard_pre_init_user()`。
  > 仅 QK61 使用（NUT65 无强覆盖、依赖 VIA 动态键位，不能这样做）。归档 `output/qk61_vim_v2.7.{bin,hex}`。
- **教训**：`.build/obj_*/src/version.h` 的 `QMK_BUILDDATE` 跨天即变 → VIA magic 失效；凡「不使用 VIA 动态键位」的键盘，应把 magic 刷新放到 USB connect **之前**（`keyboard_pre_init_user`），不要在枚举窗口内的 `via_init_kb()` 里做。

### P1″ 定盘：真因是「镜像体积/布局」，非 VIA magic（V2.8 修复）
- **推翻 P1′**：做一个与 V2.4 **逐字节仅差 3 字节**（仅 `QMK_BUILDDATE` 日期位 `23→25`）的对照固件（今天构建、源码完全相同），实测**有线正常** ⇒ **构建日期 / VIA magic 不是根因**。
- **真因（实测规律）**：QK61/FS026 上**固件镜像体积/布局**是敏感点——
  - `c8ce99c` + V2.4 keymap = **81752 B（0x13F58）→ 正常**
  - 同源仅 +16 B（加一个只读 `via_eeprom_is_valid()` 钩子）= **81768（0x13F68）→ 有线枚举失败**
  - 纯数据 padding（无任何钩子/VIA/行为）使镜像变大同样触发；`a025d24` 全功能 = 81808 → 失败
  - 蓝牙/2.4G 正常（不经过 USB 枚举窗口）
  - 单处逻辑回退无效、与代码语义无关 ⇒ **不是某个函数，而是镜像跨过 ~0x13F60 附近的边界**（FS026 具体边界未查清）。
- **修复（V2.8）**：对本 keymap 启用 **LTO**（`rules.mk: LTO_ENABLE = yes`）。全功能镜像 **81806 → 72580 B**（骤降 ~9 KB，远离敏感边界），实测**有线立即识别**。V2.9 启用 LTO 后为 **73024 B**，仍远低于阈值。
- **同时移除** V2.6/V2.7 的 `keyboard_pre_init_user`(VIA magic) 钩子——C1 对照已证明它针对的「跨天 magic」并非根因，留着徒增体积。
- 归档 `output/qk61_vim_v2.8.{bin,hex}`（72580 B）。
- **教训**：FS026/QK61 对镜像体积/布局敏感；改动 keymap/共享层后务必关注镜像大小，必要时用 `LTO_ENABLE` 压体积。另：切换 `qmk-vim-fn` 子模块后**必须 `make clean`**，否则引擎对象不会重编（会得到新旧混合的假象）。
