# NUT65 Vim 固件

基于 qmk-vim 社区项目的 NUT65 键盘固件，在 QMK 固件层面模拟 Vim 绝大多数功能，纯固件实现、无需任何系统层软件。目标系统：Windows / Linux（Ctrl 方案，非 Mac）。

> **版本 V1.0（冻结）**：引擎锁定 `qmk-vim` `v1.0`（子模块 commit `62bb338`），约定锁定 `qmk-myfn` `v1.0`。踩坑/问题记录见 **第十七节**；V1.0 固件另存 `output/leku_nut65_vim_v1.0.bin`（避免被后续重构覆盖）。

## 一、模式与开关

### Vim 永久开启

Vim 模式**默认开启**，开机即处于 **Insert（打字）模式**。可通过 `Fn` + `Caps` 关闭/开启 Vim（关闭后固件进入"透传"状态，指示灯变红，其余键位均按厂商行为使用）。

### 模式切换

| 命令 | 效果 |
| :--- | :--- |
| `i` / `a` / `A` / `I` / `o` / `O` | 进入插入模式（`i` 光标处、`a` 后移一格、`I` 行首、`A` 行尾、`o` 下一行新行、`O` 上一行新行） |
| `v` / `V` | 进入可视模式 / 可视行模式 |
| `Caps` **按住**（≥200ms） | 临时 Normal 模式（momentary），松开回到原模式（从 Insert 来则回打字） |
| `Caps` **短按** | 在 Normal / Insert 间互切（Insert→Normal；已在 Normal 则→Insert） |
| `Esc` **长按**（≥200ms） | 切到 Normal 模式（不发送键码） |
| `Fn` + `Caps` | 开关 Vim 模式 |
| `Esc` 短按 | 见下方 Esc 行为表 |

### Esc 行为

| 操作 | 效果 |
| :--- | :--- |
| 短按 `Esc` | 向宿主发送真实 Esc（清高亮/取消/退出输入法候选），固件模式**不变** |
| 长按 `Esc`（≥200ms） | 固件切到 Normal 模式，不发送任何键码 |
| Replace（`R`）中按 `Esc` | 退出替换模式回 Normal，不发送 Esc |
| Visual / Visual Line 中按 `Esc` | 交给 qmk-vim 原生处理：退出可视并回 Normal（含取消选区） |

### 电源开关（休眠 / 唤醒）

按 `fn/readme` §1/§5 约定实现：`Fn` + `L` 短按休眠，`Fn` + 右上角键是唯一唤醒组合。

| 操作 | 效果 |
| :--- | :--- |
| `Fn` + `L` **短按** | 进入深睡（关闭/省电，近似关机）。前置：无物理模式开关（NUT65 无）+ **非 USB 有线**；USB 有线时 → 空跑（吞键、不输出） |
| `Fn` + **右上角键**（`_BL` 为 `Delete`、`_MBL` 为 `Insert`，矩阵 `[0,14]`） | 唤醒，恢复无线模式；此为**唯一**唤醒组合 |
| 深睡中按其它任意键 | 仅瞬时唤醒 MCU，随即自动重回深睡（不会误开机） |
| 拔掉 USB 线 | 自动回到“切到 USB 前正在使用的无线设备”（如蓝牙 1），而非默认 2.4G |
| 插上 USB 线 | 自动进入有线模式；深睡中插线立即恢复有线 |

> 唤醒路径与厂商自动睡眠/唤醒（`wls.c`）保留；插/拔线自动切换沿用并增强原厂行为。
> 注：V1.0 的手动深睡组合 `Ctrl` + 右`Alt` + `Delete` 已于 v2.11 移除，运行中按它会作为普通 `Ctrl+Alt+Delete` 发给宿主。

## 二、功能清单

### 移动（motions）

