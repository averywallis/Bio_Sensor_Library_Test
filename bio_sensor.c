/**
 * @file bio_sensor.c
 *
 * @brief bio_sensor.c
 *
 *  Created on: Nov 12, 2020
 *      Author: AveryW
 *
 * This library is based heavily on the SparkFun Bio Sensor Hub Library: https://github.com/sparkfun/SparkFun_Bio_Sensor_Hub_Library
 * which is an open source library produced by SparkFun Electronics. We would like to thank SparkFun and Elias Santistevan (main author)
 * for writing this library and making it available to the public.
 *
 * Below is the header from the SparkFun Bio Sensor Hub Library files, which gives a description of the library ours was based on:
 *
 *  "This is an Arduino Library written for the MAXIM 32664 Biometric Sensor Hub
    The MAX32664 Biometric Sensor Hub is in actuality a small Cortex M4 microcontroller
    with pre-loaded firmware and algorithms used to interact with the a number of MAXIM
    sensors; specifically the MAX30101 Pulse Oximter and Heart Rate Monitor and
    the KX122 Accelerometer. With that in mind, this library is built to
    communicate with a middle-person and so has a unique method of communication
    (family, index, and write bytes) that is more simplistic than writing and reading to
    registers, but includes a larger set of definable values.

    SparkFun Electronics
    Date: June, 2019
    Author: Elias Santistevan
kk
    License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

    Feel like supporting our work? Buy a board from SparkFun!"
 */



#include "bio_sensor.h"
#include <ti/drivers/I2C.h>


I2C_Handle      gi2cHandle; ///< global I2C Handle object used in handle I2C issues
I2C_Transaction gi2cTransaction; ///< global I2C Transaction object. Used for I2C transactions. Includes information like peripheral address, TX/RX size, etc.

uint8_t userAlgoMode; ///< Selected User Mode (disabled, algorithm Mode 1 or Mode 2)
uint8_t userOutputMode; ///< Selected User Output Mode (Raw data, algorithm data, raw + algo data)
uint8_t sampleNum = 100; ///< Number of samples averaged by the AGC algorithm


/**
 * @brief      Takes the I2C handle object to read the current sensor hub mode
 *
 * familyByte  N/A   - multiple I2C transactions internally
 *
 * indexByte   N/A   - multiple I2C transactions internally
 *
 * writeByte0  N/A   - multiple I2C transactions internally
 *
 * writeByteN  N/A   - multiple I2C transactions internally
 *
 * @param i2cHandle I2C_Handle Object
 * @param *statusByte Pointer to status byte
 *
 * @return mode - Current device operating mode, ERR_UNKNOWN on I2C transaction issues (check status byte!)
 */
uint8_t beginI2C(I2C_Handle i2cHandle, uint8_t *statusByte){
//    gi2cTransaction = i2cTrans;
    gi2cHandle = i2cHandle; //copy over the I2C Handle object

    uint8_t mode = readDeviceMode(statusByte); //read the current device mode

    if(*statusByte != SUCCESS){ //if there was an I2C transaction error
        return ERR_UNKNOWN; //return an error
    }

    if(mode == RESET || mode == ENTER_BOOTLOADER){ //if we're in reset mode or bootloader mode
        mode = setDeviceMode(EXIT_BOOTLOADER, statusByte); //set us into operating mode
        sleep(2); //sleep for 2 seconds to ensure that it's back in application mode and ready to receive I2C transactions
    }

    if(*statusByte != SUCCESS){ //if there was an I2C transaction error
        return ERR_UNKNOWN; //return an error
    }

    return mode; //return the current operating mode
}


/**
 * @brief       Configures the MAX32664. Configures the output format of the data, set the algorithm mode, sets FIFO threshold,
 *              enables the Automatic Gain Control (AGC) algorithm, enables the MAX30101 pulse oximeter,
 *              enables the Wearable Heart Rate Monitor (WHRM)/MaximFast algorithm to the specified mode,
 *              reads number of samples averaged by the AGC algorithm.
 *
 * familyByte  N/A   - multiple I2C transactions internally
 *
 * indexByte   N/A   - multiple I2C transactions internally
 *
 * writeByte0  N/A   - multiple I2C transactions internally
 *
 * writeByteN  N/A   - multiple I2C transactions internally
 *
 * @pre beginI2C() to pass I2C handle object
 *
 * @param   outputFormat  Format of the output data (raw MAX30101 ADC reading, algorithm data, OR raw reading + algorithm data)
 * @param   algoMode    Mode you want to set the MaximFast Algorithm to (mode 1 OR mode 2)
 * @param   intTresh    Number of samples taken before interrupt is generate
 *
 * @return statusChauf - Status of I2C transactions
 */
uint8_t configMAX32664(uint8_t outputFormat, uint8_t algoMode, uint8_t intThresh){

    uint8_t statusChauf = 0;

    if(outputFormat != ALGO_DATA && outputFormat != SENSOR_DATA && outputFormat != SENSOR_AND_ALGORITHM){ //if the selected data output format is not a valid option
        return INCORR_PARAM; //return incorrect parameter error
    }
    else{ //else we've got a valid output format
        userOutputMode = outputFormat; //save the current output format
    }

    usleep(20000); //sleep for 20ms after getting ERR_TRY_AGAIN

    if(algoMode != MODE_ONE && algoMode != MODE_TWO){ //if we don't have a valid algorithm mode
        return INCORR_PARAM; //return incorrect parameter error
    }
    else{ //else we've got a valid algorithm mode
        userAlgoMode = algoMode; //save the current algorithm mode
    }

    usleep(20000); //sleep for 20ms after getting ERR_TRY_AGAIN

    statusChauf = setOutputMode(outputFormat); //set the output mode to be the passed format
    if(statusChauf != SUCCESS){ //if setting the output mode wasn't successful
        return statusChauf; //return the status byte of I2C transaction
    }

    usleep(20000); //sleep for 20ms after getting ERR_TRY_AGAIN

    statusChauf = setFifoThreshold(intThresh); //set the FIFO threshold to the passed number
    if(statusChauf != SUCCESS){ //if setting FIFO threshold wasn't successful
        return statusChauf; //return the status byte of I2C transaction
    }

    usleep(20000); //sleep for 20ms after getting ERR_TRY_AGAIN

    statusChauf = agcAlgoControl(ENABLE); //enable to AGC algorithm
    if(statusChauf != SUCCESS){ //if enabling the AGC algorithm wasn't successful
        return statusChauf; //return the status byte of I2C transaction
    }

    usleep(20000); //sleep for 20ms after getting ERR_TRY_AGAIN

    statusChauf = max30101Control(ENABLE);  //enable the MAX30101 sensor
    if(statusChauf != SUCCESS){ //if enabling the sensor wasn't successful
        return statusChauf; //return the status byte of I2C transaction
    }

    usleep(20000); //sleep for 20ms after getting ERR_TRY_AGAIN

    statusChauf = maximFastAlgoControl(algoMode);  //set the WHRM algorithm mode or disable
    if(statusChauf != SUCCESS){ //if setting the algorithm didn't work
        return statusChauf; //return the status byte of I2C transaction
    }

    usleep(20000); //sleep for 20ms after getting ERR_TRY_AGAIN

    sampleNum = readAlgoSamples(&statusChauf); //read the number of samples averaged by the AGC algorithm

    if(statusChauf != SUCCESS){ //if there was an I2C transaction issue
        return statusChauf; //return that status byte
    }

    return SUCCESS; //return success if everything went well
}


/**
 * @brief       Read sensor data from MAX32664 output FIFO. Data contents/format depend on previous settings
 *              (typically set by configMAX32664)
 *
 * familyByte  N/A   - multiple I2C transactions internally
 *
 * indexByte   N/A   - multiple I2C transactions internally
 *
 * writeByte0  N/A   - multiple I2C transactions internally
 *
 * writeByteN  N/A   - multiple I2C transactions internally
 *
 * @pre         configMAX32664() or other function(s) that configures the data output format, algorithm mode, interrupt threshold, enable the algorithms, and enable the MAX30101
 *
 * @param       *statusByte Pointer to status byte
 *
 * @return      libData - Struct containing body data (IR and Red LED ADC count, heart rate, confidence, SpO2, algorithm state, etc.)
 *                        Contents and formats depend on previous settings (typically set by configBPM).
 *                        0s when not used or I2C transaction errors (check status byte!)
 */
struct bioData readSensorData(uint8_t *statusByte){

    struct bioData libData; //struct for the data we're going to extract from the sensor
    uint8_t statusChauf; // The status chauffeur captures return values.

    statusChauf = readSensorHubStatus(statusByte);

    if(statusChauf & 0x01){ //if there was a communication error (Err0[0] bit == Sensor Communication Problem)
        libData.irLed = 0;
        libData.redLed = 0;
        libData.heartRate = 0;
        libData.confidence = 0;
        libData.oxygen = 0;
        libData.status = 0;
        libData.rValue = 0.00;
        libData.extStatus = 0;
        *statusByte = ERR_UNKNOWN;
        return libData;
    }
    else if(*statusByte != SUCCESS){ //if the status byte is non-zero (some error in I2C communication)
        libData.irLed = 0;
        libData.redLed = 0;
        libData.heartRate = 0;
        libData.confidence = 0;
        libData.oxygen = 0;
        libData.status = 0;
        libData.rValue = 0.00;
        libData.extStatus = 0;
        return libData;
    }

    uint8_t numSamples = numSamplesOutFifo(statusByte);

    if(*statusByte != SUCCESS){ //if the status byte is non-zero (some error in I2C communication)
        libData.irLed = 0;
        libData.redLed = 0;
        libData.heartRate = 0;
        libData.confidence = 0;
        libData.oxygen = 0;
        libData.status = 0;
        libData.rValue = 0.00;
        libData.extStatus = 0;
        numSamples = 0;
        return libData;
    }

    if(userOutputMode == SENSOR_DATA){ //if out data output format was just the raw ADC readings

        libData = readRawData(statusByte); //read the raw sensor data

        if(*statusByte != SUCCESS){ //if the status byte is non-zero (some error in I2C communication)
            //reset all values to zero
            libData.irLed = 0;
            libData.redLed = 0;
            libData.heartRate = 0;
            libData.confidence = 0;
            libData.oxygen = 0;
            libData.status = 0;
            libData.rValue = 0.00;
            libData.extStatus = 0;
            numSamples = 0;
            return libData; //return this data
        }

        return libData; //return the raw data
    }

    else if(userOutputMode == ALGO_DATA){ //if output data is algorithm data and we're in algorithm Mode 1

        libData = readAlgoData(statusByte); //read the algorithm data

        if(*statusByte != SUCCESS){ //if the status byte is non-zero (some error in I2C communication)
            //reset all values to zero
            libData.irLed = 0;
            libData.redLed = 0;
            libData.heartRate = 0;
            libData.confidence = 0;
            libData.oxygen = 0;
            libData.status = 0;
            libData.rValue = 0.00;
            libData.extStatus = 0;
            numSamples = 0;
            return libData; //return this data
        }

        return libData; //return the algorithm data
    }

