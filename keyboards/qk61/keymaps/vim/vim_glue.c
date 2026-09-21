// Copyright 2026 qk61-vim
// SPDX-License-Identifier: GPL-2.0-or-later
//
// vim_glue.c — adapter between QMK and the QMK-agnostic qmk-vim-fn engine.
// The engine (qmk-vim-fn/engine) is pure C; this file wires it to QMK:
//   * feeds key-downs to kv_kbd()
//   * routes the engine's emitted host keycodes back through register/unregister
//   * held-motion exception: a bare h/j/k/l that is physically held registers
//     the host arrow (auto-repeat) and unregisters on release
//   * maintains a physical modifier shadow (for the bootloader combo)
//   * drains the engine's non-blocking emit queue from kv_task()

#include QMK_KEYBOARD_H
#include "qmk-vim-fn/engine/include/kv.h"

// --------------------------------------------------------------------------
// Physical modifier shadow.  The myfn layer swallows undefined keys (incl.
// modifiers), so get_mods() is unreliable for combos that must be detected
// before the swallow.  (QK61 has no vendor Fn bootloader combo, so this is
// currently only a safety net for future keyboards.)
// --------------------------------------------------------------------------
static uint8_t phys_mods = 0;

static uint8_t mod_bit_of(uint16_t keycode) {
    switch (keycode) {
        case KC_LCTL: return MOD_BIT(KC_LCTL);
        case KC_LSFT: return MOD_BIT(KC_LSFT);
        case KC_LALT: return MOD_BIT(KC_LALT);
        case KC_LGUI: return MOD_BIT(KC_LGUI);
        case KC_RCTL: return MOD_BIT(KC_RCTL);
        case KC_RSFT: return MOD_BIT(KC_RSFT);
        case KC_RALT: return MOD_BIT(KC_RALT);
        case KC_RGUI: return MOD_BIT(KC_RGUI);
        default: return 0;
    }
}

void vim_shadow_mod(uint16_t keycode, bool pressed) {
    uint8_t bit = mod_bit_of(keycode);
    if (!bit) return;
    if (pressed) {
        phys_mods |= bit;
    } else {
        phys_mods &= (uint8_t)~bit;
    }
}

// --------------------------------------------------------------------------
// Held-motion exception (design.md §4.10).
// --------------------------------------------------------------------------
static const uint16_t s_motion_kc[4] = {KC_H, KC_J, KC_K, KC_L};
static const uint16_t s_arrow_kc[4]  = {KC_LEFT, KC_DOWN, KC_UP, KC_RIGHT};
static bool s_motion_held[4];
static bool s_arrow_reg[4];

static int arrow_index(uint16_t basic) {
    switch (basic) {
        case KC_LEFT:  return 0;
        case KC_DOWN:  return 1;
        case KC_UP:    return 2;
        case KC_RIGHT: return 3;
        default:       return -1;
    }
}

void vim_glue_key_down(uint16_t keycode) {
    for (int i = 0; i < 4; i++)
        if (keycode == s_motion_kc[i]) s_motion_held[i] = true;
}

// Remember which key-downs the engine consumed, so their releases are
// consumed too (a consumed press with a passed-through release, or vice
// versa, leaves the host stuck - design.md §4.10 / E3).
#define CONSUMED_CAP 16
static uint16_t s_consumed[CONSUMED_CAP];
static int      s_consumed_n;

static void consumed_add(uint16_t kc) {
    if (s_consumed_n < CONSUMED_CAP) s_consumed[s_consumed_n++] = kc;
}

static bool consumed_take(uint16_t kc) {
    for (int i = 0; i < s_consumed_n; i++) {
        if (s_consumed[i] == kc) {
            s_consumed[i] = s_consumed[--s_consumed_n];
            return true;
        }
    }
    return false;
}

