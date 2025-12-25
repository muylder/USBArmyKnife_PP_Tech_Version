<a href="https://github.com/i-am-shodan/USBArmyKnife/blob/master/LICENSE"><img alt="License" src="https://img.shields.io/github/license/mashape/apistatus.svg"></a>
[![PlatformIO CI](https://github.com/i-am-shodan/USBArmyKnife/actions/workflows/main.yml/badge.svg)](https://github.com/i-am-shodan/USBArmyKnife/actions/workflows/main.yml)
[![.NET](https://github.com/i-am-shodan/USBArmyKnife/actions/workflows/dotnet.yml/badge.svg)](https://github.com/i-am-shodan/USBArmyKnife/actions/workflows/dotnet.yml)
<a href="https://twitter.com/intent/follow?screen_name=therealshodan"><img src="https://img.shields.io/twitter/follow/therealshodan?style=social&logo=twitter" alt="Twitter"></a>
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/O5O8145AVW)
<a href="https://www.buymeacoffee.com/therealshodan" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/default-orange.png" alt="Buy Me A Coffee" height="41" width="174"></a>

# USB Army Knife

<div align="center">
  <h3>The Ultimate Tool for Penetration Testers and Red Teamers</h3>
  <img src="./docs/images/t-dongle-s3-side.png" width="400px" alt="USB Army Knife Device">
  <br><br>
  
  [🚀 **Getting Started**](docs/getting-started/README.md) •
  [📚 **Documentation**](docs/README.md) •
  [📖 **User Manual**](docs/guides/user-manual.md) •
  [🔧 **Hardware**](#-supported-hardware)
</div>

---

## 🧐 What is the USB Army Knife?

The USB Army Knife is a compact, versatile physical access platform. It empowers you to execute sophisticated attacks that typically require multiple devices.

- **USB HID Attacks**: DuckyScript execution with advanced triggers (Caps Lock state, timers).
- **USB Mass Storage**: Dynamic drive emulation for covert storage or exfiltration.
- **Network Impersonation**: Act as a USB Ethernet adapter to PCAP traffic or perform MiTM.
- **WiFi/Bluetooth Ops**: Full ESP32 Marauder integration (Deauth, PMKID, Evil Portal).
- **Stealth & Control**: Web-based C2 interface, "Hot Mic" streaming, and remote screen viewing.

## 🗣️ Testimonials

> *"Your device is evil. You are doing evil."*  
> — **Mr. Peoples** (via X)

---

## 📚 Documentation Matrix

We have reorganized the knowledge base to ensure clarity and accessibility.

- **[🚀 Getting Started](docs/getting-started/README.md)**: Flashing instructions and first-time setup.
- **[📖 User Guides](docs/guides/README.md)**: Detailed operational manuals and scenarios.
- **[🔧 Command Reference](docs/reference/README.md)**: Technical specs for DuckyScript payloads.
- **[🛠️ Development](docs/development/README.md)**: Architecture and contribution guidelines.

---

## ⚡ Key Features

- **USB HID Attacks**: BadUSB / DuckyScript support with multiple keyboard layouts.
- **Mass Storage Device**: Emulate USB drives and CDROMs.
- **USB Network Device**: NCM/ECM Ethernet adapter emulation.
- **WiFi & Bluetooth**: Integrated ESP32 Marauder for wireless audits.
- **Web Interface**: Bootstrap-based UI for managing payloads via mobile.
- **Advanced Control**: Agent deployment for remote command execution and VNC-like screen viewing.

## 📺 Demos

| [Ultimate Rick Roll](./examples/rickroll/) | [USB PCAP](./examples/usb_ethernet_pcap/) | [VNC Screen View](./examples/vnc/) |
|:---:|:---:|:---:|
| 🎥 **Injection + Audio** | 🎥 **Traffic Capture** | 🎥 **Remote View** |

---

## 🖥️ Supported Hardware

| Device | Type | Features | Link |
|:---|:---|:---|:---|
| **LilyGo T-Dongle S3** | **Recommended** | LCD, Button, SD, WIFI/BT | [AliExpress](https://s.click.aliexpress.com/e/_DCMq0ZX) |
| **Evil Crow Cable Wind** | Covert Cable | Stealth, No Screen, WIFI/BT | [AliExpress](https://s.click.aliexpress.com/e/_EGxBPoe) |
| **M5Stack AtomS3U** | Compact | Button, WIFI/BT | [AliExpress](https://s.click.aliexpress.com/e/_EIAUNXX) |
| **T-Watch S3** | Wearable | Screen, Watch, WIFI/BT | [AliExpress](https://s.click.aliexpress.com/e/_ExPs5dU) |
| **Generic ESP32-S2** | Budget | Basic HID/WiFi | [AliExpress](https://s.click.aliexpress.com/e/_Dn5wXe5) |

*(See [Hardware Guide](docs/README.md) for full details)*

---

## 🤝 Contributing & Support

- **Questions?** Check the [Discussions](https://github.com/i-am-shodan/USBArmyKnife/discussions).
- **Bugs?** Raise an [Issue](https://github.com/i-am-shodan/USBArmyKnife/issues).
- **Social?** Follow [@therealshodan](https://twitter.com/therealshodan).

## 📄 License

This project is licensed under the MIT License - see the **[LICENSE](LICENSE)** file for details.

---

<div align="center">
  <sub>Managed by the Architect Agent</sub>
</div>


