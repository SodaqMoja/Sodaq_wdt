#include "Sodaq_wdt.h"

#ifdef ARDUINO_ARCH_AVR

#define DIAG_STREAM Serial

#elif ARDUINO_ARCH_SAMD

#define DIAG_STREAM SerialUSB

#endif

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
  sodaq_wdt_enable(WDT_PERIOD_2X);

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
  }
}

