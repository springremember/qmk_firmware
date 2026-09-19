// Copyright 2024 nut65-vim
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "rgb_record/rgb_record.h"
#include "qmk-vim/src/vim.h"
#include "qmk-vim/src/modes.h"
#include "qmk-vim/src/process_func.h"

// Vendor housekeeping loop, renamed in nut65.c to make room for the keymap hook
extern void hs_housekeeping_task_user(void);

// qmk-vim current keycode processor (can be swapped to drive a custom mode)
extern process_func_t process_func;

// wireless / low-power hooks used by the Ctrl+RightAlt+Insert power combo
extern void suspend_wakeup_init(void);
extern void wireless_devs_change(uint8_t old_devs, uint8_t new_devs, bool reset);
extern uint8_t wireless_get_current_devs(void);
extern bool hs_usb_active(void);
extern uint8_t *md_getp_state(void);
extern uint8_t *md_getp_bat(void);
#define PW_DEVS_USB  0
#define PW_DEVS_2G4  6
// module.h MD_STATE_CONNECTED: the wireless module actually has a live link.
// The vendor wireless_send_mouse() answers a report sent while NOT connected
// with a wireless_devs_change() call per report, which floods the 40-slot
// smsg queue (each entry retries ~40x with a blocking UART write) and can
// deadlock the main thread - the "must flip the wireless switch" freeze.
// Gate every mouse-report *source* on this instead of touching vendor code.
#define PW_MD_STATE_CONNECTED 2

#ifdef VIM_DOT_REPEAT
extern void add_repeat_keycode(uint16_t keycode);
#endif

enum layers {
    _BL = 0,
    _FL,
    _MBL,
    _MFL,
    _DEFA,
    _FN,
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BL] = LAYOUT( /* win Base */
        KC_ESC,   KC_1,       KC_2,       KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,       KC_MINS,  KC_EQL,   KC_BSPC,   KC_DEL,
        KC_TAB,   KC_Q,       KC_W,       KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,       KC_LBRC,  KC_RBRC,  KC_BSLS,   KC_WFWD,
        KC_CAPS,  KC_A,       KC_S,       KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,    KC_QUOT,            KC_ENT,    KC_WBAK,
        KC_LSFT,              KC_Z,       KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,     KC_SLSH,  KC_RSFT,  KC_UP,     KC_END,
        KC_LCTL,  KC_LCMD,    KC_LALT,                        KC_SPC,                                           MO(_FN),    KC_RALT,  KC_LEFT,  KC_DOWN,   KC_RGHT,
        KC_NO,    KC_NO,      KC_NO,      KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,      KC_NO,    KC_NO,    KC_NO,     KC_NO
        ),

    [_FL] = LAYOUT( /* win FN */
        EE_CLR,   KC_F1,      KC_F2,      KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,     KC_VOLD,  KC_VOLU,  RL_MOD,    MUS_STA,
        KC_GRV,   KC_BT1,     KC_BT2,     KC_BT3,   KC_2G4,   KC_USB,   _______,  _______,  _______,  _______,  _______,    _______,  _______,  RGB_MOD,   MUS_CUT,
        _______,  _______,    _______,    DEB_TOG,  _______,  _______,  KC_SCRL,  KC_PAUS,  KC_HOME,  KC_END,   _______,    _______,            RGB_HUI,   RL_VAI,
        _______,              _______,    _______,  _______,  _______,  _______,  NK_TOGG,  SYS_CUT,  _______,  _______,    _______,  MO(_DEFA),RGB_VAI,   RL_VAD,
        LED_TOG,  GU_TOGG,    HS_FREQ,                        HS_BATQ,                                          KC_FTOG,    _______,  RGB_SPD,  RGB_VAD,   RGB_SPI,
        KC_NO,    KC_NO,      KC_NO,      KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,      KC_NO,    KC_NO,    KC_NO,     KC_NO
        ),

    [_MBL] = LAYOUT(  /* mac Base */
        KC_ESC,   KC_1,       KC_2,       KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,       KC_MINS,  KC_EQL,   KC_BSPC,   KC_INS,
        KC_TAB,   KC_Q,       KC_W,       KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,       KC_LBRC,  KC_RBRC,  KC_BSLS,   KC_DEL,
        KC_CAPS,  KC_A,       KC_S,       KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,    KC_QUOT,            KC_ENT,    KC_PGUP,
        KC_LSFT,              KC_Z,       KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,     KC_SLSH,  KC_RSFT,  KC_UP,     KC_PGDN,
        KC_LCTL,  KC_LALT,    KC_LCMD,                        KC_SPC,                                           MO(_FN),    KC_RCMD,  KC_LEFT,  KC_DOWN,   KC_RGHT,
        KC_NO,    KC_NO,      KC_NO,      KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,      KC_NO,    KC_NO,    KC_NO,     KC_NO
        ),

    [_MFL] = LAYOUT(  /*mac FN */
        EE_CLR,   KC_F1,      KC_F2,      KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,     KC_F11,   KC_F12,   RL_MOD,    MUS_STA,
        KC_GRV,   KC_BT1,     KC_BT2,     KC_BT3,   KC_2G4,   KC_USB,   _______,  _______,  _______,  _______,  _______,    _______,  _______,  RGB_MOD,   MUS_CUT,
        _______,  _______,    _______,    DEB_TOG,  _______,  _______,  KC_SCRL,  KC_PAUS,  KC_HOME,  KC_END,   _______,    _______,            RGB_HUI,   RL_VAI,
        _______,              _______,    _______,  _______,  _______,  _______,  NK_TOGG,  SYS_CUT,  _______,  _______,    _______,  MO(_DEFA),RGB_VAI,   RL_VAD,
        LED_TOG,  _______,    HS_FREQ,                        HS_BATQ,                                          KC_FTOG,    _______,  RGB_SPD,  RGB_VAD,   RGB_SPI,
        KC_NO,    KC_NO,      KC_NO,      KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,      KC_NO,    KC_NO,    KC_NO,     KC_NO
        ),

    [_DEFA] = LAYOUT(  /* FN */
        QK_BOOT,  _______,    _______,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,  _______,  _______,   _______,
        _______,  _______,    BT_TEST,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,  _______,  _______,   _______,
        _______,  _______,    _______,    _______,  _______,  _______,  _______,  _______,  _______,  KC_TEST,  _______,    _______,            _______,   _______,
        _______,              _______,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,  _______,  _______,   _______,
        _______,  _______,    _______,                        _______,                                          _______,    _______,  _______,  _______,   _______,
        KC_NO,    KC_NO,      KC_NO,      KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,      KC_NO,    KC_NO,    KC_NO,     KC_NO
        ),

    [_FN] = LAYOUT(  /* myfn 新 Fn 层（约定：音量/蓝牙/2.4G/有线/电量/F1-F10/初始化） */
        EE_CLR,   KC_F1,      KC_F2,      KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,     KC_VOLD,  KC_VOLU,  _______,   _______,
        _______,  KC_BT1,     KC_BT2,     KC_BT3,   KC_2G4,   KC_USB,   _______,  _______,  _______,  _______,  _______,    _______,  _______,  _______,   _______,
        _______,  _______,    _______,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,            _______,   _______,
        _______,              _______,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,  _______,  _______,   _______,
        _______,  _______,    _______,                        HS_BATQ,                                          _______,    _______,  _______,  _______,   _______,
        KC_NO,    KC_NO,      KC_NO,      KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,      KC_NO,    KC_NO,    KC_NO,     KC_NO
        ),

};