- `h` / `j` / `k` / `l` — 左 / 下 / 上 / 右（方向键）
- `w` / `b` — 跳词首（系统 Ctrl+→ / Ctrl+←；`b` 回上一词首）
- `e` — 与 `w` 近似（系统词跳只能到词首，无法精确到词尾）
- `0` / `$` — 行首 / 行尾（Home / End）
- `gg` / `G` — 文档首 / 文档末（Ctrl+Home / Ctrl+End）
- `Ctrl+F` / `Ctrl+B` — 下翻页 / 上翻页（PageDown / PageUp）
- `-` / `+` — 上一行行首 / 下一行行首
- `Backspace` / `Space` — 左移 / 右移一格

### 行首行尾细节

- `^`、`W` / `B` / `E` 等需要读取文本内容的移动不精确，详见"已知局限"。

### 编辑（actions）

- `x` / `X` — 删除光标处字符 / 删除光标前字符（Del / Backspace）
- `r` — 替换单个字符（Delete 后输入新字符）
- `R` — **替换模式**：进入后持续逐个覆盖字符（每键 = Delete + 输入），光标前进；行尾自动变为插入；按 `Esc` 或 `Caps` 退出（替换模式期间底部灯条变橙）
- `s` / `S` — 改写当前字符 / 改写整行（近似 vim 语义）
- `c` / `d` / `y` + motion — 改写 / 删除 / 复制并移动（`cw`、`d$`、`yw`…）
- `cc` / `dd` / `yy` — 改写 / 删除 / 复制整行
- `C` / `D` / `Y` — 改写到行尾 / 删到行尾 / 复制到行尾
- `p` / `P` — 粘贴到光标后 / 前
- `u` / `Ctrl+R` — 撤销 / 重做
- `J`（Shift+J）— 合并下一行到当前行（End + Delete，不插空格）
- `.` — 重复上一次操作（有限范围录制，见"已知局限"）

### 数字前缀（Normal 模式计数器）

- Normal 模式数字键作计数器（VIM_NUMBERED_JUMPS），可配合行操作使用（如 `3dd`）。

### 查找与定制键

- `/` / `?` — 调用宿主搜索（Ctrl+F）

### 鼠标模拟（仅 Normal 模式、无修饰键时）

| 键 | 行为 |
| :--- | :--- |
| `↑` `↓` `←` `→`（物理方向键） | 移动鼠标指针（按住连续移动，带加速） |
| `←`+`→` 同时按住 | 右键单击一次（期间停止移动，松开其一恢复移动） |
| `Space` 短按 | 鼠标左键单击 |
| `Space` 长按（≥200ms） | 按住左键拖动，松开释放 |
| `End` 键 | 右键单击一次 |

> `Enter` 在 Normal 模式仍是真实 Enter；h/j/k/l 仍是文本光标移动，互不影响；带修饰键（Shift/Ctrl 等）时方向键保持原行为；Insert/Visual 模式不受影响。
>
> **无线链路门控**：仅当 USB 有线或无线模块已连接（`MD_STATE_CONNECTED`）时才会产生鼠标报告；蓝牙/2.4G 瞬断的瞬间，方向键/空格/End 自动回落为 vim 原生行为，键盘保持可用，连接恢复后鼠标功能自动恢复。此门控用于防止厂商无线栈在模块未连接时发送鼠标报告导致的 smsg 队列洪泛死机（症状：所有按键失效、必须拨无线物理开关）。

## 三、grave：Shift + Esc 组合键

按住 `Shift` 再按 `Esc`（其它任何按键组合下 `Shift` 仍是普通 Shift）：

