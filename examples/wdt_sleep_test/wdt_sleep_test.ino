#include "Sodaq_wdt.h"

#ifdef ARDUINO_ARCH_AVR

#include <avr/sleep.h>
#define DIAG_STREAM Serial
#define SLEEP_LED LED1
#define WDT_LED LED2

#elif ARDUINO_ARCH_SAMD

#define DIAG_STREAM SerialUSB
#define SLEEP_LED LED_BUILTIN
#define WDT_LED 4

#endif

// Length of LED flash
#define LED_FLASH_MS 100

void setup() 
{
  // Startup delay
  delay(5000);
  
  // Supported periods on both platforms
  // WDT_PERIOD_1DIV64 = 1/64s
  // WDT_PERIOD_1DIV32 = 1/32s
  // WDT_PERIOD_1DIV16 = 1/16s
  // WDT_PERIOD_1DIV8  = 1/8s
  // WDT_PERIOD_1DIV4  = 1/4s
  // WDT_PERIOD_1DIV2  = 1/2s
  // WDT_PERIOD_1X     = 1s
  // WDT_PERIOD_2X     = 2s
  // WDT_PERIOD_4X     = 4s
  // WDT_PERIOD_8X     = 8s
  // Default parameter is set to WDT_PERIOD_1X = 1s

  // Enable WDT
  sodaq_wdt_enable(WDT_PERIOD_1X);

  // Setup LED's IO pins
  pinMode(SLEEP_LED, OUTPUT);
  pinMode(WDT_LED, OUTPUT);

  // Startup message (also seen after a reset)
  DIAG_STREAM.begin(57600);
  DIAG_STREAM.println("Start up");
}

void loop() 
{
  // If the WDT interrupt has been triggered
  if (sodaq_wdt_flag) {
    sodaq_wdt_flag = false;
    sodaq_wdt_reset();
    
    DIAG_STREAM.println("WDT interrupt has been triggered");

    // Flash LED
    digitalWrite(WDT_LED, HIGH);
    sodaq_wdt_safe_delay(LED_FLASH_MS);
    digitalWrite(WDT_LED, LOW);
  }

  // Try and enter sleep mode
  systemSleep();
}

void systemSleep()
{
#ifdef ARDUINO_ARCH_AVR

  // Wait till the output has been transmitted
  DIAG_STREAM.flush();
 
  ADCSRA &= ~_BV(ADEN);         // ADC disabled

  /*
  * Possible sleep modes are (see sleep.h):
  #define SLEEP_MODE_IDLE         (0)
  #define SLEEP_MODE_ADC          _BV(SM0)
  #define SLEEP_MODE_PWR_DOWN     _BV(SM1)
  #define SLEEP_MODE_PWR_SAVE     (_BV(SM0) | _BV(SM1))
  #define SLEEP_MODE_STANDBY      (_BV(SM1) | _BV(SM2))
  #define SLEEP_MODE_EXT_STANDBY  (_BV(SM0) | _BV(SM1) | _BV(SM2))
  */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  /*
   * This code is from the documentation in avr/sleep.h
   */
  cli();
  // Only go to sleep if there was no watchdog interrupt.
  if (!sodaq_wdt_flag)
  {
    // Power on LED before sleep
    digitalWrite(SLEEP_LED, HIGH);
    
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();

    // Power off LED after sleep
    digitalWrite(SLEEP_LED, LOW);
    sodaq_wdt_safe_delay(LED_FLASH_MS);
  }
  sei();

  ADCSRA |= _BV(ADEN);          // ADC enabled
  
#elif ARDUINO_ARCH_SAMD

  // Only go to sleep if there was no watchdog interrupt.
  if (!sodaq_wdt_flag)
  {
    // Power on LED before sleep
    digitalWrite(SLEEP_LED, HIGH);
    
    // Disable USB
    USBDevice.detach();

    // Set the sleep mode
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    
    // SAMD sleep
    __WFI();
    
    // Power off LED after sleep
    digitalWrite(SLEEP_LED, LOW);
    sodaq_wdt_safe_delay(LED_FLASH_MS);
  }
  
#endif
}

