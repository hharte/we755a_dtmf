/*************************************************************************
 *                                                                       *
 * Copyright (c) 2023 Howard M. Harte, WZ2Q.                             *
 * https://github.com/hharte/we755a_dtmf                                 *
 *                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining *
 * a copy of this software and associated documentation files (the       *
 * "Software"), to deal in the Software without restriction, including   *
 * without limitation the rights to use, copy, modify, merge, publish,   *
 * distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to *
 * the following conditions:                                             *
 *                                                                       *
 * The above copyright notice and this permission notice shall be        *
 * included in all copies or substantial portions of the Software.       *
 *                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 *
 * NONINFRINGEMENT. IN NO EVENT SHALL HOWARD M. HARTE BE LIABLE FOR ANY  *
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  *
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     *
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                *
 *                                                                       *
 * Except as contained in this notice, the name of Howard M. Harte shall *
 * not be used in advertising or otherwise to promote the sale, use or   *
 * other dealings in this Software without prior written authorization   *
 * Howard M. Harte.                                                      *
 *                                                                       *
 * Module Description:                                                   *
 * Western Electric 755A Crossbar PBX DTMF to Pulse Counting Relay       *
 * Converter                                                             *
 *                                                                       *
 * Arduino 1.8.57.0 or later with DxCore                                 *
 *                                                                       *
 *************************************************************************/

#define BAUD_RATE           115200
#define RELAY_SETTLE_DELAY  300   /* Delay for relays to settle, in ms. */
#define MS_PER_TICK         20
#define RELAY_HOLD_MS       200
#define POST_DIAL_MS        100
#define COUNT_PER_MS        93.6
#define RELAY_HOLD_COUNT    (RELAY_HOLD_MS / MS_PER_TICK)
#define POST_DIAL_COUNT     (POST_DIAL_MS / MS_PER_TICK)

#define DIAL_STATE_IDLE             0
#define DIAL_STATE_WAIT_DTMF_DONE   1
#define DIAL_STATE_OPERATE_TENS     2
#define DIAL_STATE_OPERATE_UNITS    3
#define DIAL_STATE_OPERATE_C1       4
#define DIAL_STATE_DIALING_COMPLETE 5

/* Pin assignments:
 * ref: https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/DA28.md
 */

/* Inputs from MT8870 DTMF Receiver */
#define DTMF_Q0           0   /* PA0 */
#define DTMF_Q1           1   /* PA1 */
#define DTMF_Q2           2   /* PA2 */
#define DTMF_Q3           3   /* PA3 */
#define DTMF_StD          6   /* PA6 */

/* Outputs: Solid-State Relays driving 755A link relays, Low=On. */
#define SSR_T2            8   /* PC0 */
#define SSR_T3            9   /* PC1 */
#define SSR_C1            10  /* PC2 */

#define SSR_1_7           12  /* PD0 */
#define SSR_2_8           13  /* PD1 */
#define SSR_3_9           14  /* PD2 */
#define SSR_4_0           15  /* PD3 */
#define SSR_5_6           16  /* PD4 */
#define SSR_6             17  /* PD5 */
#define SSR_SPARE         19  /* PD7 */

#define SSR_1_7_B         (1 << 0)  /* PD0 */
#define SSR_2_8_B         (1 << 1)  /* PD1 */
#define SSR_3_9_B         (1 << 2)  /* PD2 */
#define SSR_4_0_B         (1 << 3)  /* PD3 */
#define SSR_5_6_B         (1 << 4)  /* PD4 */
#define SSR_6_B           (1 << 5)  /* PD5 */
#define SSR_SPARE_B       (1 << 7)  /* PD7 */

/* DTMF Digit to 755A Pulse Counting Relay Mapping */
const unsigned char units_relay_table[16] = {
    0,                    // D (Not valid)
    SSR_1_7_B,            // 1
    SSR_2_8_B,            // 2
    SSR_3_9_B,            // 3
    SSR_4_0_B,            // 4
    SSR_5_6_B,            // 5
    SSR_5_6_B | SSR_6_B,  // 6
    SSR_1_7_B | SSR_6_B,  // 7
    SSR_2_8_B | SSR_6_B,  // 8
    SSR_3_9_B | SSR_6_B,  // 9
    SSR_4_0_B | SSR_6_B,  // 0
    0,                    // * (Not valid)
    0,                    // # (Not valid)
    0,                    // A (Not valid)
    0,                    // B (Not valid)
    0                     // C (Not valid)
};

