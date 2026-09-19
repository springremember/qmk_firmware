// Copyright 2024 qk61-vim
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "version.h"              // QMK_BUILDDATE (full build timestamp)
#include "dynamic_keymap.h"       // dynamic_keymap_reset()
#include "keymap_introspection.h" // keycode_at_keymap_location_raw()
#include "qmk-vim/src/vim.h"
#include "qmk-vim/src/modes.h"
#include "qmk-vim/src/process_func.h"
#include "common/rdmctmzt_common.h"
#include "common/user_battery.h"

// Vendor globals defined in qk61.c, used to yield the RGB indicator layer.
extern bool Key_Fn_Status;
extern bool User_Key_Batt_Num_Show;
extern uint8_t User_Key_Batt_Count;
extern bool Test_Led;

// qmk-vim current keycode processor (swapped to drive the replace-mode handler)
extern process_func_t process_func;

// Vendor MCU reset (qk61 common/user_system.c), used by the Fn+Esc EEPROM reset.
extern void mcu_reset(void);

#ifdef VIM_DOT_REPEAT
extern void add_repeat_keycode(uint16_t keycode);
extern void finish_recording_repeat(void);
#endif

enum layers {
    _WIN_BASE = 0,
    _MAC_BASE,
    _WIN_FN,
    _MAC_FN,
    _FN = MYFN_LAYER,
};

enum custom_keycodes {
    MENU_TAP_RIGHT = SAFE_RANGE,
};

/* Resolve every key from the compiled keymaps, ignoring the VIA dynamic keymap
 * (which QK61 otherwise loads from EEPROM and which shadows keymaps/vim). This
 * is a strong override of the weak keymap_key_to_keycode() in keymap_common.c. */
uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key) {
    if (key.row < MATRIX_ROWS && key.col < MATRIX_COLS) {
        return keycode_at_keymap_location_raw(layer, key.row, key.col);
    }
    return KC_NO;
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_WIN_BASE] = LAYOUT_60_ansi(
        KC_ESC,  KC_1,     KC_2,     KC_3,      KC_4,      KC_5,     KC_6,     KC_7,    KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,
        KC_TAB,  KC_Q,     KC_W,     KC_E,      KC_R,      KC_T,     KC_Y,     KC_U,    KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,
        KC_CAPS, KC_A,     KC_S,     KC_D,      KC_F,      KC_G,     KC_H,     KC_J,    KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,
        KC_LSFT, KC_Z,     KC_X,     KC_C,      KC_V,      KC_B,     KC_N,     KC_M,    KC_COMM,  KC_DOT,             KC_SLSH,            MT(MOD_RSFT, KC_UP),
        KC_LCTL, KC_LGUI,  KC_LALT,                        KC_SPC,                                MO(4),    MT(MOD_RALT, KC_LEFT), MT(MOD_RCTL, KC_DOWN), MENU_TAP_RIGHT
    ),
    [_MAC_BASE] = LAYOUT_60_ansi(
        KC_ESC,  KC_1,     KC_2,     KC_3,      KC_4,      KC_5,     KC_6,     KC_7,    KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,
        KC_TAB,  KC_Q,     KC_W,     KC_E,      KC_R,      KC_T,     KC_Y,     KC_U,    KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,
        KC_CAPS, KC_A,     KC_S,     KC_D,      KC_F,      KC_G,     KC_H,     KC_J,    KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,
        KC_LSFT, KC_Z,     KC_X,     KC_C,      KC_V,      KC_B,     KC_N,     KC_M,    KC_COMM,  KC_DOT,             KC_SLSH,            MT(MOD_RSFT, KC_UP),
        KC_LCTL, KC_LALT,  KC_LGUI,                        KC_SPC,                                MO(4),    MT(MOD_RGUI, KC_LEFT), MT(MOD_RCTL, KC_DOWN), MENU_TAP_RIGHT
    ),
    [_WIN_FN] = LAYOUT_60_ansi(
        KC_GRV,  KC_F1,    KC_F2,    KC_F3,     KC_F4,     KC_F5,    KC_F6,    KC_F7,   KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_BSPC,
        KC_TAB,  MD_BLE1,  MD_BLE2,  MD_BLE3,   MD_24G,    KC_PSCR,  KC_SCRL,  KC_PAUS, KC_I,     KC_O,     KC_P,     UG_SPDD,  UG_SPDU,  UG_NEXT,
        KC_CAPS, TO(0),    TO(1),    KC_D,      KC_F,      KC_INS,   KC_HOME,  KC_PGUP, KC_K,     KC_L,     UG_HUED,  UG_HUEU,            KC_ENT,
        KC_LSFT, RM_NEXT,  KC_NO,    RM_SATD,   RM_SATU,   KC_DEL,   KC_END,   KC_PGDN, RM_VALD,  RM_VALU,            KC_UP,              QK_BAT,
        KC_LCTL, QK_WLO,   KC_LALT,                        RM_TOGG,                               KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_NO
    ),
    [_MAC_FN] = LAYOUT_60_ansi(
        KC_GRV,  KC_BRID,  KC_BRIU,  KC_MCTL,   KC_LPAD,   KC_5,     KC_6,     KC_MPRV, KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_BSPC,
        KC_TAB,  MD_BLE1,  MD_BLE2,  MD_BLE3,   MD_24G,    KC_PSCR,  KC_SCRL,  KC_PAUS, KC_I,     KC_O,     KC_P,     UG_SPDD,  UG_SPDU,  UG_NEXT,
        KC_CAPS, TO(0),    TO(1),    KC_D,      KC_F,      KC_INS,   KC_HOME,  KC_PGUP, KC_K,     KC_PGUP,  UG_HUED,  UG_HUEU,            KC_ENT,
        KC_LSFT, KC_NO,    KC_NO,    RM_SATD,   RM_SATU,   KC_DEL,   KC_END,   KC_PGDN, RM_VALD,  RM_VALU,            KC_UP,              QK_BAT,
        KC_LCTL, KC_LALT,  KC_LGUI,                        RM_TOGG,                               KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_NO
    ),
    // 新 Fn 层（myfn 约定，见 qmk-myfn 文档）：音量 / 蓝牙·2.4G 切换；其余透明；
    // Fn+Space 电量、Fn+T 空跑（QK61 有物理开关）由 process_record_myfn() 处理。
    [_FN] = LAYOUT_60_ansi(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_VOLD, KC_VOLU, _______,
        _______, MD_BLE1, MD_BLE2, MD_BLE3, MD_24G,  _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,          _______,
        _______, _______, _______,                     _______,                              _______, _______, _______,          _______
    )
};
// clang-format on

/* ===== Force the compiled keymap into the VIA dynamic keymap once per flashed
 * firmware build.  VIA's own EEPROM magic only uses the year/month/day part of
 * QMK_BUILDDATE, so several builds on the same day do NOT reset the dynamic
 * keymap (which shadows keymaps/vim).  Hashing the full timestamp (including
 * time) makes every new build refresh the keymap on its first boot. ===== */
static uint32_t fw_build_id(void) {
    uint32_t h = 2166136261u;
    for (const char *s = QMK_BUILDDATE; *s; s++) {
        h ^= (uint8_t)*s;
        h *= 16777619u;
    }
    return h;
}

/* ===== Vim is always on: start in typing (Insert) mode ===== */
void keyboard_post_init_user(void) {
    uint32_t stored_id = 0;
    eeconfig_read_user_datablock(&stored_id, 0, sizeof(stored_id));
    if (stored_id != fw_build_id()) {
        dynamic_keymap_reset(); // write the compiled keymap into EEPROM
        uint32_t id = fw_build_id();
        eeconfig_update_user_datablock(&id, 0, sizeof(id));
    }

    enable_vim_mode();
    insert_mode();

    rgb_matrix_mode_noeeprom(RGB_MATRIX_CYCLE_OUT_IN_DUAL);
    rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), 106); // 53% of 200
    rgb_matrix_set_speed_noeeprom(28);                                           // ~11% of 255
}

/* ===== Esc release-swallow flag =====
 * Set when a mode exit already happened on Esc's press (visual exit handed to
 * qmk-vim, or leaving replace mode) so the matching release is swallowed and
 * does not emit a second, spurious real Esc. */