    else if(userOutputMode == SENSOR_AND_ALGORITHM){ //if the output data is raw+algorithm

        libData = readRawAndAlgoData(statusByte);

        if(*statusByte != SUCCESS){ //if the status byte is non-zero (some error in I2C communication)
            //reset all values to zero
            libData.irLed = 0;
            libData.redLed = 0;
            libData.heartRate = 0;
            libData.confidence = 0;
            libData.oxygen = 0;
            libData.status = 0;
            libData.rValue = 0.00;
            libData.extStatus = 0;
            numSamples = 0;
            return libData; //return this data
        }

        return libData; //return the raw+algorithm data
    }

    else{ //else there's an issue with the current mode (not a valid output mode supported by this library)
        libData.irLed = 0;
        libData.redLed = 0;
        libData.heartRate = 0;
        libData.confidence = 0;
        libData.oxygen = 0;
        libData.status = 0;
        libData.rValue = 0.00;
        libData.extStatus = 0;
        *statusByte = INCORR_PARAM;
        return libData;
    }

}


/**
 * @brief   Reads the raw sensor data, assuming only the raw sensor data is being output
 *
 * familyByte - READ_DATA_OUTPUT (0x12)
 *
 * indexByte  - READ_DATA (0x01)
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @pre     configMAX32664() function, set to output only raw data
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  libRawData - Struct of raw sensor data, 0s on I2C failure or incorrect setting (check status byte!)
 */
struct bioData readRawData(uint8_t *statusByte){

        struct bioData libRawData;

        if(userOutputMode != SENSOR_DATA){ //if out data output format is NOT the raw ADC readings
            //set all the data to 0
            libRawData.irLed = 0;
            libRawData.redLed = 0;
            libRawData.heartRate = 0;
            libRawData.confidence = 0;
            libRawData.oxygen = 0;
            libRawData.status = 0;
            libRawData.rValue = 0.00;
            libRawData.extStatus = 0;
            *statusByte = INCORR_PARAM; //return that we somehow called this function with incorrect parameters
            return libRawData;
        }

        uint8_t sensorData[MAX30101_LED_ARRAY]; //array of the size of the size we expect

        uint8_t fillArrayStatusByte = I2CReadFillArray(READ_DATA_OUTPUT, READ_DATA, MAX30101_LED_ARRAY, sensorData); //read the raw data

        if(fillArrayStatusByte != SUCCESS){ //if there was an error reading the sensor data
            libRawData.irLed = 0;
            libRawData.redLed = 0;
            libRawData.heartRate = 0;
            libRawData.confidence = 0;
            libRawData.oxygen = 0;
            libRawData.status = 0;
            libRawData.rValue = 0.00;
            libRawData.extStatus = 0;
            *statusByte = fillArrayStatusByte; //status byte from array
            return libRawData; //return the data
        }

        //set the data we're not using in this mode to zero
        libRawData.heartRate = 0;
        libRawData.confidence = 0;
        libRawData.oxygen = 0;
        libRawData.status = 0;
        libRawData.rValue = 0.00;
        libRawData.extStatus = 0;

        //For the IR LED
        libRawData.irLed = ( (uint32_t)sensorData[0]) << 16; //shift over the MSB of data
        libRawData.irLed |= ( (uint32_t)sensorData[1]) << 8; //shift over the second byte of data
        libRawData.irLed |= (uint32_t)sensorData[2]; //add LSB of data

        //For the RED LED
        libRawData.redLed = ( (uint32_t)sensorData[3]) << 16; //shift over the MSB of the LED data
        libRawData.redLed |= ( (uint32_t)sensorData[4]) << 8; //shift over second byte of data
        libRawData.redLed |= (uint32_t)sensorData[5]; //add LSB of data

        //The array will return 2 additional 24-bit numbers that currently have no use, so we'll ignore them

        *statusByte = fillArrayStatusByte; //save the status byte
        return libRawData; //return the raw sensor data
}


/**
 * @brief   Reads the algorithm data, assuming only the algorithm data is being output
 *
 * familyByte - READ_DATA_OUTPUT (0x12)
 *
 * indexByte  - READ_DATA (0x01)
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @pre     configMAX32664() function, set to output only algorithm data
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  libAlgoData - Struct of algorithm data, all 0s on an error
 */
struct bioData readAlgoData(uint8_t *statusByte){

    struct bioData libAlgoData; //struct for Algorithm Data

    if(userOutputMode != ALGO_DATA){ //if we called this function and the output isn't set to algorithm output
        //set all the data to 0
        libAlgoData.irLed = 0;
        libAlgoData.redLed = 0;
        libAlgoData.heartRate = 0;
        libAlgoData.confidence = 0;
        libAlgoData.oxygen = 0;
        libAlgoData.status = 0;
        libAlgoData.rValue = 0.00;
        libAlgoData.extStatus = 0;
        *statusByte = INCORR_PARAM; //return that we somehow called this function with incorrect parameters
        return libAlgoData;
    }

    if(userAlgoMode == MODE_ONE){ //if output data is algorithm data and we're in algorithm Mode 1

        uint8_t sensorData[MAXFAST_ARRAY_SIZE];

        uint8_t fillArrayStatusByte = I2CReadFillArray(READ_DATA_OUTPUT, READ_DATA, MAXFAST_ARRAY_SIZE, sensorData);

        if(fillArrayStatusByte){ //if there was an error reading the sensor data
            libAlgoData.irLed = 0;
            libAlgoData.redLed = 0;
            libAlgoData.heartRate = 0;
            libAlgoData.confidence = 0;
            libAlgoData.oxygen = 0;
            libAlgoData.status = 0;
            libAlgoData.rValue = 0.00;
            libAlgoData.extStatus = 0;
            *statusByte = fillArrayStatusByte; //status byte from array
            return libAlgoData;
        }

        //set the data we're not using in this mode to zero
        libAlgoData.irLed = 0;
        libAlgoData.redLed = 0;
        libAlgoData.rValue = 0.00;
        libAlgoData.extStatus = 0;

        //extract the sensor data
        libAlgoData.heartRate = (uint16_t)(sensorData[0]) << 8; //get the MSB of heartRate
        libAlgoData.heartRate |= sensorData[1]; //get LSB of heart rate
        libAlgoData.heartRate /= 10; //divide by 10 to get the current heart rate

        libAlgoData.confidence = sensorData[2];

        libAlgoData.oxygen = (uint16_t)(sensorData[3]) << 8; //get the MSB of SpO2 data
        libAlgoData.oxygen |= sensorData[4]; //get LSB of SpO2 data
        libAlgoData.oxygen /= 10; //divide by 10 to get current SpO2 level;

        libAlgoData.status = sensorData[5]; //get the algorithm state

        *statusByte = fillArrayStatusByte; //status byte from array
        return libAlgoData;
    }

    else if(userAlgoMode == MODE_TWO){ //if output data is algorithm data and we're in algorith Mode 2

        uint8_t sensorData[MAXFAST_ARRAY_SIZE + MAXFAST_EXTENDED_DATA];

        uint8_t fillArrayStatusByte = I2CReadFillArray(READ_DATA_OUTPUT, READ_DATA, MAXFAST_ARRAY_SIZE + MAXFAST_EXTENDED_DATA, sensorData);

        if(fillArrayStatusByte){ //if there was an error reading the sensor data
            libAlgoData.irLed = 0;
            libAlgoData.redLed = 0;
            libAlgoData.heartRate = 0;
            libAlgoData.confidence = 0;
            libAlgoData.oxygen = 0;
            libAlgoData.status = 0;
            libAlgoData.rValue = 0.00;
            libAlgoData.extStatus = 0;
            *statusByte = fillArrayStatusByte; //save the status byte from the fill array
            return libAlgoData;
        }

        //set the data we're not using in this mode to zero
        libAlgoData.irLed = 0;
        libAlgoData.redLed = 0;

        //extract the sensor data
        libAlgoData.heartRate = (uint16_t)(sensorData[0]) << 8; //get the MSB of heartRate
        libAlgoData.heartRate |= sensorData[1]; //get LSB of heart rate
        libAlgoData.heartRate /= 10; //divide by 10 to get the current heart rate

        libAlgoData.confidence = sensorData[2]; //get the HR confidence level

        libAlgoData.oxygen = (uint16_t)(sensorData[3]) << 8; //get the MSB of SpO2 data
        libAlgoData.oxygen |= sensorData[4]; //get LSB of SpO2 data
        libAlgoData.oxygen /= 10; //divide by 10 to get current SpO2 level;

        libAlgoData.status = sensorData[5]; //get the algorithm state

        //need to use temp value to because extracting to floating point
        uint16_t tempVal = (uint16_t)(sensorData[6]) << 8; //get the MSB of the rValue
        tempVal |= sensorData[7]; //get LSB of rValue, bitwise OR with MSB
        libAlgoData.rValue = tempVal; //convert the integer to float
        libAlgoData.rValue /= 10.0; //divide by 10 to get the rValue

        libAlgoData.extStatus = sensorData[8]; //get the extended algorithm status

        *statusByte = fillArrayStatusByte; //save the status byte from the fill array
        return libAlgoData;
    }

    else{ //else we're in a combination of mode and output format not supported by this function
        //set all the data to 0
        libAlgoData.irLed = 0;
        libAlgoData.redLed = 0;
        libAlgoData.heartRate = 0;
        libAlgoData.confidence = 0;
        libAlgoData.oxygen = 0;
        libAlgoData.status = 0;
        libAlgoData.rValue = 0.00;
        libAlgoData.extStatus = 0;
        *statusByte = INCORR_PARAM; //return that we somehow called this function with incorrect parameters
        return libAlgoData;
    }
}


/**
 * @brief   Reads the raw+algorithm data if that this is the output format
 *
 * familyByte - READ_DATA_OUTPUT (0x12)
 *
 * indexByte  - READ_DATA (0x01)
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @pre     configMAX32664() function, set to output raw+algorithm data
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  libRawAlgoData - Struct of raw+algorithm data, all 0s on I2C failure (check status byte!)
 */
struct bioData readRawAndAlgoData(uint8_t *statusByte){

    struct bioData libRawAlgoData; //struct for raw+algorithm data

    if(userOutputMode != SENSOR_AND_ALGORITHM){ //if we called this function and the output isn't set to raw+algorithm output
        //set all the data to 0
        libRawAlgoData.irLed = 0;
        libRawAlgoData.redLed = 0;
        libRawAlgoData.heartRate = 0;
        libRawAlgoData.confidence = 0;
        libRawAlgoData.oxygen = 0;
        libRawAlgoData.status = 0;
        libRawAlgoData.rValue = 0.00;
        libRawAlgoData.extStatus = 0;
        *statusByte = INCORR_PARAM; //return that we somehow called this function with incorrect parameters
        return libRawAlgoData;
    }

