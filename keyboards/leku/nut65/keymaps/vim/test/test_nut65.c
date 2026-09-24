/* test_nut65.c — keyboard-local host test for the nut65 vim keymap.
 *
 * Not part of qmk-vim-fn: it lives in the nut65 branch because it exercises the
 * NUT65-specific glue configuration (the myfn declared contract, the vendor
 * keys that pass through to nut65.c's process_record_kb tail, and the
 * Fn+RightShift+Esc bootloader combo) against the shared qmk-vim-fn layer.
 * The shared layer is consumed via the qmk-vim-fn submodule (../qmk-vim-fn),
 * whose generic QMK stub (qmk_stub.h) provides the host API.
 *
 * Run:  cd keyboards/leku/nut65/keymaps/vim/test && make
 */
#include "qmk_stub.h"
#include "emit.h" /* kv_emit_flush_now() */
#include "qmk-vim-fn/engine/include/kv.h"
#include "qmk-vim-fn/qmk/vim_glue.h"
#include "qmk-vim-fn/qmk/vim_keymap_common.h"

/* Vendor keycodes not present in qmk_stub.h.  Values live in QK_KB-ish space
 * (0x7Dxx) with T_OTHER low bytes so the engine classifies them as pass-through
 * and none collide with a layer-key range. */
#define VK_EE_CLR  0x7D00
#define VK_HS_BATQ 0x7D01
#define VK_BT1     0x7D02
#define VK_BT2     0x7D03
#define VK_BT3     0x7D04
#define VK_2G4     0x7D05
#define VK_USB     0x7D06

/* NUT65 uses KC_RCMD for the Mac long-press modifier; QMK's KC_RCMD is a
 * #define alias of KC_RGUI (0xE7), which the stub spells KC_RGUI. */
#ifndef KC_RCMD
#define KC_RCMD KC_RGUI
#endif

/* ---------------- host state (mirrors the shared glue tests) ---------------- */
layer_state_t layer_state         = 0;
layer_state_t default_layer_state = 0;

static uint8_t s_mods;
uint8_t get_mods(void) { return s_mods; }
void    clear_mods(void) { s_mods = 0; }
void    set_mods(uint8_t m) { s_mods = m; }
void    register_mods(uint8_t m) { s_mods |= m; }
void    unregister_mods(uint8_t m) { s_mods &= (uint8_t)~m; }

#define REG_CAP 64
static uint16_t s_reg[REG_CAP];
static int      s_reg_n;

void register_code(uint16_t kc) {
    if (IS_MODIFIER_KEYCODE(kc)) s_mods |= (uint8_t)(1u << (kc - KC_LCTL));
    if (s_reg_n < REG_CAP) s_reg[s_reg_n++] = kc;
}
void unregister_code(uint16_t kc) {
    if (IS_MODIFIER_KEYCODE(kc)) s_mods &= (uint8_t)~(1u << (kc - KC_LCTL));
    for (int i = 0; i < s_reg_n; i++) {
        if (s_reg[i] == kc) { s_reg[i] = s_reg[--s_reg_n]; return; }
    }
}
void tap_code(uint16_t kc) { register_code(kc); unregister_code(kc); }
void tap_code16(uint16_t kc) { tap_code((uint16_t)(kc & 0xFF)); }

static uint16_t g_now;
uint16_t timer_read(void) { return g_now; }
uint16_t timer_elapsed(uint16_t since) { return (uint16_t)(g_now - since); }

static int reg_count(uint16_t kc) {
    int n = 0;
    for (int i = 0; i < s_reg_n; i++) if (s_reg[i] == kc) n++;
    return n;
}

static uint16_t s_phys[REG_CAP];
static int      s_phys_n;
static int      s_orphan;

static void host_press(uint16_t kc) { register_code(kc); }
static void host_release(uint16_t kc) { unregister_code(kc); }

/* ---------------- test bookkeeping ---------------- */
static int g_pass, g_fail;
#define CHECK(cond)                                                        \
    do {                                                                   \
        if (cond) { g_pass++; }                                            \
        else { g_fail++; printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); } \
    } while (0)