static bool esc_swallow_release = false;

/* ===== Esc tap/hold: short tap = real Esc, long press = Normal mode ===== */
#define ESC_HOLD_TIME 200
static uint16_t esc_press_timer = 0;

/* ===== Caps tap/hold: long hold = momentary Normal (release restores typing),
 * short tap = toggle Normal mode ===== */
#define CAPS_HOLD_TIME 200
static uint16_t caps_press_timer = 0;
static bool     caps_was_insert  = false;

/* ===== Normal mode Space: short = left click, long = hold left (drag) ===== */
#define SPC_HOLD_TIME 200
static uint16_t spc_press_timer = 0;
static bool     spc_holding     = false;

/* ===== Normal mode Enter: short = right click, long = hold right ===== */
#define ENT_HOLD_TIME 200
static uint16_t ent_press_timer = 0;

/* ===== Replace mode (vim R: overwrite chars until Esc) ===== */
static bool replace_active = false;

/* ===== Menu key (MENU_TAP_RIGHT) =====
 * Normal mouse context: pointer right (hold = mousekey acceleration).
 * Other modes:           tap = Right arrow, long press = KC_APP (Menu key). */
static uint16_t menu_timer   = 0;
static bool     menu_pressed = false;
static bool     menu_held    = false;
static bool     menu_ms_held = false; // 本次按下已 register MS_RGHT

/* ===== 已注册的鼠标方向键（切上下文后仍能在松开时反注册，防卡指针）===== */
static bool ms_move_held[3] = {false, false, false}; // 0=left 1=down 2=up

/* ===== Space/Enter 按下是否已被鼠标上下文接管（决定抬起是否消费）===== */
static bool spc_ms_pressed = false;
static bool ent_ms_pressed = false;

/* ===== Fn+Esc：按下被 Fn 吞掉时，抬起同样吞掉，避免先松 Fn 漏出真 Esc ===== */
static bool fn_esc_swallow = false;

/* ===== Fn + Esc held >= 3s = reset EEPROM (eeconfig_init) + reboot ===== */
#define RESET_HOLD_MS 3000
static uint16_t reset_timer  = 0;
static bool     reset_armed  = false;
static bool     reset_fired  = false;

static inline bool fn_layer_active(void) {
    return layer_state_cmp(layer_state | default_layer_state, MYFN_LAYER);
}

/* 维护厂商 Fn 状态标志（Fn 指示灯 + 闪灯让位）。 */
layer_state_t layer_state_set_user(layer_state_t state) {
    Key_Fn_Status = layer_state_cmp(state | default_layer_state, MYFN_LAYER);
    return state;
}

/* ===== myfn 约定（内联实现；约定文本见 qmk-myfn 仓库文档）=====
 * 规则：前置满足→执行；前置不满足→吞键（空跑）；未声明→透传。
 * QK61：Fn+Space=电量（有电池→执行）；Fn+T=切有线（QK61 有物理开关→空跑=吞）；
 *       Q/W/E/R 由 qk61.c 处理；`-`/`=` 音量、Caps、Esc 由各自分支处理。 */
static bool fn_batt_held = false;

static bool process_record_myfn(uint16_t keycode, keyrecord_t *record) {
    if (keycode == KC_SPC) { // 电量
        if (record->event.pressed) {
            if (!fn_layer_active()) {
                return true; // 非 myfn 层 → 透传（普通 Space）
            }
            fn_batt_held           = true;
            User_Key_Batt_Num_Show = true;
            User_Key_Batt_Count    = 0;
            return false;
        }
        if (fn_batt_held) { // 松开：即使已离开 myfn 层也要收尾
            fn_batt_held           = false;
            User_Key_Batt_Num_Show = false;
            User_Key_Batt_Count    = 0;
            return false;
        }
        return true;
    }
    if (fn_layer_active() && keycode == KC_T) { // 切有线：有物理开关 → 空跑（吞）
        return false;
    }
    return true; // 其余键透传给其它分支
}