| 组合 | 输出 |
| :--- | :--- |
| 左 `Shift` + `Esc`（可同时按右 `Shift`） | `~` |
| 右 `Shift` + `Esc`（仅右 `Shift`） | `` ` `` |

> 原先的「右 `Shift` + `1`..`0`/`-`/`=` = F1..F12」**已移除**（F 区改由 `_FN` 层提供）。
> `Fn` + `1`..`0` = `F1`..`F10`；`Fn` + `-`/`=` = `F11`/`F12`；`Fn` + `[`/`]` = 音量减/增（myfn 约定）。
> Insert 模式下数字键普通输入即可（原有的「右 `Ctrl`+数字 = F1~F10」已移除；NUT65 也没有右 `Ctrl`）。
>
> 底排 Space 右侧为**鼠标模式键（轻按）/ 右`Alt`（长按）** 与 `Fn`：Win 层为 `… Space, 鼠标/右Alt, Fn, ←, ↓, →`；Mac 层为 `… Space, 鼠标/右Cmd, Fn, ←, ↓, →`。

## 四、Vim 状态指示（RGB）

模式色作用于**底部 80 灯条**（LED 71–150，亮度 6%）以及 **`Esc` 键**（LED 56，亮度 60%）；其余键位跟随全局灯效。底部灯条与 `Esc` 每帧强制写色，覆盖厂商灯效，不受 RGB_MOD / RL_MOD 切换影响：

| 模式 | 底部灯条颜色 |
| :--- | :--- |
| Normal | 蓝 |
| Insert | 绿 |
| Visual / Visual Line | 紫 |
| Vim 关闭（透传） | 红 |
| R 替换模式 | 橙 |

**底部灯条 = 电量指示（第一优先级，亮度 6%）**：80 颗底部灯中亮起的颗数固定由电量决定——从左右两端向中间熄灭，只保留中央与电量等比的灯段（如 50% 电量亮中央 40 颗）。模式/状态只影响该段灯的颜色（见上表），不影响亮灯数量。USB 有线（插线）时视为满电，底部灯条**全亮**。
> 侧灯（底部灯条两端角落小灯 10/11/13/14）不参与电量显示，恒熄灭。
> `Esc` 显示模式色（LED 56，亮度 60%）；`Caps`、`Delete`（原 Insert 位）无固定灯，参与全局灯效。
> 左 `Ctrl` 键位灯：厂商原在充电（红）/充满（绿）/低电（红闪）时长亮占用该灯，现已禁用——该键灯始终跟随 RGB 动画（动态），无需任何设置。电量请看底部灯条（或 `Fn`+`Space` 数字区闪烁）。
> 全局灯效默认 **Solid Reactive Simple**（`keyboard.json`），颜色 `#ff4efd`（HSV 213/177）、speed 38、亮度 153（60%）。

## 五、与厂商固件的差异

键盘层定义（keymap）中：
- **新增 `_FN`（myfn 新 Fn 层）**：底排 `Fn` 键由 `MO(_FL)`/`MO(_MFL)` 改为 **`MO(_FN)`**。`_FN` 内容遵循 `qmk-myfn` 约定：`1..0`=F1..F10、`-`/`=`=F11/F12、`[`/`]`=音量减/增、`Q/W/E`=蓝牙 1/2/3、`R`=2.4G、`T`=有线（`KC_USB`）、`L`=休眠（短按，见 §一）、`Space`=电量（`HS_BATQ`）、`Esc`=初始化（`EE_CLR`）；`Fn`+右上角键=唤醒；其余吞键。
- `_FL` / `_MFL`（原厂 Fn 层）**原样保留、仅无进入途径**；`_DEFA` 亦然（其 `QK_BOOT` 改由 `_FN` 上的 `Fn`+右`Shift`+`Esc` 组合触发，见第六节）。
- `_BL`（win Base 层）按用户要求改动键位（最右列，Delete 下方依次）：`row1→KC_WFWD`（浏览器前进）、`row2→KC_WBAK`（浏览器后退）、`row3→KC_END`（End；Normal 模式下=鼠标右键），最右上 `Insert→Delete`，其余一致；右 `Shift` 组合键（grave）见上文
- `_BL` / `_MBL` 底排：Space 右侧依次为**鼠标模式键（轻按）/ 右`Alt`（长按）** 与 `Fn`——Win：`… Space, 鼠标/右Alt, Fn, ←, ↓, →`；Mac：`… Space, 鼠标/右Cmd, Fn, ←, ↓, →`