    if(userAlgoMode == MODE_ONE){ //if we're in algorithm Mode 1

        uint8_t sensorData[MAX30101_LED_ARRAY + MAXFAST_ARRAY_SIZE];

        uint8_t fillArrayStatusByte = I2CReadFillArray(READ_DATA_OUTPUT, READ_DATA, MAX30101_LED_ARRAY + MAXFAST_ARRAY_SIZE, sensorData);

        if(fillArrayStatusByte){ //if there was an error reading the sensor data
            libRawAlgoData.irLed = 0;
            libRawAlgoData.redLed = 0;
            libRawAlgoData.heartRate = 0;
            libRawAlgoData.confidence = 0;
            libRawAlgoData.oxygen = 0;
            libRawAlgoData.status = 0;
            libRawAlgoData.rValue = 0.00;
            libRawAlgoData.extStatus = 0;
            *statusByte = fillArrayStatusByte; //status byte from array
            return libRawAlgoData;
        }

        //set the data we're not using in this mode to zero
        libRawAlgoData.rValue = 0.00;
        libRawAlgoData.extStatus = 0;

        //For the IR LED
        libRawAlgoData.irLed = ( (uint32_t)sensorData[0]) << 16; //shift over the MSB of data
        libRawAlgoData.irLed |= ( (uint32_t)sensorData[1]) << 8; //shift over the second byte of data
        libRawAlgoData.irLed |= (uint32_t)sensorData[2]; //add LSB of data

        //For the RED LED
        libRawAlgoData.redLed = ( (uint32_t)sensorData[3]) << 16; //shift over the MSB of the LED data
        libRawAlgoData.redLed |= ( (uint32_t)sensorData[4]) << 8; //shift over second byte of data
        libRawAlgoData.redLed |= (uint32_t)sensorData[5]; //add LSB of data
        //The array will return 2 additional 24-bit numbers that currently have no use, so we'll ignore them

        //extract the sensor data
        libRawAlgoData.heartRate = (uint16_t)(sensorData[12]) << 8; //get the MSB of heartRate
        libRawAlgoData.heartRate |= sensorData[13]; //get LSB of heart rate
        libRawAlgoData.heartRate /= 10; //divide by 10 to get the current heart rate

        libRawAlgoData.confidence = sensorData[14];

        libRawAlgoData.oxygen = (uint16_t)(sensorData[15]) << 8; //get the MSB of SpO2 data
        libRawAlgoData.oxygen |= sensorData[16]; //get LSB of SpO2 data
        libRawAlgoData.oxygen /= 10; //divide by 10 to get current SpO2 level;

        libRawAlgoData.status = sensorData[17]; //get the algorithm state

        *statusByte = fillArrayStatusByte; //status byte from array
        return libRawAlgoData; //return the raw+algo data struct
    }

    else if(userAlgoMode == MODE_TWO){ //if output data is algorithm data and we're in algorith Mode 2

        uint8_t sensorData[MAX30101_LED_ARRAY + MAXFAST_ARRAY_SIZE + MAXFAST_EXTENDED_DATA];

        uint8_t fillArrayStatusByte = I2CReadFillArray(READ_DATA_OUTPUT, READ_DATA, MAX30101_LED_ARRAY + MAXFAST_ARRAY_SIZE + MAXFAST_EXTENDED_DATA, sensorData);

        if(fillArrayStatusByte){ //if there was an error reading the sensor data
            libRawAlgoData.irLed = 0;
            libRawAlgoData.redLed = 0;
            libRawAlgoData.heartRate = 0;
            libRawAlgoData.confidence = 0;
            libRawAlgoData.oxygen = 0;
            libRawAlgoData.status = 0;
            libRawAlgoData.rValue = 0.00;
            libRawAlgoData.extStatus = 0;
            *statusByte = fillArrayStatusByte; //save the status byte from the fill array
            return libRawAlgoData;
        }

        //For the IR LED
        libRawAlgoData.irLed = ( (uint32_t)sensorData[0]) << 16; //shift over the MSB of data
        libRawAlgoData.irLed |= ( (uint32_t)sensorData[1]) << 8; //shift over the second byte of data
        libRawAlgoData.irLed |= (uint32_t)sensorData[2]; //add LSB of data

        //For the RED LED
        libRawAlgoData.redLed = ( (uint32_t)sensorData[3]) << 16; //shift over the MSB of the LED data
        libRawAlgoData.redLed |= ( (uint32_t)sensorData[4]) << 8; //shift over second byte of data
        libRawAlgoData.redLed |= (uint32_t)sensorData[5]; //add LSB of data
        //The array will return 2 additional 24-bit numbers that currently have no use, so we'll ignore them

        //extract the sensor data
        libRawAlgoData.heartRate = (uint16_t)(sensorData[12]) << 8; //get the MSB of heartRate
        libRawAlgoData.heartRate |= sensorData[13]; //get LSB of heart rate
        libRawAlgoData.heartRate /= 10; //divide by 10 to get the current heart rate

        libRawAlgoData.confidence = sensorData[14]; //get the HR confidence level

        libRawAlgoData.oxygen = (uint16_t)(sensorData[15]) << 8; //get the MSB of SpO2 data
        libRawAlgoData.oxygen |= sensorData[16]; //get LSB of SpO2 data
        libRawAlgoData.oxygen /= 10; //divide by 10 to get current SpO2 level;

        libRawAlgoData.status = sensorData[17]; //get the algorithm state

        //need to use temp value to because extracting to floating point
        uint16_t tempVal = (uint16_t)(sensorData[18]) << 8; //get the MSB of the rValue
        tempVal |= sensorData[19]; //get LSB of rValue, bitwise OR with MSB
        libRawAlgoData.rValue = tempVal; //convert the integer to float
        libRawAlgoData.rValue /= 10.0; //divide by 10 to get the rValue

        libRawAlgoData.extStatus = sensorData[20]; //get the extended algorithm status

        *statusByte = fillArrayStatusByte; //save the status byte from the fill array
        return libRawAlgoData; //return the raw+algo data struct
    }

    else{ //else we're in a combination of mode and output format not supported by this function
        //set all the data to 0
        libRawAlgoData.irLed = 0;
        libRawAlgoData.redLed = 0;
        libRawAlgoData.heartRate = 0;
        libRawAlgoData.confidence = 0;
        libRawAlgoData.oxygen = 0;
        libRawAlgoData.status = 0;
        libRawAlgoData.rValue = 0.00;
        libRawAlgoData.extStatus = 0;
        *statusByte = INCORR_PARAM; //return that we somehow called this function with incorrect parameters
        return libRawAlgoData;
    }


}


/**
 * @brief   Does a software reset of the device. Sets the device mode of the MAX32664 to reset mode, then to application mode.
 *          Finally waits 1s before turning to ensure that initialization is complete
 *
 * familyByte  N/A   - multiple I2C transactions internally
 *
 * indexByte   N/A   - multiple I2C transactions internally
 *
 * writeByte0  N/A   - multiple I2C transactions internally
 *
 * writeByteN  N/A   - multiple I2C transactions internally
 *
 * @post configMAX32664() to reconfigure the MAX32664
 *
 * @param none
 *
 * @return statusByte - Status byte of I2C transaction or ERR_UNKNOWN if we don't return to application mode
 */
uint8_t softwareResetMAX32664(void){
    uint8_t statusByte = 0;
    uint8_t mode = 0;

    setDeviceMode(RESET, &statusByte); //set it into reset mode

    if(statusByte != SUCCESS){ //if our I2C transaction had issues
        return statusByte; //return the error we had
    }

    usleep(10000); //sleep for 10ms

    mode = setDeviceMode(EXIT_BOOTLOADER, &statusByte); //set it into application mode

    if(statusByte != SUCCESS){ //if our I2C transaction had issues
        return statusByte; //return the error we had
    }

    sleep(2); //sleep for 2s to ensure initialization is complete

    if(mode != EXIT_BOOTLOADER){ //if we're not in application mode
       return ERR_UNKNOWN; //return an error
    }

    return SUCCESS;
}


/**
 * @brief   Performs a reset of MAX30101 by setting the Reset Control bit to 1
 *
 * MAX30101 Register - MODE_REGISTER (0x09)
 *
 * @post    configMAX32664() to configure the MAX30101 appropriately
 *
 * @param none
 *
 * @return statusByte - Status byte of I2C transaction
 */
uint8_t softwareResetMAX30101(void){

    uint8_t regVal = 0;
    uint8_t statusByte = 0;

    regVal = readRegisterMAX30101(MODE_REGISTER, &statusByte); //read the MAX30101 mode configuration register
    regVal &= RESET_MASK; //mask out the bits we want to change, keeping others
    regVal |= SET_RESET_BIT; //set the reset bit to 1

    if(statusByte != SUCCESS){ //if there was an I2C transaction issue while reading the register
        return statusByte; //return the status byte
    }

    statusByte = writeRegisterMAX30101(MODE_REGISTER, regVal); //write to register

    if(statusByte != SUCCESS){ //if there was an I2C transaction issue while writing to the register
        return statusByte; //return the status byte
    }

    sleep(2); //delay to ensure MAX30101 has reset

    return SUCCESS; //return success
}

/**
 * @brief Sets the data output mode: raw, algorithm, raw+algorithm, etc.
 *
 * familyByte - OUTPUT_MODE (0x10)
 *
 * indexByte  - SET_FORMAT (0x00)
 *
 * writeByte0 - outputType (0x00 to 0x07)
 *
 * writeByteN - none
 *
 * @param outputType Output type of the data. See OUTPUT_MODE_WRITE_BYTE for valid options
 *
 * @return outputModeStatus - returns SUCCESS if everything goes well, return status byte if issue with I2C transaction
 */
uint8_t setOutputMode(uint8_t outputType){

    if(outputType > 0x07){ //only accept bytes between 0x00, 0x07
        return INCORR_PARAM; //return incorrect parameter error flag
    }

    uint8_t outputModeStatus = I2CWriteByte(OUTPUT_MODE, SET_FORMAT, outputType); //Set Output mode to selected output type

    if(outputModeStatus != SUCCESS){ //if we didn't have a success
        return outputModeStatus; //return the status byte as an error
    }
    else{
        return SUCCESS; //return 0x00 (SUCCESS)
    }
}


/**
 * @brief   Sets output FIFO threshold (# data samples extracted before MRIO output interrupt generated)
 *
 * familyByte - OUTPUT_MODE (0x10)
 *
 * indexByte  - WRITE_SET_THRESHOLD (0x01)
 *
 * writeByte0 - intThresh (0x01 to 0xff)
 *
 * writeByteN - none
 *
 * @param   intThresh FIFO interrupt threshold (0x01 to 0xff)
 *
 * @return  fifoThreshStatus - SUCCESS when no I2C transaction issues, status byte when there was an i2C transaction issue
 */
uint8_t setFifoThreshold(uint8_t intThresh){ //set the FIFO threshold

    uint8_t fifoThreshStatus = I2CWriteByte(OUTPUT_MODE, WRITE_SET_THRESHOLD, intThresh);

    if(fifoThreshStatus != SUCCESS){
        return fifoThreshStatus;
    }
    else{
        return SUCCESS;
    }
}