/* ===== 按键短暂亮灯 =====
 * LED 列表：26 字母 + Backspace/Tab/Enter/Shift/Ctrl/Alt/Space/Menu。
 * 按物理矩阵位（g_led_config.matrix_co）触发，兼容 MT/MO 包裹键。
 * 注意：Win(LED54) 不在此列，改为常亮（见 rgb 渲染）。 */
#define FLASH_MS 200
static const uint8_t flash_led[] = {
    // 26 字母（A-Z 对应灯位）
    29, 46, 44, 31, 17, 32, 33, 34, 22, 35, 36, 37, 48, 47, 23, 24, 15, 18, 30, 19, 21, 45, 16, 43, 20, 42,
    // Bksp13 Tab14 Enter40 LShift41 RShift52 LCtrl53 LAlt55 Space56 RAlt/RGUI58 RCtrl59 Menu60
    13, 14, 40, 41, 52, 53, 55, 56, 58, 59, 60};
#define FLASH_LED_COUNT (sizeof(flash_led) / sizeof(flash_led[0]))
static uint16_t flash_time[FLASH_LED_COUNT] = {0};

static bool process_replace_mode(uint16_t keycode, const keyrecord_t *record) {
    if (record->event.pressed) {
        uint16_t base = keycode;
        if (keycode >= QK_MODS && keycode <= QK_MODS_MAX) {
            // only a pure Shift modifier is printable here, let others through
            if ((keycode & 0xFF00) != QK_LSFT) {
                return true;
            }
            base = keycode & 0xFF;
        }
        // printable + tab: A-Z, 1-0, space, punctuation row (vim R overwrites tab too)
        if ((base >= KC_A && base <= KC_Z) || (base >= KC_1 && base <= KC_0) || base == KC_SPC || base == KC_TAB || (base >= KC_MINS && base <= KC_SLSH)) {
            tap_code(KC_DELETE); // overwrite char under cursor
            tap_code16(keycode); // type the (possibly shifted) char, cursor advances
            return false;
        }
    }
    return true;
}

// vim R == replace mode until Esc/Caps (normal_mode clears it)
static void enter_replace_mode(void) {
    replace_active = true;
    process_func   = process_replace_mode;
}

// Called by qmk-vim whenever Normal mode is entered, keeps state in sync.
// Strong symbol so it overrides qmk-vim's weak default.
void normal_mode_user(void) {
    replace_active = false;
#ifdef VIM_NUMBERED_JUMPS
    { // consume any stale count so a leftover "3" cannot turn the next
      // dd/cc/yy into a multi-line operation
        extern int16_t motion_counter;
        motion_counter = 0;
    }
#endif
}

// 进入 Visual / Visual-Line 同样清除旧计数（真 vim 会丢弃它）。
#ifdef VIM_NUMBERED_JUMPS
void visual_mode_user(void) {
    extern int16_t motion_counter;
    motion_counter = 0;
}

void visual_line_mode_user(void) {
    extern int16_t motion_counter;
    motion_counter = 0;
}
#endif

// 通过 vim 开关离开 Replace（进入 Insert）时不能残留 replace_active。
void insert_mode_user(void) {
    replace_active = false;
}

// Normal mode bindings
bool process_normal_mode_user(uint16_t keycode, const keyrecord_t *record) {
    if (record->event.pressed) {
        switch (keycode) {
            case KC_BSPC:
                tap_code(KC_LEFT);
                return false;
            case KC_SPC:
                tap_code(KC_RIGHT);
                return false;
            case KC_MINS:
                tap_code(KC_UP);
                tap_code(KC_HOME);
                return false;
            case LSFT(KC_EQL):
                tap_code(KC_DOWN);
                tap_code(KC_HOME);
                return false;
            case LCTL(KC_F):
                tap_code(KC_PGDN);
                return false;
            case LCTL(KC_B):
                tap_code(KC_PGUP);
                return false;
            case KC_SLSH:
            case LSFT(KC_SLSH):
                tap_code16(LCTL(KC_F));
                return false;
            case KC_ENT:
                tap_code(KC_ENT); // real Enter in Normal mode
                return false;
            case LSFT(KC_J):
                tap_code(KC_END); // join next line onto this one
                tap_code(KC_DELETE);
                return false;
            case LSFT(KC_R):
                enter_replace_mode();
                return false;
            case LSFT(KC_G):
                // G (or nG) -> bottom; 1G == gg -> very top, like vim.
                {
                    extern int16_t motion_counter;
                    if (motion_counter == 1) {
                        tap_code16(LCTL(KC_HOME));
                    } else {
                        tap_code16(LCTL(KC_END));
                    }
                    motion_counter = 0;
                }
                return false;
            default:
                break;
        }
    }
    return true;
}