const uint16_t PROGMEM rgbrec_default_effects[RGBREC_CHANNEL_NUM][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        HS_GREEN, ________,   ________,   ________, ________, ________, ________, ________, ________, ________, ________,   ________, ________, ________,  ________,
        ________, ________,   HS_GREEN,   ________, ________, ________, ________, ________, ________, ________, ________,   ________, ________, ________,  ________,
        ________, HS_GREEN,   HS_GREEN,   HS_GREEN, ________, ________, ________, ________, ________, ________, ________,   ________,           ________,  ________,
        ________,             ________,   ________, ________, ________, ________, ________, ________, ________, ________,   ________, ________, HS_GREEN,  ________,
        ________, ________,   ________,                       ________,                                         ________,   ________, HS_GREEN, HS_GREEN,  HS_GREEN,
        ________, ________,   ________,   ________, ________, ________, ________, ________, ________, ________, ________,   ________, ________, ________,  ________
        ),

    [1] = LAYOUT(
        HS_GREEN, ________,   ________,   ________, ________, ________, ________, ________, ________, ________, ________,   ________, ________, ________,  ________,
        ________, ________,   HS_GREEN,   ________, ________, ________, ________, ________, ________, ________, ________,   ________, ________, ________,  ________,
        ________, HS_GREEN,   HS_GREEN,   HS_GREEN, ________, ________, ________, ________, ________, ________, ________,   ________,           ________,  ________,
        ________,             ________,   ________, ________, ________, ________, ________, ________, ________, ________,   ________, ________, HS_GREEN,  ________,
        ________, ________,   ________,                       ________,                                         ________,   ________, HS_GREEN, HS_GREEN,  HS_GREEN,
        ________, ________,   ________,   ________, ________, ________, ________, ________, ________, ________, ________,   ________, ________, ________,  ________
        ),

    [2] = LAYOUT(
        HS_GREEN, ________,   ________,   ________, ________, ________, ________, ________, ________, ________, ________,   ________, ________, ________,  ________,
        ________, ________,   HS_GREEN,   ________, ________, ________, ________, ________, ________, ________, ________,   ________, ________, ________,  ________,
        ________, HS_GREEN,   HS_GREEN,   HS_GREEN, ________, ________, ________, ________, ________, ________, ________,   ________,           ________,  ________,
        ________,             ________,   ________, ________, ________, ________, ________, ________, ________, ________,   ________, ________, HS_GREEN,  ________,
        ________, ________,   ________,                       ________,                                         ________,   ________, HS_GREEN, HS_GREEN,  HS_GREEN,
        ________, ________,   ________,   ________, ________, ________, ________, ________, ________, ________, ________,   ________, ________, ________,  ________
        ),
};
#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = {ENCODER_CCW_CW(HS_VOLU, HS_VOLD)},
    [1] = {ENCODER_CCW_CW(_______, _______)},
    [2] = {ENCODER_CCW_CW(_______, _______)},
    [3] = {ENCODER_CCW_CW(_______, _______)},
    [4] = {ENCODER_CCW_CW(_______, _______)},
    [5] = {ENCODER_CCW_CW(_______, _______)}
};
#endif

