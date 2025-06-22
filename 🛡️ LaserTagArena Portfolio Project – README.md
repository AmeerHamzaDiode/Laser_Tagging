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
![Image](https://github.com/user-attachments/assets/d20bfe5c-3e59-453f-b5cb-c78c1983bb0a)
![Image](https://github.com/user-attachments/assets/ae89eb21-9c34-4452-9875-a0571bfef2a5)
![Image](https://github.com/user-attachments/assets/680278ce-f5cd-43fe-82d8-0fcf248efa77)
![Image](https://github.com/user-attachments/assets/64ff618d-27a2-4648-b03e-4733c0d22e9c)
---



---

### 📌 How to Run:
1. Flash the firmware to ESP32 using Arduino IDE / PlatformIO  
2. Import Node-RED flow from `/flow/flow.json`  
3. Connect ESP32 to Wi-Fi and start the WebSocket server  
4. Start the game and view live scoreboard via Node-RED UI

---

### 🔗 GitHub Link (Once repo is ready):
> [GitHub – LaserTagArena 🔗]([https://github.com/your-username/LaserTagArena](https://github.com/AmeerHamzaDiode/Laser_Tagging))