// Right Shift combos: Right Shift + Esc = grave (add left Shift for ~),
// Right Shift + 1..0/-/= = F1..F12. Any other key keeps normal right-shift
// behaviour. Always mask with the 8-bit MOD_BIT_* constants.
// The key consumed on press is remembered so its release is also swallowed even
// if Shift was released first.
static uint16_t shift_combo_kc = KC_NO;
static bool pr_shift_combos(uint16_t keycode, keyrecord_t *record, uint8_t mods) {
    if (record->event.pressed) {
        if ((keycode == KC_ESC) && (mods & MOD_BIT_LSHIFT) &&
            !(mods & (MOD_BIT_LCTRL | MOD_BIT_RCTRL | MOD_BIT_LALT | MOD_BIT_RALT | MOD_BIT_LGUI | MOD_BIT_RGUI))) {
            uint8_t saved_mods = get_mods();
            clear_mods();
            tap_code16(LSFT(KC_GRV)); // ~
            set_mods(saved_mods);
            shift_combo_kc = keycode;
            return true;
        }
        if ((mods & MOD_BIT_RSHIFT) &&
            (keycode == KC_ESC || (keycode >= KC_1 && keycode <= KC_0) || keycode == KC_MINS || keycode == KC_EQL)) {
            uint16_t repl;
            if (keycode == KC_ESC) {
                repl = KC_GRV;
            } else if (keycode == KC_MINS) {
                repl = KC_F11;
            } else if (keycode == KC_EQL) {
                repl = KC_F12;
            } else {
                uint8_t num = (keycode == KC_0) ? 10 : (keycode - KC_1 + 1);
                repl        = KC_F1 + num - 1;
            }
            uint8_t saved_mods = get_mods();
            clear_mods();
            tap_code16(repl);
            set_mods(saved_mods);
            shift_combo_kc = keycode;
            return true;
        }
    } else if (shift_combo_kc == keycode) {
        // press was consumed as a combo: swallow the matching release too
        shift_combo_kc = KC_NO;
        return true;
    }
    return false;
}

// Esc: short tap sends a real Esc; long press switches to Normal mode.
#ifdef VIM_DOT_REPEAT
#    define FINISH_INSERT_REPEAT()      \
        do {                            \
            add_repeat_keycode(KC_ESC); \
            finish_recording_repeat();  \
        } while (0)
#else
#    define FINISH_INSERT_REPEAT() \
        do {                       \
        } while (0)
#endif
static bool pr_esc(uint16_t keycode, keyrecord_t *record, bool vim_on, uint8_t vmode) {
    if (keycode != KC_ESC || !vim_on) return false;

    if (record->event.pressed) {
        if (vmode == VISUAL_MODE || vmode == VISUAL_LINE_MODE) {
            normal_mode(); // 真正退出 Visual / Visual-Line（否则会被永久困住）
            esc_swallow_release = true;
        } else if (replace_active) {
            normal_mode();
            FINISH_INSERT_REPEAT();
            esc_swallow_release = true;
        } else {
            if (vmode != INSERT_MODE) {
                normal_mode();
            }
            esc_swallow_release = false;
            esc_press_timer     = timer_read();
        }
    } else {
        if (esc_swallow_release) {
            esc_swallow_release = false;
        } else if (esc_press_timer && timer_elapsed(esc_press_timer) >= ESC_HOLD_TIME) {
            esc_press_timer = 0;
            normal_mode();
            FINISH_INSERT_REPEAT(); // 长按 Esc 由 keymap 直接退出 Insert，需补收尾
        } else {
            esc_press_timer = 0;
            tap_code(KC_ESC);
        }
    }
    return true;
}

