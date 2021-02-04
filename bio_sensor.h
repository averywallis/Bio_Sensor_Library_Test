/**
 * @file
 * bio_sensor.h
 * @brief
 *
 *  Created on: Nov 12, 2020
 *      Author: AveryW
 *
 * This library is based heavily on the SparkFun Bio Sensor Hub Library: https://github.com/sparkfun/SparkFun_Bio_Sensor_Hub_Library
 * which is an open source library produced by SparkFun Electronics. We would like to thank SparkFun and Elias Santistevan (main author)
 * for writing this library and making it available to the public. Below is the short shpeel given at the beginning of the SparkFun Library
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

#ifndef BIO_SENSOR_H_
#define BIO_SENSOR_H_

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
/* Driver configuration */
#include "ti_drivers_config.h"



/////////////////////////////////////////////////////////////////////////////
/*  The following defines and enums within the large lines of ///
    are taken (almost) directly from the SparkFun Bio Sensor Hub Library. Small modifications were made to allow for easier documentation/reference
    It was easier to copy their macros than re-write our own.
    We would like to thank SparkFun and Elias Santistevan (main author)
    for writing this library and making it available to the public.
    Below is the introduction/background section taken from the
    "SparkFun_Bio_Sensor_Hub_Library.cpp" file from the library:



    This is an Arduino Library written for the MAXIM 32664 Biometric Sensor Hub
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

    Feel like supporting our work? Buy a board from SparkFun!
*/

#define WRITE_FIFO_INPUT_BYTE  0x04
#define DISABLE                0x00
#define ENABLE                 0x01
#define MODE_ONE               0x01
#define MODE_TWO               0x02
#define APP_MODE               0x00
#define BOOTLOADER_MODE        0x08
#define NO_WRITE               0x00
#define INCORR_PARAM           0xEE

#define CONFIGURATION_REGISTER 0x0A
#define PULSE_MASK             0xFC
#define READ_PULSE_MASK        0x03
#define SAMP_MASK              0xE3
#define READ_SAMP_MASK         0x1C
#define ADC_MASK               0x9F
#define READ_ADC_MASK          0x60

#define ENABLE_CMD_DELAY          50 // Milliseconds
#define CMD_DELAY                 6  // Milliseconds
#define MAXFAST_ARRAY_SIZE        6  // Number of bytes....
#define MAXFAST_EXTENDED_DATA     5
#define MAX30101_LED_ARRAY        12 // 4 values of 24 bit (3 byte) LED values

#define SET_FORMAT             0x00
#define READ_FORMAT            0x01 // Index Byte under Family Byte: READ_OUTPUT_MODE (0x11)
#define WRITE_SET_THRESHOLD    0x01 //Index Byte for WRITE_INPUT(0x14)
#define WRITE_EXTERNAL_TO_FIFO 0x00

#define BIO_ADDRESS 0x55;


/**
 * @brief Struct of algorithm output data. Specific output data is dependent on algorithm configuration
 * @struct bioData
 */
struct bioData {

  uint32_t irLed; ///< IR LED ADC Count
  uint32_t redLed; ///< RED LED ADC Count
  uint16_t heartRate; ///< Calculated WHRM algorithm heart rate. LSB = 0.1bpm
  uint8_t  confidence; ///< Calculated WHRM algorithm confidence in heart rate. 0-100% LSB = 1%
  uint16_t oxygen; ///< Calculated WHRM algorithm SpO2 level. 0-100% LSB = 1%
  uint8_t  status; ///< Algorithm current state. 0: No object detected, 1: Something on sensor, 2: Another object detected, 3: Finger Detected
  float    rValue;      ///< -- Algorithm Mode 2 vv calculated R value
  int8_t   extStatus;   ///< -- Algorithm status. 0: Success, 1: Not Ready, -1: Something is on sensor, -2: Device excessive motion, -3: No object, -4: Pressing too hard, -5: Object instead of finger, -6: Finger Excessive Motion
  uint8_t  reserveOne;  // --
  uint8_t  resserveTwo; // -- Algorithm Mode 2 ^^

};

/**
 * @brief Struct of version data. Format is typically formatted major.minor.revision
 */
