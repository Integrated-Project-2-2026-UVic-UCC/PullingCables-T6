# Drone Cable Routing Accessory: Project Documentation

**Institution:** Universitat de Vic – Universitat Central de Catalunya  
**Course:** Integrated Project II  
**Academic Year:** 2025–2026  
**Project Group:** Team 6  

---

## 1. Project Overview
This project focuses on the design, development, and integration of a remote-controlled attachable hook and pulley system for drones. The primary application is to facilitate automated and safe cable routing in environments where manual deployment is hazardous, inefficient, or physically restricted.

## 2. Objectives
The project is driven by a clear set of technical and operational goals:

### 2.1 Main Objective
To design and assemble a fully functional, remote-controlled hook attachment for drone-assisted cable deployment.

### 2.2 Specific Objectives
* **Pulley System:** Develop a reliable mechanism for cable management and tension control.
* **Safety Features:** Integrate hardware and software failsafes to ensure secure operations.
* **RF Communication:** Establish a robust Radio Frequency link for long-range control.
* **Validation:** Conduct rigorous testing to optimize performance and payload stability.

---

## 3. Team Organization and Methodology
The project follows a structured multidisciplinary model managed under the Scrum Framework.

### 3.1 Roles and Responsibilities
* **Project Leader:** Jan Font
* **Mechanical Manager:** Martí Alsina
* **Electrical Manager:** Oriol Bonet
* **Software Manager:** Xavier Vilalta

### 3.2 Professional Values (Scrum)
Our team operates under the core values of Courage to face technical challenges, Focus on sprint goals, Commitment to the project's success, Respect for individual expertise, and Openness regarding development obstacles.

---

## 4. Technical Architecture and Progress
The system is divided into three fundamental engineering pillars:

### 4.1 Mechanical tasks
* **Claw and Pulley Design:** Developed functional 3D designs for the mechanical gripping system.
* **Prototyping & Assembly:** Successful manufacturing and assembly of the fully operational physical prototype.
* **Component Housings:** Designed and manufactured custom protective enclosures (housings) to safely accommodate all onboard drone electronics, alongside a dedicated, ergonomic casing for the remote control.

### 4.2 Electrical and Software tasks
* **RF Setup:** Established a stable communication link between the Transmitter (Command) and the Receiver using ESP-NOW.
* **System Logic:** Implementation of the electrical diagram, managing 5V output controls for motor drivers.
* **Power Management:** Integration of the battery and receiver units within the system logic.
* **Software Finalization:** The final control software (including motor control, safety limits, and communication protocols) is fully integrated and operational.

---

## 5. Project Status: Completed
The project has successfully concluded its testing and integration phase. **The system has been fully tested in real-world conditions, functions correctly, and has successfully achieved all initial objectives.**

### 5.1 Final Achievements
* Successful assembly of the final prototype and all custom protective housings.
* **Successful real-world flight testing with the drone**, confirming system reliability, payload stability, and the flawless operation of all mechanical and electrical features.
* Final software logic including the ESP-NOW communication protocol and the 2-second safety interlock mechanism—is highly robust and fully operational.
* Complete validation of the electrical control circuit and hardware failsafes.

---

## 6. How to Recreate This Project
If you wish to build your own drone cable routing system, all the necessary resources are included in this GitHub repository. 

### 6.1 Mechanical Assembly
* **3D Printing:** Navigate to the repository's design folders to find the CAD/STL files for the main drone housing, the remote control casing, the claw, and the pulley mechanism. Print these components using a durable material (e.g., PLA or PETG).
* **Assembly:** Assemble the physical components utilizing standard metric screws and bearings as indicated in the mechanical folder.

### 6.2 Electronics Setup
The system uses two XIAO ESP32-S3 microcontrollers:
* **Transmitter (Remote Control):** Connect 4 pushbuttons to the microcontroller's GPIO pins (D3, D4, D5, D6), wiring the other side of each button to GND.
* **Receiver (Drone Payload):** Connect a servo motor (Pin D0), an H-bridge relay module (Pins D9, D10) for the main motor, and a Littelfuse 59170 magnetic limit switch (Pin D1 to GND) to the second microcontroller. *(Note: The final code is optimized to run without external indicator LEDs to minimize weight and power consumption).*

### 6.3 Software Installation
1. Open the Arduino IDE and ensure you have the ESP32 board manager installed.
2. Flash the `Transmitter` code to the remote control board.
3. Flash the `Receiver` code to the payload board. 
4. Power both devices. They will automatically pair via the ESP-NOW protocol and the system will be ready for ground testing before flight.

---

## 7. References
* Madsen, M. (2018). *DJI Phantom 3 – Payload Dropper v2*.
* Drone Sky Hook. *Dual Release and Drop PLUS for DJI Phantom 3*.
* 3Deddy. (2019). *DJI Phantom 3 – Payload Dropper*.