`Fn+Caps` 开关 Vim、`Fn+Esc` 初始化等由 keymap 在 `_FN` 激活时处理；Vim 功能仍为键码拦截实现。

仅对 `keyboards/leku/nut65/nut65.c` 做了 4 处最小改动（为让出 keymap 级钩子 / 适配 `_FN`）：
1. `process_record_user` 重命名为 `hs_process_record_user`（RGB 录制逻辑，原样保留）
2. `housekeeping_task_user` 重命名为 `hs_housekeeping_task_user`（充电/矩阵循环逻辑，原样保留）
3. `rgb_matrix_indicators_advanced_kb` 末尾补调用 `rgb_matrix_indicators_advanced_user`
4. `enum layers` 末尾补 `_FN`，并把 RGB 录制里对 Fn 键的 `MO(_FL)/MO(_MFL)` 特判扩展到 `MO(_FN)`（否则录制中 `Fn` 被吞、且释放时 `_FN` 会卡住）

## 六、编译与刷写

```bash
# 环境（首次）：
export PATH=<nut65>/toolchain/usr/bin:$HOME/.local/bin:$PATH
export QMK_HOME=<qmk_firmware 目录>

# 编译
make leku/nut65:vim ALLOW_WARNINGS=yes

# 刷写（先进入 bootloader）
make leku/nut65:vim:flash
```

产物：`leku_nut65_vim.bin`（复制到项目 output/ 目录留档）。

进入 bootloader 方式（任选其一）：
- 按住 `Fn` + `Right Shift` + `Esc`（**厂商原厂组合键**，按 myfn 最高优先级约束保留；本 vim keymap 已实现：`_FN` 上 `Esc` 解析为 `EE_CLR`，keymap 拦截为 `eeconfig_disable()`+`bootloader_jump()`）
- 按住 `Esc` 插入 USB 线（同时擦除持久化设置）
- 按住底部 PCB 的 Reset 键插入 USB 线

刷写工具使用 wb32-dfu（QMK Toolbox 或命令行 `wb32-dfu-updater` 均可）。

> 刷入后如默认灯效/键位未生效，可在 `Fn` 层按左上 `EE_CLR` 恢复默认设置。
> 本分支的子模块内容（`lib/chibios` 等）已随代码直接提交在 `lib/` 下，离线即可编译；压缩包归档在远程 `submodule-pack` 分支（本地 `子模板包/` 目录已移除）。

## 七、已知局限

固件无法精确实现的功能（需读取文本内容/编辑器内部状态）：
- `e` 跳词尾：系统词跳只能到词首，与 `w` 近似
- `^` 行首非空白、`W`/`B`/`E` 大写词、`f`/`t`/`;`/`,` 行内查找：无对应系统键
- `%` 括号配对、`mark`/`` ` `` 跳转、`/pattern` 正则搜索（固件以宿主 Ctrl+F 替代）、`*`/`#` 词搜索
- 文本对象（`iw`/`aw` 仅有限支持）、块选（Ctrl+V）、寄存器 `"a`、宏 `q@`、`gv`：无法通过键码实现
- `~`、`gu`/`gU`、`<`/`>` 缩进、`zz`/`zt`/`zb`：无通用宿主命令
- 数字前缀对纯移动的乘算精度取决于宿主（行操作 `dd` 等可乘算）
- `R`/`r` 在中文输入法激活时，键入字母会进入输入法预编辑，需先切英文（与真实 vim 使用习惯一致）
- `J` 合并不插空格、不去下一行缩进（固件无法感知行尾/行首空白）
- `.` 重复仅覆盖经过 vim 引擎录制范围的按键序列，非完整操作记录

## 八、硬件规格