struct version {
  // 3 bytes total
  uint8_t major; ///< major part of version information
  uint8_t minor; ///< minor part of version information
  uint8_t revision; ///<revision part of version information

};


/**
 * @brief Struct of attributes for a sensor. Includes word size and register info
 */
struct sensorAttr {

  uint8_t byteWord; ///<Number of bytes in a word for this sensor
  uint8_t availRegisters; ///<Number of registers available

};

// Status Bytes are communicated back after every I-squared-C transmission and
// are indicators of success or failure of the previous transmission.
enum READ_STATUS_BYTE_VALUE {

  SUCCESS                  = 0x00,
  ERR_UNAVAIL_CMD,
  ERR_UNAVAIL_FUNC,
  ERR_DATA_FORMAT,
  ERR_INPUT_VALUE,
  ERR_TRY_AGAIN,
  ERR_BTLDR_GENERAL        = 0x80,
  ERR_BTLDR_CHECKSUM,
  ERR_BTLDR_AUTH,
  ERR_BTLDR_INVALID_APP,
  ERR_UNKNOWN              = 0xFF

};

// The family register bytes are the larger umbrella for all the Index and
// Write Bytes listed below. You can not reference a nestled byte without first
// referencing it's larger category: Family Register Byte.
enum FAMILY_REGISTER_BYTES {

  HUB_STATUS               = 0x00,
  SET_DEVICE_MODE,
  READ_DEVICE_MODE,
  OUTPUT_MODE            = 0x10,
  READ_OUTPUT_MODE,
  READ_DATA_OUTPUT,
  READ_DATA_INPUT,
  WRITE_INPUT,
  WRITE_REGISTER           = 0x40,
  READ_REGISTER,
  READ_ATTRIBUTES_AFE,
  DUMP_REGISTERS,
  ENABLE_SENSOR,
  READ_SENSOR_MODE,
  CHANGE_ALGORITHM_CONFIG  = 0x50,
  READ_ALGORITHM_CONFIG,
  ENABLE_ALGORITHM,
  BOOTLOADER_FLASH         = 0x80,
  BOOTLOADER_INFO,
  IDENTITY                 = 0xFF

};

// All the defines below are: 1. Index Bytes nestled in the larger category of the
// family registry bytes listed above and 2. The Write Bytes nestled even
// farther under their Index Bytes.

// Write Bytes under Family Byte: SET_DEVICE_MODE (0x01) and Index
// Byte: 0x00.
enum DEVICE_MODE_WRITE_BYTES {

  EXIT_BOOTLOADER          = 0x00,
  RESET                    = 0x02,
  ENTER_BOOTLOADER         = 0x08

};

// Write Bytes under Family Byte: OUTPUT_MODE (0x10) and Index byte: SET_FORMAT
// (0x00)
enum OUTPUT_MODE_WRITE_BYTE {

  PAUSE                    = 0x00,
  SENSOR_DATA,
  ALGO_DATA,
  SENSOR_AND_ALGORITHM,
  PAUSE_TWO,
  SENSOR_COUNTER_BYTE,
  ALGO_COUNTER_BYTE,
  SENSOR_ALGO_COUNTER

};

// Index Byte under the Family Byte: READ_DATA_OUTPUT (0x12)
enum FIFO_OUTPUT_INDEX_BYTE {

  NUM_SAMPLES,
  READ_DATA

};

// Index Byte under the Family Byte: READ_DATA_INPUT (0x13)
enum FIFO_EXTERNAL_INDEX_BYTE {

  SAMPLE_SIZE,
  READ_INPUT_DATA,
  READ_SENSOR_DATA, // For external accelerometer
  READ_NUM_SAMPLES_INPUT, // For external accelerometer
  READ_NUM_SAMPLES_SENSOR

};

// Index Byte under the Family Registry Byte: WRITE_REGISTER (0x40)
enum WRITE_REGISTER_INDEX_BYTE {

  WRITE_MAX30101 = 0x03,
  WRITE_ACCELEROMETER

};

// Index Byte under the Family Registry Byte: READ_REGISTER (0x41)
enum READ_REGISTER_INDEX_BYTE {

  READ_MAX30101 = 0x03,
  READ_ACCELEROMETER

};

