# **Environmental Monitoring System with LoRaWAN Integration**

## **Overview**
This project implements an Environmental Monitoring System (EMS) using IoT technologies and LoRaWAN communication. The system monitors key environmental parameters such as temperature, humidity, light levels, and gas concentration in real-time. Data collected from sensors is transmitted wirelessly via LoRaWAN, enabling remote monitoring and analysis.

## **Features**
- **Sensor Integration:** Utilizes DHT11 sensor for temperature and humidity monitoring, light sensor for ambient light levels, and MQ-2 sensor for gas concentration detection.
- **LoRaWAN Connectivity:** Implements XLPP protocol for efficient data transmission over LoRaWAN network.
- **Threshold Alerts:** Alerts users via buzzer for critical environmental conditions exceeding predefined thresholds.
- **Actuation Control:** Controls LED indicators and relay switch based on ambient light levels detected.

## **Setup Instructions**
1. **Hardware Requirements:**
   - DHT11 sensor for temperature and humidity.
   - Light sensor for ambient light levels.
   - MQ-2 sensor for gas concentration.
   - LoRaWAN module compatible with XLPP protocol.
   - Buzzer, LED indicators, and relay switch for actuation.

2. **Software Setup:**
   - Install necessary libraries: WaziDev, xlpp, DHT.
   - Configure LoRaWAN keys (devAddr, nwkSkey, appSkey) in the code.
   - Upload the firmware to the IoT device using Arduino IDE or compatible software.

3. **Usage:**
   - Upon setup, the system will begin monitoring environmental parameters.
   - Monitor serial output for real-time sensor readings and LoRaWAN transmission status.
   - Adjust sensor thresholds (TEMP_THRESHOLD, HUMIDITY_THRESHOLD, LIGHT_THRESHOLD, MQ2_THRESHOLD) in the code as per environmental requirements.

## **Future Improvements**
- Implement data visualization using IoT platforms (e.g., ThingsBoard, TTN) for analytics and historical data tracking.
- Enhance sensor calibration and accuracy for precise environmental monitoring.
- Integrate with cloud services for remote data storage and access.

## **Contact**
For questions or feedback, please contact:
- Jean de Dieu UWINTWALI
- Email: uwintwalijeandedieu3@gmail.com
