// ----------------------------------------------------------------------------
//                CNC Vacuum Pump Controller
//
// By: Jim Kemp                                 Last Updated: July 30th, 2025
// ----------------------------------------------------------------------------

// Hardware:
// ============================================================================
//    ESP32 - WeMos D1 Mini32
//        Waveshare 1.28in Round LDC 240x240 pixels
//          See: Jessinie (JXI) Part Num -> B0CB3V1366 on Amazon
//        Rotary Encoder Knob with A/B Channel + Push Button
//        NXP MPXV4115 Vacuum Sensor with Analog Output
//        See below for wiring instructions.
//
// Software:
// ============================================================================
//    Arduino IDE 2.3.4. Board Manager: ESP32 3.2.0
//        TFT_eSPI: 2.5.43 / LVGL: 8.3.11
//
//        Arduino IDE Setup for ESP32 Module
//        ------------------------------------------
//        Board: ESP32 Dev Module
//        CPU Freq: 240MHz (WiFi/BT)
//        Core Debug Level: "None"
//        Erase All Flash Before...: "Enabled"
//        Events Run On: "Core 1"
//        Flash Frequency: "80MHz"
//        Flash Mode: "DIO"
//        Flash Size: "4MB (32Mb)"
//        JTAG Adapter: "Disabled"
//        Arduino Runs On: "Core 1"
//        Partition Scheme: "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS"
//        PSRAM: "Disabled"
//        Upload Speed: "921600"
//        Zigbee Mode: "Disabled"
//
//        See below for more software setup tasks YOU must do!




////////////////////////////////////////////////////
//   You MUST read and DO the following...        //
//      You have been warned.                     //
////////////////////////////////////////////////////


// See "User_Setup.h" in TFT_eSPI library directory.  This file defines
// the hardware interface to the LCD.  Settings in "User_Setup.h" should 
// be set as follows:
//
// #define GC9A01_DRIVER
// #define TFT_MISO  19    // Not Used - But must be defined.
// #define TFT_SCLK  18
// #define TFT_MOSI  23    // To Din on LCD
// #define TFT_CS    5     //
// #define TFT_RST   16    // Set TFT_RST to -1 if the display RESET is connected to RST or 3.3V.
// #define TFT_DC    17    // DC (Data or Command Pin)
// #define TFT_BL    21    // Backlight (could use PWM to dim LCD, I think)
// #define TOUCH_CS -1	   // -1 Means: No Touch Screen
//

// See "User_Setup_Select.h" in TFT_eSPI library directory.
//    The ONLY user setup that should be uncommented is for GC9A01 which
//    must be done in "User_Setup_Select.h" in the TFT_eSPI library directory.
//
// #include <User_Setups/Setup46_GC9A01_ESP32.h>      // Setup file for ESP32 and GC9A01 SPI bus TFT  240x240
//


//  Hardware Wiring Connections...
//


// Cable between ESP32 & JXI 1.28in Round LCD
//    Note: Wire colors set by pigtail included with LCD module.
// ====================================
// Color -> Function
// Blk -> GND
// Red -> Vcc 3.3V
// Wht -> Clk GPIO18
// Yel -> Din GPIO23 (MOSI)
// Org -> RST GPIO16
// Grn -> DC  GPIO17
// Blu -> CS  GPIO5
// Pur -> BL  GPIO21

// Rotary Encoder
// ====================================
// Push Button  -> GPIO27   Enable PullUp
// Channel A    -> GPIO25   Enable PullUp
// Channel B    -> GPIO32   Enable PullUp
//

// Analog Vacuum Sensor from NXP: MPXV4115
// ============================================================================
//  Analog signal from the sensor is applied to a resistor divider.  The analog
// signal connects to a 5k in series with a 10k.  The divider is then applied to
// GPIO35.  The max output from the sensor is 4.6V which gets divided down to 
// 3.0V before connecting to the ESP32 GPIO35 pin.  This keeps the analog value 
// in the linear range of the ESP32.


// Not Used / For reference
// Here is the setup when using the LCD from Waveshare.  The current design uses 
// the JXI LCD as documented above.  The Waveshare uses different colors and a 
// different ordering of pins.  Otherwise, the LCDs are the same and interchangeable. 
//    Note: Wire colors set by pigtail included with LCD module.
// ====================================
// Color -> Function
// Pur -> Vcc 3.3V
// Wht -> GND
// Grn -> Din GPIO23 (MOSI)
// Org -> Clk GPIO18
// Yel -> CS  GPIO5
// Blu -> DC  GPIO17
// Brn -> RST GPIO16
// Gry -> BL  GPIO21



// General Notes:
// ============================================================================
//
// When reading LVGL docs on their web site - make sure to set the displayed 
// revision level back to "8.3".  A lot has changed in the new versions.  Use
// the drop-down, top left-hand corner, to select the version to display.  Here
// is a link directly to version 8.3:  https://docs.lvgl.io/8.3/index.html
//
// There is an irritating warning coming out of a LVGL function in 
// "lv_obj_style.h".  My fix was to add the two (uint64_t) casts as shown below.
// This fixes the compiler warning.  See: https://github.com/lvgl/lvgl/issues/5119
//
// static inline void lv_obj_remove_style_all(struct _lv_obj_t * obj)
// {
//     // Original Code:
//     // lv_obj_remove_style(obj, NULL, LV_PART_ANY | LV_STATE_ANY );
//
//     // Fixed (Patched) Code:
//     lv_obj_remove_style(obj, NULL, ((uint64_t)LV_PART_ANY) | ((uint64_t)LV_STATE_ANY) );
// }
//



