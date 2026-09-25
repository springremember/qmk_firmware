// Copyright 2024-2026 nut65-vim
// SPDX-License-Identifier: GPL-2.0-or-later
//
// NUT65 vim keymap, built on the qmk-vim-fn engine (qmk-vim-fn/engine).
// The engine is QMK-agnostic; the shared qmk/ layer adapts it and owns the
// vim interception chain (vim_pipeline_process), the mouse-mode state machine,
// the Caps tap/hold, Shift+Esc and the §2.1 keyboard-layer shortcut table.
// Only NUT65-specific pieces live here: the layers/keyboard, the vendor
// RGB/battery/wireless wiring (whose Fn keys are declared and passed through to
// nut65.c's process_record_kb tail), the rgbrec effect data, the Fn+RShift+Esc
// bootloader combo and the myfn declared table.
// Design authority: qmk-vim-fn/vim/design.md §4.7/§4.9/§4.10/§4.12.

#include QMK_KEYBOARD_H
#include "rgb_record/rgb_record.h"
#include "qmk-vim-fn/qmk/vim_glue.h"
#include "qmk-vim-fn/qmk/vim_keymap_common.h"

// Vendor housekeeping loop, renamed in nut65.c to make room for the keymap hook
extern void hs_housekeeping_task_user(void);

// Wireless / low-power hooks.  wireless_devs_change()/wireless_get_current_devs(),
// md_getp_state()/md_getp_bat() and lpwr_set_state()/lpwr_set_timeout_manual()
// already reach the keymap through rgb_record.h -> wls/wireless.h.  Only this
// one needs an explicit declaration.
extern bool hs_usb_active(void); // nut65.c
extern void suspend_wakeup_init(void); // power combo wake path

// module.h MD_STATE_CONNECTED: the wireless module actually has a live link.
// The vendor wireless_send_mouse() answers a report sent while NOT connected
// with a wireless_devs_change() call per report, which floods the 40-slot
// smsg queue (each entry retries ~40x with a blocking UART write) and can
// deadlock the main thread - the "must flip the wireless switch" freeze.
// Gate every mouse-report *source* on this instead of touching vendor code.
#define PW_DEVS_USB          0
#define PW_DEVS_2G4          6
#define PW_MD_STATE_CONNECTED 2

enum layers {
    _BL = 0,
    _FL,
    _MBL,
    _MFL,
    _DEFA,
    _FN,
};

