//
// CNC Vacuum Pump Controller
//
// See "notes.h" for documentation.
//

// Setup the vacuum high and low default thresholds before including "encoder.h".
// The values are in "inHg * 10".  So, a value of 200 is really 20.0inHg.
int16_t red=200, grn=250;

#include "encoder.h"

// Comment out the next line to build program WITHOUT the LCD / GUI interface.
// With the GUI, the ESP32 will just switch the pump on/off at the threshold
// points.  You will need to recompile the code to change the switch points.
#define USE_GUI

#ifdef USE_GUI
  #include <TFT_eSPI.h>
  #include <lvgl.h>
  #include "src/ui_lvgl/ui.h"

  // Waveshare 1.28in Round LCD
  static const uint16_t screenWidth  = 240;
  static const uint16_t screenHeight = 240;

  static lv_disp_draw_buf_t draw_buf;
  static lv_color_t buf[ screenWidth * 10 ];

  TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); // TFT Instance
  lv_disp_drv_t disp_drv;
  bool guiActive = true;
#else
  bool guiActive = false;
#endif


const int ledPin = 2;             // Debug LED on ESP32 board.
const int vacSensorPin = 35;      // Analog Input Pin for Vacuum Sensor
const int sensorPwrPin = 4;       // Dummy pin provides power for Vac Sensor
const int vacPumpPin = 26;        // Output to Vacuum Pump Relay
const int vacAlarmPin = 21;       // Output to Alarm Relay

// Relays are active low.  Enum makes for a nice way to control them, I think.
enum relayType{ On = 0, Off = 1 };

uint16_t getFilteredVacSensor();
int cmpfunc (const void * a, const void * b);
void printTypes();
void startGUI();

// Timers used with millis().
uint64_t tm, lv_tm1, lv_tm2, lv_tm3, tm4;

#ifdef USE_GUI

  // LCD Display Flushing for LVGL
  // ============================================================================
  void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p ) {
      uint32_t w = ( area->x2 - area->x1 + 1 );
      uint32_t h = ( area->y2 - area->y1 + 1 );

      tft.startWrite();
      tft.setAddrWindow( area->x1, area->y1, w, h );
      tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
      tft.endWrite();

      lv_disp_flush_ready( disp );
  }

  // Use Arduino millis() as tick source.
  static uint32_t my_tick(void) {
      return millis();
  }
#endif

// ============================================================================
void setup() {
    Serial.begin( 115200 );

    pinMode( sensorPwrPin, OUTPUT );      // Dummy 3.3V to Vac Sensor
    digitalWrite( sensorPwrPin, HIGH );
    pinMode( vacSensorPin, INPUT );       // Vac Analog Sensor Input    

    pinMode( ledPin, OUTPUT );            // ESP32 Board LED
    digitalWrite( ledPin, HIGH );

    pinMode( vacPumpPin, OUTPUT );        // Outputs to dual realy board.
    digitalWrite( vacPumpPin, Off );
    pinMode( vacAlarmPin, OUTPUT );
    digitalWrite( vacAlarmPin, Off );

    xTaskCreate(
      encTask,		  // My Simple Encoder Task
      "Encoder",		// Task Name String
      10000,			  // Stack size in bytes.
      NULL,			    // Parameter passed as input of the task.
      1,				    // Priority of the task.  Higher Num = Higher Priority
      NULL);			  // Task handle.

    Serial.printf("\nCNC Vac Pump Controller:\n");
    #ifdef USE_GUI
      startGUI();     // Init and startup GUI display here.
      Serial.printf("\tLVGL: %d.%d.%d\n",
        lv_version_major(), lv_version_minor(), lv_version_patch() );
    #endif
    Serial.printf( "Setup Done\n" );

    // Init the timers so they are staggered.  This keeps them from happening 
    // at the same time.  Also, to keep the stagger working, jump the timer(s) forward 
    // on multiples of 5 milliseconds.
    tm = millis();
    lv_tm1 = tm + 1;
    lv_tm2 = tm + 2;
    lv_tm3 = tm + 3;
    tm4 = tm + 4;
}

