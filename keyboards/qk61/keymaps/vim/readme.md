# QK61 Vim 方案（keymaps/vim）

参考 NUT65 Vim 固件，在 QK61 上以 qmk-vim 引擎实现 Vim 绝大多数功能，纯固件实现、无需系统层软件。目标系统：Windows / Linux（Ctrl 方案，非 Mac）。

硬件：CIDOO QK61（VID `0x36B0` / PID `0x3035`，矩阵 6×16，RGB Matrix 64 灯，VIA，FS026）。与 NUT65 的差异见文末「未移植项」。

## 一、键位与层

沿用 QK61 厂商默认键位（`keymaps/default`），共 4 层：

| 层 | 含义 |
| :--- | :--- |
| 0 | Win Base |
| 1 | Mac Base（开机 Mac 模式时自动开启） |
| 2 | Win Fn |
| 3 | Mac Fn |

Base 层 Esc 由 `QK_GESC` 改为 **`KC_ESC`**（供 vim 拦截）；`~` 仍可在 Fn 层左上 `[2,0]=KC_GRV` 输入。

Win Fn 层（layer 2）的 `-` / `=` 改为 **`KC_VOLD` / `KC_VOLU`**（对齐 NUT65 `_FL`；Mac Fn 层 layer 3 厂商默认已是音量减/增）。其余键位与厂商默认逐键一致。

### 底排方案（Base 层，Win/Mac 同步）

| 物理位置 | 厂商默认 | 本方案 | 短按 | 长按 |
| :--- | :--- | :--- | :--- | :--- |
| 右 Alt | `KC_RALT` | `MO(2)` | — | Fn 层 |
| 原 Menu | `KC_APP` | `MT(MOD_RALT, KC_LEFT)` | `←`（Normal 下=鼠标左移） | 右 Alt |
| 右 Ctrl | `KC_RCTL` | `MT(MOD_RCTL, KC_DOWN)` | `↓`（Normal 下=鼠标下移） | 右 Ctrl |
| 原 Fn | `MO(2)` | `MENU_TAP_RIGHT` | `→` | 鼠标右键（仅 Normal，一次） |
| 右 Shift | `KC_RSFT` | `MT(MOD_RSFT, KC_UP)` | `↑`（Normal 下=鼠标上移） | 右 Shift |

> 物理右 Alt 位改为纯 Fn（`MO(2)`，不带方向/鼠标）；右 Alt 移到原 Menu 位，方向键与鼠标功能保留在原 Menu 位（Win：`MT(MOD_RALT, KC_LEFT)`；Mac：`MO(3)` 与 `MT(MOD_RGUI, KC_LEFT)`）。

- 判定阈值 = QMK `TAPPING_TERM`（默认 **200ms**）。
- `Fn` / 右 Ctrl / 右 Shift 使用 QMK 原生 `LT` / `MT`；Menu 键为自定义 tap-hold（`MENU_TAP_RIGHT`）。
- 在 Normal 鼠标上下文（见第五节）下底排 ←/↓/↑ 三键不再触发 右 Alt / 右 Ctrl / 右 Shift，而是移动鼠标指针；Menu 键仍为短按 `→`、长按鼠标右键。

## 二、Vim 模式与开关

Vim 模式**默认开启**，开机即处于 **Insert（打字）模式**。

| 操作 | 效果 |
| :--- | :--- |
| `Caps` **按住**（≥200ms） | 临时 Normal 模式（momentary），松开回到原模式 |
| `Caps` **短按** | 在 Normal / Insert 间互切 |
| `Fn` + `Caps` | 开关 Vim 模式（关闭后进入透传，底灯变红，其余键位按厂商行为） |
| `Esc` **短按** | 向宿主发送真实 Esc，固件模式不变 |
| `Esc` **长按**（≥200ms） | 切到 Normal 模式，不发送键码 |
| Replace（`R`）中 `Esc` | 退出替换模式回 Normal，不发送 Esc |
| Visual / Visual Line 中 `Esc` | 交给 qmk-vim 原生处理：退出可视并回 Normal |

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
- `r` — 替换单个字符；`R` — 替换模式（持续覆盖，`Esc` / `Caps` 退出）
- `s` / `S` — 改写当前字符 / 整行
- `c` / `d` / `y` + motion — 改写 / 删除 / 复制并移动（`cw`、`d$`、`yw`…）
- `cc` / `dd` / `yy` — 改写 / 删除 / 复制整行
- `C` / `D` / `Y` — 改写到行尾 / 删到行尾 / 复制到行尾
- `p` / `P` — 粘贴到光标后 / 前（Ctrl+V / Ctrl+C 方案）
- `u` / `Ctrl+R` — 撤销 / 重做
- `J`（`Shift+J`）— 合并下一行到当前行（End + Delete）
- `.` — 重复上一次操作（有限范围录制）

### 数字前缀

Normal 模式数字键作计数器（`VIM_NUMBERED_JUMPS`），可配合行操作（如 `3dd`）。

### 查找

`/` / `?` — 调用宿主搜索（Ctrl+F）。

## 四、右 Shift 组合键

按住**右 `Shift`** 再按以下键（其它组合下右 `Shift` 仍是普通 Shift）：