// Right-Alt position (design §0.1): tap = mouse mode, hold = Win/Mac modifier.
// keyboard.json's `keycodes` reserve QK_KB_0..QK_KB_29 for the vendor keycodes,
// so a QK_KB_* slot would collide (QK_KB_18 == RP_END).  SAFE_RANGE is QK_USER,
// which starts above the whole QK_KB_0..31 block, and is deliberately left out
// of the VIA customKeycodes list (NUT65.json) - it is not mapped in VIA.
enum custom_keycodes {
    VIM_MOUSE = SAFE_RANGE,
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BL] = LAYOUT( /* win Base */
        KC_ESC,   KC_1,       KC_2,       KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,       KC_MINS,  KC_EQL,   KC_BSPC,   KC_DEL,
        KC_TAB,   KC_Q,       KC_W,       KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,       KC_LBRC,  KC_RBRC,  KC_BSLS,   KC_WFWD,
        KC_CAPS,  KC_A,       KC_S,       KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,    KC_QUOT,            KC_ENT,    KC_WBAK,
        KC_LSFT,              KC_Z,       KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,     KC_SLSH,  KC_RSFT,  KC_UP,     KC_END,
        KC_LCTL,  KC_LCMD,    KC_LALT,                        KC_SPC,                                           VIM_MOUSE,  MO(_FN),  KC_LEFT,  KC_DOWN,   KC_RGHT,
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
        KC_LCTL,  KC_LALT,    KC_LCMD,                        KC_SPC,                                           VIM_MOUSE,  MO(_FN),  KC_LEFT,  KC_DOWN,   KC_RGHT,
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

    [_FN] = LAYOUT(  /* myfn 新 Fn 层（约定：F1-F12/音量/蓝牙/2.4G/有线/电量/初始化） */
        EE_CLR,   KC_F1,      KC_F2,      KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,     KC_F11,   KC_F12,   _______,   _______,
        _______,  KC_BT1,     KC_BT2,     KC_BT3,   KC_2G4,   KC_USB,   _______,  _______,  _______,  _______,  _______,    KC_VOLD,  KC_VOLU,  _______,   _______,
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

/* ===== Vim is always on: start in typing (Insert) mode ===== */
void keyboard_post_init_user(void) {
    vim_glue_init();
    kv_enable();
    kv_set_mode(KV_MODE_INSERT);

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

    // NOTE (post-flash action, not code): NUT65 has no strong
    // keymap_key_to_keycode() override, so after flashing this firmware run
    // EE_CLR / a VIA reset once to refresh the EEPROM dynamic keymap with the
    // new _BL/_MBL/_FN keycodes; otherwise the old dynamic keymap shadows them.
}

/* Mouse reports may only be generated while the link can actually carry
 * them: wired USB, or the wireless module reporting MD_STATE_CONNECTED.
 * (Used as cfg.link_ok - the shared mouse state machine gates on it.) */
static bool mouse_link_ok(void) {
    return wireless_get_current_devs() == PW_DEVS_USB || *md_getp_state() == PW_MD_STATE_CONNECTED;
}

/* ===== USB / wireless auto switch (kept from V1.0) =====
 * This section is only the automatic wired/wireless switching; the manual
 * deep-sleep power combo lives below (nut65_hook_pre + the pw_* state checked
 * in housekeeping_task_user). */
static uint8_t  pw_last_wls      = PW_DEVS_2G4; // remembered non-USB device
static uint8_t  pw_frozen_wls    = PW_DEVS_2G4; // device right before switching to USB
static bool     pw_frozen_valid  = false;
static uint32_t pw_cable_timer   = 0;
static bool     pw_wired_prev    = false;       // last cycle's wired state (edge detect)
static bool     pw_recover_armed = false;       // force-back window after unplug

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
        pw_last_wls = new_devs;
    }
}

static bool pw_no_cable(void) {
    return !readPin(HS_BAT_CABLE_PIN);
}

/* ===== Fn + Right Shift + Esc -> bootloader (DFU) =====
 * On _FN the Esc position resolves to EE_CLR.  This runs as pipeline step 1
 * (cfg.hook_pre) so it intercepts before the myfn skeleton consumes EE_CLR.
 * The modifier check reads the shared *physical shadow* (vim_glue_mods()) and
 * not get_mods(): the myfn skeleton swallows Right Shift, so get_mods() would
 * no longer contain it.  wait_ms(50) is the保命-key exception to the
 * "no blocking wait" rule (design §4.8 note). */
static bool pr_boot_combo(uint16_t keycode, keyrecord_t *record) {
    if (keycode == EE_CLR && IS_LAYER_ON(_FN) && (vim_glue_mods() & MOD_BIT(KC_RSFT))) {
        if (record->event.pressed) {
            // Release every held key/modifier (esp. the Right Shift used by
            // this combo) and send a clean report before resetting. Jumping
            // with the modifier still asserted leaves the host stuck with
            // Shift held (numbers -> symbols, dd -> Ctrl+Shift+X).
            clear_keyboard();
            wait_ms(50); // give USB a moment to flush the report
            eeconfig_disable();
            bootloader_jump();
        }
        return true;
    }
    return false;
}

/* ===== Deep-sleep power: Fn+L (short press) sleeps; Fn + the layout
 * top-right key ([0,14], _BL Delete / _MBL Insert) is the ONLY wake combo
 * (fn/readme §1/§5).  Any other key only wakes the MCU momentarily and goes
 * straight back to sleep, so an accidental key never powers the board back on.
 * The manual V1.0 Ctrl+RightAlt+Delete combo was removed in v2.11. */
static bool pw_off = false; // true while in (or entering) deep sleep
static bool pw_wfn = false; // Fn ([4,11]) held (wake-combo half)
static bool pw_wtop = false; // top-right key ([0,14]) held (wake-combo half)