/* Inputs from 756A Link */
#define LINK_ACTIVE       18  /* PD6 */

/* Output: Status LED, Low=On */
#define STATUS_LED        11  /* PIN_PC3 */

#define STATUS_LED_B      (1 << 3)

const char          menu[] = "\n\rCommands:\n\r" \
                             "    w   - Dial 2x (tens digit)\n\r" \
                             "    h   - Dial 3x (tens digit)\n\r" \
                             "    0-9 - Dial (units digit)\n\r" \
                             "    a-c - Activate T2, T3, C1\n\r" \
                             "    d-g - Activate 1-7, 2-8, 3-9, 4-0\n\r" \
                             "    i-k - Activate 5-6, 6, Spare\n\r" \
                             "    r   - Release all relays\n\r";

typedef struct dtmf_event_record {
    uint32_t timestamp;
    uint16_t dtmf_duration;
    uint8_t dtmf_digit;
} dtmf_event_record_t;

typedef struct {
    uint8_t r_index;
    uint8_t w_index;
    dtmf_event_record_t events[8];
} dtmf_events_t;

dtmf_events_t dtmf_events;

volatile uint8_t dtmf_digit = 0;            /* DTMF Digit read by ISR */
uint8_t dtmf_digits[2] = { 0, 0 };          /* Storage for two dialed digits */
uint8_t digits_collected = 0;               /* Number of digits collected so far. */
bool link_active;
bool link_updated = false;
bool dtmf_updated = false;
bool pet_watchdog = true;
volatile uint32_t timer_tick = 0;
volatile uint8_t dtmf_dial_state = DIAL_STATE_IDLE;
static uint8_t dtmf_delay = 0;
unsigned long millis1, millis2, timedelta = 0;
char outstr[255];

#define wdt_reset() __asm__ __volatile__ ("wdr"::)  /* reset watchdog timer. */

/* Runs once during boot. */
void setup(void) {

  /* Initialize serial port */
  Serial.swap(1); /* Use PA4/PA5 for UART */
  Serial.begin(BAUD_RATE);

  VPORTC.OUT = 0xFF;
  VPORTD.OUT = 0xFF;

  /* Initialize GPIO Pins */
  pinMode (SSR_T2,      OUTPUT);
  pinMode (SSR_T3,      OUTPUT);
  pinMode (SSR_C1,      OUTPUT);
  pinMode (SSR_1_7,     OUTPUT);
  pinMode (SSR_2_8,     OUTPUT);
  pinMode (SSR_3_9,     OUTPUT);
  pinMode (SSR_4_0,     OUTPUT);
  pinMode (SSR_5_6,     OUTPUT);
  pinMode (SSR_6,       OUTPUT);
  pinMode (SSR_SPARE,   OUTPUT);
  pinMode (STATUS_LED,  OUTPUT);

  pinMode (DTMF_StD,    INPUT);
  pinMode (DTMF_Q0,     INPUT);
  pinMode (DTMF_Q1,     INPUT);
  pinMode (DTMF_Q2,     INPUT);
  pinMode (DTMF_Q3,     INPUT);
  pinMode (LINK_ACTIVE, INPUT);

  takeOverTCA0();
  TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
  TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
  TCA0.SINGLE.PER = MS_PER_TICK * COUNT_PER_MS;
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; /* Enable timer overflow interrupt */
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm;

  dtmf_events.w_index = 0;
  dtmf_events.r_index = 0;
  memset(&dtmf_events, 0, sizeof(dtmf_events));

  attachInterrupt(digitalPinToInterrupt(LINK_ACTIVE), link_isr,   CHANGE);
  attachInterrupt(digitalPinToInterrupt(DTMF_StD),    DTMF_isr,   RISING);

  Serial.print("Western Electric 755A Crossbar PBX DTMF Converter\r\n");
  Serial.print("(c) 2023 Howard M. Harte - github.com/hharte/we755a_dtmf\r\n");

  snprintf(outstr, sizeof(outstr), "\r\nWE755A> ");
  Serial.print(outstr);
}