// Index Byte under the Family Registry Byte: READ_ATTRIBUTES_AFE (0x42)
enum GET_AFE_INDEX_BYTE {

  RETRIEVE_AFE_MAX30101 = 0x03,
  RETRIEVE_AFE_ACCELEROMETER

};

// Index Byte under the Family Byte: DUMP_REGISTERS (0x43)
enum DUMP_REGISTER_INDEX_BYTE {

  DUMP_REGISTER_MAX30101 = 0x03,
  DUMP_REGISTER_ACCELEROMETER

};

// Index Byte under the Family Byte: ENABLE_SENSOR (0x44)
enum SENSOR_ENABLE_INDEX_BYTE {

  ENABLE_MAX30101 = 0x03,
  ENABLE_ACCELEROMETER

};

// Index Byte for the Family Byte: READ_SENSOR_MODE (0x45)
enum READ_SENSOR_ENABLE_INDEX_BYTE {

  READ_ENABLE_MAX30101 = 0x03,
  READ_ENABLE_ACCELEROMETER

};

// Index Byte under the Family Byte: CHANGE_ALGORITHM_CONFIG (0x50)
enum ALGORITHM_CONFIG_INDEX_BYTE {

  SET_TARG_PERC            = 0x00,
  SET_STEP_SIZE            = 0x00,
  SET_SENSITIVITY          = 0x00,
  SET_AVG_SAMPLES          = 0x00,
  SET_PULSE_OX_COEF        = 0x02,

};

// Write Bytes under the Family Byte: CHANGE_ALGORITHM_CONFIG (0x50) and the
// Index Byte: ALGORITHM_CONFIG_INDEX_BYTE - SET_TARG_PERC
enum ALGO_AGC_WRITE_BYTE {

  AGC_GAIN_ID              = 0x00,
  AGC_STEP_SIZE_ID,
  AGC_SENSITIVITY_ID,
  AGC_NUM_SAMP_ID,
  MAXIMFAST_COEF_ID        = 0x0B

};

// Index Bytes under the Family Byte: READ_ALGORITHM_CONFIG (0x51)
enum READ_ALGORITHM_INDEX_BYTE {

  READ_AGC_PERCENTAGE      = 0x00,
  READ_AGC_STEP_SIZE       = 0x00,
  READ_AGC_SENSITIVITY     = 0x00,
  READ_AGC_NUM_SAMPLES     = 0x00,
  READ_MAX_FAST_COEF       = 0x02

};

// Write Bytes under the Family Byte: READ_ALGORITHM_CONFIG (0x51) and Index Byte:
// READ_ALGORITHM_INDEX_BYTE - AGC
enum READ_AGC_ALGO_WRITE_BYTE {

  READ_AGC_PERC_ID           = 0x00,
  READ_AGC_STEP_SIZE_ID,
  READ_AGC_SENSITIVITY_ID,
  READ_AGC_NUM_SAMPLES_ID,
  READ_MAX_FAST_COEF_ID      = 0x0B

};

// Index Byte under the Family Byte: ENABLE_ALGORITHM (0x52).
enum ALGORITHM_MODE_ENABLE_INDEX_BYTE {

  ENABLE_AGC_ALGO  = 0x00,
  ENABLE_WHRM_ALGO = 0x02

};

// Index Byte under the Family Byte: BOOTLOADER_FLASH (0x80).
enum BOOTLOADER_FLASH_INDEX_BYTE {

  SET_INIT_VECTOR_BYTES    = 0x00,
  SET_AUTH_BYTES,
  SET_NUM_PAGES,
  ERASE_FLASH,
  SEND_PAGE_VALUE

};

// Index Byte under the Family Byte: BOOTLOADER_INFO (0x81).
enum BOOTLOADER_INFO_INDEX_BYTE {

  BOOTLOADER_VERS          = 0x00,
  PAGE_SIZE

};

// Index Byte under the Family Byte: IDENTITY (0xFF).
enum IDENTITY_INDEX_BYTES {

  READ_MCU_TYPE            = 0x00,
  READ_SENSOR_HUB_VERS     = 0x03,
  READ_ALGO_VERS           = 0x07

};