static void pw_enter_sleep(void) {
    pw_off = true;
    clear_keyboard();
    lpwr_set_state(1); // LPWR_PRESLEEP -> LPWR_STOP (deep sleep)
}

static void pw_boot_wireless(void) {
    pw_off = false;
    pw_wfn = false;
    pw_wtop = false;
    // Stale frozen device from before the sleep must not force itself back
    // after boot - manual BT/2.4G switching has to work again.
    pw_frozen_valid  = false;
    pw_recover_armed = false;
    // Vendor wakeup_cb may skip RGB when its rgb_enable_bak got cleared by the
    // intermediate presleep: force it back, then drive the vendor LPWR through
    // WAKEUP to finish the normal wake path.
    rgb_matrix_enable_noeeprom();
    suspend_wakeup_init();
    lpwr_set_state(3); // LPWR_WAKEUP
    if (wireless_get_current_devs() == PW_DEVS_USB) {
        wireless_devs_change(PW_DEVS_USB, pw_last_wls, false);
    }
}

// Returns true when the event is consumed by the deep-sleep wake handling.
static bool power_combo_process(uint16_t keycode, keyrecord_t *record) {
    (void)keycode;
    if (!pw_off) return false;

    bool is_fn  = (record->event.key.row == 4 && record->event.key.col == 11);
    bool is_top = (record->event.key.row == 0 && record->event.key.col == 14);
    if (record->event.pressed) {
        if (is_fn) pw_wfn = true;
        if (is_top) pw_wtop = true;
        if (pw_wfn && pw_wtop) { // both halves down (any press order) -> wake
            pw_boot_wireless();
            return true;
        }
        if (is_fn || is_top) return true; // hold: stay awake for the partner half
        pw_enter_sleep();                 // unrelated key: momentary wake, back to sleep
        return true;
    }
    if (is_fn) pw_wfn = false;
    if (is_top) pw_wtop = false;
    if (!pw_wfn && !pw_wtop) pw_enter_sleep(); // neither half held: re-arm sleep
    return true;
}

// Pipeline step 1 (cfg.hook_pre): Fn+RShift+Esc bootloader combo, then the
// deep-sleep wake handling.  Step 0 (the physical shadow) has already run, so
// consuming a modifier here still leaves vim_glue_mods() accurate.
static bool nut65_hook_pre(uint16_t keycode, keyrecord_t *record) {
    if (pr_boot_combo(keycode, record)) return true;
    if (power_combo_process(keycode, record)) return true;
    return false;
}

/* ===== myfn (design: qmk-vim-fn/fn/readme.md) =====
 * The declared table lists every real key the _FN layer can produce.  Declared
 * keys are either handled here (KC_L = Fn+L sleep) or handed straight back to
 * QMK by the shared skeleton (design §4.12 "已声明放行/分发"): F1-F12 / volume
 * are ordinary QMK output, and the vendor keys (EE_CLR, HS_BATQ, BT1/2/3, 2.4G,
 * USB) fall through to nut65.c's process_record_kb tail (hs_process_record ->
 * process_record_wls -> vendor switch), so the vendor owns them with zero keymap
 * lines.  Everything else on _FN - including "Fn+top-right" while awake (the
 * wake combo is handled by hook_pre while asleep) and bare modifiers - is
 * swallowed by the shared skeleton. */
static bool nut65_myfn_declared(uint16_t keycode) {
    if (keycode >= KC_F1 && keycode <= KC_F12) return true;
    if (keycode == KC_VOLD || keycode == KC_VOLU) return true;
    if (keycode == KC_CAPS || keycode == KC_ESC) return true; // pipeline steps 5/6
    if (keycode == EE_CLR) return true;                       // vendor 3s reset
    if (keycode == HS_BATQ) return true;                      // vendor Fn+Space battery
    if (keycode == KC_BT1 || keycode == KC_BT2 || keycode == KC_BT3 ||
        keycode == KC_2G4 || keycode == KC_USB) return true; // vendor wireless
    if (keycode == KC_L) return true;                         // Fn+L sleep (below)
    return false;
}

