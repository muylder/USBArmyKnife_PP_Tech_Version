# 🛠️ Development & Contributing

*"We do not just use the tools. We build them."*

## 🏗️ Architecture Overview
The project is built on **PlatformIO** using the Arduino framework for ESP32. It leverages the ESP32-S3's native USB capabilities for HID and MSC emulation.

### Directory Structure
- `src/`: Core C++ source code.
- `include/`: Header files and configuration.
- `lib/`: Shared libraries and modules.
- `ui/`: Web interface assets (Bootstrap based).
- `examples/`: DuckyScript payload examples.

## 🤝 How to Contribute

1.  **Fork** the repository.
2.  **Create** a feature branch (`git checkout -b feature/matrix-upgrade`).
3.  **Commit** your changes.
4.  **Push** to the branch.
5.  **Open** a Pull Request.

### Code Style
- Follow standard C++ guidelines.
- Ensure DuckyScript commands are backward compatible where possible.
- **Document** all new features using the Architect's standards.

## 🧪 Testing
- Run local tests using PlatformIO.
- Verify payloads on actual hardware (T-Dongle-S3 recommended).
