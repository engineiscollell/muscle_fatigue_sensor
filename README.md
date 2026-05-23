## Muscle Fatigue Sensor (NIRS)

Development of a wearable muscle fatigue sensor based on the Near-Infrared Spectroscopy (NIRS) technique for monitoring muscle oxygenation (SmO₂). The project combines custom hardware and embedded software around an ESP32 microcontroller, with the objective of creating an accessible and reproducible wearable system.

The hardware side includes custom PCB design in EasyEDA, integration of optical sensing electronics, and practical work with SMD components and microscope soldering. The embedded software development includes firmware architecture, task processing, SmO₂ calculation algorithms, BLE communication, and a Python interface for data acquisition and visualization.

A future goal of the project is the integration of a 3D-printed enclosure and wearable band to transform the prototype into a complete open-source wearable device. The project was mainly developed as a hands-on introduction to real-world electronics engineering, embedded systems, PCB design, firmware development, and hardware/software integration.

---

### Hardware
Hardware development and PCB design were created in EasyEDA, including the `.epro` project design files for the custom boards and electronic schematics.

### Software
Embedded software and firmware development were implemented using PlatformIO for the ESP32 environment. BLE communication was mainly developed using the NimBLE library.
