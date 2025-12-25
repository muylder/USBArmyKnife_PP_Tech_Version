# ⚡ MITRE ATT&CK® TTP Mapping: USB Army Knife

**Intelligence Report ID:** GHOST-INT-2025-002
**Target Asset:** USB Army Knife (Physical Access Platform)
**Analyst:** Ghost (CTI Specialist)
**Date:** 2025-12-25

## 📊 Executive Summary
The USB Army Knife represents a sophisticated convergence of physical access vectors and network-based attacks. By analyzing its capabilities against the MITRE ATT&CK framework, we observe a tool capable of spanning the entire kill chain, from **Initial Access** via physical hardware to **Exfiltration** over covert channels.

Recent enhancements (Encryption, DuckyScript Visualization) have elevated its operational security (OpSec) and stealth capabilities.

## 🗺️ TTP Matrix

| Tactic | ID | Technique | USB Army Knife Capability |
| :--- | :--- | :--- | :--- |
| **Initial Access** | **T1200** | **Hardware Additions** | The device itself is a malicious hardware addition plugged into a USB port. |
| | **T1091** | **Replication Through Removable Media** | Can function as a mass storage device to introduce payloads or malware. |
| **Execution** | **T1059.001** | **PowerShell** | DuckyScript payloads often inject PowerShell commands via HID keyboard emulation. |
| | **T1059.003** | **Windows Command Shell** | DuckyScript injecting CMD commands. |
| | **T1204.002** | **User Execution: Malicious File** | Social engineering functionality (Fake UI/Progress Bar) tricks users into running payloads. |
| **Persistence** | **T1098** | **Account Manipulation** | Scripts can be designed to add local user accounts or modify privileges. |
| | **T1547.001** | **Registry Run Keys / Startup Folder** | Payloads can inject persistence mechanisms during the brief execution window. |
| **Privilege Escalation**| **T1548.002** | **Bypass UAC** | Common DuckyScript payloads include UAC bypass techniques. |
| **Defense Evasion** | **T1036** | **Masquerading** | Device identifies as a benign keyboard (VID/PID spoofing). |
| | **T1218** | **System Binary Proxy Execution** | Using LOLBins (Living off the Land Binaries) via HID injection to evade AV. |
| | **T1027** | **Obfuscated Files or Information** | **(New)** Encrypted Logs (AES-256) prevent forensic analysis of captured data on the device. |
| **Discovery** | **T1040** | **Network Sniffing** | **(New)** "Silent Sentinel" passively logs WiFi probe requests to map device locations and identities. |
| | **T1430** | **Location Tracking** | **(New)** Cataloging Probe Requests reveals previous locations/networks of victims. |
| **Credential Access** | **T1557.001** | **LLMNR/NBT-NS Poisoning** | **(New)** "Ether-Harvest" responds to broadcast multicast name queries to spoof services. |
| | **T1557** | **Adversary-in-the-Middle** | USB Ethernet mode captures traffic; Wi-Fi "Evil Portal" captures credentials. |
| | **T1040** | **Network Sniffing** | PCAP functionality in USB Ethernet mode captures unencrypted traffic. |
| | **T1110** | **Brute Force** | Theoretically capable of HID-based PIN/Password guessing (though slow). |
| | **T1552** | **Unsecured Credentials** | **(Mitigated)** Credentials captured are now encrypted at rest, mitigating simple forensic acquisition by defenders. |
| **Collection** | **T1123** | **Audio Capture** | "Hot Mic" feature streams audio from the environment. |
| | **T1113** | **Screen Capture** | VNC-like feature captures victim screen data. |
| | **T1056.001** | **Input Capture: Keylogging** | Software agent can potentially hook keys; Evil Portal captures web inputs. |
| **Command & Control** | **T1090** | **Proxy** | Can relay traffic via WiFi or act as a network bridge. |
| | **T1095** | **Non-Standard Protocol** | Communicates via Serial over USB, often bypassing network firewalls. |
| | **T1102** | **Web Service** | Deployed agent or web interface (C2) over WiFi. |
| **Exfiltration** | **T1052.001** | **Exfiltration over USB** | Saving stolen data to the device's internal mass storage or SD card. |
| | **T1048** | **Exfiltration Over Alternative Protocol** | Egressing data via the device's independent WiFi connection (bypassing corporate DLP). |
| | **T1041** | **Exfiltration Over C2 Channel** | **(New)** Encrypted logs can be retrieved over the web interface. |

## 🛡️ Detailed Analysis

### 1. The HID Vector (T1200 / T1059)
The device's primary strength is **Human Interface Device (HID)** emulation. By acting as a keyboard, it bypasses:
*   Autorun restrictions (CD/USB policies).
*   Network firewalls (payload is typed locally).
*   Air gaps (if physically connected).

**Detection Strategy:** Correlation of rapid keystroke injection anomalies and simultaneous device insertion events (USB logs).

### 2. Network Interception (T1557 / T1040)
In **USB Network Device** mode, the USB Army Knife positions itself as the preferred network gateway (due to higher metric priority usually given to wired connections over WiFi).
*   **Impact:** Allows full capture of local traffic (PCAP) and manipulation of requests (DNS Spoofing potential).

### 3. Out-of-Band Exfiltration (T1048)
The ability to egress data via **WiFi** while plugged into a target machine effectively creates a bridge between the secure internal network and the attacker's external control. This renders traditional DLP (Data Loss Prevention) monitoring on the corporate gateway ineffective for this stream.

### 4. Operational Security (OpSec) Improvements
The addition of **AES-256-ECB** encryption for stored credentials ensures that even if the physical device is captured by defenders, the harvested data remains protected. This aligns with advanced threat actor TTPs (e.g., APT29, FIN7) who employ anti-forensic measures to protect their loot.

## 🎯 Threat Attribution Context
Tools of this nature are consistent with TTPs observed in:
*   **Physical Red Team Operations**: Simulating insider threats or cleaning crews.
*   **FIN7 (Carbonak)**: History of using BadUSB devices mailed to victims.
*   **Raspberry Robin Worm**: Propagation via USB devices (though largely automated, the vector is similar).

---
**Recommendation:** Integrate these TTPs into your SOC's detection logic to identify anomalies correlating with physical USB insertion events.
