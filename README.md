# OMOTE - Open Universal Remote - Firmware

![Ubuntu build](https://github.com/OMOTE-Community/OMOTE-Firmware/actions/workflows/build-platformio-ubuntu.yml/badge.svg)
![Windows Build](https://github.com/OMOTE-Community/OMOTE-Firmware/actions/workflows/build-platformio-windows.yml/badge.svg)
![MacOS Build](https://github.com/OMOTE-Community/OMOTE-Firmware/actions/workflows/build-platformio-macos.yml/badge.svg)
[![OMOTE Discord](https://discordapp.com/api/guilds/1138116475559882852/widget.png?style=shield)][link1]

## Overview

This it the ESP32 arduino based firmware for the OMOTE - Open Universal Remote.

To run this firmware, you have two options
*  run it on the [OMOTE ESP32 Hardware](https://github.com/OMOTE-Community/OMOTE-Hardware/)
*  run it in the simulator on Linux, macOs or Windows

### The state of this project

The software can be adjusted to your needs. You can add your own amplifier, TV and media player. Smart home devices can be controlled with MQTT. The software is an example made up of:
* a TV and an amplifier controlled with infrared
* a Fire TV media player controlled with BLE (bluetooth keyboard)
* some smart home devices controlled with MQTT
* an IR receiver for decoding the IR codes from your remote

Please see the [wiki on how to understand and modify the firmware.](https://github.com/OMOTE-Community/OMOTE-Firmware/wiki/How-to-understand-and-modify-the-firmware)

You need to have PlatformIO running, and you need to know how to compile and flash your own firmware with PlatformIO. There is no prebuild firmware.

The remote can be charged and programmed via its USB-C port. Open the PlatformIO project to compile and upload the code to the ESP32.

As a long term goal, maybe a prebuild firmware will be published, where you can configure your OMOTE via a web interface.

### LVGL GUI simulator for Windows, Linux, and macOS

A simulator for running the LVGL UI on your local Windows, Linux, or macOS machine is available.

You can run the simulator in Visual Studio Code with PlatformIO. No need for any other compiler or development environment (no Visual Studio needed as often done in other LVGL simulators).
<div align="center">
  <img src="images/WindowsSimulator.gif" width="60%">
</div>

For details, please see the [wiki for the software simulator for fast creating and testing of LVGL GUIs.](https://github.com/OMOTE-Community/OMOTE-Firmware/wiki/Software-simulator-for-fast-creating-and-testing-of-LVGL-GUIs)

### To Dos for software

Long term goals (not yet scheduled)
- [ ] Add an interface for graphically editing the configuration
- [ ] Store the configuration in Flash (e.g. as a editable json file)

See the [open issues](https://github.com/OMOTE-Community/OMOTE-Firmware/issues) and [discussions](https://github.com/OMOTE-Community/OMOTE-Firmware/discussions) for a full list of proposed features (and known issues).

## Contributing

If you have a suggestion for an improvement, please fork the repo and create a pull request. You can also simply open an issue or for more general feature requests, head over to the [discussions](https://github.com/OMOTE-Community/OMOTE-Firmware/discussions).

## License

Distributed under the GPL v3 License. See [LICENSE](https://github.com/OMOTE-Community/OMOTE-Firmware/blob/main/LICENSE) for more information.

## Contact

[![OMOTE Discord](https://discordapp.com/api/guilds/1138116475559882852/widget.png?style=banner2 "OMOTE Discord")][link1]

Join the OMOTE Discord: [https://discord.gg/5PnYFAsKsG](https://discord.gg/5PnYFAsKsG)

[link1]: https://discord.gg/5PnYFAsKsG