/**
 * @brief   Reads number of samples available in the output FIFO
 *
 * familyByte - READ_DATA_OUTPUT (0x12)
 *
 * indexByte  - NUM_SAMPLES (0x00)
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  sampAvail - Number of samples available in the output FIFO, 0 on failure
 */
uint8_t numSamplesOutFifo(uint8_t *statusByte){ //get the number of samples available in the FIFO

    uint8_t sampAvail = I2CReadByte(READ_DATA_OUTPUT, NUM_SAMPLES, statusByte);

    return sampAvail; //return the samples available
}


/**
 * @brief   Controls the AGC algorithm, enables or disables it
 *
 * familyByte - ENABLE_ALGORITHM (0x52)
 *
 * indexByte  - ENABLE_AGC_ALGO (0x00)
 *
 * writeByte0 - enable
 *
 * writeByteN - none
 *
 * @param   enable Algorithm enable (0x01)/disable (0x00) parameter
 *
 * @return  agcStatusByte - Status byte of I2C transaction
 */
uint8_t agcAlgoControl(uint8_t enable){ //enable or disable the AGC Algorithm

    if(enable == 0 || enable == 1){} //if passed an incorrect parameter
    else{
        return INCORR_PARAM; //return error byte
    }

    uint8_t agcStatusByte = I2CenableWriteByte(ENABLE_ALGORITHM, ENABLE_AGC_ALGO, enable); //Enable or disable the AGC Algorithm

    if(agcStatusByte != SUCCESS){ //if we didn't get a successful transaction
        return agcStatusByte;
    }
    else{
        return SUCCESS;
    }
}


/**
 * @brief   Controls the MAX30101 pulse oximeter, enables or disables it
 *
 * familyByte - ENABLE_SENSOR (0x44)
 *
 * indexByte  - ENABLE_MAX30101 (0x03)
 *
 * writeByte0 - senSwitch
 *
 * writeByteN - none
 *
 * @param   senSwitch MAX30101 enable (0x01)/disable(0x00) parameter
 *
 * @return  maxStatusByte - Returns SUCCESS if everything goes well, return status byte if issue with I2C transaction
 */
uint8_t max30101Control(uint8_t senSwitch){ //enable or disable the MAX30101 sensor

    if(senSwitch == 0 || senSwitch == 1){} //if passed an incorrect parameter
    else{
        return INCORR_PARAM; //return error byte
    }

    uint8_t maxStatusByte = I2CenableWriteByte(ENABLE_SENSOR, ENABLE_MAX30101, senSwitch); //Enable or disable the MAX30101 sensor

    if(maxStatusByte != SUCCESS){ //if we didn't get a successful transaction
        return maxStatusByte;
    }
    else{
        return SUCCESS;
    }
}


/**
 * @brief   Reads the MAX30101 pulse oximeter state
 *
 * familyByte - READ_SENSOR_MODE (0x45)
 *
 * indexByte  - READ_ENABLE_MAX30101 (0x03)
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  maxState - Read MAX30101 state (0x00 == disabled / 0x01 == enabled), 0 on a failure (check status byte!)
 */
uint8_t readMAX30101State(uint8_t *statusByte){ //read the MAX30101 sensor mode

    uint8_t maxState = I2CReadByte(READ_SENSOR_MODE, READ_ENABLE_MAX30101, statusByte); //read the MAX30101 sensor mode

    return maxState;
}


/**
 * @brief   Controls the WHRM/Maxim Algorithm, disables it or sets the mode (mode 1 or mode 2)
 *
 * familyByte - ENABLE_ALGORITHM (0x52)
 *
 * indexByte  - ENABLE_WHRM_ALGO (0x02)
 *
 * writeByte0 - mode
 *
 * writeByteN - none
 *
 * @param   mode Disables (0x00) or sets algorithm mode to mode 1 (0x01) or mode 2 (0x02)
 *
 * @return  maximAlgoStatusByte - Returns SUCCESS if everything goes well, return status byte if issue with I2C transaction
 */
uint8_t maximFastAlgoControl(uint8_t mode){ //enable or disable the wearable heart rate monitor algorithm

    if(mode == 0 || mode == 1 || mode == 2){}
    else{
        return INCORR_PARAM;
    }

    uint8_t maximAlgoStatusByte = I2CenableWriteByte(ENABLE_ALGORITHM, ENABLE_WHRM_ALGO, mode); //Enable or disable the maxim Algorithm

    if(maximAlgoStatusByte != SUCCESS){
        return maximAlgoStatusByte;
    }
    else{
        return SUCCESS;
    }
}


/**
 * @brief   Reads the current MAX32664 operating mode.
 *
 *          0x00: Application operating mode
 *
 *          0x02: Reset
 *
 *          0x08: Bootloader operating mode
 *
 * familyByte - READ_DEVICE_MODE (0x02)
 *
 * indexByte  - 0x00
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  deviceMode - Operating mode of the MAX32664, 0 on a failure (check status byte flag!)
 */
uint8_t readDeviceMode(uint8_t *statusByte){ //read the sensor hub status

    uint8_t deviceMode = I2CReadByte(READ_DEVICE_MODE, 0x00, statusByte); //read the device mode

    return deviceMode;
}


/**
 * @brief       Sets the operating mode for the MAX32664. Alternative method to setting operating mode (to external pin method)
 *
 * familyByte  - SET_DEVICE_MODE (0x01)
 *
 * indexByte   - 0x00
 *
 * writeByte0  - none
 *
 * writeByteN  - none
 *
 * @param   operatingMode Desired operating mode to be written out. 0x00: exit bootloader, enter application mode, 0x02: reset, 0x08: enter bootloader
 * @param   *statusByte   Pointer to the status byte
 *
 * @return  deviceMode - The current operating mode, should be equal to what it was set to. ERR_UNKNOWN on transaction issue or not equal to what it's set to (check status byte!)
 */
uint8_t setDeviceMode(uint8_t operatingMode, uint8_t *statusByte){

    if(operatingMode != EXIT_BOOTLOADER && operatingMode != RESET && operatingMode != ENTER_BOOTLOADER){ //if not passed a valid device mode
        *statusByte = INCORR_PARAM;
        return INCORR_PARAM; //return incorrect parameter value
    }

    uint8_t setModeStatus = I2CWriteByte(SET_DEVICE_MODE, 0x00, operatingMode); //write the correct device mode

    if(setModeStatus){ //if there was an error in setting the device mode
        *statusByte = setModeStatus; //set the status byte to the error byte
        return ERR_UNKNOWN; //return error
    }

    //Now check what mode we are in
    uint8_t deviceMode = I2CReadByte(READ_DEVICE_MODE, 0x00, statusByte); //read the device mode

    if(deviceMode != operatingMode){ //if we're not in the mode we set it too...
        return ERR_UNKNOWN; //return error
    }
    else{
        return deviceMode; //return the current operating mode
    }
}


/**
 * @brief   Reads the current sensor hub status. Includes sensor communication status, current FIFO threshold,
 *          FIFO input overflow, FIFO output overflow, and host accelerometer underflow info
 *
 * familyByte - HUB_STATUS (0x00)
 *
 * indexByte  - 0x00
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  status - sensor hub status (see Table 7 in MAX32664 User's Guide for bitfield details (https://pdfserv.maximintegrated.com/en/an/user-guide-6806-max32664.pdf), 0 on I2C failure (check status byte!)
 */
uint8_t readSensorHubStatus(uint8_t *statusByte){ //read the sensor hub status

    uint8_t status = I2CReadByte(HUB_STATUS, 0x00, statusByte); //sensor hub status read

    return status;
}


/**
 * @brief   Reads the number of samples available in the output FIFO
 *
 * familyByte - READ_ALGORITHM_CONFIG (0x51)
 *
 * indexByte  - READ_AGC_NUM_SAMPLES (0x00)
 *
 * writeByte0 - READ_AGC_NUM_SAMPLES_ID (0x03)
 *
 * writeByteN - none
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  samples - Number of samples available in output FIFO, 0 on a failure
 */
uint8_t readAlgoSamples(uint8_t *statusByte){ //read the number of samples averaged to get a reading

    uint8_t samples = I2CReadBytewithWriteByte(READ_ALGORITHM_CONFIG, READ_AGC_NUM_SAMPLES, READ_AGC_NUM_SAMPLES_ID, statusByte);

    return samples;
}


/**
 * @brief   Reads the percent of full scale ADC range that the AGC algorithm is using
 *
 * familyByte - READ_ALGORITHM_CONFIG (0x51)
 *
 * indexByte  - READ_AGC_PERCENTAGE (0x00)
 *
 * writeByte0 - READ_AGC_PERC_ID (0x00)
 *
 * writeByteN - none
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  range - Percent of the full scale ADC range that the AGC algorithm is using, 0 on an error
 */
uint8_t readAlgoRange(uint8_t *statusByte){

    uint8_t range = I2CReadBytewithWriteByte(READ_ALGORITHM_CONFIG, READ_AGC_PERCENTAGE, READ_AGC_PERC_ID, statusByte);

    return range;
}


/**
 * @brief   Reads the step size towards the target for the AGC algorithm
 *
 * familyByte - READ_ALGORITHM_CONFIG (0x51)
 *
 * indexByte  - READ_AGC_STEP_SIZE (0x00)
 *
 * writeByte0 - READ_AGC_STEP_SIZE_ID (0x01)
 *
 * writeByteN - none
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  stepSize - step size towards the target for the AGC algorithm, 0 on a failure
 */
uint8_t readAlgoStepSize(uint8_t *statusByte){

    uint8_t stepSize = I2CReadBytewithWriteByte(READ_ALGORITHM_CONFIG, READ_AGC_STEP_SIZE, READ_AGC_STEP_SIZE_ID, statusByte);

    return stepSize;
}


/**
 * @brief   Reads the sensitivity of the AGC algorithm
 *
 * familyByte - READ_ALGORITHM_CONFIG (0x51)
 *
 * indexByte  - READ_AGC_SENSITIVITY (0x00)
 *
 * writeByte0 - READ_AGC_SENSITIVITY_ID (0x02)
 *
 * writeByteN - none
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  algoSens - Sensitivity of AGC algorithm, 0 on a failure
 */
uint8_t readAlgoSensitivity(uint8_t *statusByte){

    uint8_t algoSens = I2CReadBytewithWriteByte(READ_ALGORITHM_CONFIG, READ_AGC_SENSITIVITY, READ_AGC_SENSITIVITY_ID, statusByte);

    return algoSens;
}


/**
 * @brief   Reads the WHRM algorithm sample rate
 *
 * familyByte - READ_ALGORITHM_CONFIG (0x51)
 *
 * indexByte  - READ_MAX_FAST_RATE (0x02)
 *
 * writeByte0 - READ_MAX_FAST_RATE_ID (0x00)
 *
 * writeByteN - none
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  algoSampleRate - Sample rate of WHRM algorithm, 0s on a failure
 */
uint16_t readAlgoSampleRate(uint8_t *statusByte){

    uint16_t algoSampleRate = I2CReadIntWithWriteByte(READ_ALGORITHM_CONFIG, READ_MAX_FAST_RATE, READ_MAX_FAST_RATE_ID, statusByte);

    if(*statusByte){
        algoSampleRate = 0;
    }

    return algoSampleRate;
}