- 型号：nut65（LEKU）
- 厂家源码：https://github.com/hangshengkeji/qmk_firmware/tree/master/keyboards/leku/nut65
- MCU：WB32FQ95（ARM，`wb32-dfu` bootloader）
- 三模：USB / 蓝牙×3 / 2.4G；RGB Matrix（键位 82 灯 + 底部灯条 80 灯）；旋钮编码器；VIA（6 层动态键位）
- 矩阵 6×15，LAYOUT 82 键；默认 6 层：`_BL` / `_FL` / `_MBL` / `_MFL` / `_DEFA` / `_FN`

## 九、项目目标与约束

**目标**：产出一个 nut65 固件，实现 vim 绝大多数功能。

**约束**
1. 音量 / F 区等按 **myfn 约定**放在新增的 `_FN` 层（`Fn+1..0`=F1..F10、`Fn+-`/`Fn+=`=F11/F12、`Fn+[`/`Fn+]`=音量减/增）；厂商 `_FL`/`_MFL` 原样保留、无进入途径。原厂 `Fn`+右`Shift`+`Esc` 刷机组合键按 myfn 最高优先级约束保留。
2. 键位改动以 `_BL`（win Base 层）为准（2026-09-17 最新）：`Insert`(row0 col14)→`Delete`；`Delete` 位(row1 col14)→`KC_WFWD`；PageUp 位(row2 col14)→`KC_WBAK`；PageDown 位(row3 col14)→`KC_END`。（历史曾为 Alt+F4 / 真 PageUp/PageDown，均已被用户否决）
3. 目标系统：Windows / Linux（Ctrl 方案，非 Mac）。

## 十、实现细节

- 采用 qmk-vim 引擎（全局状态机），所有 vim 功能为键码拦截实现；额外新增 `_FN` 层承载 myfn 约定键（F1-F12/音量/蓝牙/2.4G/有线/电量/初始化），原厂 `_FL/_MFL/_DEFA` 保留但无进入途径。保留 qmk-vim 原生 let-through（F 键/方向键/Ctrl/Alt 组合按默认透传）。
- 仅对 `keyboards/leku/nut65/nut65.c` 做 4 处最小改动：
  1. `process_record_user` 重命名 `hs_process_record_user`（RGB 录制逻辑原样保留）
  2. `housekeeping_task_user` 重命名 `hs_housekeeping_task_user`（充电/矩阵循环原样保留）
  3. `rgb_matrix_indicators_advanced_kb` 末尾补调 `rgb_matrix_indicators_advanced_user`
  4. `enum layers` 末尾补 `_FN`；RGB 录制里 Fn 键特判由 `MO(_FL)/MO(_MFL)` 扩展为含 `MO(_FN)`
- `keymaps/vim/keymap.c` 的 `process_record_user` 拆分为静态 handler（`pr_power_ins` / `pr_power_mods` / `pr_boot_combo` / `pr_shift_combos` / `pr_esc` / `pr_caps` / `pr_end_click` / `pr_mouse_emu`），判定顺序与原 if 链一致；电源组合键（RALT/Ctrl/row0col14）始终吞键，vim 引擎不收到裸修饰键事件。
- `keyboard.json` `dynamic_keymap.layer_count=6`；`encoder_map` 6 层，编码器旋钮 = 音量。
- 电源判定以 USB 主机活跃为主（`nut65.c hs_usb_active()`），充电引脚仅兜底；厂商 lpwr 的 `allow_timeout`/`allow_presleep` 钩子在"仍处 USB 档且无主机"时禁止/中止睡眠。工程改动位置：`keymaps/vim/keymap.c`（组合状态机、电量条、拔线恢复、RGB 恢复）+ `keyboards/leku/nut65/nut65.c`（`hs_usb_active`、lpwr 钩子否决）。

## 十一、Vim 功能宏（keymaps/vim/config.h）

