# USB Army Knife User Manual

Welcome to the USB Army Knife, the ultimate tool for penetration testers and red teamers. This manual provides a comprehensive guide to the capabilities, commands, and usage of the software.

## Table of Contents
1. [Introduction](#introduction)
2. [Supported Hardware](#supported-hardware)
3. [Getting Started](#getting-started)
4. [Web Interface](#web-interface)
5. [DuckyScript Extensions](#duckyscript-extensions)
6. [Feature Details](#feature-details)
   - [HID Attacks](#hid-attacks)
   - [Mass Storage](#mass-storage)
   - [WiFi & Marauder](#wifi--marauder)
   - [USB Ethernet & PCAP](#usb-ethernet--pcap)
   - [Agent Mode](#agent-mode)
   - [Display & UI](#display--ui)

---

## Introduction
The USB Army Knife acts as a swiss-army knife for physical access attacks. It combines the capabilities of a BadUSB (HID attacks), Mass Storage emulation, Network impersonation, and WiFi attacks (via ESP32 Marauder) into a single, low-cost device.

## Supported Hardware
The software supports various ESP32-S3 and RP2040 based boards.
*   **LilyGo T-Dongle S3** (Recommended)
*   **Evil Crow Cable Wind**
*   **T-Watch S3**
*   **Waveshare ESP32-S3 1.47inch**
*   **M5Stack AtomS3U**
*   **ESP32 Udisk** / **ESP32 Key**
*   **Waveshare ESP32-GEEK**

## Getting Started
1.  **Flash the Firmware**: Use a web flasher or PlatformIO to flash the specific firmware for your device.
2.  **Connect**: Plug the device into a USB port.
3.  **WiFi Setup**: Connect to the WiFi Access Point (`iPhone14`, password: `password`).
4.  **Access UI**: Open a browser and navigate to `http://4.3.2.1:8080`.

## Web Interface
The web interface allows you to:
*   **Status**: View uptime, storage usage, and active attack status.
*   **File Manager**: Upload, download, delete, and edit DuckyScript payloads on the SD card/Internal Flash.
*   **Run Payloads**: Execute scripts directly from the browser.
*   **Marauder Control**: Send commands to the WiFi attack subsystem.
*   **Agent Interaction**: View output from connected agents.

## DuckyScript Extensions
The USB Army Knife extends standard DuckyScript with powerful commands.

### Human Interface Device (HID)
*   `RAW_HID [0xXX] [modifier]`: Send raw HID codes.
*   `MOUSE_MOVE [x] [y]`: Move the mouse cursor.
*   `MOUSE_JIGGLE`: Wiggle the mouse to prevent sleep.
*   `CALC`: Launch the system calculator.
*   `KEYBOARD_LAYOUT [layout]`: Switch keyboard layout dynamically (e.g., `US`, `UK`, `DE`).

### Mass Storage (MSC)
*   `USB_MOUNT_DISK_READ_ONLY [image.img]`: Mount a disk image as a USB Drive.
*   `USB_MOUNT_CDROM_READ_ONLY [image.iso]`: Mount an ISO as a USB CD-ROM.
*   `WAIT_FOR_USB_STORAGE_ACTIVITY`: Pause execution until the host reads/writes to the drive.
*   `WAIT_FOR_USB_STORAGE_ACTIVITY_TO_STOP`: Wait for disk activity to settle (e.g., after copying files).

### Display (TFT)
*   `DISPLAY_TEXT [x] [y] [text]`: Draw text on the screen. Variables supported.
*   `DISPLAY_PNG [file.png]`: Render a PNG image.
*   `DISPLAY_CLEAR`: Clear the screen.
*   `TFT_ON` / `TFT_OFF`: Control display power.

### WiFi & Marauder
*   `ESP32_MARAUDER [command]`: Send a command to the Marauder engine (e.g., `scanap`, `sniffbeacon`).
*   `WIFI_ON` / `WIFI_OFF`: Toggle the management AP.
*   `GET_RECV_PACKETS`: Returns the number of packets captured.
*   `USB_NCM_PCAP_ON` / `USB_NCM_PCAP_OFF`: Start/Stop capturing traffic over the USB Ethernet interface.

### System & Flow Control
*   `LOG [message]`: Write to the internal debug log.
*   `BUTTON_DEF [TIMEOUT]`: Wait for button press.
*   `BUTTON_LONG_PRESS()` / `BUTTON_SHORT_PRESS()`: Use in `IF` statements to branch logic.
*   `FILE_EXISTS [file]`: Check if a file exists.
*   `CREATE_FILE [file]`: Create an empty file.
*   `DELETE_FILE [file]`: Delete a file.
*   `RUN_PAYLOAD [script.ds]`: Chain payloads by running another script.
*   `PARTITION_SWAP`: Swap boot partitions (useful for dual-boot setups or updates).
*   `RANDOM_DELAY [min] [max]`: Wait for a random duration between min and max milliseconds.
*   `WAIT_FOR_CAPS_CHANGE` / `_ON` / `_OFF`: Wait for Caps Lock state changes.
*   `WAIT_FOR_NUM_CHANGE` / `_ON` / `_OFF`: Wait for Num Lock state changes.
*   `WAIT_FOR_SCROLL_CHANGE` / `_ON` / `_OFF`: Wait for Scroll Lock state changes.

### Agent
*   `AGENT_RUN [command]`: Send a shell command to the victim via the Serial Agent.
*   `WAIT_FOR_AGENT_RUN_RESULT`: Block until the agent returns output.
*   `AGENT_CONNECTED`: Check if the agent is active.

## Feature Details

### HID Attacks
Standard DuckyScript is fully supported (`STRING`, `DELAY`, `GUI`, etc.). The device handles high-speed typing and can emulate different keyboard locales without reflashing.

### Mass Storage
The device can present itself as a USB Drive or CD-ROM. This is useful for:
*   **BYOD (Bring Your Own Driver)**: Mount a CD-ROM with drivers to force installation.
*   **Data Exfiltration**: Capture files to a hidden disk image.
*   **Covert Storage**: Hide payloads inside benign-looking images.

### WiFi & Marauder
Integrated [ESP32Marauder](https://github.com/justcallmekoko/ESP32Marauder) functionality allows for:
*   **Beacon Sniffing**: Capture WiFi beacons to identify networks.
*   **Deauthentication**: Disconnect devices from their APs.
*   **PCAP Capture**: Save WiFi traffic to the SD card.

### USB Ethernet & PCAP
The device can emulate a USB NCM (Network Control Model) adapter. It can:
*   **Sniff Traffic**: Capture the first few seconds of negotiation (DHCP, etc.) to a PCAP file.
*   **PoisonTap-style Attacks**: (Advanced) Respond to network requests.

### Agent Mode
When combined with a small executable running on the target (the "Agent"), the USB Army Knife gains a C2 channel over the USB Serial interface. This allows for invisible command execution and output retrieval, bypassing network firewalls.

### Display & UI
For devices with screens (T-Dongle S3, T-Watch), you can build interactive menus using DuckyScript:
1.  Display a PNG background.
2.  Draw text options.
3.  Wait for button inputs (`WAIT_FOR_BUTTON_PRESS`).
4.  Branch execution (`IF BUTTON_SHORT_PRESS() ...`).

---

## Troubleshooting
*   **Device not recognized**: Try a different USB cable and ensure you are in the correct mode (hold button on insert for bootloader).
*   **Script errors**: Check the web interface logs for syntax errors in your DuckyScript.
*   **WiFi not appearing**: The device may have thermally throttled. Let it cool down.