// clang-format on

/* ===== Vim is always on: start in typing (Insert) mode - hold Caps for
 * Normal, long-press Esc for Normal ===== */
void keyboard_post_init_user(void) {
    enable_vim_mode();
    insert_mode();

    // The bottom strip (LEDs 71-150) and the four corner LEDs (10/11/13/14)
    // are dedicated pixels (battery bar / always off): their final colour is
    // written by rgb_matrix_indicators_advanced_user, so flagging them
    // INDICATOR stops the effect layer from rendering them every frame. The
    // effect then only computes the ~67 key LEDs, roughly halving the
    // per-frame reactive work that caused Bluetooth key lag on fast typing.
    extern led_config_t g_led_config;
    for (uint8_t i = 71; i < RGB_MATRIX_LED_COUNT; i++) {
        g_led_config.flags[i] = LED_FLAG_INDICATOR;
    }
    g_led_config.flags[10] = LED_FLAG_INDICATOR;
    g_led_config.flags[11] = LED_FLAG_INDICATOR;
    g_led_config.flags[13] = LED_FLAG_INDICATOR;
    g_led_config.flags[14] = LED_FLAG_INDICATOR;
    rgb_matrix_set_flags_noeeprom(LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER | LED_FLAG_UNDERGLOW);
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
 * short tap = toggle Normal mode (Insert -> Normal stays, Normal -> Insert) */
#define CAPS_HOLD_TIME 200
static uint16_t caps_press_timer = 0;
static bool     caps_was_insert  = false; // mode before the Caps press

#define SPC_HOLD_TIME 200 // Normal mode Space: short=left click, long=hold left (drag)
static uint16_t spc_press_timer = 0;
static bool     spc_holding     = false; // left button currently held down

/* ===== Normal mode arrows: L+R together = right click ===== */
static bool arrow_l = false, arrow_r = false, arrow_combo = false;

/* ===== End key right click (Normal mode): register on press, auto-release
 * from housekeeping after RC_RELEASE_MS so the host reliably registers the
 * click without ever blocking the main thread (no tap_code_delay). ===== */
#define RC_RELEASE_MS 40
static bool     rc_held  = false;
static uint16_t rc_timer = 0;

/* Mouse reports may only be generated while the link can actually carry
 * them: wired USB, or the wireless module reporting MD_STATE_CONNECTED.
 * Release/cleanup paths stay unconditional so a key held across a
 * disconnect can never leave a stuck button. */
static bool mouse_link_ok(void) {
    return wireless_get_current_devs() == PW_DEVS_USB || *md_getp_state() == PW_MD_STATE_CONNECTED;
}

/* ===== Replace mode (vim R: overwrite chars until Esc) ===== */
static bool replace_active = false;

/* ===== Power combo: Ctrl + Right Alt + the original Insert key (row0 col14)
 * held >= 3s with no USB cable = deep-sleep power toggle. While the combo is
 * pending the involved keys are swallowed so nothing leaks to the host.
 * pw_off: any key wakes the MCU, but only a fresh >=3s combo boots wireless
 * again; every other key is swallowed and it goes straight back to sleep.
 */
#define PW_HOLD_MS 3000
static bool     pw_off = false;
static bool     pw_ctrl = false;        // combo key 1: either Ctrl
static bool     pw_ralt = false;        // combo key 2: Right Alt
static bool     pw_ins  = false;        // combo key 3: original Insert (row0 col14)
static bool     pw_combo = false;
static bool     pw_combo_latch = false; // block re-engage until all released
static uint32_t pw_combo_timer = 0;
static uint8_t  pw_last_wls = PW_DEVS_2G4;   // remembered non-USB device
static bool     pw_last_valid = false;       // whether a real wireless devs seen
static uint8_t  pw_frozen_wls = PW_DEVS_2G4; // device right before switching to USB
static bool     pw_frozen_valid = false;
static uint32_t pw_cable_timer = 0;
static bool     pw_wired_prev = false;     // last cycle's wired state (edge detect)
static bool     pw_recover_armed = false;  // force-back window after unplug

// Fired by the wireless stack on every device switch - freeze the wireless
// device in use right before we go to USB (that is what unplug must return
// to), and keep the last wireless slot for boot.
void wireless_devs_change_user(uint8_t old_devs, uint8_t new_devs, bool reset) {
    if (new_devs == PW_DEVS_USB) {
        if (old_devs != PW_DEVS_USB) { // leaving wireless for wired: freeze it
            pw_frozen_wls   = old_devs;
            pw_frozen_valid = true;
        }
    } else {
        pw_last_wls   = new_devs;
        pw_last_valid = true;
    }
}

static bool pw_no_cable(void) {
    return !readPin(HS_BAT_CABLE_PIN);
}

static void pw_enter_sleep(void) {
    pw_off   = true;
    pw_combo = false;
    pw_ctrl  = false;
    pw_ralt  = false;
    pw_ins   = false;
    clear_keyboard();
    lpwr_set_state(1); // LPWR_PRESLEEP -> LPWR_STOP (deep sleep)
}

static void pw_boot_wireless(void) {
    pw_off   = false;
    pw_combo = false;
    pw_ctrl  = false;
    pw_ralt  = false;
    pw_ins   = false;
    // Stale frozen device from before the sleep must not force itself back
    // after boot - manual BT/2.4G switching has to work again.
    pw_frozen_valid  = false;
    pw_recover_armed = false;
    // Restore LEDs/RGB explicitly (vendor wakeup_cb may skip RGB when its
    // rgb_enable_bak got cleared by the intermediate presleep), then drive
    // the vendor LPWR through WAKEUP to finish the normal wake path.
    rgb_matrix_enable_noeeprom();
    suspend_wakeup_init();
    lpwr_set_state(3); // LPWR_WAKEUP
    if (wireless_get_current_devs() == PW_DEVS_USB) {
        wireless_devs_change(PW_DEVS_USB, pw_last_wls, false);
    }
}

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
// Strong symbol so it overrides qmk-vim's weak default (two weak symbols would
// make the linker pick either one).
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
            case KC_TAB:
                tap_code(KC_TAB); // real Tab passes through in Normal mode
                return false;
            case LSFT(KC_J):
                tap_code(KC_END);   // join next line onto this one
                tap_code(KC_DELETE);
                return false;
            case LSFT(KC_R):
                enter_replace_mode();
                return false;
            case LSFT(KC_G):
                // G (or nG) -> bottom; 1G == gg -> very top, like vim.
                // Exact line jumps are not possible without line navigation.
                {
                    extern int16_t motion_counter;
                    if (motion_counter == 1) {
                        tap_code16(LCTL(KC_HOME));
                    } else {
                        tap_code16(LCTL(KC_END));
                    }
                    motion_counter = 0; // consume the count
                }
                return false;
            default:
                break;
        }
    }
    return true;
}