/* ---------------- simulated NUT65 cfg ---------------- */
#define FN_LAYER 5 /* NUT65 _FN index (0 _BL,1 _FL,2 _MBL,3 _MFL,4 _DEFA,5 _FN) */

/* Exact transcription of nut65_myfn_declared() (nut65 keymap.c). */
static bool nut65_declared_sim(uint16_t keycode) {
    if (keycode >= KC_F1 && keycode <= KC_F12) return true;
    if (keycode == KC_VOLD || keycode == KC_VOLU) return true;
    if (keycode == KC_CAPS || keycode == KC_ESC) return true;
    if (keycode == VK_EE_CLR) return true;
    if (keycode == VK_HS_BATQ) return true;
    if (keycode == VK_BT1 || keycode == VK_BT2 || keycode == VK_BT3 ||
        keycode == VK_2G4 || keycode == VK_USB) return true;
    return false;
}

/* ---------------- simulated NUT65 pr_boot_combo (Fn+RightShift+Esc) --------
 * Faithful transcription of the decision logic in
 * keyboards/leku/nut65/keymaps/vim/keymap.c (pr_boot_combo).  The real version
 * then does clear_keyboard()/wait_ms()/eeconfig_disable()/bootloader_jump();
 * the reset itself is outside the shared layer, so we record that the jump
 * would have happened instead of invoking it.  The modifier is read off the
 * physical shadow (vim_glue_mods()) because the myfn skeleton swallows Right
 * Shift on _FN, so get_mods() would no longer contain it. */
static int  s_boot_jump_calls;
static bool pr_boot_combo_sim(uint16_t keycode, keyrecord_t *record) {
    if (keycode == VK_EE_CLR && layer_state_cmp(layer_state, FN_LAYER) &&
        (vim_glue_mods() & MOD_BIT(KC_RSFT))) {
        if (record->event.pressed) {
            s_boot_jump_calls++; /* real code: clear_keyboard(); bootloader_jump(); */
        }
        return true;
    }
    return false;
}

static const vim_cfg_t g_cfg = {
    .fn_layer         = FN_LAYER,
    .trigger_kc       = TEST_TRIGGER_KC, /* NUT65 VIM_MOUSE is SAFE_RANGE; value irrelevant here */
    .mod_win          = KC_RALT,
    .mod_mac          = KC_RCMD,
    .is_mac           = NULL,
    .link_ok          = NULL,
    .hold_ms          = 200,
    .shift_esc_enable = true,
    .led_index        = 0,
    .hook_pre         = pr_boot_combo_sim, /* NUT65 pr_boot_combo (Fn+RShift+Esc) */
    .hook_post_myfn   = NULL,
    .myfn_declared    = nut65_declared_sim,
    .myfn             = NULL, /* NUT65: every _FN key is declared -> pass through */
    .vim_set_enabled  = NULL,
    .shortcuts        = vim_default_shortcuts,
};

static bool pipeline(uint16_t kc, bool pressed) {
    keyrecord_t r = {0};
    r.event.pressed = pressed;
    return vim_pipeline_process(kc, &r, &g_cfg);
}

static bool feed(uint16_t kc, bool pressed) {
    bool pass = pipeline(kc, pressed);
    if (pressed) {
        if (pass) { host_press(kc); if (s_phys_n < REG_CAP) s_phys[s_phys_n++] = kc; }
    } else {
        if (pass) {
            bool found = false;
            for (int i = 0; i < s_phys_n; i++) {
                if (s_phys[i] == kc) { s_phys[i] = s_phys[--s_phys_n]; found = true; break; }
            }
            if (!found) s_orphan++;
            host_release(kc);
        }
    }
    return pass;
}

static void reset_engine(void) {
    g_now = 1000;
    s_mods = 0;
    s_reg_n = 0;
    s_phys_n = 0;
    s_orphan = 0;
    s_boot_jump_calls = 0;
    layer_state = 0;
    default_layer_state = 0;
    vim_glue_init(); /* kv_init + enable + INSERT */
}

static void fn_on(void) { layer_state = (1UL << FN_LAYER); }
static void fn_off(void) { layer_state = 0; }

/* ======================================================================
 * 1. myfn declared table membership (static contract check)
 * ====================================================================== */