// Caps: long hold = momentary Normal, short tap = toggle Normal/Insert.
// Fn+Caps toggles vim; vim off lets Caps act as Caps Lock.
static bool pr_caps(uint16_t keycode, keyrecord_t *record, bool vim_on, uint8_t vmode) {
    if (keycode != KC_CAPS) return false;

    bool fn_active = fn_layer_active();
    if (fn_active) {
        if (record->event.pressed) {
            toggle_vim_mode();
            insert_mode();
        }
        return true;
    }
    if (!vim_on) {
        return false;
    }
    if (record->event.pressed) {
        caps_was_insert  = (vmode != NORMAL_MODE);
        caps_press_timer = timer_read();
        normal_mode();
#ifdef VIM_DOT_REPEAT
        add_repeat_keycode(KC_NO);
#endif
    } else {
        bool held = caps_press_timer && timer_elapsed(caps_press_timer) >= CAPS_HOLD_TIME;
        caps_press_timer = 0;
        if (held) {
            if (caps_was_insert) insert_mode();
        } else if (!caps_was_insert) {
            insert_mode();
        }
    }
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    const uint8_t mods   = get_mods();
    const bool    vim_on = vim_mode_enabled();
    const uint8_t vmode  = get_vim_mode();
    // Mouse context: Normal mode with vim on, not replace-typing, no modifiers.
    const bool    mouse_ctx = vim_on && vmode == NORMAL_MODE && !replace_active && mods == 0;

    // ---- myfn 约定：Fn+Space 电量 / Fn+T 空跑（在 vim / 鼠标处理之前拦截） ----
    if (!process_record_myfn(keycode, record)) {
        return false;
    }

    // ---- 按键短暂亮灯：按键触发（按物理矩阵位取灯，兼容 MT/MO） ----
    if (record->event.pressed && record->event.key.row < MATRIX_ROWS && record->event.key.col < MATRIX_COLS) {
        uint8_t led = g_led_config.matrix_co[record->event.key.row][record->event.key.col];
        if (led != NO_LED) {
            for (uint8_t i = 0; i < FLASH_LED_COUNT; i++) {
                if (flash_led[i] == led) {
                    uint16_t t    = timer_read();
                    flash_time[i] = t ? t : 1;
                    break;
                }
            }
        }
    }

    // ---- 左Ctrl + 左Alt + 退格 = Ctrl+Alt+Delete ----
    if (keycode == KC_BSPC && (mods & MOD_BIT(KC_LCTL)) && (mods & MOD_BIT(KC_LALT))) {
        if (record->event.pressed) {
            tap_code(KC_DEL); // 已按住的 LCTL/LALT 会带出 Ctrl+Alt+Del
        }
        return false; // 吞掉原退格
    }

    // ---- Fn + Esc (physical [0,0]) held >= 3s = EEPROM reset (see matrix_scan_user).
    //      按下若在 Fn 层：吞掉并置 flag；抬起时按 flag 吞掉（先松 Fn 也不漏真 Esc）。 ----
    if (record->event.key.row == 0 && record->event.key.col == 0) {
        if (record->event.pressed) {
            if (fn_layer_active()) {
                fn_esc_swallow = true;
                if (!reset_armed) {
                    reset_armed = true;
                    reset_fired = false;
                    reset_timer = timer_read();
                }
            }
        } else {
            reset_armed = false;
            if (fn_esc_swallow) {
                fn_esc_swallow = false;
                return false;
            }
        }
        if (fn_layer_active()) return false; // Fn+Esc 吞掉，不输出
    }

    // ---- Menu key: Normal = pointer right; other modes = tap Right / hold Menu.
    //      按下时记录上下文，避免中途切换导致卡键 / 误发 KC_APP。 ----
    if (keycode == MENU_TAP_RIGHT) {
        if (record->event.pressed) {
            if (mouse_ctx) {
                menu_ms_held = true;
                register_code(MS_RGHT);
            } else {
                menu_pressed = true;
                menu_held    = false;
                menu_timer   = timer_read();
            }
        } else {
            if (menu_ms_held) {
                menu_ms_held = false;
                unregister_code(MS_RGHT);
            } else if (menu_pressed) {
                menu_pressed = false;
                if (!menu_held) tap_code(KC_RGHT);
                menu_held = false;
            }
        }
        return false;
    }

    // ---- Normal-mode mouse movement: bottom-row direction keys (hold).
    //      记录实际注册的方向，切上下文后仍能在松开时反注册（防卡指针）。 ----
    {
        int8_t   ms_i  = -1;
        uint16_t ms_kc = 0;
        if (keycode == MT(MOD_RALT, KC_LEFT) || keycode == MT(MOD_RGUI, KC_LEFT)) {
            ms_i = 0; ms_kc = MS_LEFT;
        } else if (keycode == MT(MOD_RCTL, KC_DOWN)) {
            ms_i = 1; ms_kc = MS_DOWN;
        } else if (keycode == MT(MOD_RSFT, KC_UP)) {
            ms_i = 2; ms_kc = MS_UP;
        }
        if (ms_i >= 0) {
            if (record->event.pressed) {
                if (mouse_ctx) {
                    register_code(ms_kc);
                    ms_move_held[ms_i] = true;
                    return false;
                }
            } else if (ms_move_held[ms_i]) {
                unregister_code(ms_kc);
                ms_move_held[ms_i] = false;
                return false;
            }
        }
    }

    // ---- Right Shift combos ----
    if (pr_shift_combos(keycode, record, mods)) return false;

    // ---- Esc / Caps ----
    if (pr_esc(keycode, record, vim_on, vmode)) return false;
    if (pr_caps(keycode, record, vim_on, vmode)) return false;

    // ---- Space in Normal mode: left click / drag（抬起仅当按下被接管时消费）----
    if (keycode == KC_SPC) {
        if (record->event.pressed) {
            if (mouse_ctx) {
                spc_ms_pressed  = true;
                spc_press_timer = timer_read();
                return false;
            }
        } else if (spc_ms_pressed) {
            spc_ms_pressed = false;
            if (spc_holding) {
                unregister_code(MS_BTN1);
                spc_holding = false;
            } else if (spc_press_timer) {
                tap_code(MS_BTN1);
            }
            spc_press_timer = 0;
            return false;
        }
    }

    // ---- Normal-mode Enter：短按=真实 Enter / 长按=单击一次鼠标右键（抬起时判定；抬起仅当按下被接管时消费）----
    if (keycode == KC_ENT) {
        if (record->event.pressed) {
            if (mouse_ctx) {
                ent_ms_pressed  = true;
                ent_press_timer = timer_read();
                return false;
            }
        } else if (ent_ms_pressed) {
            ent_ms_pressed = false;
            if (ent_press_timer && timer_elapsed(ent_press_timer) >= ENT_HOLD_TIME) {
                tap_code(MS_BTN2); // 长按 = 单击一次右键
            } else {
                tap_code(KC_ENT); // 短按 = 真实 Enter
            }
            ent_press_timer = 0;
            return false;
        }
    }

    // ---- 普通模式 Tab 直接透传（真实按下 / 抬起 / 重复），使 Alt+Tab 自然工作 ----
    if (keycode == KC_TAB) {
        return true;
    }

    if (!process_vim_mode(keycode, record)) {
        return false;
    }

    // Insert mode: Right Ctrl + digit -> F1..F10 (Left Ctrl is left alone).
    if (vmode == INSERT_MODE && keycode >= KC_1 && keycode <= KC_0 && (mods & MOD_BIT(KC_RCTL)) && record->event.pressed) {
        uint8_t num = (keycode == KC_0) ? 10 : (keycode - KC_1 + 1);
        tap_code(KC_F1 + num - 1);
        return false;
    }

    return true;
}