/**
 * @brief   Reads the 3 Maxim Fast algorithm coefficients, which are 32-bit signed values * 100,000
 *
 * familyByte  - READ_ALGORITHM_CONFIG (0x51)
 *
 * indexByte   - READ_MAX_FAST_COEF (0x02)
 *
 * writeByte0  - READ_MAX_FAST_COEF_ID (0x0B)
 *
 * @param   *coefArray Pointer to array to fill with the 3 coefficients. Needs to be 32-bit
 *
 * @return statusByte - Status of I2C transaction
 */
uint8_t readMaximFastCoef(int32_t *coefArray){
    uint8_t numOfReads = NUM_MAXIM_FAST_COEF;

    uint8_t statusByte = I2CReadMultiple32BitValues(READ_ALGORITHM_CONFIG, READ_MAX_FAST_COEF, READ_MAX_FAST_COEF_ID, numOfReads, coefArray);

    return statusByte; //return the status byte
}


/**
 * @brief   Reads the current version of the sensor hub. Format of version is
 *          major.minor.revision
 *
 * familyByte - IDENTITY (0xFF)
 *
 * indexByte  - READ_SENSOR_HUB_VERS (0x03)
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @see version
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  sensorHubVer - Struct of version info, all 0s on a failure (check status byte!)
 */
struct version readSensorHubVersion(uint8_t *statusByte){
    struct version sensorHubVers; //struct for version data
    uint8_t versionArray[3]; //array for read data
    uint8_t readStatus = I2CReadFillArray(IDENTITY, READ_SENSOR_HUB_VERS, 3, versionArray); //perform I2C transaction, reading into array

    if(readStatus != SUCCESS){ //if we get a non-zero response (NOT a success)
        sensorHubVers.major = 0;
        sensorHubVers.minor = 0;
        sensorHubVers.revision = 0;
        *statusByte = readStatus; //save the status byte value
        return sensorHubVers; //return zeros for all fields
    }


    sensorHubVers.major = versionArray[0]; //get sensor hub major version number
    sensorHubVers.minor = versionArray[1]; //get sensor hub minor version number
    sensorHubVers.revision = versionArray[2]; //get sensor hub revision number
    *statusByte = SUCCESS; //successful transaction, so set status byte
    return sensorHubVers; //return the version data
}


/**
 * @brief   Reads the current version of the algorithm. Format of version is
 *          major.minor.revision
 *
 * familyByte - IDENTITY (0xFF)
 *
 * indexByte  - READ_ALGO_VERS (0x07)
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @see version
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  algoVers - Struct of version info, all 0s on failure
 */
struct version readAlgorithmVersion(uint8_t *statusByte){
    struct version algoVers; //struct for version data
    uint8_t versionArray[3]; //array for read data
    uint8_t readStatus = I2CReadFillArray(IDENTITY, READ_ALGO_VERS, 3, versionArray); //perform I2C transaction, reading into array

    if(readStatus){ //if we get a non-zero response (NOT a success)
        algoVers.major = 0;
        algoVers.minor = 0;
        algoVers.revision = 0;
        *statusByte = readStatus; //save status byte
        return algoVers; //return zeros for all fields
    }


    algoVers.major = versionArray[0]; //get major algorithm version number
    algoVers.minor = versionArray[1]; //get minor algorithm version number
    algoVers.revision = versionArray[2]; //get algorithm revision number
    *statusByte = SUCCESS; //successful transaction, so set status byte appropriately
    return algoVers;
}


/**
 * @brief   Reads the current version of the bootloader. Format of version is
 *          major.minor.revision
 *
 * familyByte - BOOTLOADER_INFO (0x81)
 *
 * indexByte  - BOOTLOADER_VERS (0x00)
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @see version
 *
 * @param   *statusByte pointer to status byte
 *
 * @return  bootVers - Struct of version info, 0s on a failure
 */
struct version readBootloaderVersion(uint8_t *statusByte){
    struct version bootVers; //struct for version data
    uint8_t versionArray[3]; //array for read data
    uint8_t readStatus = I2CReadFillArray(BOOTLOADER_INFO, BOOTLOADER_VERS, 3, versionArray); //perform I2C transaction, reading into array

    if(readStatus){ //if we get a non-zero response (NOT a success)
        bootVers.major = 0;
        bootVers.minor = 0;
        bootVers.revision = 0;
        *statusByte = readStatus;
        return bootVers; //return zeros for all fields
    }


    bootVers.major = versionArray[0]; //get major algorithm version number
    bootVers.minor = versionArray[1]; //get minor algorithm version number
    bootVers.revision = versionArray[2]; //get algorithm revision number
    *statusByte = SUCCESS;
    return bootVers;
}


/**
 * @brief   Reads the MCU type of biometric sensor hub (expect MAX32660/MAX32664)
 *
 * familyByte - IDENTITY (0xFF)
 *
 * indexByte  - READ_MCU_TYPE (0x00)
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  mcuType - Type of MCU
 */
uint8_t getMcuType(uint8_t *statusByte){

    uint8_t mcuType = I2CReadByte(IDENTITY, READ_MCU_TYPE, statusByte); //read the mcu Type

    if(mcuType != SUCCESS && mcuType != 0x01){ //if we don't get a valid MCU type
        return ERR_UNKNOWN; //return error message
    }

    return mcuType; //return the MCU type (0x00 = MAX32625, 0x01 = MAX32660/MAX32664)
}


/**
 * @brief   Reads the ADC sample rate of the MAX30101 internal ADC
 *
 * MAX30101 Register - CONFIGURATION_REGISTER (0x0A)
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  Sample rate of MAX30101 internal ADC in Hertz, ERR_UNKNOWN on a issue reading register or invalid number read
 */
uint16_t  readADCSampleRate(uint8_t *statusByte){

    uint8_t regVal;

    regVal = readRegisterMAX30101(CONFIGURATION_REGISTER, statusByte); //get the current Configuration Register value
    regVal &= READ_SAMP_MASK; //need to mask to get just the ADC sample rates bits
    regVal = (regVal >> 2); //shift right 2 to align 0th bit of ADC rate with 0th bit of vale

    if(*statusByte){ //if we had an error reading the registers
        return ERR_UNKNOWN; //return the error unknown byte
    }

    //return appropriate sampling rate (Table 6: SpO2 Sample Rate Control in MAX30101 datasheet)
    if      (regVal == 0) return 50;
    else if (regVal == 1) return 100;
    else if (regVal == 2) return 200;
    else if (regVal == 3) return 400;
    else if (regVal == 4) return 800;
    else if (regVal == 5) return 1000;
    else if (regVal == 6) return 1600;
    else if (regVal == 7) return 3200;
    else return ERR_UNKNOWN;
}


/**
 * @brief   Reads the internal ADC range of the MAX30101
 *
 * MAX30101 Register - CONFIGURATION_REGISTER (0x0A)
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  ADC full scale range of MAX30101, ERR_UNKNOWN if invalid number
 */
uint16_t  readADCRange(uint8_t *statusByte){

    uint8_t regVal;

    regVal = readRegisterMAX30101(CONFIGURATION_REGISTER, statusByte); //get the current Configuration Register value
    regVal &= READ_ADC_MASK; //need to mask to get just the ADC bits
    regVal = (regVal >> 5); //shift right 5 to align 0th bit of ADC range with 0th bit of vale

    if(*statusByte){ //if the status byte was non-zero (I2C transaction issues)
        return ERR_UNKNOWN;
    }

    //return appropriate ADC full scale range (Table 5:  SpO2 ADC Range Control in MAX30101 datasheet)
    if      (regVal == 0) return 2048;
    else if (regVal == 1) return 4096;
    else if (regVal == 2) return 8192;
    else if (regVal == 3) return 16384;
    else return ERR_UNKNOWN; //if unknown value, return error
}


/**
 * @brief   Reads the LED pulse width of the LEDs in the MAX30101
 *
 * MAX30101 Register - CONFIGURATION_REGISTER (0x0A)
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  LED pulse width of MAX30101 in us, ERR_UNKNOWN for invalid read value or I2C transaction (check status byte!)
 */
uint16_t  readPulseWidth(uint8_t *statusByte){

    uint8_t regVal;

    regVal = readRegisterMAX30101(CONFIGURATION_REGISTER, statusByte);
    regVal &= READ_PULSE_MASK;

    if(*statusByte){ //if there was an I2C transaction issue
        return ERR_UNKNOWN; //return error
    }

    if      (regVal == 0) return 69;
    else if (regVal == 1) return 118;
    else if (regVal == 2) return 215;
    else if (regVal == 3) return 411;
    else return ERR_UNKNOWN; //if unknown value, return error
}


/**
 * @brief   Reads the LED pulse amplitude of the LEDs in the MAX30101
 *
 * MAX30101 Register - LEDX_REGISTER (0x0C <-> 0x0F)
 *
 * @param   *ledArray Array that the LED data is put into
 * @param   *statusByte Pointer to status byte
 *
 * @return  SUCCESS on successful sequence of I2C transactions, ERR_UNKNOWN when there's an issue (refer to status byte!)
 */
uint8_t  readPulseAmp(uint8_t *ledArray, uint8_t *statusByte){
    uint8_t regVal;
    uint8_t success = 0x00;

    regVal = readRegisterMAX30101(LED1_REGISTER, statusByte); //get the current LED1 Register value

    if(*statusByte){ //if there was an I2C communication issue
        ledArray[0] = 0;
        return ERR_UNKNOWN; //return error byte
    }
    else{
        ledArray[0] = regVal; //save it to the array we want
        success |= regVal; //or the value with success register
    }

    regVal = readRegisterMAX30101(LED2_REGISTER, statusByte); //get the current LED2 Register value

    if(*statusByte){ //if there was an I2C communication issue
        ledArray[1] = 0;
        return ERR_UNKNOWN; //error byte
    }
    else{
        ledArray[1] = regVal; //save it to the array we want
        success |= regVal; //or the value with success register
    }

    regVal = readRegisterMAX30101(LED3_REGISTER, statusByte); //get the current LED3 Register value

    if(*statusByte){ //if there was an I2C communication issue
        ledArray[2] = 0;
        return ERR_UNKNOWN; //error byte
    }
    else{
        ledArray[2] = regVal; //save it to the array we want
        success |= regVal; //or the value with success register
    }

    regVal = readRegisterMAX30101(LED4_REGISTER, statusByte); //get the current LED4 Register value

    if(*statusByte){ //if there was an I2C communication issue
        ledArray[3] = 0;
        return ERR_UNKNOWN; //error byte
    }
    else{
        ledArray[3] = regVal; //save it to the array we want
        success |= regVal; //or the value with success register
    }

    if(success){ //if we had non-zero numbers in any of the amplitudes (SUCCESS)
        return SUCCESS;
    }
    else{
        return ERR_UNKNOWN;
    }

}