static void test_declared_table(void) {
    const uint16_t must[] = {
        KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10,
        KC_F11, KC_F12, KC_VOLD, KC_VOLU, KC_CAPS, KC_ESC,
        VK_EE_CLR, VK_HS_BATQ, VK_BT1, VK_BT2, VK_BT3, VK_2G4, VK_USB,
    };
    for (unsigned i = 0; i < sizeof(must) / sizeof(must[0]); i++) {
        CHECK(nut65_declared_sim(must[i]) == true);
    }
    CHECK(nut65_declared_sim(KC_Z) == false);
    CHECK(nut65_declared_sim(KC_LSFT) == false);
    CHECK(nut65_declared_sim(KC_RSFT) == false);
}

/* ======================================================================
 * 2. every declared _FN key passes on BOTH edges (pipeline == true)
 * ====================================================================== */
static void test_declared_pass_both_edges(void) {
    const uint16_t keys[] = {
        VK_EE_CLR, VK_HS_BATQ, VK_BT1, VK_2G4, VK_USB,
        KC_F1, KC_VOLD,
    };
    for (unsigned i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        reset_engine();
        fn_on();
        uint16_t kc = keys[i];
        if (feed(kc, true) != true) {
            g_fail++;
            printf("FAIL %s:%d  declared key 0x%04X press was not passed through on _FN\n",
                   __FILE__, __LINE__, kc);
        } else {
            g_pass++;
        }
        if (feed(kc, false) != true) {
            g_fail++;
            printf("FAIL %s:%d  declared key 0x%04X release was not passed through on _FN\n",
                   __FILE__, __LINE__, kc);
        } else {
            g_pass++;
        }
        CHECK(s_orphan == 0);
        fn_off();
    }

    /* The rest of the declared set gets the same both-edge contract. */
    const uint16_t more[] = {KC_F2, KC_F6, KC_F12, KC_VOLU, VK_BT2, VK_BT3};
    for (unsigned i = 0; i < sizeof(more) / sizeof(more[0]); i++) {
        reset_engine();
        fn_on();
        CHECK(feed(more[i], true) == true);
        CHECK(feed(more[i], false) == true);
        CHECK(s_orphan == 0);
        fn_off();
    }
}

/* ======================================================================
 * 3. undeclared key on _FN: press swallowed, release paired (no orphan)
 * ====================================================================== */
static void test_undeclared_z_swallowed(void) {
    reset_engine();
    fn_on();
    CHECK(feed(KC_Z, true) == false);  /* swallowed on press */
    CHECK(s_orphan == 0);
    CHECK(feed(KC_Z, false) == false); /* paired release consumed */
    CHECK(s_orphan == 0);
    fn_off();

    /* Sanity: with Fn off, z is an ordinary pass-through on both edges. */
    reset_engine();
    CHECK(feed(KC_Z, true) == true);
    CHECK(feed(KC_Z, false) == true);
    CHECK(s_orphan == 0);
}

/* ======================================================================
 * 4. bare modifier on _FN: press swallowed (no Fn+Shift+Esc leak), release
 *    never strands the host modifier; a modifier registered before Fn is
 *    unregistered normally when released on _FN.
 * ====================================================================== */
static void test_bare_modifier_no_leak_no_stick(void) {
    /* (a) Fn+Shift: the Shift press must not reach the host. */
    reset_engine();
    fn_on();
    CHECK(feed(KC_LSFT, true) == false); /* swallowed: no leak */
    CHECK(reg_count(KC_LSFT) == 0);
    CHECK(get_mods() == 0);
    bool rel = feed(KC_LSFT, false);
    CHECK(rel == false);
    CHECK(s_orphan == 0);
    CHECK(reg_count(KC_LSFT) == 0);
    CHECK(get_mods() == 0);
    fn_off();

    /* (b) A modifier QMK registered BEFORE Fn went down must be released
     * normally while Fn is held (no stuck modifier). */
    reset_engine();
    CHECK(feed(KC_LSFT, true) == true); /* Fn off: QMK registers Shift */
    CHECK(reg_count(KC_LSFT) == 1);
    fn_on();
    if (feed(KC_LSFT, false) != true) {
        g_fail++;
        printf("FAIL %s:%d  Shift release on _FN was swallowed -> stuck Shift\n",
               __FILE__, __LINE__);
    } else {
        g_pass++;
    }
    CHECK(reg_count(KC_LSFT) == 0);
    CHECK(get_mods() == 0);
    CHECK(s_orphan == 0);
    fn_off();

    /* (c) Right Shift must behave identically (pr_boot_combo reads the
     * physical shadow, so the shadow must record RSFT even though the skeleton
     * swallows it). */
    reset_engine();
    fn_on();
    CHECK(feed(KC_RSFT, true) == false);
    CHECK((vim_glue_mods() & MOD_BIT(KC_RSFT)) != 0);
    CHECK(feed(KC_RSFT, false) == false);
    CHECK((vim_glue_mods() & MOD_BIT(KC_RSFT)) == 0);
    CHECK(s_orphan == 0);
    fn_off();
}