//End of section taken from SparkFun Bio Sensor Hub Library
/////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
//Section of new variables we created to add certain functionality to our system

#define I2_READ_STATUS_BYTE_COUNT 0x01

#define MODE_REGISTER          0x09 //Mode Configuration register of MAX30101
#define MODE_MASK              0xF8 //mask for modifying the mode bits while keeping other bits in register
#define READ_MODE_MASK         0x07 //mask for reading just the mode bits [2:0]
#define RESET_MASK             0xBF //mask for modifying the reset bit while keeping other bits in register
#define READ_RESET_MASK        0x40 //mask for reading just the reset bit [6]
#define SET_RESET_BIT          0x40 //mask for setting the reset bit to 1
#define SHDN_MASK              0x7F //mask for modifying the shutdown bit while keeping other bits in register
#define READ_SHDN_MASK         0x80 //mask for reading just the shutdown bit [7]

#define LEDOFFSET_REGISTER     0x0B //reserved register, just using as an offset
#define LED1_REGISTER          0x0C //LED1 Pulse Amplitude register
#define LED2_REGISTER          0x0D //LED2 Pulse Amplitude register
#define LED3_REGISTER          0x0E //LED3 Pulse Amplitude register
#define LED4_REGISTER          0x0F //LED4 Pulse Amplitude register

#define READ_MAX_FAST_RATE     0x02 //Maxim Fast sampling rate
#define READ_MAX_FAST_RATE_ID  0x00 //Maxim Fast sampling rate ID

#define NUM_MAXIM_FAST_COEF    0x03 //number of Maxim Fast algorithm coefficients
/////////////////////////////////////////////////////////////////////////////


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
uint8_t beginI2C(I2C_Handle i2cHandle, uint8_t *statusByte);


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
uint8_t configMAX32664(uint8_t outputFormat, uint8_t algoMode, uint8_t intThresh);


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
struct bioData readSensorData(uint8_t *statusByte);


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
struct bioData readRawData(uint8_t *statusByte);


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
struct bioData readAlgoData(uint8_t *statusByte);


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
struct bioData readRawAndAlgoData(uint8_t *statusByte);


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
uint8_t softwareResetMAX32664(void);


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
uint8_t softwareResetMAX30101(void);


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
uint8_t setOutputMode(uint8_t outputType);


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
uint8_t setFifoThreshold(uint8_t intThresh);


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
uint8_t numSamplesOutFifo(uint8_t *statusByte);


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
uint8_t agcAlgoControl(uint8_t enable);


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
uint8_t max30101Control(uint8_t senSwitch);


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
uint8_t readMAX30101State(uint8_t *statusByte);


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
uint8_t maximFastAlgoControl(uint8_t mode);


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
uint8_t readDeviceMode(uint8_t *statusByte);


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
uint8_t setDeviceMode(uint8_t operatingMode, uint8_t *statusByte);


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
uint8_t readSensorHubStatus(uint8_t *statusByte);


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
uint8_t readAlgoSamples(uint8_t *statusByte);


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
uint8_t readAlgoRange(uint8_t *statusByte);


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
uint8_t readAlgoStepSize(uint8_t *statusByte);


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
uint8_t readAlgoSensitivity(uint8_t *statusByte);


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
uint16_t readAlgoSampleRate(uint8_t *statusByte);


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
uint8_t readMaximFastCoef(int32_t *coefArray);

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
struct version readSensorHubVersion(uint8_t *statusByte);


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
struct version readAlgorithmVersion(uint8_t *statusByte);


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
struct version readBootloaderVersion(uint8_t *statusByte);


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
uint8_t getMcuType(uint8_t *statusByte);


/**
 * @brief   Reads the ADC sample rate of the MAX30101 internal ADC
 *
 * MAX30101 Register - CONFIGURATION_REGISTER (0x0A)
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  Sample rate of MAX30101 internal ADC in Hertz, ERR_UNKNOWN on a issue reading register or invalid number read
 */
uint16_t  readADCSampleRate(uint8_t *statusByte);


/**
 * @brief   Reads the internal ADC range of the MAX30101
 *
 * MAX30101 Register - CONFIGURATION_REGISTER (0x0A)
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  ADC full scale range of MAX30101, ERR_UNKNOWN if invalid number
 */
