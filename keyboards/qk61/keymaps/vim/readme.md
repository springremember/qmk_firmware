# QK61 Vim 方案（keymaps/vim）

基于厂商默认键位（`keymaps/default`）改造的 QK61 方案。硬件：CIDOO QK61（VID `0x36B0` / PID `0x3035`，矩阵 6×16，RGB Matrix 64 灯，VIA）。

## 一、本方案改动

1. **原 Menu 键 → Fn**（底排 `KC_APP` 位）：长按 = Fn 层（layer 2），**短按 = `←`**
2. **原 Fn 键 → Menu**（底排 `MO(2)` 位）：长按 = `Menu`，**短按 = `→`**
3. **右 Ctrl**：长按 = 右 Ctrl，**短按 = `↓`**
4. **右 Shift**：长按 = 右 Shift，**短按 = `↑`**

> win Base 层（`_BL`）与 Mac Base 层（layer 1）同步应用；Fn 层（layer 2/3）保持厂商默认。

## 二、短按 / 长按

- 判定阈值 = QMK `TAPPING_TERM`（默认 **200ms**）
- 短按（<200ms）发方向键；长按（≥200ms）发原功能
- Fn / 右 Ctrl / 右 Shift 使用 QMK 原生 `LT` / `MT`；Menu 键为自定义 tap-hold（`MENU_TAP_RIGHT`，在 `matrix_scan_user` 里过阈值注册 `KC_APP`）

## 三、底排对照

| 物理位置 | 厂商默认 | 本方案 |
| :--- | :--- | :--- |
| 右 Alt | `KC_RALT` | `KC_RALT` |
| 原 Menu | `KC_APP` | `LT(2, KC_LEFT)` |
| 右 Ctrl | `KC_RCTL` | `MT(MOD_RCTL, KC_DOWN)` |
| 原 Fn | `MO(2)` | `MENU_TAP_RIGHT`（短按 `→`，长按 `Menu`） |

行尾：`KC_RSFT` → `MT(MOD_RSFT, KC_UP)`。

## 四、编译与刷写

```bash
export PATH=<nut65>/toolchain/usr/bin:$HOME/.local/bin:$PATH
export QMK_HOME=<qmk_firmware 目录>

make qk61:vim ALLOW_WARNINGS=yes
make qk61:vim:flash
```

产物：`qk61_vim.bin` / `qk61_vim.hex`（留档于仓库 `output/`）。

## 五、说明

- 目录结构：`keyboards/qk61/keymaps/vim/keymap.c`（本文件同目录）。
- 其余键位、Fn 层功能与厂商默认逐键一致，仅上表 4 处改动。