// ============================================================================
void loop() {
  static int ang;
  char sVal[30];
  static bool ledFlg = true;
  static int cnt_old;
  static int vacSensor, vacVal;

  #ifdef USE_GUI
    // This should be moved to a high priority FreeRTOS task.
    if( guiActive && lv_tm1 < millis() ) {      
      lv_tick_inc( 5 );           // Update GUI animations.
      lv_tm1 += 5;
    }

    if( guiActive && lv_tm2 < millis() ) {
      lv_timer_handler();         // Update GUI every 5ms.
      lv_tm2 += 5;
    }
    
    // Update vacuum label and needle angle.
    if( guiActive && millis() > lv_tm3 ) {
      // lv_tm3 += 45;
      lv_tm3 += 15;
      // int vacSensor = analogRead( VAC_SENSOR );
      // vacSensor = getFilteredVacSensor();
    
      sprintf( sVal, "%0.1f", vacVal / 10.0 );
      lv_label_set_text( ui_vacLabel, sVal );

      ang = map( vacSensor, 0, 4096, 0, 2700 );
      lv_img_set_angle( ui_Needle, ang );

      // Pump On
      if( vacVal <= red )
        lv_obj_clear_flag( ui_ledRedImage, LV_OBJ_FLAG_HIDDEN );

      // Pump Off
      if( vacVal >= grn )
        lv_obj_add_flag( ui_ledRedImage, LV_OBJ_FLAG_HIDDEN );
    }

      // On encoder counts change...
    if( guiActive && cnt != cnt_old ) {
      // Serial.printf("Cnt: %d   Red: %d   Grn: %d\n", cnt, red, grn );
      // In case one bug is getting pushed by the other bug, update both bugs.
      uint16_t redBugAng = map( red, 300, 0, -1350, 1350 );
      lv_img_set_angle( ui_redBugImg, redBugAng );
      uint16_t grnBugAng = map( grn, 300, 0, -1350, 1350 );
      lv_img_set_angle( ui_grnBugImg, grnBugAng );
    }
    cnt_old = cnt;
  #endif

  // Toggle Board Debug LED - just to show some life.
  if( millis() > tm ) {
    tm += 250;
    ledFlg ^= true;                   // Toggle flag.
    digitalWrite( ledPin, ledFlg );
  }

  // Read Vacuum sensor and update the pump and alarm outputs.
  if( tm4 < millis() ) {
    tm4 += 10;
    
    vacSensor = getFilteredVacSensor();
    vacVal = map( vacSensor, 0, 4090, 300, 0 );

    if( vacVal <= red )  digitalWrite( vacPumpPin, On );
    if( vacVal >= grn )  digitalWrite( vacPumpPin, Off );

    // ToDo: Add code to drive the alarm output too.
  }
}



// Size of median filter for incoming analog values.
#define NUM_VAC_VALUES	15
#define NUM_VAC_AVG     15

// Get Filter Vacuum Sensor: Reads a new value from the analog input pis.
// The new value is added to a median filter who's output is then averaged.
// The median filter gets rid of "flyer" values, and then, the averaging
// smooths just a bit.  The downside to all this filtering, of course, is a 
// delay in seeing real changes.  In other words, it takes some time for 
// a large jump in the actual value to percolate through the filters.
// However, in this application, slow moving values are the norm!
// ============================================================================
uint16_t getFilteredVacSensor( ) {
  static uint16_t vacValuesRaw[ NUM_VAC_VALUES ];       // Ring Buffer of Raw Values
  static uint16_t vacValuesSorted[ NUM_VAC_VALUES ];    // Sorted Copy of Ring Buff
  static uint8_t vacValIndex = 0;                       // Current Index into Ring Buffer
	static uint32_t avgVac = 0;                           // Running Average
  
  // Overwritten oldest val in the ring buffer with new analog value.
	vacValuesRaw[ vacValIndex ] = analogRead( vacSensorPin );
	vacValIndex = ++vacValIndex > NUM_VAC_VALUES ? 0 : vacValIndex;   // If needed, wrap index.

	// Copy the unsorted raw values and overwrite the old sorted values.
	memcpy( vacValuesSorted, vacValuesRaw, sizeof(uint16_t) * NUM_VAC_VALUES );

	// QSort the values so we can find the 'median' value in the sorted list.
  // Note, QSort is super fast and efficient - truly magical and worth the effort.
	qsort( vacValuesSorted, NUM_VAC_VALUES, sizeof(uint16_t), cmpfunc);

  // Take the middle value from the sorted array - this is the 'median' value.
  // Compute an average of that median value to smooth it.
	avgVac = (avgVac * (NUM_VAC_AVG - 1) + vacValuesSorted[ NUM_VAC_VALUES / 2 ]) / NUM_VAC_AVG;
	return avgVac;
}

// Used by the QSort function to compare two values.
// ============================================================================
int cmpfunc (const void * a, const void * b) {
	if( *(int*)a > *(int*)b ) return 1;
	else if( *(int*)a == *(int*)b ) return 0;
	else return -1;
}

// Everything needed to startup the LCD display using TFT_eSPI and LVGL.
// ============================================================================
#ifdef USE_GUI
void startGUI( ) {

  // If USE_GUI is not defined then this function does nothing.
    lv_init();

    tft.begin();            // TFT Init
    tft.setRotation( 0 );   // No landscape or flipped needed.

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

    // Initialize the display
    lv_disp_drv_init( &disp_drv );
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    ui_init();
}
#endif

// // Debugging Stuff...
// // Dump how many bytes it takes to store each type on an ESP32.  Note, the 'int'
// // and 'long' use 32bits.  That's because the ESP32 is a 32bit processor so there
// // is no bonus using two bytes for an 'int'.  Also note, 'double' and 'long double'
// // are both eight bytes, which seems strange.
// // ============================================================================
// void printTypes() {
//     Serial.println("ESP32 Type Sizes:");                                  // Answers:
//     Serial.print("bool ");           Serial.println(sizeof(bool));        // 1
//     Serial.print("char ");           Serial.println(sizeof(char));        // 1
//     Serial.print("int ");            Serial.println(sizeof(int));         // 4
//     Serial.print("long ");           Serial.println(sizeof(long));        // 4
//     Serial.print("long long ");      Serial.println(sizeof(long long));   // 8
//     Serial.print("float ");          Serial.println(sizeof(float));       // 4
//     Serial.print("double ");         Serial.println(sizeof(double));      // 8
//     Serial.print("long double ");    Serial.println(sizeof(long double)); // 8
// }