uint16_t  readADCRange(uint8_t *statusByte);


/**
 * @brief   Reads the LED pulse width of the LEDs in the MAX30101
 *
 * MAX30101 Register - CONFIGURATION_REGISTER (0x0A)
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  LED pulse width of MAX30101 in us, ERR_UNKNOWN for invalid read value or I2C transaction (check status byte!)
 */
uint16_t  readPulseWidth(uint8_t *statusByte);


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
uint8_t  readPulseAmp(uint8_t *ledArray, uint8_t *statusByte);


/**
 * @brief   Reads the operating mode of the MAX30101 PO sensor. Results indicate which LEDs are being used
 *
 * MAX30101 Register - MODE_REGISTER (0x09)
 *
 * @param   *statusByte Pointer to status byte
 *
 * @return  Operating mode of the MAX30101 (which LEDs are being used), ERR_UNKNOWN on invalid return value or I2C transaciton issue
 */
uint8_t readMAX30101Mode(uint8_t *statusByte);


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
struct sensorAttr getAfeAttributesMAX30101(uint8_t *statusByte);


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
struct sensorAttr getAfeAttributesAccelerometer(uint8_t *statusByte);


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
uint8_t getExtAccelMode(uint8_t *statusByte);


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
uint8_t  readRegisterMAX30101(uint8_t regAddr, uint8_t *statusByte);


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
uint8_t writeRegisterMAX30101(uint8_t regAddr, uint8_t regVal);





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
uint8_t I2CReadByte(uint8_t familyByte, uint8_t indexByte, uint8_t *statusByte);


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
uint8_t I2CReadBytewithWriteByte(uint8_t familyByte, uint8_t indexByte, uint8_t dataByte, uint8_t *statusByte);


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
uint8_t I2CReadFillArray(uint8_t familyByte, uint8_t indexByte, uint8_t arraySize, uint8_t *arraytoFill);


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
uint16_t I2CReadInt(uint8_t familyByte, uint8_t indexByte, uint8_t *statusByte);


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
uint16_t I2CReadIntWithWriteByte(uint8_t familyByte, uint8_t indexByte, uint8_t writeByte, uint8_t *statusByte);


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
int32_t I2CRead32BitValue(uint8_t familyByte, uint8_t indexByte, uint8_t dataByte, uint8_t *statusByte);


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
uint8_t I2CReadMultiple32BitValues(uint8_t familyByte, uint8_t indexByte, uint8_t dataByte, uint8_t numReads, int32_t *numArray);


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
uint8_t I2CWriteByte(uint8_t familyByte, uint8_t indexByte, uint8_t dataByte);


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
uint8_t I2CWrite2Bytes(uint8_t familyByte, uint8_t indexByte, uint8_t dataByte0, uint8_t dataByte1);


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
uint8_t I2CenableWriteByte(uint8_t familyByte, uint8_t indexByte, uint8_t dataByte);





/////////////////////////////////////////////////////////////////////////////////////////
//older versions of the lower level I2C transaction functions

//char I2CWriteByte(I2C_Transaction *i2cTrans, I2C_Handle *i2cHandle, uint8_t familyByte, uint8_t indexByte, uint8_t dataByte);

//char I2CReadByte(I2C_Transaction *i2cTrans, I2C_Handle *i2cHandle, uint8_t familyByte, uint8_t indexByte);

//char I2CReadBytewithWriteByte(I2C_Transaction *i2cTrans, I2C_Handle *i2cHandle, uint8_t familyByte, uint8_t indexByte, uint8_t dataByte);

//char I2CenableWrite(I2C_Transaction *i2cTrans, I2C_Handle *i2cHandle, uint8_t familyByte, uint8_t indexByte, uint8_t dataByte);

//char I2CReadFillArray(I2C_Transaction *i2cTrans, I2C_Handle *i2cHandle, uint8_t familyByte, uint8_t indexByte, uint8_t arraySize, uint8_t *arraytoFill);
/////////////////////////////////////////////////////////////////////////////////////////


#endif /* BIO_SENSOR_H_ */