/* Main loop: Process serial console input */
void loop() {
  uint8_t choice;

  if (pet_watchdog) {
    wdt_reset();  /* Pet watchdog timer. */
  }

  if (Serial.available() > 0) {
    /* Read character from the serial port. */
    choice = Serial.read();
    Serial.write(choice);

    switch(choice) {
      case '?':
        Serial.print(menu);
        break;
      case 'w':
      case 'W':
        dtmf_digits[0] = 2;
        dtmf_digits[1] = 0;
        dtmf_dial_state = DIAL_STATE_OPERATE_TENS;
        VPORTC.OUT &= ~STATUS_LED_B;
        Serial.print("\r\nDial 2 (tens)\r\n");
        while (dtmf_dial_state != DIAL_STATE_IDLE);
        dtmf_updated = 0;
        break;
      case 'h':
      case 'H':
        dtmf_digits[0] = 3;
        dtmf_digits[1] = 0;
        dtmf_dial_state = DIAL_STATE_OPERATE_TENS;
        VPORTC.OUT &= ~STATUS_LED_B;
        Serial.print("\r\nDial 3 (tens)\r\n");
        while (dtmf_dial_state != DIAL_STATE_IDLE);
        dtmf_updated = 0;
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        uint8_t r = choice - '0';

        sprintf(outstr, "\r\nDial %d (units)\r\n", r);
        Serial.print(outstr);
        if (r == 0) r = 10; /* '0' is 10 for the 8870. */
        dtmf_digits[1] = r;
        dtmf_dial_state = DIAL_STATE_OPERATE_UNITS;
        VPORTC.OUT &= ~STATUS_LED_B;
        while (dtmf_dial_state != DIAL_STATE_IDLE);
        break;
      }
      case 'a':
      case 'A':
      {
        digitalWrite(SSR_T2, LOW);
        Serial.print("\r\Activate T2\r\n");
        break;
      }
      case 'b':
      case 'B':
      {
        digitalWrite(SSR_T3, LOW);
        Serial.print("\r\Activate T3\r\n");
        break;
      }
      case 'c':
      case 'C':
      {
        digitalWrite(SSR_C1, LOW);
        Serial.print("\r\Activate C1\r\n");
        break;
      }
      case 'd':
      case 'D':
      {
        digitalWrite(SSR_1_7, LOW);
        Serial.print("\r\Activate 1-7\r\n");
        break;
      }
      case 'e':
      case 'E':
      {
        digitalWrite(SSR_2_8, LOW);
        Serial.print("\r\Activate 2-8\r\n");
        break;
      }
      case 'f':
      case 'F':
      {
        digitalWrite(SSR_3_9, LOW);
        Serial.print("\r\Activate 3-9\r\n");
        break;
      }
      case 'g':
      case 'G':
      {
        digitalWrite(SSR_4_0, LOW);
        Serial.print("\r\Activate 4-0\r\n");
        break;
      }
      case 'i':
      case 'I':
      {
        digitalWrite(SSR_5_6, LOW);
        Serial.print("\r\Activate 5-6\r\n");
        break;
      }
      case 'j':
      case 'J':
      {
        digitalWrite(SSR_6, LOW);
        Serial.print("\r\Activate 6\r\n");
        break;
      }
      case 'k':
      case 'K':
      {
        digitalWrite(SSR_SPARE, LOW);
        Serial.print("\r\Activate Spare SSR\r\n");
        break;
      }
      case 'r':
      case 'R':
      {
        digitalWrite(SSR_T2,    HIGH);
        digitalWrite(SSR_T3,    HIGH);
        digitalWrite(SSR_C1,    HIGH);
        digitalWrite(SSR_1_7,   HIGH);
        digitalWrite(SSR_2_8,   HIGH);
        digitalWrite(SSR_3_9,   HIGH);
        digitalWrite(SSR_4_0,   HIGH);
        digitalWrite(SSR_5_6,   HIGH);
        digitalWrite(SSR_6,     HIGH);
        digitalWrite(SSR_SPARE, HIGH);
        Serial.print("\r\Released all relays\r\n");
        break;
      }

      default:
        Serial.print("\r\nInvalid choice.\r\n");
        break;
    }

    snprintf(outstr, sizeof(outstr), "\r\nWE755A> ");
    Serial.print(outstr);
  }

  if (dtmf_updated == 1) {
    sprintf(outstr, "Dialing x%d%d\r\n", dtmf_digits[0], (dtmf_digits[1] == 10) ? 0 : dtmf_digits[1]);
    dtmf_updated = 0;
    Serial.print(outstr);
  }
  if (link_updated == 1) {
    sprintf(outstr, "Link changed to %s\r\n", link_active == 1 ? "ACTIVE" : "IDLE");
    link_updated = 0;
    Serial.print(outstr);
  }