| 组合 | 输出 |
| :--- | :--- |
| 左 `Shift` + `Esc`（可同时按右 `Shift`） | `~` |
| 右 `Shift` + `Esc`（仅右 `Shift`） | `` ` `` |
| 右 `Shift` + `1` `2` … `0` `-` `=` | F1 F2 … F10 F11 F12 |

> `Fn` + 数字（layer 2）仍是厂商原厂 F1~F12，未改动。

## 五、鼠标功能（部分移植）

在 Normal 模式、无修饰键、非替换模式（以下称「鼠标上下文」）下：

| 按键 | 行为 |
| :--- | :--- |
| 底排方向键 ← / ↓ / ↑（原 Menu / 右 Ctrl / 右 Shift 的轻按位） | **按住移动鼠标指针**（左 / 下 / 上；QMK mousekey 连续移动并加速），松开停止 |
| `Space` | 短按 = 鼠标左键单击；长按（≥200ms）= 按住左键拖动，松开释放 |
| Menu 键（= → 位） | 短按 = `→` 方向键；长按（≥200ms）= **一次鼠标右键** |

- 上述 ←/↓/↑ 三键在此上下文内不再输出方向键、也不再触发各自的 右 Alt / 右 Ctrl / 右 Shift 层/修饰；离开该上下文（Insert / Visual / 替换模式，或按住修饰键）后恢复原生 tap-hold 行为。
- Menu 键不参与鼠标移动：短按始终发 `→`；长按的右键仅在鼠标上下文内触发，其它上下文长按无动作。
- 鼠标报告通过 QMK `mousekey`（已启用）产生，由 qk61 的 `es_send_mouse` 按当前模式（USB / BLE / 2.4G）发送。
- 与 NUT65 不同，QK61 的无线鼠标发送路径无需额外的连接门控（厂商栈无 NUT65 的队列洪泛问题）。

## 六、灯效

RGB Matrix 64 灯：键位 0–60，logo 三灯 61–63。qk61.c 原有的 Caps / Win-Lock / 无线模式 / 电量数字等指示逻辑保持不变。

| 范围 | 效果 |
| :--- | :--- |
| 全局（除 26 字母、Esc、logo、以及 qk61.c 指示占用的灯） | `cycle_out_in_dual`，亮度 **60%**（120/200），速度 **11%**（≈28/255） |
| 26 字母键（A–Z） | **按键触发短暂亮灯**：跟随全局色相，亮度由满亮线性衰减到 0（约 200ms）；平时熄灭，不参与全局动画 |
| `Esc` | 显示当前 vim 模式色，与 logo 同步（index 0） |
| logo 61–63 | 颜色 = 模式色，**亮灯个数 = 电量** |

**模式色**：Vim 关闭 = 红；Normal = 蓝；Insert = 绿；Visual / Visual Line = 紫；替换（R）= 橙。

**logo 电量个数**：`lit = (User_Batt_BaiFen * 3 + 99) / 100`，即 0% 全灭、1–33% 亮 1 颗、34–66% 亮 2 颗、67–100% 亮 3 颗；USB 有线 / 充电时 `User_Batt_BaiFen = 100` → 3 颗全亮。

字母灯位（A–Z，由 `g_led_config.matrix_co` 与 `keyboard.json` 得出）：

```
A29 B46 C44 D31 E17 F32 G33 H34 I22 J35 K36 L37 M48 N47 O23 P24 Q15 R18 S30 T19 U21 V45 W16 X43 Y20 Z42
```

**让位规则**（避免破坏 qk61.c 既有指示）：
- `Esc` / logo：`User_Power_Low` 或 `Test_Led` 激活时跳过覆盖。
- 字母遮挡：`Key_Fn_Status`、`Led_Rf_Pair_Flg`、`User_Power_Low`、`Test_Led` 任一激活时跳过（Fn 模式指示恰好用 Q/W/E/R/T 五个字母灯）。

> 本键位下 logo 三灯被模式/电量占用，覆盖 VIA 的 logo 灯效；`keymaps/default` 仍保留 VIA logo 控制。

## 七、与 NUT65 的差异（未移植项）

按需求**未移植**以下与 QK61 无关或硬件不兼容的部分：

- **鼠标（部分移植）**：保留底排方向键移动指针、Space 左键（单击 / 长按拖动）、Menu 长按 = `Enter`；未移植 NUT65 的 End 键右键、左右方向键同按右键等（见第五节）。
- **休眠与电源管理**：NUT65 的 `Ctrl+右Alt+Insert` 深睡组合、插拔线自动切回无线、`lpwr` 相关逻辑移除。QK61 沿用其自带的软睡眠（`Fn` + 回车位）。
- NUT65 keymap 的 `rgb_record` 灯效录制、编码器音量等 QK61 不存在的部分未引入。

## 八、文件结构

- `keyboards/qk61/keymaps/vim/keymap.c` — 键位、vim 交互、鼠标、灯效
- `keyboards/qk61/keymaps/vim/config.h` — qmk-vim 功能开关
- `keyboards/qk61/keymaps/vim/rules.mk` — qmk-vim 源文件
- `keyboards/qk61/keymaps/vim/qmk-vim/` — qmk-vim 引擎（含 `dd` 末行修复、yank/paste 改 Ctrl+C/V）
- `keyboards/qk61/qk61.c` — 仅将指示灯用户钩子改名并转调 keymap（1 处）

## 九、编译与刷写

```bash
export PATH=<nut65>/toolchain/usr/bin:$HOME/.local/bin:$PATH
export QMK_HOME=<qmk_firmware 目录>

make qk61:vim ALLOW_WARNINGS=yes
make qk61:vim:flash
```

产物：`qk61_vim.bin` / `qk61_vim.hex`（留档于仓库 `output/`）。

## 十、已知局限

与 NUT65 Vim 一致：`e` 近似 `w`；`^`、`W`/`B`/`E`、`f`/`t`/`;`/`,`、`%`、mark、正则搜索、文本对象/块选/寄存器/宏、`~`、缩进、`zz` 等无法通过键码实现；`R`/`r` 在中文输入法激活时需先切英文；`.` 重复仅覆盖 vim 引擎录制范围。
