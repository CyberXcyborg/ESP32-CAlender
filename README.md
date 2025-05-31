# 📅 ESP32 Calendar – A Web-Based Scheduler

![GitHub stars](https://img.shields.io/github/stars/CyberXcyborg/ESP32-CAlender?style=social)  
![License](https://img.shields.io/github/license/CyberXcyborg/ESP32-CAlender)  
![Last Commit](https://img.shields.io/github/last-commit/CyberXcyborg/ESP32-CAlender)

**Website:** [cyberxcyborg.github.io/ESP32-CAlender](https://cyberxcyborg.github.io/ESP32-CAlender)

---

Transform your ESP32 into a **smart personal calendar** that runs directly from a web browser.  
Store events, visualize your schedule, and manage your time from any device — all locally on your ESP32!

---

## 🔧 Features

- 📅 **Event Management** with persistent JSON storage  
- 🌐 **Web Interface** served over your local WiFi network  
- 💡 **Responsive UI** optimized for desktop and mobile  
- 🔐 100% local and private — no cloud, no tracking  
- ⚡ Efficient use of ESP32 resources via SPIFFS

---

## 📡 WiFi Configuration

Update these lines in the code to connect to your WiFi:

```cpp
// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

ESP32 serves the calendar UI via HTTP:

```cpp
// Create WebServer object on port 80
WebServer server(80);
```

### 📁 File Paths

```cpp
const char* eventsFilePath = "/calendar_events.json";
const char* indexHtmlPath = "/index.html";
```

---

## 🚀 Key Implementation Highlights

### 🧠 SPIFFS Storage  
All event data is stored in JSON format using ESP32’s SPIFFS system for persistent and reliable access.

### 🌐 WebServer Interface  
Access and control your calendar via browser at `http://[ESP32_IP_ADDRESS]`.

### 📱 Responsive Design  
Modern HTML/CSS + JS interface with glassmorphism design for clean aesthetics on mobile or desktop.

### 🔋 Resource Efficiency  
Designed for ESP32’s memory constraints — no bloated libraries or overhead.

---

## 🛠️ Installation Instructions

### 🧰 Requirements

- ESP32 development board  
- Micro USB cable  
- Arduino IDE  
- ESP32 board definition installed via Boards Manager  
- Required libraries:
  - `WiFi`
  - `WebServer`
  - `SPIFFS`
  - `ArduinoJson`

---

### ⚙️ Quick Setup

1. **Clone this repository**  
2. **Open the `.ino` file** in Arduino IDE  
3. **Update WiFi credentials** in the code  
4. **Connect your ESP32 and select the board/port**  
5. **Upload** the sketch  
6. **Open Serial Monitor** at `115200 baud`  
7. Note the IP address and visit `http://[IP_ADDRESS]`  
8. **Start managing your calendar!**

---

## ✨ Why Use ESP32 Calendar?

Tired of overcomplicated or cloud-bound calendars?  
The **ESP32 Smart Calendar** is:

- Local-first, privacy-respecting  
- Clean and beautiful  
- Reliable and offline-accessible  
- Hackable and open-source

---

## ❤️ Support the Project

If this helped you or inspired your own builds, please consider:

- ⭐ Giving a **Star** to the repo  
- 🛠️ Submitting **Issues** or Pull Requests  
- ☕ [Buying me a coffee](https://www.buymeacoffee.com/cyberxcyborg)

---

## 📂 Repository

- **GitHub:** [github.com/CyberXcyborg/ESP32-CAlender](https://github.com/CyberXcyborg/ESP32-CAlender)  
- **Live Demo Site:** [cyberxcyborg.github.io/ESP32-CAlender](https://cyberxcyborg.github.io/ESP32-CAlender)

---