/* ===== Per-key handlers, ordered exactly like the original branch chain in
 * process_record_user - order matters: the power-combo keys must be seen
 * before anything else so nothing leaks while the combo is pending. Each
 * handler returns true when it consumed the event. ===== */

// row0 col14 (original Insert, now Delete): power-combo key 3
static bool pr_power_ins(keyrecord_t *record) {
    pw_ins = record->event.pressed;
    if (pw_off) {
        if (!record->event.pressed) pw_enter_sleep(); // released: aborted combo
        return true;
    }
    bool pw_ok = (wireless_get_current_devs() != PW_DEVS_USB); // combo only off-wire
    if (pw_combo || (pw_ok && pw_ralt && pw_ctrl)) return true;
    return false;
}

// Right Alt / Ctrl: power-combo keys 2 and 1. These are always consumed
// (both press and release) exactly like the original early-return chain, so
// the vim engine never sees a bare modifier event.
static void pr_power_mods(uint16_t keycode, keyrecord_t *record) {
    if (keycode == KC_RALT) {
        pw_ralt = record->event.pressed;
    } else if (keycode == KC_LCTL || keycode == KC_RCTL) {
        pw_ctrl = record->event.pressed;
    }
}

// Right Shift combos (replaces the old _GO layer): Right Shift + Esc = grave
// (add left Shift for ~), Right Shift + 1..0/-/= = F1..F12. Any other key
// keeps normal right-shift behaviour.
// NOTE: get_mods() is the 8-bit HID mod byte, while the MOD_R*/MOD_L*
// constants are QMK's 5-bit packed encoding (MOD_RSFT=0x12 would match
// Left-Shift 0x02 and Right-Ctrl 0x10 instead!). Always mask with the
// 8-bit MOD_BIT_* constants here.
static bool pr_shift_combos(uint16_t keycode, keyrecord_t *record, uint8_t mods) {
    if ((keycode == KC_ESC) && (mods & MOD_BIT_LSHIFT) &&
        !(mods & (MOD_BIT_LCTRL | MOD_BIT_RCTRL | MOD_BIT_LALT | MOD_BIT_RALT | MOD_BIT_LGUI | MOD_BIT_RGUI))) {
        // Left Shift + Esc = ~ (Right Shift may also be held); Right-Shift-only
        // + Esc falls through to the combo block below and sends grave.
        // All other modifiers still let Esc through.
        if (record->event.pressed) {
            uint8_t saved_mods = get_mods();
            clear_mods();
            tap_code16(LSFT(KC_GRV)); // ~
            set_mods(saved_mods);
        }
        return true;
    }
    if ((mods & MOD_BIT_RSHIFT) &&
        (keycode == KC_ESC || (keycode >= KC_1 && keycode <= KC_0) || keycode == KC_MINS || keycode == KC_EQL)) {
        if (record->event.pressed) {
            uint16_t repl;
            if (keycode == KC_ESC) {
                repl = KC_GRV; // Left+Right Shift = ~ is handled above
            } else if (keycode == KC_MINS) {
                repl = KC_F11;
            } else if (keycode == KC_EQL) {
                repl = KC_F12;
            } else {
                uint8_t num = (keycode == KC_0) ? 10 : (keycode - KC_1 + 1);
                repl        = KC_F1 + num - 1;
            }
            uint8_t saved_mods = get_mods();
            clear_mods(); // the held Right Shift must not shift the F-keys
            tap_code16(repl);
            set_mods(saved_mods);
        }
        return true;
    }
    return false;
}