启用：`VIM_I_TEXT_OBJECTS`, `VIM_A_TEXT_OBJECTS`, `VIM_G_MOTIONS`, `VIM_PASTE_BEFORE`, `VIM_REPLACE`, `VIM_DOT_REPEAT`, `VIM_NUMBERED_JUMPS`, `BETTER_VISUAL_MODE`

禁用：`VIM_COLON_CMDS`（冒号命令 `:w`/`:q` 完全删除）, `VIM_FOR_MAC`, `VIM_FOR_ALL`, `ONESHOT_VIM`, `VIM_W_BEGINNING_OF_WORD`

## 十二、构建环境补充

- 工具链：`<nut65>/toolchain/usr/bin`（gcc-arm-none-eabi 14.2，Debian trixie deb 解包）
- python 依赖：`~/.local/bin`（`pip install --user --break-system-packages`）
- 子模块版本：ChibiOS `2365f844`（HAL8.4）+ ChibiOS-Contrib `3ac181e4`（含 WB32FQ95xx）
- 编译：见"六、编译与刷写"，命令 `make leku/nut65:vim ALLOW_WARNINGS=yes`

## 十三、文件结构与产物

- `keyboards/leku/nut65/keymaps/vim/`（`keymap.c` / `config.h` / `rules.mk` / `readme.md` / `qmk-vim/`（**子模块**：`git@github.com:springremember/qmk-vim.git`））
- `output/`：`leku_nut65_default.bin`（基线）+ `leku_nut65_vim.bin`（vim 固件）

## 十四、修复历史（已知问题，均已修复）

- **dd（末行可删）**：`Home`×2 → `Shift+End`（选中本行文本）→ `Ctrl+X`（单次剪切，进剪贴板，`p` 可粘）→ `Backspace`（删掉残留空行的前换行）；`Ndd` 先按 count 扩展选区。**首行会留一个空行**。因剪切+退格是两次主机编辑，引擎用 `vim_extra_undos` 让一次 `u` / 重做自动重复一次，从而一次恢复/重做（切到其它键后失效）。见 `qmk-vim/src/{actions,modes}.c`。
- **dd 后 k 误删行**：dd 执行完必须调 `normal_mode()` 清 pending，否则 `process_func` 停在 `process_vim_action`、后续 motion 会再触发 delete。Esc 短按/长按在 Normal 模式也先 `normal_mode()` 取消 pending operator。
- **可视模式（Visual / Visual Line）Esc 退出**：可视模式下 Esc 直接落到 qmk-vim 原生处理（真实 Esc 退出）。
- **R 替换模式 Esc 无法退出 / 底条不恢复**：`normal_mode_user` 需用强符号覆盖（weak 会随机选中导致 `replace_active` 不清）。
- **dot repeat 多了 j**：Caps 切 Normal 后调 `add_repeat_keycode(KC_NO)` 停止记录。
- **开机（电源组合）后灯光不恢复**：`pw_boot` 显式 enable RGB + 走厂商 LPWR_WAKEUP 路径。
- **拔线回无线错误落到默认 2.4G**：冻结"切 USB 前"设备并在拔线后重试切回（如蓝牙1）。
- **Normal 模式 Alt+Tab 无任务视图（Alt 未保持）**：Tab 带 Alt 修饰时直接透传，不进 vim 引擎（否则 vim 会 tap 发出 LALT(Tab) 导致 Alt 松开）。**并用 `alt_tab_held` 记住这次透传**，使 Tab 的释放也总是放行——否则「先松 Alt、再松 Tab」时释放会被引擎吞掉，导致 **Tab 卡住/自动重复**；透传时若存在半途操作符（`d/y/c`）或 `g` 前缀则先 `normal_mode()` 取消。
- **无线模式死机（按键全无反应、必须拨无线物理开关）**：根因是鼠标报告在模块未连接时触发厂商 `wireless_send_mouse` 的 `devs_change` 假切换，洪泛 smsg 队列（40 槽 × 重试 40 次 + 阻塞 UART 写）导致 `wireless_send_keyboard` 的 `while(smsg_is_busy())` 自旋死锁。修复：keymap 侧 `mouse_link_ok()` 门控所有鼠标报告源头，厂商无线栈未动。
- **电源组合键演进**：旧 `grave+Space` 组合已移除 → 现 `Ctrl+右Alt+原Insert`；`ALT_TAB` 键已移除。
- **右键方案迭代**：右Alt右键 → 右Shift 单击右键 → 定稿 Normal 模式 `End` 键右键（非阻塞，housekeeping 40ms 自动释放）。
- **SQL 模板补全**：已删除（用户要求），`keymap.c` 中 `sql_*` 模块、`Ctrl+P` 触发、按键跟踪全部移除。

