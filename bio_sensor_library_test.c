/*
 * Copyright (c) 2018-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== I2C_test.c ========
 */
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/display/Display.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#define TASKSTACKSIZE       640

#include "bio_sensor.h"


static Display_Handle display;


/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{

    I2C_Handle      i2c;
    I2C_Params      i2cParams;

    struct bioData body; //struct of biodata
    struct version sensorHubVer; //struct of version, used to read sensor hub version
    struct version algoVer; //struct of version, used to read algorithm version
    struct version bootVer; //struct of version, used to read bootloader version
    struct sensorAttr max30101Attr; //struct of sensor attributes, used with MAX30101
    struct sensorAttr accelAttr; //struct of sensor attributes, used with accelerometer
    uint8_t mcuType = 0xff; //MCU type of biometric sensor hub

    uint8_t globalStatus = 0x01; //global status for testing library
    bool libraryTest = true; //variable for testing the library
    bool dataStream = false; //log data via excel data streamer


    uint8_t userMode = MODE_ONE;
    uint8_t intThresh = 0x01;
    uint8_t outFormat = ALGO_DATA;

    int numSamples = 1500; //number of samples to take during the test

    int sampleLoop = 0;

    uint8_t extAccelMode = 0;
    uint8_t statusByte = 0;
    uint8_t deviceMode = 0;
    uint16_t adcRate = 0; //sample rate of the MAX30101 ADC
    uint16_t adcRange = 0; //full scale range of MAX30101 ADC
    uint8_t ledArray[4]; //array of LED data
    uint8_t ledStatus = 0; //status for reading LED pulse amplitude
    uint8_t operatingMode = 0; //operating mode of MAX30101 (which LEDs are being used)
    uint16_t ledPulseWidth = 0; //pulse width of MAX30101 LEDs (all equal)
    uint8_t algoRange = 0; //percent of full scale ADC range that algo is using
    uint8_t algoStepSize = 0; //step size towards the target for the AGC algorithm
    uint8_t algoSensitivity = 0; //sensitivity of the AGC algorithm
    uint16_t algoSampleRate = 0; //WHRM algorithm sample rate
    uint16_t defaultHeight = 0; //default height for algorithm
    uint16_t extInputFifoSize = 0; //external sensor input FIFO size
    int32_t maximFastCoef[NUM_MAXIM_FAST_COEF] = {0, 0, 0}; //Maxim Fast Algorithm Coefficients: A, B, C
    uint8_t coefStatus = 0;
    int32_t motionThreshold = 0;
    int32_t coefA = 0;
    uint8_t maxState = 0;

    /* Call driver init functions */
    Display_init();
//    Board_init();
    GPIO_init();
    I2C_init();

    sleep(1);

    /* Configure the LEDs*/
    GPIO_setConfig(Board_GPIO_LED0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH); //LED0 Pin
    GPIO_setConfig(Board_GPIO_LED1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH); //LED1 Pin
    GPIO_setConfig(Board_GPIO_DIO1_MFIO, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH); //mfio pin, pull high
    GPIO_setConfig(Board_GPIO_DIO0_RESET, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH); //reset pin, pull low
//
    GPIO_write(Board_GPIO_DIO0_RESET, 0x0); //pull reset low
    usleep(10000); //sleep for 10 ms
    GPIO_write(Board_GPIO_DIO0_RESET, 0x1); //set the reset pin high
    sleep(1); //sleep for 1 second
    GPIO_setConfig(Board_GPIO_DIO1_MFIO, GPIO_CFG_IN_PU); //setup the MFIO as an input so MAC32664 can use it
//    GPIO_setConfig(Board_GPIO_DIO1_MFIO, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);


    sleep(1);

    GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF); //turn the green LED off
    GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF); //turn the red LED off



    /* Open the HOST display for output */
    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        while (1);
    }

    /* Create I2C for usage */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