/* Fn+L short press = deep sleep (fn/readme §1).  Preconditions: no physical
 * mode switch (NUT65 has none) and not USB-wired; otherwise it is swallowed
 * (空跑) so nothing leaks.  All other declared keys pass through. */
static bool nut65_myfn(uint16_t keycode, bool pressed) {
    if (keycode == KC_L) {
        if (pressed && pw_no_cable()) pw_enter_sleep();
        return true; // consume either edge (release via the shared pairing table)
    }
    return false;
}

/* ===== Shared vim configuration ===== */
static bool nut65_is_mac(void) { return get_highest_layer(default_layer_state) == _MBL; }

static const vim_cfg_t g_cfg = {
    .fn_layer         = _FN,
    .trigger_kc       = VIM_MOUSE,
    .mod_win          = KC_RALT,
    .mod_mac          = KC_RCMD,
    .is_mac           = nut65_is_mac,
    .link_ok          = mouse_link_ok,
    .hold_ms          = 200,
    .shift_esc_enable = true,
    .led_index        = VIM_LED_INDEX,
    .hook_pre         = nut65_hook_pre,
    .hook_post_myfn   = NULL,
    .myfn_declared    = nut65_myfn_declared,
    .myfn             = nut65_myfn, // Fn+L sleep; every other declared key passes through
    .vim_set_enabled  = NULL, // engine default (kv_enable restarts in INSERT)
    .shortcuts        = vim_default_shortcuts,
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Vendor keys on _FN (EE_CLR, HS_BATQ, BT1/2/3, 2.4G, USB, SYS_CUT, DEB_TOG)
    // are declared in nut65_myfn_declared and passed straight through by the
    // pipeline, so QMK continues into nut65.c's process_record_kb tail
    // (hs_process_record -> process_record_wls -> vendor switch) and the vendor
    // keeps full ownership.  No vendor call is duplicated here (design §4.12).
    return vim_pipeline_process(keycode, record, &g_cfg);
}

void housekeeping_task_user(void) {
    // USB plugged (or devs switched to USB) while powered off -> recover to
    // normal wired operation (RGB was disabled by the vendor presleep, so force
    // it back on here just like the wake path does).
    if (pw_off && (wireless_get_current_devs() == PW_DEVS_USB || !pw_no_cable())) {
        pw_off = false;
        pw_wfn = false;
        pw_wtop = false;
        rgb_matrix_enable_noeeprom();
        suspend_wakeup_init();
        lpwr_set_state(3); // LPWR_WAKEUP: finish the vendor wake path (wls.c may
                           // otherwise force STOP again on the low-battery path)
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

    hs_housekeeping_task_user();
    vim_keymap_common_task(timer_read());
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    (void)led_min;
    (void)led_max;

    // Six-state mode colour (design §4.9/§4.12): vim off red, mouse cyan,
    // visual purple, normal blue (pending yellow), insert green.  Per readme
    // §4 the mode colour is shown ONLY on the bottom strip below — the key area
    // (incl. Caps) keeps the global effect, so no fixed LED is written here.
    uint8_t r = 0, g = 0, b = 0;
    vim_rgb_state_color(kv_vim_enabled(), kv_get_mode(), kv_pending(), kv_get_mode() == KV_MODE_MOUSE, &r, &g, &b);

    // Esc key: vim/mouse mode colour indicator at 60% brightness (readme §4).
    rgb_matrix_set_color(VIM_LED_INDEX,
                         (uint8_t)((uint16_t)r * VIM_ESC_BRIGHTNESS / 100),
                         (uint8_t)((uint16_t)g * VIM_ESC_BRIGHTNESS / 100),
                         (uint8_t)((uint16_t)b * VIM_ESC_BRIGHTNESS / 100));

    // Bottom strip = battery level (highest priority). Number of lit LEDs is
    // fixed by the charge level (both ends turned off toward the middle), the
    // mode only chooses the colour. Strip brightness: 6%.
    uint8_t pct = 6; // strip brightness
    uint8_t c_r = r * pct / 100;
    uint8_t c_g = g * pct / 100;
    uint8_t c_b = b * pct / 100;

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
