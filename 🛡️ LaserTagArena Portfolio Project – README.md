# LaserTagArena Portfolio-Project-1

### 🔫 Overview:
The LaserTagArena project is a real-time multiplayer laser tag game system built using **ESP32 microcontrollers**. It enables players to participate in a physical laser tag game with **live scoring, wireless hit detection**, and **dynamic feedback** via LEDs and sound.

The system uses **WebSocket communication** for ultra-low latency data transfer between each player's hardware and the central controller. **Node-RED** is used for score management, game logic orchestration, and real-time visualization of the game state.

---

### 🛠️ Tools & Technologies:

- **Hardware:** ESP32 microcontroller, IR sensors, buzzer, LEDs  
- **Communication:** WebSockets 🌐  
- **Backend / Logic Engine:** Node-RED 🔁  
- **Frontend :** Node-RED UI or web interface  

---

### ⚙️ Functionalities Implemented:

- ✅ Real-time hit detection using IR sensors  
- ✅ WebSocket-based low-latency messaging  
- ✅ Dynamic scoring and player state management  
- ✅ Game logic and scoreboard handled in Node-RED  
- ✅ Visual feedback through LED indicators  
- ✅ Sound feedback via onboard buzzer  
- ✅ Future scope: Bluetooth-based player registration, power management

---

### 📸 Project Showcase

---

### 📁 Folder Structure:
```
/hardware         # ESP32 firmware code  
/flow             # Node-RED flow export  
/assets           # Images or circuit diagrams  
README.md
```

---

### 📌 How to Run:
1. Flash the firmware to ESP32 using Arduino IDE / PlatformIO  
2. Import Node-RED flow from `/flow/flow.json`  
3. Connect ESP32 to Wi-Fi and start the WebSocket server  
4. Start the game and view live scoreboard via Node-RED UI

---

### 🔗 GitHub Link (Once repo is ready):
> [GitHub – LaserTagArena 🔗](https://github.com/your-username/LaserTagArena)