/**
 * @brief   Reads the operating mode of the MAX30101 PO sensor. Results indicate which LEDs are being used
 *
 * MAX30101 Register - MODE_REGISTER (0x09)
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  Operating mode of the MAX30101 (which LEDs are being used), ERR_UNKNOWN on invalid return value or I2C transaciton issue
 */
uint8_t readMAX30101Mode(uint8_t *statusByte){

    uint8_t regVal;

    regVal = readRegisterMAX30101(MODE_REGISTER, statusByte); //get the current Mode Configuration Register value
    regVal &= READ_MODE_MASK; //need to mask to get just the mode configuration bits

    if(*statusByte){ //if had an I2C transaction issue
        return ERR_UNKNOWN; //return error byte
    }

    //return appropriate operating mode  (Table 4: Mode Control in MAX30101 datasheet)
    //need to process this value after it's called, since returned value indicates a specific combination of LEDs being used
    if      (regVal == 2) return 2;
    else if (regVal == 3) return 3;
    else if (regVal == 7) return 7;
    else return ERR_UNKNOWN; //if there was unknown value, return an error
}

/**
 * @brief   Gets the attributes for the MAX30101 sensor. Includes how many bytes to a word and number of registers
 *
 * familyByte  - READ_ATTRIBUTES_AFE (0x42)
 *
 * indexByte   - RETRIEVE_AFE_MAX30101 (0x03)
 *
 * writeByte0  - none
 *
 * writeByteN  - none
 *
 * @see sensorAttr
 *
 * @param *statusByte Pointer to status byte
 *
 * @return maxAttr - Struct of sensor attributes
 */
struct sensorAttr getAfeAttributesMAX30101(uint8_t *statusByte){

    struct sensorAttr maxAttr;
    uint8_t tempArray[2] = {0, 0};
    uint8_t status = 0;

    status = I2CReadFillArray(READ_ATTRIBUTES_AFE, RETRIEVE_AFE_MAX30101, 2, tempArray);

    if(status != SUCCESS){ //if we had an I2C transaction error
        maxAttr.byteWord = 0;
        maxAttr.availRegisters = 0;
        *statusByte = status; //save the status byte
        return maxAttr; //return the sensor attributes
    }

    maxAttr.byteWord = tempArray[0]; //save the number of bytes per word
    maxAttr.availRegisters = tempArray[1]; //save the number of registers
    *statusByte = status; //save the status byte
    return maxAttr; //return the sensor attributes
}


/**
 * @brief   Gets the attributes for the accelerometer. Includes how many bytes to a word and number of registers
 *
 * familyByte  - READ_ATTRIBUTES_AFE (0x42)
 *
 * indexByte   - RETRIEVE_AFE_ACCELEROMETER (0x04)
 *
 * writeByte0  - none
 *
 * writeByteN  - none
 *
 * @see sensorAttr
 *
 * @param *statusByte Pointer to status byte
 *
 * @return accelAttr - Struct of sensor attributes, 0 on a failure
 */
struct sensorAttr getAfeAttributesAccelerometer(uint8_t *statusByte){

    struct sensorAttr accelAttr;
    uint8_t tempArray[2] = {0, 0};

    *statusByte = I2CReadFillArray(READ_ATTRIBUTES_AFE, RETRIEVE_AFE_ACCELEROMETER, 2, tempArray);

    if(*statusByte != SUCCESS){ //if we had an I2C transaction error
        accelAttr.byteWord = 0;
        accelAttr.availRegisters = 0;
        return accelAttr; //return the sensor attributes
    }

    accelAttr.byteWord = tempArray[0]; //save the number of bytes per word
    accelAttr.availRegisters = tempArray[1]; //save the number of registers
    return accelAttr; //return the sensor attributes
}


/**
 * @brief   Gets the current mode of the external accelerometer. Tells info about sensor hub accelerometer or host accelerometer
 *
 * familyByte  - READ_SENSOR_MODE (0x45)
 *
 * indexByte   - READ_ENABLE_ACCELEROMETER (0x04)
 *
 * writeByte0  - none
 *
 * writeByteN  - none
 *
 * @param *statusByte Pointer to status byte
 *
 * @return accelMode - Mode of the accelerometer. 0: sensor hub accel disabled, 1: external host accel disabled, 2: sensor hub accel enabled, 3: external host accel enabled, 255 (0xFF) on an invalid value
 */
uint8_t getExtAccelMode(uint8_t *statusByte){

    uint8_t modeArray[2] = {0, 0}; //array for the two bytes we're reading for accel info

    *statusByte = I2CReadFillArray(READ_SENSOR_MODE, READ_ENABLE_ACCELEROMETER, 2, modeArray); //read the two bytes corresponding to the external accelerometer mode

    if     (modeArray[0] == 0 && modeArray[1] == 0) return 0; //return 0 (bit "equivalent")
    else if(modeArray[0] == 0 && modeArray[1] == 1) return 1; //return 1 (bit "equivalent")
    else if(modeArray[0] == 1 && modeArray[1] == 0) return 2; //return 2 (bit "equivalent")
    else if(modeArray[0] == 1 && modeArray[1] == 1) return 3; //return 3 (bit "equivalent")
    else return ERR_UNKNOWN; //else we got a combo that doesn't make sense, so return an error
}


/**
 * @brief   Reads the value in specific register in the MAX30101. Passed parameter selects the specific register
 *
 * familyByte - READ_REGISTER (0x41)
 *
 * indexByte  - READ_MAX30101 (0x03)
 *
 * writeByte0 - regAddr
 *
 * writeByteN - none
 *
 * @param   regAddr Register address in the MAX30101
 * @param   *statusByte Pointer to statusByte
 *
 * @return  regCont - Register value, 0 on failure (check status byte!)
 */
uint8_t  readRegisterMAX30101(uint8_t regAddr, uint8_t *statusByte){

    uint8_t regCont = I2CReadBytewithWriteByte(READ_REGISTER, READ_MAX30101, regAddr, statusByte);

    return regCont; //return the read register value
}


/**
 * @brief   Writes the given value to the given register address in the MAX30101. Returns status of I2C transaction
 *
 * familyByte - WRITE_REGISTER (0x40)
 *
 * indexByte  - WRITE_MAX30101 (0x03)
 *
 * writeByte0 - regAddr
 *
 * writeByteN - regVal
 *
 * @param regAddr Address of the register we want to write to in the MAX30101
 * @param regVal  Value you want to write into that register
 *
 * @return  status - Status byte of I2C transaction
 */
uint8_t writeRegisterMAX30101(uint8_t regAddr, uint8_t regVal){

    uint8_t status = I2CWrite2Bytes(WRITE_REGISTER, WRITE_MAX30101, regAddr, regVal);

    return status; //return the status of the I2C transaction
}



///////////////////////////////////////////////////////////////////

/*
 * Low level I2C transaction functions, called by all higher lever functions
 * Overall I2C transaction requires a write AND read I2C transaction with a small delay between
 */


/**
 * @brief   Does an I2C read transaction with the MAX32664 that will read a single byte
 *
 * familyByte - familyByte
 *
 * indexByte  - indexByte
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @param   familyByte Desired family byte for I2C transaction
 * @param   indexByte  Desired index byte for I2C transaction
 * @param   *statusByte Pointer to status byte of transaction
 *
 * @return  localRxBuffer[1] - Read data byte
 */
uint8_t I2CReadByte(uint8_t familyByte, uint8_t indexByte, uint8_t *statusByte){

    uint8_t localTxBuffer[2]; //Family Byte, Index Byte
    uint8_t localRxBuffer[2]; //Read Status Byte, Read Byte 0
    localTxBuffer[0] = familyByte; //set the family byte
    localTxBuffer[1] = indexByte; //set the index byte
    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 2;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 0;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C write did not work
        *statusByte = ERR_UNKNOWN; //set the status byte to be an error
        return 0; //return a read byte of 0
    }

    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds

    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 0;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 2;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C read did not work
        *statusByte = ERR_UNKNOWN; //set the status byte to be an error
        return 0; //return a read byte of 0
    }

    *statusByte = localRxBuffer[0]; //save the status byte

    if(localRxBuffer[0] != SUCCESS){ //if we had an I2C transaction error
        return 0; //return 0
    }
    else{
        return localRxBuffer[1]; //else return the read byte
    }
}


/**
 * @brief   Does an I2C read transaction with the MAX32664 that requires a single write byte, that will read a single byte
 *
 * familyByte - familyByte
 *
 * indexByte  - indexByte
 *
 * writeByte0 - dataByte
 *
 * writeByteN - none
 *
 * @param   familyByte Desired family byte for I2C transaction
 * @param   indexByte  Desired index byte for I2C transaction
 * @param   dataByte   Write data byte for I2C transaction
 * @param   *statusByte Pointer to the status byte
 *
 * @return  localRxBuffer[1] - Read data byte
 */
uint8_t I2CReadBytewithWriteByte(uint8_t familyByte, uint8_t indexByte, uint8_t dataByte, uint8_t *statusByte){

    uint8_t localTxBuffer[3]; //Family Byte, Index Byte, data Byte
    uint8_t localRxBuffer[2]; //Read Status Byte, Read Byte 0
    localTxBuffer[0] = familyByte; //set the family byte
    localTxBuffer[1] = indexByte; //set the index byte
    localTxBuffer[2] = dataByte; //set the data byte
    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 3;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 0;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C write did not work
        *statusByte = ERR_UNKNOWN; //set the status byte to be an error
        return 0; //return a read byte of 0
    }

    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds

    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 0;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 2;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C read did not work
        *statusByte = ERR_UNKNOWN; //set the status byte to be an error
        return 0; //return a read byte of 0
    }

    *statusByte = localRxBuffer[0]; //save the returned status byte from I2C transaction to the pointer

    if(localRxBuffer[0] != SUCCESS){ //if there was an I2C transaction error
        return 0; //return 0
    }
    else{
        return localRxBuffer[1]; //else return read byte 0
    }
}


/**
 * @brief   Does an I2C read transaction with the MAX32664 that will read an array of data back,
 *
 * familyByte - familyByte
 *
 * indexByte  - indexByte
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @param   familyByte  Desired family byte for I2C transaction
 * @param   indexByte   Desired index byte for I2C transaction
 * @param   arraySize   Size of expected read array
 * @param   *arraytoFill Pointer to array to fill
 *
 * @return  localRxBuffer[0] - Status byte of I2C transaction
 */