// Release: returns true if the matching press was consumed (so the caller
// consumes the release too); false to pass the release through to QMK.
bool vim_glue_key_up(uint16_t keycode) {
    for (int i = 0; i < 4; i++) {
        if (keycode == s_motion_kc[i]) {
            s_motion_held[i] = false;
            if (s_arrow_reg[i]) {
                unregister_code(s_arrow_kc[i]);
                s_arrow_reg[i] = false;
            }
        }
    }
    return consumed_take(keycode);
}

// --------------------------------------------------------------------------
// Emit callback: the engine hands us a host keycode (basic + modifier bits).
// We must NOT write back the modifier report (design §4.10 / E2): register the
// needed modifier as a real key around the tap instead of set_mods().
// --------------------------------------------------------------------------
static uint8_t mods_to_qmk(uint8_t m) {
    uint8_t out = 0;
    if (m & (KV_MOD_LCTL >> 8)) out |= MOD_BIT(KC_LCTL);
    if (m & (KV_MOD_LSFT >> 8)) out |= MOD_BIT(KC_LSFT);
    if (m & (KV_MOD_LALT >> 8)) out |= MOD_BIT(KC_LALT);
    if (m & (KV_MOD_LGUI >> 8)) out |= MOD_BIT(KC_LGUI);
    if (m & (KV_MOD_RCTL >> 8)) out |= MOD_BIT(KC_RCTL);
    if (m & (KV_MOD_RSFT >> 8)) out |= MOD_BIT(KC_RSFT);
    if (m & (KV_MOD_RALT >> 8)) out |= MOD_BIT(KC_RALT);
    if (m & (KV_MOD_RGUI >> 8)) out |= MOD_BIT(KC_RGUI);
    return out;
}

static void vim_emit(kv_keycode_t kc) {
    uint16_t basic = (uint16_t)(kc & 0x00FF);
    uint8_t  mods  = (uint8_t)((kc >> 8) & 0x1F);

    // Held motion: a bare arrow whose physical motion key is still down is
    // registered (host auto-repeat) and released on the physical key-up.
    int ai = arrow_index(basic);
    if (ai >= 0 && mods == 0 && s_motion_held[ai]) {
        if (!s_arrow_reg[ai]) {
            register_code(basic);
            s_arrow_reg[ai] = true;
        }
        return;
    }

    uint8_t qmk_mods = mods_to_qmk(mods);
    if (qmk_mods) {
        for (uint8_t bit = 0x01; bit; bit <<= 1)
            if (qmk_mods & bit) register_mods(bit);
    }
    register_code(basic);
    unregister_code(basic);
    if (qmk_mods) {
        for (uint8_t bit = 0x01; bit; bit <<= 1)
            if (qmk_mods & bit) unregister_mods(bit);
    }
}

void vim_glue_init(void) {
    kv_init();
    kv_set_emit(vim_emit);
    kv_enable();
    kv_set_mode(KV_MODE_INSERT); // start typing
}

void vim_glue_task(uint32_t now_ms) {
    kv_task(now_ms);
}

// Feed a key-down.  Returns true if the engine consumed it (caller must not
// emit); false if the caller should pass the key through to QMK.
//
// QMK reports the *base* keycode plus the held modifiers, while the engine
// uses QMK-style shifted keycodes (e.g. KV_C_G for 'G').  So wrap Shift in
// when only Shift is held.
bool vim_glue_kbd(uint16_t keycode) {
    if (!kv_vim_enabled()) return false;
    vim_glue_key_down(keycode);

    kv_keycode_t kc = (kv_keycode_t)keycode;
    uint8_t      m  = get_mods();
    if ((m & MOD_MASK_SHIFT) && !(m & (MOD_MASK_CTRL | MOD_MASK_ALT | MOD_MASK_GUI))) {
        kc |= KV_MOD_LSFT;
    }
    bool consumed = (kv_kbd(kc) == KV_CONSUMED);
    if (consumed) consumed_add(keycode);
    return consumed;
}