void matrix_scan_user(void) {
    // Fn + Esc held >= 3s: full EEPROM reset (compiled keymap gets reloaded
    // via eeconfig_init() -> eeconfig_init_via() -> dynamic_keymap_reset()),
    // then reboot.
    if (reset_armed && !reset_fired && fn_layer_active() && timer_elapsed(reset_timer) >= RESET_HOLD_MS) {
        reset_fired = true;
        reset_armed = false;
        clear_keyboard();
        eeconfig_init();
        mcu_reset();
    }

    // Menu long press (non-mouse context): Menu/application key.
    // Win-Lock 时抑制 KC_APP（与厂商 case KC_APP 一致；合成键不经 process_record_kb）。
    if (menu_pressed && !menu_held && !Keyboard_Info.Win_Lock && timer_elapsed(menu_timer) >= TAPPING_TERM) {
        menu_held = true;
        tap_code(KC_APP);
    }

    // Space long press (Normal mode): hold the left button (drag) until release.
    if (spc_press_timer && !spc_holding && timer_elapsed(spc_press_timer) >= SPC_HOLD_TIME && vim_mode_enabled() && get_vim_mode() == NORMAL_MODE && !replace_active &&
        get_mods() == 0) {
        register_code(MS_BTN1);
        spc_holding = true;
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    (void)led_min;
    (void)led_max;

    bool special = User_Power_Low || Test_Led;

    // ---- Esc + logo: vim mode colour; logo lit count = battery ----
    if (!special) {
        uint8_t r = 0, g = 0, b = 0;
        bool    have = true;
        if (!vim_mode_enabled()) {
            r = 0xFF; g = 0x00; b = 0x00; // red: vim off
        } else {
            switch (get_vim_mode()) {
                case NORMAL_MODE:      r = 0x00; g = 0x00; b = 0xFF; break;
                case INSERT_MODE:      r = 0x00; g = 0xFF; b = 0x00; break;
                case VISUAL_MODE:
                case VISUAL_LINE_MODE: r = 0x80; g = 0x00; b = 0x80; break;
                default:               have = false; break;
            }
        }
        if (replace_active) { // replace (R) mode is shown orange
            r = 0xFF; g = 0x80; b = 0x00; have = true;
        }
        if (have) {
            rgb_matrix_set_color(0, r, g, b); // Esc key LED
            uint8_t bat = User_Batt_BaiFen;
            if (bat > 100) bat = 100;
            uint8_t lit = (uint8_t)(((uint16_t)bat * 3 + 99) / 100);
            for (uint8_t i = 0; i < 3; i++) {
                if (i < lit) {
                    rgb_matrix_set_color(61 + i, r, g, b);
                } else {
                    rgb_matrix_set_color(61 + i, 0, 0, 0);
                }
            }
        }
    }

    // ---- Win 键常亮（跟随全局色相，不参与按键短亮）----
    if (!special) {
        hsv_t hsv = {.h = rgb_matrix_get_hue(), .s = 255, .v = RGB_MATRIX_MAXIMUM_BRIGHTNESS};
        rgb_t rgb = hsv_to_rgb(hsv);
        rgb_matrix_set_color(54, rgb.r, rgb.g, rgb.b);
    }

    // ---- 按键短暂亮灯：跟随全局色相，亮度线性衰减；不亮时置 0（退出全局动画） ----
    if (!(Key_Fn_Status || Led_Rf_Pair_Flg || User_Power_Low || Test_Led)) {
        uint8_t hue = rgb_matrix_get_hue();
        for (uint8_t i = 0; i < FLASH_LED_COUNT; i++) {
            uint8_t idx = flash_led[i];
            if (flash_time[i]) {
                uint16_t elapsed = timer_elapsed(flash_time[i]);
                if (elapsed < FLASH_MS) {
                    uint8_t v   = (uint8_t)((uint16_t)RGB_MATRIX_MAXIMUM_BRIGHTNESS * (FLASH_MS - elapsed) / FLASH_MS);
                    hsv_t   hsv = {.h = hue, .s = 255, .v = v};
                    rgb_t   rgb = hsv_to_rgb(hsv);
                    rgb_matrix_set_color(idx, rgb.r, rgb.g, rgb.b);
                } else {
                    flash_time[i] = 0;
                    rgb_matrix_set_color(idx, 0, 0, 0);
                }
            } else {
                rgb_matrix_set_color(idx, 0, 0, 0);
            }
        }
    }

    return false;
}