// Esc: short tap sends a real Esc to the host (firmware mode unchanged);
// long press (>=200ms, decided on release) switches the firmware to
// Normal mode without sending a key. Visual modes hand the tap to
// qmk-vim (native exit); replace mode exits on the press.
static bool pr_esc(uint16_t keycode, keyrecord_t *record, bool vim_on, uint8_t vmode) {
    if (keycode != KC_ESC || !vim_on) return false;

    if (record->event.pressed) {
        if (vmode == VISUAL_MODE || vmode == VISUAL_LINE_MODE) {
            // real Esc handed to qmk-vim to leave visual selection
            esc_swallow_release = true;
        } else if (replace_active) {
            normal_mode();              // leave replace mode
            esc_swallow_release = true; // swallow this Esc's release too (no
                                        // spurious real Esc sent after the exit)
        } else {
            // Cancel any pending vim operator (d/y/c ... awaiting a motion
            // or double tap). normal_mode() is idempotent in Normal mode and
            // resets process_func so a following key cannot be eaten by the
            // half-typed action. Insert mode is untouched (it must keep
            // typing and only deliver the real Esc to the host).
            if (vmode != INSERT_MODE) {
                normal_mode();
            }
            esc_swallow_release = false;
            esc_press_timer = timer_read(); // decide tap vs hold on release
        }
    } else {
        if (esc_swallow_release) {
            esc_swallow_release = false;
        } else if (esc_press_timer && timer_elapsed(esc_press_timer) >= ESC_HOLD_TIME) {
            esc_press_timer = 0;
            normal_mode(); // long press -> Normal mode
        } else {
            esc_press_timer = 0;
            tap_code(KC_ESC); // short tap -> real Esc
        }
    }
    return true;
}

// Caps: long hold = momentary Normal (release restores the previous mode),
// short tap = toggle Normal mode (Insert -> Normal stays, Normal ->
// Insert). Fn+Caps still toggles vim; vim off lets Caps act as Caps Lock.
static bool pr_caps(uint16_t keycode, keyrecord_t *record, bool vim_on, uint8_t vmode) {
    if (keycode != KC_CAPS) return false;

    bool fn_active = IS_LAYER_ON(_FN) || IS_LAYER_ON(_FL) || IS_LAYER_ON(_MFL);
    if (fn_active) {
        if (record->event.pressed) {
            toggle_vim_mode();
            insert_mode(); // vim on -> resting typing mode; off -> harmless
                           // engine reset (process_func / replace state)
        }
        return true;
    }
    if (!vim_on) {
        return false; // vim off: fall through, Caps acts as normal Caps Lock
    }
    if (record->event.pressed) {
        caps_was_insert  = (vmode != NORMAL_MODE); // remember entry mode
        caps_press_timer = timer_read();
        normal_mode();
#ifdef VIM_DOT_REPEAT
        add_repeat_keycode(KC_NO); // stop repeat recording on Caps->Normal
#endif
    } else {
        bool held = caps_press_timer && timer_elapsed(caps_press_timer) >= CAPS_HOLD_TIME;
        caps_press_timer = 0;
        if (held) {
            // momentary: restore the mode we came from
            if (caps_was_insert) insert_mode();
        } else if (!caps_was_insert) {
            // short tap while already Normal -> back to typing
            insert_mode();
        }
        // short tap from Insert: normal_mode() on press already left us in
        // Normal, so it stays there
    }
    return true;
}

