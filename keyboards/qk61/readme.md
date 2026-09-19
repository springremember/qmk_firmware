# CIDOO QK61

A customizable 61-key 60% wireless keyboard.

* Hardware Supported: CIDOO QK61 PCB with ES32 FS026 microcontroller
* Hardware Availability: CIDOO QK61

## Building

Compile the default firmware after setting up your build environment:

    qmk compile -kb qk61 -km default

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with the [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Flashing

Enter the bootloader by connecting the keyboard to your PC while holding Esc, then copy the generated firmware file to the appeared drive.

## Bootloader

Enter the bootloader in 2 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (Esc key) and plug in the keyboard
* **Physical reset button**: Briefly press the button on the back of the PCB

## VIA

`VIA_ENABLE = yes` is intentionally enabled in `rules.mk`. Do not disable it: this keyboard has been observed to fail initialization without VIA/dynamic keymap support, and the common wireless code uses dynamic keymap APIs.

Build the `default` keymap and import `CIDOO QK61 VIA.JSON` in <https://usevia.app/> if VIA configuration is needed. In VIA settings, enable **Show Design tab**, then load the draft definition in the Design tab.

## Sleep / power management

The vendor deep-sleep/wake path (`common/user_system.c`) is known to hang this MCU after the
cable is unplugged, so it is disabled with `DISABLE_CUSTOM_SLEEP` in `config.h`. A keyboard-level
state machine in `qk61.c` ("C1") owns wireless sleep instead:

* After 5 minutes of inactivity in a wireless mode it cuts the RF module power (`ES_SDB_POWER_IO`
  low) and turns the LEDs off.
* Any key press, or inserting the USB cable, restores SDB via `Init_Gpio_Infomation()`,
  re-handshakes the SPI link and re-sends the current mode so the module reconnects.
* Inserting the cable also switches to USB and fully re-enumerates the device
  (`es_restart_usb_driver()`).

There is no `Fn+Enter` manual sleep. All waits are bounded; a failed wake falls back to
`Board_Wakeup_Init()` and finally `mcu_reset()`.

## Notes

* VIA lighting support is partial. RGB Matrix firmware support exists, but the VIA draft definition still contains legacy logo-lighting controls, and the small indicator/light to the left of Esc is not controllable from VIA.
* [Upstreaming guide](https://docs.qmk.fm/newbs_git_using_your_master_branch)
