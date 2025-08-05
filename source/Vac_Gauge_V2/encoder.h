
#define RED_BUG 1
#define GRN_BUG 2

#define MAX_CNT	295
#define MIN_CNT 5

// Define GPIO used by the encoder knob.
const int enc_SW = 27;
const int enc_A = 32;
const int enc_B = 25;

uint8_t activeBug;
int16_t cnt;

// FreeRTOS Task: 
// 	Scan the encoder input pins for rotation or button presses.
//  Also, keep the setpoint bug values updated.  Note, the green bug is always
// above the red bug.  And the red bug is always below the green bug.  So, when
// moving a bug, the other bug is "pushed" if needed.
// ============================================================================
void encTask( void * parameter ) {
	int16_t cnt_old;
	bool a, b, sw, sw_old=LOW, cnt_lck=false;

  // Setup encoder knob GPIO pins.
	pinMode( enc_A, INPUT_PULLUP );		
	pinMode( enc_B, INPUT_PULLUP );
	pinMode( enc_SW, INPUT_PULLUP );

  activeBug = GRN_BUG;
  cnt = cnt_old = grn;

	while( true ) {
		a = digitalRead(enc_A);		  // Encoder: A Channel
		b = digitalRead(enc_B);		  //          B
		sw = digitalRead(enc_SW);	  //          Switch Input

		// Count up on falling edge of A with B still true.
		if( !a && b ) {
			if( !cnt_lck ) {
				if( cnt < MAX_CNT ) cnt += 5;
				if( cnt > MAX_CNT ) cnt = MAX_CNT;
				cnt_lck = true;
			}
		}

		// Count down on falling edge of B with A still true.
		if( a && !b ) {
			if( !cnt_lck ) {
				if( cnt > MIN_CNT ) cnt -= 5;
				if( cnt < MIN_CNT ) cnt = MIN_CNT;
				cnt_lck = true;
			}
		}

		// Lockout any further counts until both
		// A and B return back to true.  Thus, we only
		// count on falling edges.
		if( a && b )
			cnt_lck = false;

		// Switch active bug on switch press.
		if( (sw == LOW) && (sw_old == HIGH) ) {
			if( ++activeBug > GRN_BUG ) {
				activeBug = RED_BUG;
				cnt = red;
			}
			else {
				cnt = grn;
			}
      // Serial.printf("Pb: %s -> %d\n", activeBug == RED_BUG ? "Red" : "Grn", cnt );
		} 
		sw_old = sw;

    // Sync counter into active bug.
    if( activeBug == RED_BUG ) red = cnt;
    if( activeBug == GRN_BUG ) grn = cnt;

    // Push the inactive bug value, if needed.
    if( activeBug == RED_BUG && red >= grn ) grn = red + 5;
    if( activeBug == GRN_BUG && grn <= red ) red = grn - 5;

		// Print count on change.
		if( cnt != cnt_old )
  			Serial.printf("Cnt: %d   Red: %d   Grn: %d\n", cnt, red, grn );

		cnt_old = cnt;
		delay( 10 );
	}
}