// End key: a single right click in Normal mode. Link-gated (a report sent
// while the wireless module is down would flood the vendor queue and can
// hard-lock the board) and non-blocking: the button is registered here and
// released from housekeeping after RC_RELEASE_MS.
static bool pr_end_click(uint16_t keycode, keyrecord_t *record, bool vim_on, uint8_t vmode) {
    if (keycode != KC_END || !vim_on || vmode != NORMAL_MODE || replace_active) return false;
    if (record->event.pressed && !rc_held && mouse_link_ok()) {
        register_code(KC_BTN2); // right button down, housekeeping releases it
        rc_held  = true;
        rc_timer = timer_read();
    }
    return true; // consume both press and release
}

// Normal mode mouse emulation: physical arrows move the pointer (hjkl stays
// the text cursor), Space short = left click, Space long (>=200ms) = hold
// left button (drag), Left+Right arrows pressed together = right click.
// Runs purely in Normal mode (not Insert, Visual or replace-typing - replace
// keeps vim_current_mode = NORMAL internally so it is excluded explicitly),
// with no modifiers so Shift+arrow etc. keep their plain behaviour. The
// caller's mouse_ctx already contains the link gate, so a wireless blip
// simply falls the keys back to their vim behaviour.
static bool pr_mouse_emu(uint16_t keycode, keyrecord_t *record) {
    uint16_t ms = 0;
    switch (keycode) {
        case KC_UP:   ms = KC_MS_UP;   break;
        case KC_DOWN: ms = KC_MS_DOWN; break;
        case KC_LEFT:
        case KC_RGHT: {
            if (record->event.pressed) {
                if (keycode == KC_LEFT) arrow_l = true;
                else                    arrow_r = true;
                if (arrow_l && arrow_r) { // both held -> right click, no move
                    if (!arrow_combo) {
                        arrow_combo = true;
                        tap_code(KC_BTN2);
                    }
                    unregister_code(KC_MS_LEFT);
                    unregister_code(KC_MS_RIGHT);
                } else {
                    register_code(keycode == KC_LEFT ? KC_MS_LEFT : KC_MS_RIGHT);
                }
            } else {
                if (keycode == KC_LEFT) arrow_l = false;
                else                    arrow_r = false;
                if (!(arrow_l && arrow_r)) arrow_combo = false;
                unregister_code(KC_MS_LEFT);
                unregister_code(KC_MS_RIGHT);
                // resume the still-held single direction after the combo
                if (arrow_l)      register_code(KC_MS_LEFT);
                else if (arrow_r) register_code(KC_MS_RIGHT);
            }
            return true;
        }
        case KC_SPC:
            if (record->event.pressed) {
                spc_press_timer = timer_read(); // short=click, long=hold
            } else {
                if (spc_holding) { // long hold -> release left button (drag end)
                    unregister_code(KC_BTN1);
                    spc_holding = false;
                } else if (spc_press_timer) { // short tap -> left click
                    tap_code(KC_BTN1);
                }
                spc_press_timer = 0;
            }
            return true;
        default:
            break;
    }
    if (ms) {
        if (record->event.pressed) {
            register_code(ms);   // hold to keep moving (QMK mouse accel)
        } else {
            unregister_code(ms);
        }
        return true;
    }
    return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Snapshot the volatile state once per event so every handler below reads
    // the same consistent values. None of the handlers mutate this state and
    // then rely on the changed value later in the same event - each one that
    // does (Esc->Normal, Caps toggle, i/a/o->insert ...) consumes the event.
    const uint8_t mods    = get_mods();
    const bool    vim_on  = vim_mode_enabled();
    const uint8_t vmode   = get_vim_mode();
    // Mouse-emulation context: Normal mode with vim on, not replace-typing,
    // no modifiers held, and a link that can actually carry mouse reports
    // (mouse_link_ok is evaluated last: the cheap Insert-mode case
    // short-circuits before the wireless state is even read).
    const bool    mouse_ctx = vim_on && vmode == NORMAL_MODE && !replace_active && mods == 0 && mouse_link_ok();

    // Leak-guard: if a mouse-emulating key (Space drag / arrows) is still
    // held when the mode leaves Normal (e.g. i/o entered, Esc, Caps), the
    // release event arrives outside the mouse handler and would otherwise
    // leave a stuck button or stale arrow flags. Only runs when NOT in the
    // active mouse context, and only when there is mouse state to clean up.
    if (!record->event.pressed && !mouse_ctx && (spc_holding || spc_press_timer || arrow_l || arrow_r || arrow_combo)) {
        if (spc_holding || spc_press_timer) {
            unregister_code(KC_BTN1);
            spc_holding      = false;
            spc_press_timer  = 0;
        }
        if (arrow_l || arrow_r || arrow_combo) {
            arrow_l = false;
            arrow_r = false;
            arrow_combo = false;
            unregister_code(KC_MS_LEFT);
            unregister_code(KC_MS_RIGHT);
        }
    }

    // ---- Original Insert key (row0 col14): third key of the power combo
    // (Ctrl + Right Alt + Ins). Swallowed while the combo is pending so no
    // key leaks; normal Delete / Insert otherwise. ----
    if (record->event.key.row == 0 && record->event.key.col == 14) {
        return !pr_power_ins(record);
    }

    // ---- Right Alt / Ctrl: power-combo keys; consumed here (press+release)
    // so nothing downstream sees a bare modifier, exactly like before. In
    // deep sleep a release aborts the combo, any other key re-sleeps. ----
    if (keycode == KC_RALT || keycode == KC_LCTL || keycode == KC_RCTL) {
        pr_power_mods(keycode, record);
        if (pw_off) {
            if (!record->event.pressed) pw_enter_sleep(); // released: aborted combo
            return false;
        }
        return true;
    }

    // ---- Powered off (deep sleep): only the combo wakes, everything else
    // is swallowed and the board goes straight back to sleep. ----
    if (pw_off) {
        if (record->event.pressed) pw_enter_sleep();
        return false;
    }

    // ---- Right Shift combos ----
    if (pr_shift_combos(keycode, record, mods)) return false;

    // ---- Esc / Caps / End-right-click ----
    if (pr_esc(keycode, record, vim_on, vmode)) return false;
    if (pr_caps(keycode, record, vim_on, vmode)) return false;
    if (pr_end_click(keycode, record, vim_on, vmode)) return false;

    // ---- Normal mode mouse emulation ----
    if (mouse_ctx && pr_mouse_emu(keycode, record)) return false;

    // ---- Alt+Tab: pass Tab through with the held Alt so the task switcher
    // stays open; vim normal mode would tap LALT(Tab), releasing Alt. ----
    if (keycode == KC_TAB && (mods & MOD_MASK_ALT)) {
        return true;
    }

    if (!process_vim_mode(keycode, record)) {
        return false;
    }

    // Insert mode digit keys: plain digits pass straight through (no more
    // long-press F-row). Right Ctrl + digit emits F1..F10; Left Ctrl is left alone.
    if (vmode == INSERT_MODE && keycode >= KC_1 && keycode <= KC_0 && (mods & MOD_BIT(KC_RCTL)) && record->event.pressed) {
        uint8_t num = (keycode == KC_0) ? 10 : (keycode - KC_1 + 1);
        tap_code(KC_F1 + num - 1);
        return false;
    }

    return true;
}

