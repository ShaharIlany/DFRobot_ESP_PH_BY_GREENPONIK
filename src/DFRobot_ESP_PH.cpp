/*
 * file DFRobot_ESP_PH.cpp * @ https://github.com/GreenPonik/DFRobot_ESP_PH_BY_GREENPONIK
 *
 * Arduino library for Gravity: Analog pH Sensor / Meter Kit V2, SKU: SEN0161-V2
 * 
 * Based on the @ https://github.com/DFRobot/DFRobot_PH
 * Copyright   [DFRobot](http://www.dfrobot.com), 2018
 * Copyright   GNU Lesser General Public License
 *
 * ##################################################
 * ##################################################
 * ########## Fork on github by GreenPonik ##########
 * ############# ONLY ESP COMPATIBLE ################
 * ##################################################
 * ##################################################
 * 
 * version  V1.0
 * date  2019-05
 */

#include "Arduino.h"
#include "DFRobot_ESP_PH.h"
#include "EEPROM.h"

#define PH_3_VOLTAGE 2010

DFRobot_ESP_PH::DFRobot_ESP_PH()
{
    this->_temperature = 25.0;
    this->_phValue = 7.0;
    this->_acidVoltage = 2032.44;   //buffer solution 4.0 at 25C
    this->_neutralVoltage = 1500.0; //buffer solution 7.0 at 25C
    this->_voltage = 1500.0;
}

DFRobot_ESP_PH::~DFRobot_ESP_PH()
{
}

void DFRobot_ESP_PH::begin()
{
    //check if calibration values (neutral and acid) are stored in eeprom
    this->_neutralVoltage = EEPROM.readFloat(PHVALUEADDR); //load the neutral (pH = 7.0)voltage of the pH board from the EEPROM
    if (this->_neutralVoltage == float() || isnan(this->_neutralVoltage))
    {
        this->_neutralVoltage = 1500.0; // new EEPROM, write typical voltage
        EEPROM.writeFloat(PHVALUEADDR, this->_neutralVoltage);
        EEPROM.commit();
    }

    this->_acidVoltage = EEPROM.readFloat(PHVALUEADDR + sizeof(float)); //load the acid (pH = 4.0) voltage of the pH board from the EEPROM
    if (this->_acidVoltage == float() || isnan(this->_acidVoltage))
    {
        this->_acidVoltage = 2032.44; // new EEPROM, write typical voltage
        EEPROM.writeFloat(PHVALUEADDR + sizeof(float), this->_acidVoltage);
        EEPROM.commit();
    }
}

float DFRobot_ESP_PH::readPH(float voltage, float temperature)
{
    // Serial.print("_neutraVoltage:");
    // Serial.print(this->_neutralVoltage);
    // Serial.print(", _acidVoltage:");
    // Serial.print(this->_acidVoltage);
    float slope = (7.0 - 4.0) / ((this->_neutralVoltage - 1500.0) / 3.0 - (this->_acidVoltage - 1500.0) / 3.0); // two point: (_neutralVoltage,7.0),(_acidVoltage,4.0)
    float intercept = 7.0 - slope * (this->_neutralVoltage - 1500.0) / 3.0;
    // Serial.print(", slope:");
    // Serial.print(slope);
    // Serial.print(", intercept:");
    // Serial.println(intercept);
    this->_phValue = slope * (voltage - 1500.0) / 3.0 + intercept; //y = k*x + b
    Serial.print("[readPH]... phValue ");
    Serial.println(this->_phValue);
    return this->_phValue;
}

String DFRobot_ESP_PH::phCalibration(float voltage, float temperature)
{    
    this->_voltage = voltage;
    this->_temperature = temperature;

    if ((this->_voltage > PH_8_VOLTAGE) && (this->_voltage < PH_6_VOLTAGE))
    { // buffer solution:7.0
        Serial.println();
        Serial.print(F(">>>Buffer Solution:7.0"));
        this->_neutralVoltage = this->_voltage;
        EEPROM.writeFloat(PHVALUEADDR, this->_neutralVoltage);
        EEPROM.commit();
        return "OK:7:" + String(this->_neutralVoltage);
    }
    else if ((this->_voltage > PH_5_VOLTAGE) && (this->_voltage < PH_3_VOLTAGE))
    { //buffer solution:4.0
        Serial.println();
        Serial.print(F(">>>Buffer Solution:4.0"));
        this->_acidVoltage = this->_voltage;
        EEPROM.writeFloat(PHVALUEADDR + sizeof(float), this->_acidVoltage);
        EEPROM.commit();
        return "OK:4:" + String(this->_acidVoltage);
    }
    else
    {
        Serial.println();
        Serial.print(F(">>>Buffer Solution Error Try Again<<<"));
        Serial.println(); // not buffer solution or faulty operation
        return "ERR";
    }
}