#if 0
  while (dtmf_events.r_index != dtmf_events.w_index) {
    if (dtmf_events.events[dtmf_events.r_index].timestamp == 0) continue;
    printf("[%lu: '%d' %dms index: %d\n\r", dtmf_events.events[dtmf_events.r_index].timestamp,
            dtmf_events.events[dtmf_events.r_index].dtmf_digit,
            dtmf_events.events[dtmf_events.r_index].dtmf_duration,
            dtmf_events.r_index);
    dtmf_events.r_index++;
    if (dtmf_events.r_index >= 8) {
        dtmf_events.r_index = 0;
    }
  }
#endif // 0
}

/* Process low to high transitions on the MT8870 StD input. */
void DTMF_isr(void) {

  if (digitalRead(DTMF_StD) == 1) {
    VPORTC.OUT &= ~STATUS_LED_B;
    dtmf_digit = VPORTA.IN & 0x0F;
    dtmf_events.events[dtmf_events.w_index].timestamp = timer_tick;
  }
}

/* Process high to low transitions on the LINK_ACTIVE line. */
void link_isr(void) {
  bool link_prev = link_active;
  if (digitalRead(LINK_ACTIVE) == 0) {
    digits_collected = 0;
    link_active = true;
  } else {
    link_active = false;
  }

  if (link_prev != link_active) {
    link_updated = true;
  }
}