//    i2cParams.bitRate = I2C_100kHz;

    while(1){

        if(!dataStream) Display_printf(display, 0, 0, "Biometric Sensor Hub Library Testing is starting...");

        if(libraryTest && !dataStream){ //if we're testing function and not streaming data
            Display_printf(display, 0, 0, "\nFunctions expected to fail: ");
            Display_printf(display, 0, 0, "readAlgorithmVersion: deprecated");
            Display_printf(display, 0, 0, "I2CReadIntWithWriteByte (default height): unknown error (doesn't exist/protected by encryption?) ");
            Display_printf(display, 0, 0, "I2CRead32BitValue (read motion threshold): deprecated ");
        }


        GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON); //turn the green LED on to symbolize testing
        GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON); //turn the red LED on to symbolize testing start
        sleep(1);
        GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF); //turn the green LED off
        GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF); //turn the red LED off

        i2c = I2C_open(CONFIG_I2C_0, &i2cParams); //open the on-board I2C channel
        if (i2c == NULL) {
            if(!dataStream) Display_printf(display, 0, 0, "\nError Opening I2C\n");
            while (1); //wait forever
        }
        else {
            if(!dataStream) Display_printf(display, 0, 0, "\nI2C Opened!");
        }


        if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting beginI2C...");
        deviceMode = beginI2C(i2c, &statusByte);
        if(statusByte){ //if we get a non-zero status byte from I2C transaction
            GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON);
            GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
            if(!dataStream){
                Display_printf(display, 0, 0, "beginI2C Failed."); //print that there was a transfer issue
                Display_printf(display, 0, 0, "Error byte: 0x%02x ", statusByte); //print that there was a transfer issue
            }
            globalStatus &= 0x00;
        }
        else {
            GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON);
            GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
            if(!dataStream){
                if(libraryTest) Display_printf(display, 0, 0, "beginI2C Passed.");
                Display_printf(display, 0, 0, "Device Mode: 0x%02x ", deviceMode);
            }
            globalStatus &= 0x01;
        }


        if(libraryTest){
            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting setDeviceMode...");
            deviceMode = setDeviceMode(RESET, &statusByte);
            if(statusByte || deviceMode != RESET){ //if had a I2C transaction error or we're not in the reset mode
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error testing setDeviceMode "); //indicate error
                    Display_printf(display, 0, 0, "Error: 0x%02x ", statusByte); //print error message
                    Display_printf(display, 0, 0, "Read state: 0x%02x ", deviceMode); //print out data
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "setDeviceMode Passed.");
                    Display_printf(display, 0, 0, "Device Operating Mode: 0x%02x ", deviceMode); //print out the current motion threshold
                }
                globalStatus &= 0x01; //successful transaction
            }
            setDeviceMode(EXIT_BOOTLOADER, &statusByte); //set it back into operating mode
            if(!dataStream)  Display_printf(display, 0, 0, "Delay to ensure application mode");
            sleep(1); //sleep for 1 second to ensure that it's back in application mode and ready to receive I2C transactions
        }

        if(libraryTest){ //if we're testing the library and want to test the software reset function
            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting softwareResetMAX32664...");
            statusByte = softwareResetMAX32664();
            if(statusByte){ //if we get a non-zero status byte from I2C transaction
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
                if(!dataStream){
                    Display_printf(display, 0, 0, "softwareResetMAX32664 Failed."); //print that there was a transfer issue
                    Display_printf(display, 0, 0, "Error byte: 0x%02x ", statusByte); //print that there was a transfer issue
                }
                globalStatus &= 0x00;
            }
            else{
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
                if(!dataStream){
                    if(libraryTest) Display_printf(display, 0, 0, "softwareResetMAX32664 Passed.");
                }
                globalStatus &= 0x01;
            }
            if(!dataStream)  Display_printf(display, 0, 0, "Delay to ensure application mode");
            sleep(5); //sleep for a few seconds to ensure everything is back online
        }


        if(libraryTest){ //if we're testing the library (testing lower level functions)
            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readRawData...");
            configMAX32664(SENSOR_DATA, MODE_ONE, 1); //configure the MAX32664 so we can test the reeadRawData function
            if(!dataStream) Display_printf(display, 0, 0, "Delay to let FIFO fill");
            sleep(5); //sleep to allow FIFO to fill with some data
            body = readRawData(&statusByte); //read a set of raw data
            if(statusByte){
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
                if(!dataStream){
                    Display_printf(display, 0, 0, "readRawData Failed."); //print that there was an issue with the function
                    Display_printf(display, 0, 0, "Error: 0x%02x", statusByte);
                }
                globalStatus &= 0x00;
            }
            else{
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
                if(!dataStream && libraryTest) Display_printf(display, 0, 0, "readRawData Passed.");
                if(!dataStream){
                    Display_printf(display, 0, 0, "IR LED Count: %u ", body.irLed); //print out the IR LED count
                    Display_printf(display, 0, 0, "Red LED Count: %u ", body.redLed); //print out the Red LED count
                }
                globalStatus &= 0x01;
            }
        }

        if(libraryTest){
            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readAlgoData...");
            configMAX32664(ALGO_DATA, MODE_TWO, 1); //configure the MAX32664 so we can test the reeadRawData function
            if(!dataStream) Display_printf(display, 0, 0, "Delay to let FIFO fill");
            sleep(5); //sleep to allow FIFO to fill with some data
            body = readAlgoData(&statusByte); //read a set of raw data
            if(statusByte){
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
                if(!dataStream){
                    Display_printf(display, 0, 0, "readAlgoData Failed."); //print that there was an issue with the function
                    Display_printf(display, 0, 0, "Error: 0x%02x", statusByte);
                }
                globalStatus &= 0x00;
            }
            else{
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
                if(!dataStream && libraryTest) Display_printf(display, 0, 0, "readAlgoData Passed.");
                if(!dataStream){
                    Display_printf(display, 0, 0, "Heart Rate: %02u ", body.heartRate); //print out the heart rate
                    Display_printf(display, 0, 0, "HR Confidence: %u ", body.confidence); //print our HR confidence level
                    Display_printf(display, 0, 0, "SpO2 Level: %02u ", body.oxygen); //print out SpO2 level
                    Display_printf(display, 0, 0, "Algorithm state: %u ", body.status); //print algorithm state
                    Display_printf(display, 0, 0, "Algorithm status: %d ", body.extStatus); //print algorithm state
                    Display_printf(display, 0, 0, "Blood Oxygen R value: %.2f ", body.rValue); //print algorithm state
                }
                globalStatus &= 0x01;
            }
        }


        if(libraryTest){
            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readRawAndAlgoData...");
            configMAX32664(SENSOR_AND_ALGORITHM, MODE_TWO, 1); //configure the MAX32664 so we can test the reeadRawData function
            if(!dataStream) Display_printf(display, 0, 0, "Delay to let FIFO fill");
            sleep(5); //sleep to allow FIFO to fill with some data
            body = readRawAndAlgoData(&statusByte); //read a set of raw data
            if(statusByte){
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
                if(!dataStream){
                    Display_printf(display, 0, 0, "readRawAndAlgoData Failed."); //print that there was an issue with the function
                    Display_printf(display, 0, 0, "Error: 0x%02x", statusByte);
                }
                globalStatus &= 0x00;
            }
            else{
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
                if(!dataStream && libraryTest) Display_printf(display, 0, 0, "readAlgoData Passed.");
                if(!dataStream){
                    Display_printf(display, 0, 0, "IR LED Count: %u ", body.irLed); //print out the IR LED count
                    Display_printf(display, 0, 0, "Red LED Count: %u ", body.redLed); //print out the Red LED count
                    Display_printf(display, 0, 0, "Heart Rate: %02u ", body.heartRate); //print out the heart rate
                    Display_printf(display, 0, 0, "HR Confidence: %u ", body.confidence); //print our HR confidence level
                    Display_printf(display, 0, 0, "SpO2 Level: %02u ", body.oxygen); //print out SpO2 level
                    Display_printf(display, 0, 0, "Algorithm state: %u ", body.status); //print algorithm state
                    Display_printf(display, 0, 0, "Algorithm status: %d ", body.extStatus); //print algorithm state
                    Display_printf(display, 0, 0, "Blood Oxygen R value: %.2f ", body.rValue); //print algorithm state
                }
                globalStatus &= 0x01;
            }
        }


        if(libraryTest){
            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting getAfeAttributesMAX30101...");
            max30101Attr = getAfeAttributesMAX30101(&statusByte); //get the MAX30101 attributes
            if(statusByte){
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
                if(!dataStream){
                    Display_printf(display, 0, 0, "getAfeAttributesMAX30101 Failed."); //print that there was an issue with the function
                    Display_printf(display, 0, 0, "Error: 0x%02x", statusByte);
                }
                globalStatus &= 0x00;
            }
            else{
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
                if(!dataStream && libraryTest) Display_printf(display, 0, 0, "getAfeAttributesMAX30101 Passed.");
                if(!dataStream){ //if we're not printing in a specific format
                    Display_printf(display, 0, 0, "MAX30101 number of bytes per word: %u", max30101Attr.byteWord); //print out the bytes per word
                    Display_printf(display, 0, 0, "MAX30101 number of registers: %u / 0x%02x", max30101Attr.availRegisters, max30101Attr.availRegisters); //print out the number of available registers
                }
                globalStatus &= 0x01;
            }
        }


        if(libraryTest){
            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting getAfeAttributesAccelerometer...");
            accelAttr = getAfeAttributesAccelerometer(&statusByte); //get the accelerometer attributes
            if(statusByte){
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
                if(!dataStream){
                    Display_printf(display, 0, 0, "getAfeAttributesAccelerometer Failed."); //print that there was an issue with the function
                    Display_printf(display, 0, 0, "Error: 0x%02x", statusByte);
                }
                globalStatus &= 0x00;
            }
            else{
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
                if(!dataStream && libraryTest) Display_printf(display, 0, 0, "getAfeAttributesAccelerometer Passed.");
                if(!dataStream){ //if we're not printing in a specific format
                    Display_printf(display, 0, 0, "Accelerometer number of bytes per word: %u", accelAttr.byteWord); //print out the bytes per word
                    Display_printf(display, 0, 0, "Accelerometer number of registers: %u / 0x%02x", accelAttr.availRegisters, accelAttr.availRegisters); //print out the number of available registers
                }
                globalStatus &= 0x01;
            }
        }


        if(libraryTest){
            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting getExtAccelMode...");
            extAccelMode = getExtAccelMode(&statusByte); //get the mode for the external accelerometer
            if(statusByte || extAccelMode == ERR_UNKNOWN){
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
                if(!dataStream){
                    Display_printf(display, 0, 0, "getExtAccelMode Failed."); //print that there was an issue with the function
                    Display_printf(display, 0, 0, "Error: 0x%02x", statusByte);
                    Display_printf(display, 0, 0, "Ext. Accl. Mode: %d ", extAccelMode);
                }
                globalStatus &= 0x00;
            }
            else{
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
                if(!dataStream && libraryTest) Display_printf(display, 0, 0, "getExtAccelMode Passed.");
                if(!dataStream){ //if we're not printing in a specific format
                    if(extAccelMode == 0){
                        Display_printf(display, 0, 0, "Ext. Accl. Mode: Sensor Hub accelerometer disabled");
                    }
                    else if(extAccelMode == 1){
                        Display_printf(display, 0, 0, "Ext. Accl. Mode: External Host accelerometer disabled");
                    }
                    else if(extAccelMode == 2){
                        Display_printf(display, 0, 0, "Ext. Accl. Mode: Sensor Hub accelerometer enabled");
                    }
                    else if(extAccelMode == 3){
                        Display_printf(display, 0, 0, "Ext. Accl. Mode: External Host accelerometer enabled");
                    }
                }
                globalStatus &= 0x01;
            }
        }


        if(libraryTest){
            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting softwareResetMAX30101...");
            statusByte = softwareResetMAX30101(); //reset the MAX30101
            if(statusByte){
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
                if(!dataStream){
                    Display_printf(display, 0, 0, "softwareResetMAX30101 Failed."); //print that there was an issue with the function
                    Display_printf(display, 0, 0, "Error: 0x%02x", statusByte);
                }
                globalStatus &= 0x00;
            }
            else{
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
                if(!dataStream && libraryTest) Display_printf(display, 0, 0, "softwareResetMAX30101 Passed.");
                globalStatus &= 0x01;
            }
            if(!dataStream) Display_printf(display, 0, 0, "Delay to ensure MAX30101 reset");
            sleep(5); //sleep to allow the MAX30101 to reset properly (don't know appropriate timing)
        }




        if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting configMAX32664...");
        uint8_t status = configMAX32664(outFormat, userMode, intThresh);
        if(status){
            GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON);
            GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
            if(!dataStream){
                Display_printf(display, 0, 0, "configMAX32664 Failed."); //print that there was an issue with the function
                Display_printf(display, 0, 0, "Error: 0x%02x", status);
            }
            globalStatus &= 0x00;
        }
        else {
            GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON);
            GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
            if(!dataStream && libraryTest) Display_printf(display, 0, 0, "configMAX32664 Passed.");
            globalStatus &= 0x01;
        }


        if(libraryTest){
            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readSensorData...");
            if(!dataStream) Display_printf(display, 0, 0, "Delay to let FIFO fill");
            sleep(5); //sleep to allow FIFO to fill with some data
            body = readSensorData(&statusByte); //read appropriate sensor data
            if(statusByte){
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
                if(!dataStream){
                    Display_printf(display, 0, 0, "readSensorData Failed."); //print that there was an issue with the function
                    Display_printf(display, 0, 0, "Error: 0x%02x", statusByte);
                }
                globalStatus &= 0x00;
            }
            else{
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON);
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
                if(!dataStream && libraryTest) Display_printf(display, 0, 0, "readAlgoData Passed.");
                if(!dataStream){
                    Display_printf(display, 0, 0, "IR LED Count: %u ", body.irLed); //print out the IR LED count
                    Display_printf(display, 0, 0, "Red LED Count: %u ", body.redLed); //print out the Red LED count
                    Display_printf(display, 0, 0, "Heart Rate: %02u ", body.heartRate); //print out the heart rate
                    Display_printf(display, 0, 0, "HR Confidence: %u ", body.confidence); //print our HR confidence level
                    Display_printf(display, 0, 0, "SpO2 Level: %02u ", body.oxygen); //print out SpO2 level
                    Display_printf(display, 0, 0, "Algorithm state: %u ", body.status); //print algorithm state
                    Display_printf(display, 0, 0, "Algorithm status: %d ", body.extStatus); //print algorithm state
                    Display_printf(display, 0, 0, "Blood Oxygen R value: %.2f ", body.rValue); //print algorithm state
                }
                globalStatus &= 0x01;
            }
        }



        //////////////////////////////////////////////////////////////////


        sleep(4);
        GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
        GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
        if(!dataStream) Display_printf(display, 0, 0, "\nStarting sensor data read....\n\n");


        if(libraryTest){ //if want to test the library (run through once)
            sampleLoop = 1;
        }
        else{
            sampleLoop = numSamples;
        }

        int i = 0;
        for(i = 0; i < sampleLoop; i++){ //collect a certain number of samples

            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readSensorData...");
            body = readSensorData(&statusByte); //read the sensor data
            if(statusByte){ //if had a I2C transaction error from reading the sensor data
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading sensor data "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readSensorData Passed.");
                    if( (userMode == MODE_TWO || userMode == MODE_ONE) && (outFormat == ALGO_DATA || outFormat == SENSOR_AND_ALGORITHM) ){ //if algorithm output and mode 1
                        Display_printf(display, 0, 0, "Heart Rate: %02u ", body.heartRate); //print out the heart rate
                        Display_printf(display, 0, 0, "HR Confidence: %u ", body.confidence); //print our HR confidence level
                        Display_printf(display, 0, 0, "SpO2 Level: %02u ", body.oxygen); //print out SpO2 level
                        Display_printf(display, 0, 0, "Algorithm state: %u ", body.status); //print algorithm state
                    }
                    if(userMode == MODE_TWO && (outFormat == ALGO_DATA || outFormat == SENSOR_AND_ALGORITHM)){ //if we're in mode 2, need extended data
                        Display_printf(display, 0, 0, "Algorithm status: %d ", body.extStatus); //print algorithm state
                        Display_printf(display, 0, 0, "Blood Oxygen R value: %.2f ", body.rValue); //print algorithm state
                    }

                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readSensorHubVersion...");
            sensorHubVer = readSensorHubVersion(&statusByte); //read the sensor hub version
            if(statusByte){ //if had a I2C transaction error
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading sensor hub version "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "Sensor Hub Version: %d.%d.%d ", sensorHubVer.major, sensorHubVer.minor, sensorHubVer.revision); //print out the version
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readSensorHubVersion Passed.");
                    Display_printf(display, 0, 0, "Sensor Hub Version: %d.%d.%d ", sensorHubVer.major, sensorHubVer.minor, sensorHubVer.revision); //print out the version
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readAlgorithmVersion...");
            algoVer = readAlgorithmVersion(&statusByte); //read the algorithm version
            if(statusByte){ //if had a I2C transaction error
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading algorithm version "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "Algorithm Version: %d.%d.%d ", algoVer.major, algoVer.minor, algoVer.revision); //print out the algorithm version
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readAlgorithmVersion Passed.");
                    Display_printf(display, 0, 0, "Algorithm Version: %d.%d.%d ", algoVer.major, algoVer.minor, algoVer.revision); //print out the algorithm version
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readBootloaderVersion...");
            bootVer = readBootloaderVersion(&statusByte); //read the bootloader version
            if(statusByte){ //if had a I2C transaction error
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading bootloader version "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "Bootloader Version: %d.%d.%d ", bootVer.major, bootVer.minor, bootVer.revision); //print out the bootloader version
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readBootloaderVersion Passed.");
                    Display_printf(display, 0, 0, "Bootloader Version: %d.%d.%d ", bootVer.major, bootVer.minor, bootVer.revision); //print out the bootloader version
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting getMcuType...");
            mcuType = getMcuType(&statusByte); //get the MCU type of the biometric sensor hub
            if(statusByte || (mcuType == ERR_UNKNOWN) ){ //if had a I2C transaction error OR we got an invalid MCU type
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading MCU Type "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "mcyType: %u ", mcuType); //print out the read value
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "getMcuType Passed.");
                    if(mcuType == 0){ // 0 == MAX32625
                        Display_printf(display, 0, 0, "MCU Type: MAX32625"); //print out the MCU type
                    }
                    else if(mcuType == 1){ //1 == MAX32660/MAX32664
                        Display_printf(display, 0, 0, "MCU Type: MAX32660/MAX32664"); //print out the MCU type
                    }
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readADCSampleRate...");
            adcRate = readADCSampleRate(&statusByte); //read the ADC sampling rate of the MAX30101 internal ADC
            if(statusByte || (adcRate == ERR_UNKNOWN) ){ //if had a I2C transaction error OR got an invalid data reading
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading MAX30101 ADC Sampling Rate "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "MAX30101 ADC Sampling Rate: %u ", adcRate); //print out the read value
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readADCSampleRate Passed.");
                    Display_printf(display, 0, 0, "MAX30101 ADC Sampling Rate: %u ", adcRate); //print out the ADC sampling rate
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readADCRange...");
            adcRange = readADCRange(&statusByte); //read the ADC full scale range of MAX30101
            if(statusByte || (adcRange == ERR_UNKNOWN) ){ //if had a I2C transaction error OR got an invalid data reading
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading MAX30101 ADC Full Scale Range "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "MAX30101 ADC Full Scale Range: %u ", adcRange); //print out the read value
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readADCRange Passed.");
                    Display_printf(display, 0, 0, "MAX30101 ADC Full Scale Range: %u ", adcRange); //print out the ADC full scale range
                }
                globalStatus &= 0x01; //successful transaction
            }

            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting ledPulseWidth...");
            ledPulseWidth = readPulseWidth(&statusByte); //read the LED pulse width of MAX30101 LEDs
            if(statusByte || (ledPulseWidth == ERR_UNKNOWN) ){ //if had a I2C transaction error OR got an invalid data reading
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading MAX30101 LED Pulse Width "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "MAX30101 LED Pulse Width: %u us ", ledPulseWidth); //print out the read value
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readPulseWidth Passed.");
                    Display_printf(display, 0, 0, "MAX30101 LED Pulse Width: %u us ", ledPulseWidth); //print out the LED pulse width (same for all LEDs)
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readPulseAmp...");
            ledStatus = readPulseAmp(ledArray, &statusByte); //read the
            if(statusByte || (ledStatus == ERR_UNKNOWN) ){ //if had a I2C transaction error OR invalid data
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading LED Pulse Amplitude "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "MAX30101 LED1 Pulse Amplitude: %f ", (float)ledArray[0] * 0.2f); //print out the LED1 pulse amplitude
                    Display_printf(display, 0, 0, "MAX30101 LED2 Pulse Amplitude: %f ", (float)ledArray[1] * 0.2f); //print out the LED2 pulse amplitude
                    Display_printf(display, 0, 0, "MAX30101 LED3 Pulse Amplitude: %f ", (float)ledArray[2] * 0.2f); //print out the LED3 pulse amplitude
                    Display_printf(display, 0, 0, "MAX30101 LED4 Pulse Amplitude: %f ", (float)ledArray[3] * 0.2f); //print out the LED4 pulse amplitude
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readPulseAmp Passed.");
                    Display_printf(display, 0, 0, "MAX30101 LED1 Pulse Amplitude: %f ", (float)ledArray[0] * 0.2f); //print out the LED1 pulse amplitude
                    Display_printf(display, 0, 0, "MAX30101 LED2 Pulse Amplitude: %f ", (float)ledArray[1] * 0.2f); //print out the LED2 pulse amplitude
                    Display_printf(display, 0, 0, "MAX30101 LED3 Pulse Amplitude: %f ", (float)ledArray[2] * 0.2f); //print out the LED3 pulse amplitude
                    Display_printf(display, 0, 0, "MAX30101 LED4 Pulse Amplitude: %f ", (float)ledArray[3] * 0.2f); //print out the LED4 pulse amplitude
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readMAX30101Mode...");
            operatingMode = readMAX30101Mode(&statusByte); //read the current operating mode of the MAX30101 (which LEDs are being used)
            if(statusByte || (operatingMode == ERR_UNKNOWN) ){ //if had a I2C transaction error or invalid data
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading MAX30101 Operating Mode "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "MAX30101 Operating Mode: %u ", operatingMode); //print out the read value
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readMAX30101Mode Passed.");
                    //deal with the operating mode parsing
                    if(operatingMode == 2){
                        Display_printf(display, 0, 0, "MAX30101 Operating Mode: Heart Rate Mode; Red LED only "); //print out the MAX30101 operating mode
                    }
                    else if(operatingMode == 3){
                        Display_printf(display, 0, 0, "MAX30101 Operating Mode: SpO2 Mode; Red and IR LEDs "); //print out the MAX30101 operating mode
                    }
                    else if(operatingMode == 7){
                        Display_printf(display, 0, 0, "MAX30101 Operating Mode: Multi-LED Mode; Green, Red and/or IR LEDs "); //print out the MAX30101 operating mode
                    }                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readAlgoRange...");
            algoRange = readAlgoRange(&statusByte); //read the percent of the full scale ADC range that the AGC algo is using
            if(statusByte || (algoRange > 100) ){ //if had a I2C transaction error
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading Algorithm ADC Range "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "AGC Algorithm ADC Range: %u%% ", algoRange); //print out the read value
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readAlgoRange Passed.");
                    Display_printf(display, 0, 0, "AGC Algorithm ADC Range: %u%% ", algoRange); //print algorithm range %
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readAlgoStepSize...");
            algoStepSize = readAlgoStepSize(&statusByte); //read the step size towards the target for the AGC algorithm
            if(statusByte || (algoStepSize > 100) ){ //if had a I2C transaction error OR invalid data
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading AGC algo step size "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "AGC algo step size: %u%% ", algoStepSize); //print out the read value
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readAlgoStepSize Passed.");
                    Display_printf(display, 0, 0, "AGC Algorithm Step Size: %u%% ", algoStepSize);
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readAlgoSensitivity...");
            algoSensitivity = readAlgoSensitivity(&statusByte); //read the sensitivity of the AGC algorithm
            if(statusByte || (algoSensitivity > 100) ){ //if had a I2C transaction error OR invalid data
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading AGC algo step size "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "AGC Algorithm sensitivity: %u%% ", algoSensitivity); //print out the read value
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readAlgoSensitivity Passed.");
                    Display_printf(display, 0, 0, "AGC Algorithm sensitivity: %u%% ", algoSensitivity);
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readAlgoSampleRate...");
            algoSampleRate = readAlgoSampleRate(&statusByte); //read the WHRM sample rate
            if(statusByte){ //if had a I2C transaction error
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading WHRM sample rate "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "WHRM Algorithm sample rate: %u%% ", algoSampleRate); //print out the read value
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readAlgoSampleRate Passed.");
                    Display_printf(display, 0, 0, "WHRM Algorithm Sample Rate: %u ", algoSampleRate); //print out WHRM algo sample rate
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting I2CReadIntWithWriteByte (default height)...");
            defaultHeight = I2CReadIntWithWriteByte(0x51, 0x02, 0x07, &statusByte);
            if(statusByte){ //if had a I2C transaction error
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading Default Algorithm Height "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "Default Algorithm Height: %u ", defaultHeight); //print out the default algorithm height
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "I2CReadIntWithWriteByte (default height) Passed.");
                    Display_printf(display, 0, 0, "Default Algorithm Height: %u ", defaultHeight); //print out the default algorithm height
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting I2CReadInt (read external input FIFO num samples)...");
            extInputFifoSize = I2CReadInt(0x13, 0x04, &statusByte);
            if(statusByte){ //if had a I2C transaction error
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading External Sensor Input FIFO Sample Number "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "Number of External Sensor Input FIFO Samples: %u ", extInputFifoSize); //print out the external sensor input FIFO size
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "I2CReadInt (read external input FIFO) Passed.");
                    Display_printf(display, 0, 0, "Number of External Sensor Input FIFO Samples: %u ", extInputFifoSize); //print out the external sensor input FIFO size
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting I2CReadInt (read external input FIFO size for max samples)...");
            extInputFifoSize = I2CReadInt(0x13, 0x01, &statusByte);
            if(statusByte){ //if had a I2C transaction error
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading Max samples External Sensor Input FIFO Size"); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", statusByte); //print out the status byte
                    Display_printf(display, 0, 0, "External Sensor Input FIFO Size for max number of samples FIFO can hold: %u ", extInputFifoSize); //print out the external sensor input FIFO size
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "I2CReadInt (read external input FIFO) Passed.");
                    Display_printf(display, 0, 0, "Max samples External Sensor Input FIFO Size: %u ", extInputFifoSize); //print out the external sensor input FIFO size
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readMaximFastCoef...");
            coefStatus = readMaximFastCoef(maximFastCoef); //get the Maxim Fast Coefficients
            if(coefStatus){ //if had a I2C transaction error
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading Maxim Fast Algo Coef "); //had error
                    Display_printf(display, 0, 0, "Status byte: 0x%02x ", coefStatus); //print out the status byte
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readMaximFastCoef Passed.");
                    Display_printf(display, 0, 0, "Maxim Fast Coefficients ");
                    Display_printf(display, 0, 0, "A: %d / %f ", maximFastCoef[0], (float)maximFastCoef[0] / (float)100000); //print out the Maxim Fast Coefficient
                    Display_printf(display, 0, 0, "B: %d / %f ", maximFastCoef[1], (float)maximFastCoef[1] / (float)100000); //print out the Maxim Fast Coefficient
                    Display_printf(display, 0, 0, "C: %d / %f ", maximFastCoef[2], (float)maximFastCoef[2] / (float)100000); //print out the Maxim Fast Coefficient
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting I2CRead32BitValue (read motion threshold)...");
            motionThreshold = I2CRead32BitValue(0x51, 0x05, 0x06, &statusByte);
            if(statusByte){ //if had a I2C transaction error
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error reading motion threshold "); //indicate error
                    Display_printf(display, 0, 0, "Error: 0x%02x ", statusByte); //print error message
                    Display_printf(display, 0, 0, "Motion threshold: %d ", motionThreshold); //print error message
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "I2CRead32BitValue (read motion threshold) Passed.");
                    Display_printf(display, 0, 0, "Motion Threshold: %d ", motionThreshold); //print out the current motion threshold
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting I2CRead32BitValue (read coefA)...");
            coefA = I2CRead32BitValue(0x51, 0x02, 0x0B, &statusByte);
            if(statusByte){ //if had a I2C transaction error
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error testing read 32-bit value "); //indicate error
                    Display_printf(display, 0, 0, "Error: 0x%02x ", statusByte); //print error message
                    Display_printf(display, 0, 0, "Read number: %d ", coefA); //print error message
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "I2CRead32BitValue (read coefA) Passed.");
                    Display_printf(display, 0, 0, "Read 32-bit value: %d ", coefA); //print out the current motion threshold
                }
                globalStatus &= 0x01; //successful transaction
            }


            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nTesting readMAX30101State...");
            maxState = readMAX30101State(&statusByte);
            if(statusByte){ //if had a I2C transaction error
                if(!dataStream){ //if we're not printing to serial in data logging format
                    Display_printf(display, 0, 0, "Error testing readMAX30101State "); //indicate error
                    Display_printf(display, 0, 0, "Error: 0x%02x ", statusByte); //print error message
                    Display_printf(display, 0, 0, "Read state: %d ", maxState); //print error message
                }
                globalStatus &= 0x00; //put error into global status
            }
            else{
                if(!dataStream){ //if we're not printing to serial in data logging format
                    if(libraryTest) Display_printf(display, 0, 0, "readMAX30101State Passed.");
                    Display_printf(display, 0, 0, "MAX30101 State: %u ", maxState); //print out the current motion threshold
                }
                globalStatus &= 0x01; //successful transaction
            }




            if(!libraryTest && dataStream){ //if we want to log data and we're not testing the library, put it into usable format
                Display_printf(display, 0, 0, "%02.2u,%u,%02.2u,%u,%d,%f,%u,%u,%u,%u,%f,%f,%f,%f,%u,%u,%u",
                               body.heartRate, body.confidence, body.oxygen, body.status, body.extStatus, body.rValue,
                               adcRate, adcRange, operatingMode, ledPulseWidth,
                               (float)ledArray[0] * 0.2f, (float)ledArray[1] * 0.2f, (float)ledArray[2] * 0.2f, (float)ledArray[3] * 0.2f,
                               algoRange, algoStepSize, algoSensitivity
                                ); //print out data in format that is usable by Excel Data Stream
            }



            //reset all values so if we have a communication error we aren't using old data
            body.heartRate = 0;
            body.confidence = 0;
            body.oxygen = 0;
            body.status = 0;
            body.extStatus = 0;
            body.rValue = 0.0;
            sensorHubVer.major = 0;
            sensorHubVer.minor = 0;
            sensorHubVer.revision = 0;
            algoVer.major = 0;
            algoVer.minor = 0;
            algoVer.revision = 0;
            bootVer.major = 0;
            bootVer.minor = 0;
            bootVer.revision  = 0;
            mcuType = 0xff;
            adcRate = 0;
            adcRange = 0;
            ledArray[0] = 0;
            ledArray[1] = 0;
            ledArray[2] = 0;
            ledArray[3] = 0;
            ledStatus = 0;
            operatingMode = 0;
            ledPulseWidth = 0;
            algoRange = 0;
            algoSensitivity = 0;
            algoSampleRate = 0;
            defaultHeight = 0;
            extInputFifoSize = 0;
            for(coefStatus = 0; coefStatus < 3; coefStatus++){
                maximFastCoef[coefStatus] = 0;
            }
            coefStatus = 0;
            motionThreshold = 0;
            coefA = 0;
            maxState = 0;
            statusByte = 0;


            usleep(1000); //sleep for 1ms
            usleep(10000); //sleep for 10ms
            sleep(1); //sleep for 1s

        }


        I2C_close(i2c);
        if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nI2C closed!");

        GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
        GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
        usleep(500000);
        char xLoop = 0;
        if(globalStatus){
            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nBiometric Sensor Hub Library Test Passed! \n\n");
            for(xLoop = 0; xLoop < 10; xLoop++){ //flash green LED to signal success
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_ON);
                usleep(50000);
                GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
                usleep(50000);
            }
            GPIO_write(Board_GPIO_LED1, CONFIG_GPIO_LED_OFF);
        }
        else{
            if(libraryTest && !dataStream) Display_printf(display, 0, 0, "\nBiometric Sensor Hub Library Test Failed! Refer to above errors for what failed and expected error list \n\n");
            for(xLoop = 0; xLoop < 10; xLoop++){ //flash red LED to signal failure
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_ON);
                usleep(50000);
                GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
                usleep(50000);
            }
            GPIO_write(Board_GPIO_LED0, CONFIG_GPIO_LED_OFF);
        }
        globalStatus = 0x01; //reset the global status
        sleep(5);
        if(!dataStream) Display_printf(display, 0, 0, "\n\n");

    }

    return (NULL);
}
