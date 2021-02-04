## Project Summary

This project is designed for the development and testing of a Biometric Sensor Hub from SparkFun on the CC2652R1 LaunchPad from TI.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `Board_GPIO_LED0` - Red Indicator LED
* `Board_GPIO_LED1` - Green Indicator LED
* `CONFIG_I2C_0` - I2C bus used to communicate with the MAX32664A Biometric Sensor Hub
* `Board_GPIO_DIO0_RESET` - GPIO DIO0 used to reset the MAX32664A
* `Board_GPIO_DIO1_MFIO` - GPIO DIO1 used as a MFIO with the MAX32664A

## External Devices, Board Resources & Jumper Settings

This example requires the ["SparkFun Pulse Oximeter and Heart Rate Sensor - MAX30101 & MAX32664 (Qwiic)"](https://www.sparkfun.com/products/15219) from SparkFun Electronics

For board specific jumper settings, resources and BoosterPack modifications,
refer to the __Board.html__ file.

> If you're using an IDE such as Code Composer Studio (CCS) or IAR, please
refer to Board.html in your project directory for resources used and
board-specific jumper settings.

The Board.html can also be found in your SDK installation:

        <SDK_INSTALL_DIR>/source/ti/boards/<BOARD>

## Example Usage

* Example output is generated through use of Display driver APIs. Refer to the
Display driver documentation found in the SimpleLink MCU SDK User's Guide.

* Open a serial session (e.g. [`PuTTY`](http://www.putty.org/ "PuTTY's
 Homepage"), etc.) to the appropriate COM port.
 * The COM port can be determined via Device Manager in Windows or via
 `ls /dev/tty*` in Linux.

The connection will have the following settings:
```
    Baud-rate:     115200
    Data bits:          8
    Stop bits:          1
    Parity:          None
    Flow Control:    None
```

* Run the example.

1. `CONFIG_GPIO_LED_0` turns ON to indicate the I2C driver initialization
is complete.
```
    I2C Initialized!
```

2. If required, the TMP sensor is enabled using a GPIO output.

3. The I2C bus is queried for all supported TMP sensors. The output varies depending on
which TMP sensor(s) is found.
```
    I2C slave address 0x41 not acknowledged!
    I2C slave address 0x49 not acknowledged!
    Detected TMP116 sensor with slave address 0x48
```

4. The last known TMP sensor will be used to read temperature samples. Samples
are converted to celcius and displayed via UART.
```
    Sample 0: 24 (C)
    Sample 1: 24 (C)
           .
           .
           .
    Sample 18: 24 (C)
    Sample 19: 24 (C)
```

5. After a number of samples, the I2C driver instance is closed.
```
    I2C closed!
```

*  If the I2C communication fails, an error message describing
the failure is displayed.
```
    I2C slave address 0x41 not acknowledged!
```

## Application Design Details

This application uses one task:

`'mainThread'` - performs the following actions:

1. Opens and initializes an I2C driver object.

2. Determines which TMP sensor is available. Sensors are described in the
`sensors` data structure at the top of the source.

3. Uses the I2C driver in blocking mode to get data from the TMP sensor.

4. Extracts the temperature (in Celsius) and prints the value via the UART.

5. The task sleeps for 1 second.

TI-RTOS:

* When building in Code Composer Studio, the kernel configuration project will
be imported along with the example. The kernel configuration project is
referenced by the example, so it will be built first. The "release" kernel
configuration is the default project used. It has many debug features disabled.
These feature include assert checking, logging and runtime stack checks. For a
detailed difference between the "release" and "debug" kernel configurations and
how to switch between them, please refer to the SimpleLink MCU SDK User's
Guide. The "release" and "debug" kernel configuration projects can be found
under &lt;SDK_INSTALL_DIR&gt;/kernel/tirtos/builds/&lt;BOARD&gt;/(release|debug)/(ccs|gcc).

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.

SimpleLink Sensor and Actuator Plugin:

* This example uses a TMP sensor. It interfaces via the I2C Driver. For a
richer API set for the TMP sensors, please refer to the SimpleLink Sensor
and Actuator Plugin. This plugin also offers a Button module to handle
debounce also.

[bp-bassensorsmkii]: http://www.ti.com/tool/bp-bassensorsmkii