/* TCA0 Timer Tick Interrupt */
ISR(TCA0_OVF_vect) {
  TCA0.SINGLE.INTFLAGS  = TCA_SINGLE_OVF_bm; /* Clear interrupt flags */
  timer_tick += 1;

  millis2 = millis1;
  millis1 = millis();
  timedelta = millis1 - millis2;

#ifdef BLINK_LED
  if (timer_tick & 0x40) {
    VPORTC.OUT &= ~STATUS_LED_B;
  } else {
    VPORTC.OUT |= STATUS_LED_B;
  }
#endif /* BLINK_LED */

  switch(dtmf_dial_state) {
    case DIAL_STATE_IDLE:
      /* Waiting for DTMF from Link. */
      VPORTC.OUT |= STATUS_LED_B;

      if (dtmf_digit > 0) {
          dtmf_dial_state = DIAL_STATE_WAIT_DTMF_DONE;
          dtmf_digits[digits_collected] = dtmf_digit;
          if (digits_collected == 0) {
            switch (dtmf_digit) {
              case 10:
              case 2:
              case 3:
              case 8:
              case 9:
                /* Operate T2 to break dialtone. */
                digitalWrite(SSR_T2, LOW);
                break;
              default:
                break;
            }
          }
          dtmf_digit = 0;
          dtmf_events.events[dtmf_events.w_index].dtmf_digit = dtmf_digits[digits_collected];
          dtmf_events.events[dtmf_events.w_index].dtmf_duration = 0;
      }
      break;
    case DIAL_STATE_WAIT_DTMF_DONE:
      /* Wait in this state until DTMFx_StD is de-asserted. */
      dtmf_events.events[dtmf_events.w_index].dtmf_duration += 20;

      if (digitalRead(DTMF_StD) == 0) {
        digits_collected++;
        VPORTC.OUT &= ~STATUS_LED_B;
        if (digits_collected == 1) {
          digitalWrite(SSR_T2, HIGH);
          switch (dtmf_digits[0]) {
            case 10: /* Operator */
              dtmf_digits[0] = 2;
              dtmf_digits[1] = 10;
              digits_collected = 2;
              break;
            case 2: /* STA 2x */
            case 3: /* STA 3x */
              dtmf_dial_state = DIAL_STATE_IDLE;
              break;
            case 8: /* Tie Lines: x26/x27 */
              dtmf_digits[0] = 2;
              dtmf_digits[1] = 6;
              digits_collected = 2;
              break;
            case 9: /* Tie Lines: x36/x37 */
              dtmf_digits[0] = 3;
              dtmf_digits[1] = 6;
              digits_collected = 2;
              break;
            case 1: /* Ignore these digits... */
            case 4:
            case 5:
            case 6:
            case 7:
            default:
              dtmf_digits[0] = 0;
              digits_collected = 0;
              dtmf_dial_state = DIAL_STATE_IDLE;
          }
        }
        if (digits_collected == 2) {
          dtmf_dial_state = DIAL_STATE_OPERATE_TENS;
        }
      }
      break;
    case DIAL_STATE_OPERATE_TENS:
      dtmf_delay++;
      if (dtmf_delay == POST_DIAL_COUNT) {
        dtmf_delay = 0;
        dtmf_updated = 1;

        dtmf_events.w_index ++;
        if (dtmf_events.w_index == 8) {
            dtmf_events.w_index = 0;
        }

        /* Operate the 755A Tens Relays T2, T3 */
        if ((dtmf_digits[0] == 2) || (dtmf_digits[0] == 3)) {
          if (dtmf_digits[0] >= 2) {
            digitalWrite(SSR_T2, LOW);
          }
          if (dtmf_digits[0] == 3) {
            digitalWrite(SSR_T3, LOW);
          }
        } else {
          dtmf_dial_state = DIAL_STATE_IDLE;
        }

        dtmf_dial_state = DIAL_STATE_OPERATE_UNITS;
      }
      break;
    case DIAL_STATE_OPERATE_UNITS:
      /* Wait for relays to be active for 300ms */
      dtmf_delay++;
      if (dtmf_delay == RELAY_HOLD_COUNT) {
        dtmf_delay = 0;
        /* Release T2 and T3 relays */
        digitalWrite(SSR_T2, HIGH);
        digitalWrite(SSR_T3, HIGH);

        if (dtmf_digits[1] > 0) {
          /* Operate C1 relay */
          digitalWrite(SSR_C1, LOW);
          /* Operate Units relays */
          VPORTD.OUT &= ~(units_relay_table[dtmf_digits[1]]);
          dtmf_dial_state = DIAL_STATE_OPERATE_C1;
        } else {
          dtmf_dial_state = DIAL_STATE_IDLE;
        }
      }
      break;
    case DIAL_STATE_OPERATE_C1:
      dtmf_delay++;
      if (dtmf_delay == RELAY_HOLD_COUNT) {
        dtmf_delay = 0;
        if (dtmf_digits[1] > 0) {
          /* Release units relays */
          VPORTD.OUT |= units_relay_table[dtmf_digits[1]];

          /* Operate C1 relay */
          digitalWrite(SSR_C1, LOW);
          dtmf_dial_state = DIAL_STATE_DIALING_COMPLETE;
        } else {
          dtmf_dial_state = DIAL_STATE_IDLE;
        }
      }
      break;
    case DIAL_STATE_DIALING_COMPLETE:
      dtmf_delay++;
      if (dtmf_delay == RELAY_HOLD_COUNT) {
        dtmf_delay = 0;
        digitalWrite(SSR_C1, HIGH);
        dtmf_dial_state = DIAL_STATE_IDLE;
        digits_collected = 0;
        dtmf_digits[0] = 0;
        dtmf_digits[1] = 0;
        dtmf_events.w_index++;
        if (dtmf_events.w_index == 8) {
          dtmf_events.w_index = 0;
        }
      }
      break;
    default: /* Invalid state, go back to idle. */
      dtmf_dial_state = DIAL_STATE_IDLE;
      break;
  }
}