uint8_t I2CReadFillArray(uint8_t familyByte, uint8_t indexByte, uint8_t arraySize, uint8_t *arraytoFill){

    uint8_t arrayCount = 0;
    uint8_t localTxBuffer[2]; //Family Byte, Index Byte
    uint8_t localRxBuffer[1 + MAX30101_LED_ARRAY + MAXFAST_ARRAY_SIZE + MAXFAST_EXTENDED_DATA]; //Read Status Byte, data bytes (+12 for MAX30101 LED array, +6 for standard algorithm output, +5 for extended algorithm output)
    localTxBuffer[0] = familyByte; //set the family byte
    localTxBuffer[1] = indexByte; //set the index byte
    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 2;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 0;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C write did not work
        for(arrayCount = 0; arrayCount < arraySize; arrayCount++){ //for the full array
            arraytoFill[arrayCount] = 0; //set all values to 0
        }
        return ERR_UNKNOWN; //return and error statusy byte
    }

    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds

    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 0;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = I2_READ_STATUS_BYTE_COUNT + arraySize;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C read did not work
        for(arrayCount = 0; arrayCount < arraySize; arrayCount++){ //for the full array
            arraytoFill[arrayCount] = 0; //set all values to 0
        }
        return ERR_UNKNOWN; //return and error statusy byte
    }

    if(localRxBuffer[0]){ //if the status byte is non-zero
        for(arrayCount = 0; arrayCount < arraySize; arrayCount++){
            arraytoFill[arrayCount] = 0; //set all values to 0
        }
    }
    else{
        for(arrayCount = 0; arrayCount < arraySize; arrayCount++){
            arraytoFill[arrayCount] = localRxBuffer[I2_READ_STATUS_BYTE_COUNT + arrayCount];
        }
    }

    return localRxBuffer[0]; //return status byte
}


/**
 * @brief   Performs a read transaction with no write bytes. Will perform an I2C transaction and translate result into a 16 bit value
 *
 * familyByte - familyByte
 *
 * indexByte  - indexByte
 *
 * writeByte0 - none
 *
 * writeByteN - none
 *
 * @param   familyByte  Desired family byte for I2C transaction
 * @param   indexByte   Desired index byte for I2C transaction
 * @param   *statusByte Pointer to status byte
 *
 * @return  returnInt - 16-bit unsigned read data, 0 on a failure
 */
uint16_t I2CReadInt(uint8_t familyByte, uint8_t indexByte, uint8_t *statusByte){

    uint8_t localTxBuffer[2]; //Family Byte, Index Byte
    uint8_t localRxBuffer[3]; //Read Status Byte, data bytes (2)
    localTxBuffer[0] = familyByte; //set the family byte
    localTxBuffer[1] = indexByte; //set the index byte
    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 2;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 0;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C write did not work
        *statusByte = ERR_UNKNOWN; //set the status byte to be an error
        return 0; //return a read byte of 0
    }

    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds

    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 0;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 3;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C read did not work
        *statusByte = ERR_UNKNOWN; //set the status byte to be an error
        return 0; //return a read byte of 0
    }

    *statusByte = localRxBuffer[0]; //save the status byte

    uint16_t returnInt = 0;

    if(localRxBuffer[0]){ //if we had an I2C transaction error
        returnInt = 0; //return 0
    }
    else{ //else
        returnInt = ((uint16_t)localRxBuffer[1]) << 8; //shift MSB into proper location
        returnInt |= (uint16_t)localRxBuffer[2]; //bitwise OR LSB with shifted MSB to get proper 16-bit number
    }

    return returnInt; //return the value we wanted
}


/**
 * @brief   Performs a read transaction with a single write byte. Will perform an I2C transaction and translate result into a 16 bit value
 *
 * familyByte - familyByte
 *
 * indexByte  - indexByte
 *
 * writeByte0 - writeByte
 *
 * writeByteN - none
 *
 * @param   familyByte  Desired family byte for I2C transaction
 * @param   indexByte   Desired index byte for I2C transaction
 * @param   writeByte   Desired write byte for I2C transaction
 * @param   *statusByte Pointer to status byte
 *
 * @return  returnInt - 16-bit read data, 0 on a failure
 */
uint16_t I2CReadIntWithWriteByte(uint8_t familyByte, uint8_t indexByte, uint8_t writeByte, uint8_t *statusByte){

    uint8_t localTxBuffer[3]; //Family Byte, Index Byte, Write Byte 0
    uint8_t localRxBuffer[3]; //Read Status Byte, data bytes (2)
    localTxBuffer[0] = familyByte; //set the family byte
    localTxBuffer[1] = indexByte; //set the index byte
    localTxBuffer[2] = writeByte; //set the write byte 0
    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 3;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 0;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C write did not work
        *statusByte = ERR_UNKNOWN; //set the status byte to be an error
        return 0; //return a read byte of 0
    }

    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds

    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 0;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 3;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C read did not work
        *statusByte = ERR_UNKNOWN; //set the status byte to be an error
        return 0; //return a read byte of 0
    }

    *statusByte = localRxBuffer[0]; //save the read status byte of I2C transaction to the pointer
    uint16_t returnInt = 0;

    if(localRxBuffer[0]){ //if we had an I2C transaction error
        returnInt = 0; //set it to 0
    }
    else{
        returnInt = ((uint16_t)localRxBuffer[1]) << 8; //shift MSB into proper location
        returnInt |= (uint16_t)localRxBuffer[2]; //bitwise OR LSB with shifted MSB to get proper 16-bit number
    }

    return returnInt; //return our read Int
}


/**
 * @brief Does an I2C read transaction with the MAX32664 that reads a single 32-bit variable
 *
 * familyByte  - familyByte
 *
 * indexByte   - indexByte
 *
 * writeByte0  - writeBtye0
 *
 * writeByteN  - none
 *
 * @param   familyByte  Desired family byte for I2C transaction
 * @param   indexByte   Desired index byte for I2C transaction
 * @param   dataByte    Desired write byte 0 for I2C transaction
 * @param   *statusByte Pointer to status byte
 *
 * @return value - 32-bit signed value
 */
int32_t I2CRead32BitValue(uint8_t familyByte, uint8_t indexByte, uint8_t dataByte, uint8_t *statusByte){

    int32_t value = 0;
    uint8_t localTxBuffer[3]; //Family Byte, Index Byte, Write Byte 0
    uint8_t localRxBuffer[I2_READ_STATUS_BYTE_COUNT + sizeof(int32_t)]; //Read Status Byte, 32-bit value (4-bytes)
    localTxBuffer[0] = familyByte; //set the family byte
    localTxBuffer[1] = indexByte; //set the index byte
    localTxBuffer[2] = dataByte; //set the index byte
    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 3;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 0;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C write did not work
        *statusByte = ERR_UNKNOWN; //set the status byte to be an error
        return 0; //return a read byte of 0
    }

    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds

    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 0;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = I2_READ_STATUS_BYTE_COUNT + sizeof(int32_t);

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C read did not work
        *statusByte = ERR_UNKNOWN; //set the status byte to be an error
        return 0; //return a read byte of 0
    }
    *statusByte = localRxBuffer[0]; //save status byte

    if(localRxBuffer[0]){ //if we had a transaction error
        value = 0; //set it to 0
    }
    else{
        value = ((int32_t)localRxBuffer[I2_READ_STATUS_BYTE_COUNT]) << 24; //get the appropriate index and shift
        value |= ((int32_t)localRxBuffer[I2_READ_STATUS_BYTE_COUNT + 1]) << 16; //get the appropriate index and shift
        value |= ((int32_t)localRxBuffer[I2_READ_STATUS_BYTE_COUNT + 2])  << 8; //get the appropriate index and shift
        value |= (int32_t)localRxBuffer[I2_READ_STATUS_BYTE_COUNT + 3];
    }

    return value;
}


/**
 * @brief Does an I2C read transaction with the MAX32664 that reads multiple 32-bit variables
 *
 * familyByte  - familyByte
 *
 * indexByte   - indexByte
 *
 * writeByte0  - dataByte
 *
 * writeByteN  - none
 *
 * @param   familyByte  Desired family byte for I2C transaction
 * @param   indexByte   Desired index byte for I2C transaction
 * @param   dataByte    Desired write byte 0 for I2C transaction
 * @param   numReads    Number of 32-bit signed values you want to read
 * @param   *numArray    Pointer to array to be filled. Must be a 32-bit array and be the same size as numReads
 *
 * @return localRxBuffer[0] - Status byte of I2C transaction
 */
uint8_t I2CReadMultiple32BitValues(uint8_t familyByte, uint8_t indexByte, uint8_t dataByte, uint8_t numReads, int32_t *numArray){

    uint8_t arrayCount = 0;
    uint8_t localTxBuffer[3]; //Family Byte, Index Byte, Write Byte 0
    uint8_t localRxBuffer[I2_READ_STATUS_BYTE_COUNT + sizeof(int32_t) * 3]; //Read Status Byte, data bytes (4-bytes * 3)
    localTxBuffer[0] = familyByte; //set the family byte
    localTxBuffer[1] = indexByte; //set the index byte
    localTxBuffer[2] = dataByte; //set the index byte
    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 3;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 0;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C write did not work
        for(arrayCount = 0; arrayCount < numReads; arrayCount++){ //for everything in the array
            numArray[arrayCount] = 0; //set it to zero
        }
        return ERR_UNKNOWN; //return an error status byte
    }

    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds

    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 0;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = I2_READ_STATUS_BYTE_COUNT + sizeof(int32_t) * numReads;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C read did not work
        for(arrayCount = 0; arrayCount < numReads; arrayCount++){ //for everything in the array
            numArray[arrayCount] = 0; //set it to zero
        }
        return ERR_UNKNOWN; //return an error status byte
    }

    if(localRxBuffer[0]){ //if the status byte is non-zero
        for(arrayCount = 0; arrayCount < numReads; arrayCount++){ //for everything in the array
            numArray[arrayCount] = 0; //set it to zero
        }
    }
    else{ //else status byte was zero, so process the data we received
        for(arrayCount = 0; arrayCount < numReads; arrayCount++){ //for all expected 32-bit values
            //take the appropriate 8-bit value, shift it, and bitwise OR it with the array location
            numArray[arrayCount] = ((int32_t)localRxBuffer[I2_READ_STATUS_BYTE_COUNT + arrayCount * 4]) << 24; //get the appropriate index and shift
            numArray[arrayCount] |= ((int32_t)localRxBuffer[I2_READ_STATUS_BYTE_COUNT + arrayCount * 4 + 1]) << 16; //get the appropriate index and shift
            numArray[arrayCount] |= ((int32_t)localRxBuffer[I2_READ_STATUS_BYTE_COUNT + arrayCount * 4 + 2])  << 8; //get the appropriate index and shift
            numArray[arrayCount] |= (int32_t)localRxBuffer[I2_READ_STATUS_BYTE_COUNT + arrayCount * 4 + 3];
        }
    }

    return localRxBuffer[0]; //return status byte from transaction

}

/**
 * @brief   Does an I2C write transaction with the MAX32664 that will write a single byte
 *
 * familyByte - familyByte
 *
 * indexByte  - indexByte
 *
 * writeByte0 - dataByte
 *
 * writeByteN - none
 *
 * @param   familyByte Desired family byte for I2C transaction
 * @param   indexByte  Desired index byte for I2C transaction
 * @param   dataByte   Write data byte for I2C transaction
 *
 * @return  localRxBuffer[0] - Status byte of I2C transaction
 */