## 十五、协作规则

1. 禁止过度思考、冗长分析、长篇解释、自行猜测。
2. 需求不明确时，先直接向用户提问确认，再动手。
3. 响应必须简短直接，一句话能说清就不写两句。

## 十六、参考项目地址

- QMK：https://github.com/qmk/qmk_firmware
- 厂家：https://github.com/hangshengkeji/qmk_firmware
- 社区 QMK-VIM：https://github.com/andrewjrae/qmk-vim
- 本方案 qmk-vim（fork）：https://github.com/springremember/qmk-vim
- 「新 Fn 层」myfn 约定：https://github.com/springremember/qmk-myfn

## 十七、问题记录（V1.0）

> 为 V1.0 冻结整理的踩坑记录，供日后重构参考。引擎级细节见 `qmk-vim/CHANGES.md` 的「V1.0 问题记录」。
> **V1.0 固件另存改名**：`output/leku_nut65_vim_v1.0.bin` / `.hex`（与 `output/leku_nut65_vim.bin` 内容一致，仅作版本留档，避免被后续重构覆盖）。

### 通用（引擎，两键盘共有）
- **E1 dd**：C1 的裸 `Ctrl+X` 只在 VSCode 类有效（Notepad 无效）；末行曾是难点。V1.0 定为 `Home×2 + Shift+End + Ctrl+X + Backspace`：**末行可删**、`p` 可粘；代价是**首行留空行**。
- **E2 卡 Shift**：引擎无条件回写修饰键会放大一次丢失的释放；触发点 `pr_boot_combo` 带 Shift 跳 bootloader → 修复为跳转前清报告。
- **E3 Alt+Tab 卡 Tab**：只处理按下、先松 Alt 时 Tab 释放被引擎吞 → 对称透传 + 取消半途操作符。
- **E4 双撤销**：dd 是两次编辑 → `u`/重做自动双步（间隔 50ms）。
- **E5 子模块错配**：bump 后必须重编并校验产物哈希。
- **E6 集成**：`layer_count` 5→6 需重导 VIA layout、重编 default/vim、注意 EEPROM；分支裁剪误删 `keyboards/linker/wireless` 已恢复。

### NUT65 特有
- Alt+Tab 透传从导入起就是**有条件**的（仅 Alt 按住时）→ 「先松 Alt」时 Tab 释放被引擎吞 → 卡 Tab 的**根源**；V1.0 用 `alt_tab_held` 修好。
- 刷机组合 **`Fn`+右`Shift`+`Esc`**（原厂层叠语义）由 `pr_boot_combo` 实现；**跳转前 `clear_keyboard()`**，否则把 Shift 卡在宿主（E2）。
- `_FN`：`-`/`=`=F11/F12、`[`/`]`=音量；`Fn+1..0`=F1..F10。
- 移除「`Shift`+数字/`-`/`=` 出 F 区」与「Insert 模式右 `Ctrl`+数字」（NUT65 **无右 Ctrl**）。
- rgbrec：厂商 `nut65.c` 的 Fn 特判必须含 `MO(_FN)`，否则 RGB 录制中 Fn 被吞、释放卡层。
- 厂商 `_FL`/`_MFL`/`_DEFA` 原样保留、无进入途径。