/* ======================================================================
 * 5. Fn + RightShift + Esc(EE_CLR) -> bootloader (pr_boot_combo)
 *    The Esc position on _FN resolves to EE_CLR; the combo must fire on that
 *    exact combination and on nothing else.  Myfn swallows Right Shift on _FN,
 *    so this also proves the combo reads the physical shadow, not get_mods().
 * ====================================================================== */
static void test_boot_combo(void) {
    /* (a) Fn + RightShift + Esc fires (shadow holds RSFT even though myfn
     *     swallowed the bare Right Shift press). */
    reset_engine();
    fn_on();
    CHECK(feed(KC_RSFT, true) == false);                  /* myfn swallows the bare modifier */
    CHECK((vim_glue_mods() & MOD_BIT(KC_RSFT)) != 0);     /* ...but the shadow records it */
    CHECK(feed(VK_EE_CLR, true) == false);                /* hook_pre consumes it */
    CHECK(s_boot_jump_calls == 1);
    CHECK(feed(VK_EE_CLR, false) == false);               /* paired release */
    CHECK(s_boot_jump_calls == 1);                        /* no second jump on release */
    CHECK(feed(KC_RSFT, false) == false);
    CHECK(s_orphan == 0);
    fn_off();

    /* (b) Left Shift + Esc must NOT fire (only Right Shift is the combo). */
    reset_engine();
    fn_on();
    CHECK(feed(KC_LSFT, true) == false);
    CHECK(feed(VK_EE_CLR, true) == true);                 /* declared: passes through */
    CHECK(s_boot_jump_calls == 0);
    CHECK(feed(VK_EE_CLR, false) == true);
    CHECK(feed(KC_LSFT, false) == false);
    fn_off();

    /* (c) RightShift + Esc without Fn must NOT fire. */
    reset_engine();
    CHECK(feed(KC_RSFT, true) == true);                   /* Fn off: QMK registers RSFT */
    CHECK(feed(VK_EE_CLR, true) == true);
    CHECK(s_boot_jump_calls == 0);
    CHECK(feed(VK_EE_CLR, false) == true);
    CHECK(feed(KC_RSFT, false) == true);
    fn_off();

    /* (d) Fn + RightShift + a non-Esc _FN key (F1) must NOT fire. */
    reset_engine();
    fn_on();
    CHECK(feed(KC_RSFT, true) == false);
    CHECK(feed(KC_F1, true) == true);                     /* declared: passes through */
    CHECK(s_boot_jump_calls == 0);
    CHECK(feed(KC_F1, false) == true);
    CHECK(feed(KC_RSFT, false) == false);
    CHECK(s_orphan == 0);
    fn_off();

    /* (e) Shift released before Esc: combination never completes -> no jump. */
    reset_engine();
    fn_on();
    CHECK(feed(KC_RSFT, true) == false);
    CHECK(feed(KC_RSFT, false) == false);                 /* shadow RSFT cleared */
    CHECK(feed(VK_EE_CLR, true) == true);
    CHECK(s_boot_jump_calls == 0);
    CHECK(feed(VK_EE_CLR, false) == true);
    fn_off();
}

int main(void) {
    test_declared_table();
    test_declared_pass_both_edges();
    test_undeclared_z_swallowed();
    test_bare_modifier_no_leak_no_stick();
    test_boot_combo();
    printf("nut65-sim: pass=%d fail=%d\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
