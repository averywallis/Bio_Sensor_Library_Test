/*
 *  ======== ti_drivers_config.h ========
 *  Configured TI-Drivers module declarations
 *
 *  The macros defines herein are intended for use by applications which
 *  directly include this header. These macros should NOT be hard coded or
 *  copied into library source code.
 *
 *  Symbols declared as const are intended for use with libraries.
 *  Library source code must extern the correct symbol--which is resolved
 *  when the application is linked.
 *
 *  DO NOT EDIT - This file is generated for the CC26X2R1_LAUNCHXL
 *  by the SysConfig tool.
 */
#ifndef ti_drivers_config_h
#define ti_drivers_config_h

#define CONFIG_SYSCONFIG_PREVIEW

#define CONFIG_CC26X2R1_LAUNCHXL
#ifndef DeviceFamily_CC26X2
#define DeviceFamily_CC26X2
#endif

#include <ti/devices/DeviceFamily.h>

#include <stdint.h>

/* support C++ sources */
#ifdef __cplusplus
extern "C" {
#endif


/*
 *  ======== CCFG ========
 */


/*
 *  ======== GPIO ========
 */

/* DIO6, LaunchPad LED Red */
extern const uint_least8_t              Board_GPIO_LED0_CONST;
#define Board_GPIO_LED0                 0
/* DIO1 */
extern const uint_least8_t              Board_GPIO_DIO0_RESET_CONST;
#define Board_GPIO_DIO0_RESET           1
/* DIO0 */
extern const uint_least8_t              Board_GPIO_DIO1_MFIO_CONST;
#define Board_GPIO_DIO1_MFIO            2
/* DIO7, LaunchPad LED Green */
extern const uint_least8_t              Board_GPIO_LED1_CONST;
#define Board_GPIO_LED1                 3
#define CONFIG_TI_DRIVERS_GPIO_COUNT    4

/* LEDs are active high */
#define CONFIG_GPIO_LED_ON  (1)
#define CONFIG_GPIO_LED_OFF (0)

#define CONFIG_LED_ON  (CONFIG_GPIO_LED_ON)
#define CONFIG_LED_OFF (CONFIG_GPIO_LED_OFF)


/*
 *  ======== I2C ========
 */

/*
 *  SCL: DIO4
 *  SDA: DIO5
 *  BAS Sensors Mark II I2C
 */
extern const uint_least8_t              CONFIG_I2C_0_CONST;
#define CONFIG_I2C_0                    0
#define CONFIG_TI_DRIVERS_I2C_COUNT     1

/* ======== I2C Addresses and Speeds ======== */
#include <ti/drivers/I2C.h>

/* ---- CONFIG_I2C_0 I2C bus components ---- */

/* BP-BASSENSORSMKII/TMP117 address and max speed */
#define CONFIG_I2C_0_BP_BASSENSORSMKII_TMP117_ADDR     (0x48)
#define CONFIG_I2C_0_BP_BASSENSORSMKII_TMP117_MAXSPEED (400U) /* Kbps */

/* BP-BASSENSORSMKII/OPT3001 address and max speed */
#define CONFIG_I2C_0_BP_BASSENSORSMKII_OPT3001_ADDR     (0x44)
#define CONFIG_I2C_0_BP_BASSENSORSMKII_OPT3001_MAXSPEED (2600U) /* Kbps */

/* BP-BASSENSORSMKII/HDC2080 address and max speed */
#define CONFIG_I2C_0_BP_BASSENSORSMKII_HDC2080_ADDR     (0x40)
#define CONFIG_I2C_0_BP_BASSENSORSMKII_HDC2080_MAXSPEED (400U) /* Kbps */

/* BP-BASSENSORSMKII/BMI160_BMM150 address and max speed */
#define CONFIG_I2C_0_BP_BASSENSORSMKII_BMI160_BMM150_ADDR     (0x69)
#define CONFIG_I2C_0_BP_BASSENSORSMKII_BMI160_BMM150_MAXSPEED (1000U) /* Kbps */

/* CONFIG_I2C_0 max speed (supported by all components) */
#define CONFIG_I2C_0_MAXSPEED   (400U) /* Kbps */
#define CONFIG_I2C_0_MAXBITRATE ((I2C_BitRate)I2C_400kHz)


/*
 *  ======== PIN ========
 */
#include <ti/drivers/PIN.h>

extern const PIN_Config BoardGpioInitTable[];

/* LaunchPad LED Red, Parent Signal: Board_GPIO_LED0 GPIO Pin, (DIO6) */
#define CONFIG_PIN_2                   0x00000006
/* Parent Signal: Board_GPIO_DIO0_RESET GPIO Pin, (DIO1) */
#define CONFIG_PIN_6                   0x00000001
/* Parent Signal: Board_GPIO_DIO1_MFIO GPIO Pin, (DIO0) */
#define CONFIG_PIN_7                   0x00000000
/* LaunchPad LED Green, Parent Signal: Board_GPIO_LED1 GPIO Pin, (DIO7) */
#define CONFIG_PIN_3                   0x00000007
/* BAS Sensors Mark II I2C, Parent Signal: CONFIG_I2C_0 SDA, (DIO5) */
#define CONFIG_PIN_4                   0x00000005
/* BAS Sensors Mark II I2C, Parent Signal: CONFIG_I2C_0 SCL, (DIO4) */
#define CONFIG_PIN_5                   0x00000004
/* XDS110 UART, Parent Signal: CONFIG_UART_0 TX, (DIO3) */
#define CONFIG_PIN_0                   0x00000003
/* XDS110 UART, Parent Signal: CONFIG_UART_0 RX, (DIO2) */
#define CONFIG_PIN_1                   0x00000002
#define CONFIG_TI_DRIVERS_PIN_COUNT    8


/*
 *  ======== UART ========
 */

/*
 *  TX: DIO3
 *  RX: DIO2
 *  XDS110 UART
 */
extern const uint_least8_t              CONFIG_UART_0_CONST;
#define CONFIG_UART_0                   0
#define CONFIG_TI_DRIVERS_UART_COUNT    1


/*
 *  ======== Board_init ========
 *  Perform all required TI-Drivers initialization
 *
 *  This function should be called once at a point before any use of
 *  TI-Drivers.
 */
extern void Board_init(void);

/*
 *  ======== Board_initGeneral ========
 *  (deprecated)
 *
 *  Board_initGeneral() is defined purely for backward compatibility.
 *
 *  All new code should use Board_init() to do any required TI-Drivers
 *  initialization _and_ use <Driver>_init() for only where specific drivers
 *  are explicitly referenced by the application.  <Driver>_init() functions
 *  are idempotent.
 */
#define Board_initGeneral Board_init

#ifdef __cplusplus
}
#endif

#endif /* include guard */
