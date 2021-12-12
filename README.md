# MQTT-Based-Dashboard-for-Solar-Inverter
'Home Assistant MQTT Sensor' Based Dashboard for Voltronic Solar Inverter  
![Dashboard Image](https://github.com/amishv/MQTT-Based-Dashboard-for-Solar-Inverter/blob/main/SolarDashboard.png)
A dashboard to montior voltronic brand of solar inverters using the MQTT protocol. This is a companion application to the MQTT based reader for Voltronic Solar inverter https://github.com/amishv/Voltronics_Solar_protocol18 following Home assistant https://www.home-assistant.io/ MQTT sensors.

Hosting here for version control with MIT licence.

steps to build
1. Clone the repository
    git clone https://github.com/amishv/MQTT-Based-Dashboard-for-Solar-Inverter.git
2. go to the directory
    cd MQTT-Based-Dashboard-for-Solar-Inverter
3.  change the MQTT credentials in ./include/solarmon.h
    #define ADDRESS     "tcp://yo.ur.mq.tt:1883"
4. configure cmake
    cmake .
5. build the executable
    make all
6. execute the binary
    ./SolarDashboard