void housekeeping_task_user(void) {
    // End-key right click: release the button once the minimum hold has
    // elapsed (a click shorter than this can be dropped by the host).
    if (rc_held && timer_elapsed(rc_timer) >= RC_RELEASE_MS) {
        unregister_code(KC_BTN2);
        rc_held = false;
    }

    // Space long-press (Normal mode): once SPC_HOLD_TIME elapses while the
    // key is still down, hold the left button down (drag) until release. Only
    // while still in the mouse context (link included) - if the mode left
    // Normal mid-hold the release leak-guard in process_record_user cleans
    // the state up.
    if (spc_press_timer && !spc_holding && timer_elapsed(spc_press_timer) >= SPC_HOLD_TIME &&
        vim_mode_enabled() && get_vim_mode() == NORMAL_MODE && !replace_active && get_mods() == 0 && mouse_link_ok()) {
        register_code(KC_BTN1);
        spc_holding = true;
    }

    // USB plugged (or devs switched to USB) while powered off -> recover to
    // normal wired operation (RGB was disabled by the vendor presleep, so
    // force it back on here just like the combo boot does)
    if (pw_off && (wireless_get_current_devs() == PW_DEVS_USB || !pw_no_cable())) {
        pw_off = false;
        pw_combo = false;
        pw_ctrl = pw_ralt = pw_ins = false;
        rgb_matrix_enable_noeeprom();
        suspend_wakeup_init();
    }

    // USB auto switch driven by the live USB host (plus cable pin fallback).
    // Plug  -> switch to wired USB.
    // Unplug -> return to the wireless device that was in use right before
    // USB (frozen in wireless_devs_change_user), even if the vendor stack
    // already hopped to another slot (e.g. default 2.4G). The force-back only
    // lives inside a short recovery window right after unplug - once the
    // frozen device is reached (or the user picks a device manually) manual
    // BT/2.4G switching is free again.
    if (!pw_off) {
        bool usb_host   = hs_usb_active();
        bool line_cable = !pw_no_cable(); // pin fall back
        bool wired      = usb_host || line_cable;

        if (wired != pw_wired_prev) { // plug / unplug edge
            pw_wired_prev    = wired;
            pw_cable_timer   = timer_read32();
            pw_recover_armed = !wired; // arm the force-back window on unplug
        }

        if (wired) {
            pw_recover_armed = false;
            uint8_t cur = wireless_get_current_devs();
            if (cur != PW_DEVS_USB) {
                if (!pw_cable_timer) pw_cable_timer = timer_read32();
                if (timer_elapsed32(pw_cable_timer) >= 80) {
                    pw_cable_timer = 0;
                    wireless_devs_change(cur, PW_DEVS_USB, false); // plug -> wired
                }
            } else {
                pw_cable_timer = 0;
            }
        } else {
            uint8_t cur = wireless_get_current_devs();
            if (pw_recover_armed) {
                uint8_t target = pw_frozen_valid ? pw_frozen_wls : pw_last_wls;
                if (cur != target) {
                    // retry (throttled) until the frozen wireless device wins
                    if (!pw_cable_timer || timer_elapsed32(pw_cable_timer) >= 60) {
                        pw_cable_timer = timer_read32();
                        wireless_devs_change(cur, target, false);
                    }
                } else {
                    pw_cable_timer   = 0;
                    pw_recover_armed = false; // recovered; manual switching free
                }
            } else {
                pw_cable_timer = 0;
            }
            // extra safety: never sleep while stuck on USB with no host
            if (cur == PW_DEVS_USB && !usb_host) {
                lpwr_set_timeout_manual(false);
                lpwr_set_state(0);
            }
        }
    }

    // Power combo: Ctrl + Right Alt + original Insert key held >= 3s
    // (wireless only) -> toggle deep sleep / boot wireless. Engage clears the
    // keyboard report so nothing leaks while holding; the latch blocks a
    // re-engage until all three keys are released again.
    bool combo_now = (wireless_get_current_devs() != PW_DEVS_USB) && pw_ctrl && pw_ralt && pw_ins;
    if (pw_combo_latch) {
        if (!combo_now) pw_combo_latch = false; // all released -> re-arm
    } else if (combo_now && !pw_combo) {
        pw_combo       = true;
        pw_combo_timer = timer_read32();
        clear_keyboard();
    } else if (pw_combo && !combo_now) {
        pw_combo = false; // aborted before firing
    }
    if (pw_combo && timer_elapsed32(pw_combo_timer) >= PW_HOLD_MS) {
        pw_combo       = false;
        pw_combo_latch = true;
        if (pw_off) {
            pw_boot_wireless(); // deep-sleep -> boot wireless
        } else {
            pw_enter_sleep();   // running -> power off (deep sleep)
        }
    }

    hs_housekeeping_task_user();
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t r, g, b;
    if (!vim_mode_enabled()) {
        r = 0xFF; g = 0x00; b = 0x00; // red: vim off
    } else {
        switch (get_vim_mode()) {
            case NORMAL_MODE:
                r = 0x00; g = 0x00; b = 0xFF; // blue
                break;
            case INSERT_MODE:
                r = 0x00; g = 0xFF; b = 0x00; // green
                break;
            case VISUAL_MODE:
            case VISUAL_LINE_MODE:
                r = 0x80; g = 0x00; b = 0x80; // purple
                break;
            default:
                return true;
        }
    }

    // Bottom strip = battery level (highest priority). Number of lit LEDs is
    // fixed by the charge level (both ends turned off toward the middle), the
    // mode only chooses the colour. Strip brightness: 6%.
    uint8_t c_r = r;
    uint8_t c_g = g;
    uint8_t c_b = b;
    uint8_t pct = 6; // strip brightness
    if (replace_active) { // replace (R) mode is shown orange
        c_r = 0xFF;
        c_g = 0x80;
        c_b = 0x00;
    }
    c_r = c_r * pct / 100;
    c_g = c_g * pct / 100;
    c_b = c_b * pct / 100;

    // The battery level and USB/wireless mode change at human speed, but this
    // callback runs every frame - cache the resulting band for 250ms so a
    // frame costs nothing but the (forced) colour writes below.
    static uint16_t band_timer = 0;
    static uint8_t  band_lo = 0, band_lit = 0;
    if (!band_timer || timer_elapsed(band_timer) >= 250) {
        band_timer = timer_read();
        uint8_t bat;
        if (wireless_get_current_devs() == PW_DEVS_USB) {
            bat = 100; // wired: bottom strip stays fully lit
        } else {
            bat = *md_getp_bat(); // wireless: real battery level
        }
        if (bat > 100) bat = 100;
        band_lit = (uint8_t)(((uint16_t)bat * 80) / 100); // 80 strip LEDs
        band_lo  = (80 - band_lit) / 2;                    // lit band centred
    }

    for (uint8_t i = 71; i <= 150; i++) {
        uint8_t pos = i - 71;
        if (pos >= band_lo && pos < band_lo + band_lit) {
            rgb_matrix_set_color(i, c_r, c_g, c_b);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0); // turned off from both ends
        }
    }
    rgb_matrix_set_color(10, 0, 0, 0);
    rgb_matrix_set_color(11, 0, 0, 0);
    rgb_matrix_set_color(13, 0, 0, 0);
    rgb_matrix_set_color(14, 0, 0, 0);

    return true;
}
