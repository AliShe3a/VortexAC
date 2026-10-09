# 🛡️ Vortex Anti-Cheat (VAC) - CrossFire Security Core

Vortex Anti-Cheat is a comprehensive, custom-engineered user-mode client-server security solution designed to protect game servers (specifically tailored for CrossFire) from memory manipulation, unauthorized modifications, session hijacking, and reverse engineering.

Built primarily in low-level C++, this project features a real-time heuristic detection engine, a dedicated stateful validation server, on-the-fly resource decryption, and a fully integrated Web Dashboard for server administration.

## 🚀 Core Features & Architecture

### 1. Strict Stateful Session Validation (Anti-Bypass)
Engineered a zero-trust architecture bridging the Anti-Cheat server and the core game database to prevent direct client launches and session spoofing.
*   **Database Integration:** Utilizes custom SQL Stored Procedures (`CF_AUTH` & `SP_GS_GAME_LOGIN`).
*   **Session Purging:** On initial login, any existing or stale anti-cheat sessions associated with the user ID are immediately purged to prevent session reuse.
*   **Real-time Heartbeat:** The game server strictly validates the session during the game login sequence. It verifies if a valid, active anti-cheat heartbeat exists in the database. If missing, the login is rejected instantly.
*   **Hardware ID Enforcement:** Comprehensive HWID generation (UUID, GUID, Volume ID, CPU ID) verified against a dynamic ban list before session creation.

### 2. Advanced User-Mode Client Detections
A robust, multi-layered detection engine operating within the client memory space.
*   **API Hook Detection:** Deep inspection of the `Direct3D9` VTable and `Text Section` for unauthorized `JMP`/`CALL` hooks (e.g., Overlay hacks, Wallhacks).
*   **Process & Thread Monitoring:** Continuous scanning for known cheating tools (Cheat Engine, Debuggers, Macros) and illegal process handles.
*   **Memory Integrity (CRC32):** Real-time CRC32 hashing of critical game modules and memory segments to detect unauthorized patches or byte alterations.
*   **Heuristic Analytics:** Detects anomalous in-game behaviors (e.g., rapid firing modifications, recoil manipulation, bounding box alterations).
*   **Smart Whitelisting:** Implements cryptographic signature verification (`WinVerifyTrust`) to whitelist legitimate overlays (e.g., Discord, OBS) while blocking malicious injections.

### 3. On-the-Fly Asset Decryption (Vortex Encryption)
A sophisticated virtualized file system layer to protect game assets (`.REZ` files) from extraction and modification.
*   **Win32 API Detours:** Hooks core Windows APIs (`CreateFileA`, `ReadFile`, `CloseHandle`) using Microsoft Detours.
*   **In-Memory Decryption:** Transparently decrypts game assets directly in memory during the read operation using a custom XOR cipher and signature verification, preventing static analysis and asset stealing.

### 4. Live Streaming & VOIP Subsystem (UDP)
Built a custom UDP-based networking protocol to monitor suspected players and facilitate team communication without straining the main TCP validation server.
*   **Real-time Screen Capture:** Captures the player's screen (GDI+), compresses it to JPEG, fragments it into UDP chunks, and transmits it to the server for live admin monitoring.
*   **Integrated VOIP:** Custom voice communication routing with channel separation (Global/Team) based on the current game room state.

### 5. Multi-Tiered Ban Management & Suspicion Scoring
*   **Suspicion System:** Assigns "weights" to detections (e.g., Memory Alteration = 100 pts, Suspicious Overlay = 15 pts). Players reaching the threshold are automatically disconnected or banned.
*   **Automated Punishments:** Executes SQL procedures (`ANTICHEAT_BANUSER`, `ANTICHEAT_HWIDBAN`) to enforce tiered bans (3-day, 7-day, Permanent) based on the severity of the offense.
*   **Evidence Collection:** Automatically captures screenshots upon detection and archives them for administrator review.

### 6. Embedded Web Server & Discord Gateway
*   **C++ HTTP Web Server:** A lightweight, custom-built HTTP/TCP server embedded within the Anti-Cheat core to serve the Web Dashboard, REST APIs, and image assets.
*   **Discord WebSocket Gateway:** Maintains a persistent WebSocket connection to Discord, streaming real-time alerts, player logs, and ban evidence directly to designated Discord channels with rich embeds.

## 🛠️ Tech Stack & Technologies

*   **Client/Server Engine:** C++17, Winsock2, Microsoft Detours (API Hooking), GDI+ (Screen Capture), WinTrust/Wincrypt (Cryptography & Signatures).
*   **Networking:** Stateful TCP (Auth & Heartbeat), UDP (Live Stream & VOIP), HTTP (Embedded Web Server), WebSockets (Discord API).
*   **Database:** MS SQL Server, Custom Stored Procedures.
*   **Security:** Memory manipulation, Reverse Engineering countermeasures, CRC32 Integrity, Custom XOR Encryption, PE Header Parsing.

## 📷 System Preview
*(Add your screenshots here)*

1.  **Web Dashboard (Live Tracking):** `![Dashboard](link-to-dashboard-image.png)`
2.  **Server Console / Discord Alerts:** `![Console](link-to-console-image.png)`
3.  **Live Stream Monitor:** `![LiveStream](link-to-livestream-image.png)`

## 💡 Technical Accomplishments
Vortex Anti-Cheat showcases an advanced understanding of **Low-level Windows Internals**, **Security Architecture**, and **Network Programming**. By successfully implementing memory-level API hooking, custom cryptographic file streams, and a hybrid TCP/UDP server architecture, this project effectively neutralizes sophisticated manipulation techniques used in modern game exploitation.