uint8_t I2CWriteByte(uint8_t familyByte, uint8_t indexByte, uint8_t dataByte){

    uint8_t localTxBuffer[3];
    uint8_t localRxBuffer[1];
    localTxBuffer[0] = familyByte;
    localTxBuffer[1] = indexByte;
    localTxBuffer[2] = dataByte;
    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 3;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 0;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C write did not work
        return ERR_UNKNOWN; //return an error status byte
    }

    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds

    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 0;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 1; //expect a status byte

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C read did not work
        return ERR_UNKNOWN; //return a read byte of 0
    }

    return localRxBuffer[0]; //return the status byte
}


/**
 * @brief   Does an I2C write transaction with the MAX32664 that will write two bytes
 *
 * familyByte - familyByte
 *
 * indexByte  - indexByte
 *
 * writeByte0 - dataByte0
 *
 * writeByteN - dataByte1
 *
 * @param   familyByte Desired family byte for I2C transaction
 * @param   indexByte  Desired index byte for I2C transaction
 * @param   dataByte0   Write data byte 0 for I2C transaction
 * @param   dataByte1   Write data byte 1 for I2C transaction
 *
 * @return  localRxBuffer[0] - Status byte of I2C transaction */
uint8_t I2CWrite2Bytes(uint8_t familyByte, uint8_t indexByte, uint8_t dataByte0, uint8_t dataByte1){

    uint8_t localTxBuffer[4]; //family byte, index byte, data byte 0, data byte 1
    uint8_t localRxBuffer[1]; //status byte
    localTxBuffer[0] = familyByte;
    localTxBuffer[1] = indexByte;
    localTxBuffer[2] = dataByte0;
    localTxBuffer[3] = dataByte1;
    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 4;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 0;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C write did not work
        return ERR_UNKNOWN; //return an error status byte
    }

    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds

    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 0;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 1; //expect a status byte

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C read did not work
        return ERR_UNKNOWN; //return an error status byte
    }

    return localRxBuffer[0]; //return the status byte of the I2C transaction
}


/**
 * @brief   Does an I2C write transaction with the MAX32664 that will write a single byte,
 *          with a longer delay between write and read
 *
 * familyByte - familyByte
 *
 * indexByte  - indexByte
 *
 * writeByte0 - dataByte
 *
 * writeByteN - none
 *
 * @param   familyByte Desired family byte for I2C transaction
 * @param   indexByte  Desired index byte for I2C transaction
 * @param   dataByte   Write data byte for I2C transaction
 *
 * @return  localRxBuffer[0] - Status byte of I2C transaction
 */
uint8_t I2CenableWriteByte(uint8_t familyByte, uint8_t indexByte, uint8_t dataByte){

    uint8_t localTxBuffer[3];
    uint8_t localRxBuffer[1];
    localTxBuffer[0] = familyByte;
    localTxBuffer[1] = indexByte;
    localTxBuffer[2] = dataByte;
    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 3;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 0;

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C write did not work
        return ERR_UNKNOWN; //return an error status byte
    }

    usleep(ENABLE_CMD_DELAY * 1000); //sleep for 45 milliseconds

    gi2cTransaction.slaveAddress = BIO_ADDRESS;
    gi2cTransaction.writeBuf = localTxBuffer;
    gi2cTransaction.writeCount = 0;
    gi2cTransaction.readBuf = localRxBuffer;
    gi2cTransaction.readCount = 1; //expect a status byte

    if(!I2C_transfer(gi2cHandle, &gi2cTransaction)){ //if I2C write did not work
        return ERR_UNKNOWN; //return an error status byte
    }

    return localRxBuffer[0]; //return the status byte
}







///////////////////////////////////////////////////////////////////////////////////////////
//older write/read functions, rebuilt to make function calling easier

////Used to read a single data byte from the I2C device (assumed no Write Bytes
//char I2CReadByte(I2C_Transaction *i2cTrans, I2C_Handle *i2cHandle, uint8_t familyByte, uint8_t indexByte){
//
//    uint8_t localTxBuffer[2]; //Family Byte, Index Byte
//    uint8_t localRxBuffer[2]; //Read Status Byte, Read Byte 0
//    localTxBuffer[0] = familyByte; //set the family byte
//    localTxBuffer[1] = indexByte; //set the index byte
//    (*i2cTrans).slaveAddress = BIO_ADDRESS;
//    (*i2cTrans).writeBuf = localTxBuffer;
//    (*i2cTrans).writeCount = 2;
//    (*i2cTrans).readBuf = localRxBuffer;
//    (*i2cTrans).readCount = 0;
//
//    I2C_transfer(*i2cHandle, i2cTrans);
//
//    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds
//
//    (*i2cTrans).slaveAddress = BIO_ADDRESS;
//    (*i2cTrans).writeBuf = localTxBuffer;
//    (*i2cTrans).writeCount = 0;
//    (*i2cTrans).readBuf = localRxBuffer;
//    (*i2cTrans).readCount = 2;
//
//    I2C_transfer(*i2cHandle, i2cTrans);
//
//    if(localRxBuffer[0]){ //if the status byte is non-zero
//        return localRxBuffer[0]; //we had a read error, so return that
//    }
//
//    return localRxBuffer[1]; //return read byte 0
//}


////Used to read a single data byte from the I2C device with one write byte
//char I2CReadBytewithWriteByte(I2C_Transaction *i2cTrans, I2C_Handle *i2cHandle, uint8_t familyByte, uint8_t indexByte, uint8_t dataByte){
//
//    uint8_t localTxBuffer[3]; //Family Byte, Index Byte, data Byte
//    uint8_t localRxBuffer[2]; //Read Status Byte, Read Byte 0
//    localTxBuffer[0] = familyByte; //set the family byte
//    localTxBuffer[1] = indexByte; //set the index byte
//    localTxBuffer[2] = dataByte; //set the data byte
//    (*i2cTrans).slaveAddress = BIO_ADDRESS;
//    (*i2cTrans).writeBuf = localTxBuffer;
//    (*i2cTrans).writeCount = 3;
//    (*i2cTrans).readBuf = localRxBuffer;
//    (*i2cTrans).readCount = 0;
//
//    I2C_transfer(*i2cHandle, i2cTrans);
//
//    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds
//
//    (*i2cTrans).slaveAddress = BIO_ADDRESS;
//    (*i2cTrans).writeBuf = localTxBuffer;
//    (*i2cTrans).writeCount = 0;
//    (*i2cTrans).readBuf = localRxBuffer;
//    (*i2cTrans).readCount = 2;
//
//    I2C_transfer(*i2cHandle, i2cTrans);
//
//    if(localRxBuffer[0]){ //if the status byte is non-zero
//        return localRxBuffer[0]; //we had a read error, so return that
//    }
//
//    return localRxBuffer[1]; //return read byte 0
//}


//char I2CWriteByte(I2C_Transaction *i2cTrans, I2C_Handle *i2cHandle, uint8_t familyByte, uint8_t indexByte, uint8_t dataByte)
//{
//
//    uint8_t localTxBuffer[3];
//    uint8_t localRxBuffer[1];
//    localTxBuffer[0] = familyByte;
//    localTxBuffer[1] = indexByte;
//    localTxBuffer[2] = dataByte;
//    (*i2cTrans).slaveAddress = BIO_ADDRESS;
//    (*i2cTrans).writeBuf = localTxBuffer;
//    (*i2cTrans).writeCount = 3;
//    (*i2cTrans).readBuf = localRxBuffer;
//    (*i2cTrans).readCount = 0;
//
//    I2C_transfer(*i2cHandle, i2cTrans);
//
//    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds
//
//    (*i2cTrans).slaveAddress = BIO_ADDRESS;
//    (*i2cTrans).writeBuf = localTxBuffer;
//    (*i2cTrans).writeCount = 0;
//    (*i2cTrans).readBuf = localRxBuffer;
//    (*i2cTrans).readCount = 1; //expect a status byte
//
//    I2C_transfer(*i2cHandle, i2cTrans);
//
//    return localRxBuffer[0]; //return the status byte
//}

//enable write for enabling sensor, need extra long cmd delay
//char I2CenableWrite(I2C_Transaction *i2cTrans, I2C_Handle *i2cHandle, uint8_t familyByte, uint8_t indexByte, uint8_t dataByte)
//{
//
//    uint8_t localTxBuffer[3];
//    uint8_t localRxBuffer[1];
//    localTxBuffer[0] = familyByte;
//    localTxBuffer[1] = indexByte;
//    localTxBuffer[2] = dataByte;
//    (*i2cTrans).slaveAddress = BIO_ADDRESS;
//    (*i2cTrans).writeBuf = localTxBuffer;
//    (*i2cTrans).writeCount = 3;
//    (*i2cTrans).readBuf = localRxBuffer;
//    (*i2cTrans).readCount = 0;
//
//    I2C_transfer(*i2cHandle, i2cTrans);
//
//    usleep(ENABLE_CMD_DELAY * 1000); //sleep for 6 milliseconds
//
//    (*i2cTrans).slaveAddress = BIO_ADDRESS;
//    (*i2cTrans).writeBuf = localTxBuffer;
//    (*i2cTrans).writeCount = 0;
//    (*i2cTrans).readBuf = localRxBuffer;
//    (*i2cTrans).readCount = 1; //expect a status byte
//
//    I2C_transfer(*i2cHandle, i2cTrans);
//
//    return localRxBuffer[0]; //return the status byte
//}

////Used to read an array of bytes
//char I2CReadFillArray(I2C_Transaction *i2cTrans, I2C_Handle *i2cHandle, uint8_t familyByte, uint8_t indexByte, uint8_t arraySize, uint8_t *arraytoFill){
//
//    uint8_t localTxBuffer[2]; //Family Byte, Index Byte
//    uint8_t localRxBuffer[7]; //Read Status Byte, data bytes (6)
//    localTxBuffer[0] = familyByte; //set the family byte
//    localTxBuffer[1] = indexByte; //set the index byte
//    (*i2cTrans).slaveAddress = BIO_ADDRESS;
//    (*i2cTrans).writeBuf = localTxBuffer;
//    (*i2cTrans).writeCount = 2;
//    (*i2cTrans).readBuf = localRxBuffer;
//    (*i2cTrans).readCount = 0;
//
//    I2C_transfer(*i2cHandle, i2cTrans);
//
//    usleep(CMD_DELAY * 1000); //sleep for 6 milliseconds
//
//    (*i2cTrans).slaveAddress = BIO_ADDRESS;
//    (*i2cTrans).writeBuf = localTxBuffer;
//    (*i2cTrans).writeCount = 0;
//    (*i2cTrans).readBuf = localRxBuffer;
//    (*i2cTrans).readCount = 1 + arraySize;
//
//    I2C_transfer(*i2cHandle, i2cTrans);
//
//    int arrayCount = 0;
//    if(localRxBuffer[0]){ //if the status byte is non-zero
//        return localRxBuffer[0]; //we had a read error, so return that
//    }
//    else{
//        for(arrayCount = 0; arrayCount < arraySize; arrayCount++){
//            arraytoFill[arrayCount] = localRxBuffer[1 + arrayCount];
//        }
//    }
//
//    return localRxBuffer[0]; //return status byte
//}
///////////////////////////////////////////////////////////////////////////////////////////

