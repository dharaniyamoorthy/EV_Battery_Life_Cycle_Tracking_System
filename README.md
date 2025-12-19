# EV_Battery_Life_Cycle_Tracking_System
Real-Time IoT Electric Vehicle Battery Life Cycle Tracking System using ESP32 and TinyML for SoH and RUL Prediction

Project Overview:  
  Electric Vehicles (EVs) play a vital role in sustainable transportation, but battery degradation and high replacement costs remain major challenges—especially in low-cost EVs such as two-wheelers and electric rickshaws. Most budget EVs lack advanced Battery Management Systems (BMS) with predictive analytics, leading to unexpected battery failures and reduced lifespan.
  This project presents a low-cost IoT-based Battery Life Cycle Tracking System using ESP32, TinyML, and Machine Learning models to monitor and predict battery health in real time. The system accurately estimates State of Health (SoH) and Remaining Useful Life (RUL) directly on the edge device, improving battery reliability and performance without expensive hardware.

Objectives:  
  Develop a low-cost IoT-based battery monitoring system for Electric Vehicles
  Monitor voltage, current, temperature, and charge cycles in real time
  Predict State of Health (SoH) and Remaining Useful Life (RUL) using ML models
  Deploy TinyML models on ESP32 for on-device prediction
  Improve battery lifespan and reduce maintenance costs in budget-friendly EVs

System Architecture:   
   Sensing Layer:
      INA219 sensor for voltage and current measurement
      DS18B20 sensor for battery temperature monitoring
   Edge Processing Layer:
      ESP32 microcontroller processes sensor data
      Coulomb Counting algorithm tracks charge–discharge cycles
      TinyML models predict battery health metrics
   Output & Communication Layer:
      SSD1306 OLED displays real-time battery parameters
      Serial communication and optional IoT cloud connectivity (MQTT/HTTP)

Machine Learning Models Used:  
      Random Forest
      Gradient Boosting
      LSTM (Long Short-Term Memory)
    These models are trained using historical battery data and optimized using TensorFlow Lite Micro for deployment on the ESP32.


