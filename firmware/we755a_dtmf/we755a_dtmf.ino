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
 * Converter.                                                            *
 *                                                                       *
 * Receives DTMF digits and activates T2/T3 and pulse counting relays    *
 * to their terminal count for the digit dialed.  This circuit's ground  *
 * comes from the Link B relay so that the converter is powered off when *
 * the link is idle.  This helps discriminate between first and second   *
 * dialed digits.  Additionally, the link E relay is monitored to        *
 * determine when it is ok to allow dialing again for conference calls.  *
 * The Link E relay is activated when the called station answers.  This  *
 * takes the state machine out of DIAL_STATE_DIALING_BLOCKED, returning  *
 * it to DIAL_STATE_IDLE so that further dialing is allowed.  In the     *
 * DIAL_STATE_IDLE state, dialing is inhibited if the E relay is active, *
 * which indicates that the called party is off-hook.                    *
 *                                                                       *
 * Arduino 1.8.57.0 or later with DxCore                                 *
 *                                                                       *
 *************************************************************************/

#define BAUD_RATE           115200
#define MS_PER_TICK         20
#define RELAY_HOLD_MS       100
#define POST_DIAL_MS        100
#define C1_DELAY_MS         60
#define COUNT_PER_MS        93.6
#define RELAY_HOLD_COUNT    (RELAY_HOLD_MS / MS_PER_TICK)
#define POST_DIAL_COUNT     (POST_DIAL_MS / MS_PER_TICK)
#define C1_DELAY_COUNT      (C1_DELAY_MS / MS_PER_TICK)

#define DIAL_STATE_IDLE             0
#define DIAL_STATE_WAIT_DTMF_DONE   1
#define DIAL_STATE_OPERATE_C1       2
#define DIAL_STATE_OPERATE_RELAYS   3
#define DIAL_STATE_DIALING_COMPLETE 4
#define DIAL_STATE_DIALING_BLOCKED  5

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
    0,                    /* D (Not valid) */
    SSR_1_7_B,            /* 1 */
    SSR_2_8_B,            /* 2 */
    SSR_3_9_B,            /* 3 */
    SSR_4_0_B,            /* 4 */
    SSR_5_6_B,            /* 5 */
    SSR_5_6_B | SSR_6_B,  /* 6 */
    SSR_1_7_B | SSR_6_B,  /* 7 */
    SSR_2_8_B | SSR_6_B,  /* 8 */
    SSR_3_9_B | SSR_6_B,  /* 9 */
    SSR_4_0_B | SSR_6_B,  /* 0 */
    0,                    /* * (Not valid) */
    0,                    /* # (Not valid) */
    0,                    /* A (Not valid) */
    0,                    /* B (Not valid) */
    0                     /* C (Not valid) */
};

/* Inputs from 755A Link */
#define LINK_E            18  /* PD6 */

/* Output: Status LED, Low=On */
#define STATUS_LED        11  /* PIN_PC3 */

#define STATUS_LED_B      (1 << 3)

#define LED_ON(x)         VPORTC.OUT &= ~STATUS_LED_B;
#define LED_OFF(x)        VPORTC.OUT |=  STATUS_LED_B;

#define DIAL_IN_PROGRESS(x) \
  ((dtmf_dial_state != DIAL_STATE_IDLE) && \
   (dtmf_dial_state != DIAL_STATE_DIALING_BLOCKED))

const char          menu[] = "\n\rCommands:\n\r" \
                             "    w   - Dial 2x (tens digit)\n\r" \
                             "    h   - Dial 3x (tens digit)\n\r" \
                             "    0-9 - Dial (units digit)\n\r" \
                             "    a-c - Activate T2, T3, C1\n\r" \
                             "    d-g - Activate 1-7, 2-8, 3-9, 4-0\n\r" \
                             "    i-k - Activate 5-6, 6, Spare\n\r" \
                             "    r   - Release all relays\n\r";

volatile uint8_t dtmf_digit = 0;            /* DTMF Digit read by ISR */
uint8_t dtmf_digits[2] = { 0, 0 };          /* Storage for two dialed digits */
uint8_t digits_collected = 0;               /* Number of digits collected so far. */
bool link_e;
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
  pinMode (LINK_E,      INPUT_PULLUP);

  takeOverTCA0();
  TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
  TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
  TCA0.SINGLE.PER = MS_PER_TICK * COUNT_PER_MS;
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; /* Enable timer overflow interrupt */
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm;

  attachInterrupt(digitalPinToInterrupt(LINK_E), link_isr,   CHANGE);
  attachInterrupt(digitalPinToInterrupt(DTMF_StD),    DTMF_isr,   RISING);

  Serial.print("Western Electric 755A Crossbar PBX DTMF Converter v1.0\r\n");
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
        dtmf_dial_state = DIAL_STATE_OPERATE_C1;
        LED_ON();
        Serial.print("\r\nDial 2 (tens)\r\n");
        while (DIAL_IN_PROGRESS());
        LED_OFF();
        dtmf_updated = 0;
        break;
      case 'h':
      case 'H':
        dtmf_digits[0] = 3;
        dtmf_digits[1] = 0;
        dtmf_dial_state = DIAL_STATE_OPERATE_C1;
        LED_ON();
        Serial.print("\r\nDial 3 (tens)\r\n");
        while (DIAL_IN_PROGRESS());
        LED_OFF();
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
        dtmf_dial_state = DIAL_STATE_OPERATE_C1;
        LED_ON();
        while (DIAL_IN_PROGRESS());
        LED_OFF();
        break;
      }
      case 'a':
      case 'A':
      {
        digitalWrite(SSR_T2, LOW);
        Serial.print("\r\nActivate T2\r\n");
        break;
      }
      case 'b':
      case 'B':
      {
        digitalWrite(SSR_T3, LOW);
        Serial.print("\r\nActivate T3\r\n");
        break;
      }
      case 'c':
      case 'C':
      {
        digitalWrite(SSR_C1, LOW);
        Serial.print("\r\nActivate C1\r\n");
        break;
      }
      case 'd':
      case 'D':
      {
        digitalWrite(SSR_1_7, LOW);
        Serial.print("\r\nActivate 1-7\r\n");
        break;
      }
      case 'e':
      case 'E':
      {
        digitalWrite(SSR_2_8, LOW);
        Serial.print("\r\nActivate 2-8\r\n");
        break;
      }
      case 'f':
      case 'F':
      {
        digitalWrite(SSR_3_9, LOW);
        Serial.print("\r\nActivate 3-9\r\n");
        break;
      }
      case 'g':
      case 'G':
      {
        digitalWrite(SSR_4_0, LOW);
        Serial.print("\r\nActivate 4-0\r\n");
        break;
      }
      case 'i':
      case 'I':
      {
        digitalWrite(SSR_5_6, LOW);
        Serial.print("\r\nActivate 5-6\r\n");
        break;
      }
      case 'j':
      case 'J':
      {
        digitalWrite(SSR_6, LOW);
        Serial.print("\r\nActivate 6\r\n");
        break;
      }
      case 'k':
      case 'K':
      {
        digitalWrite(SSR_SPARE, LOW);
        Serial.print("\r\nActivate Spare SSR\r\n");
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
        Serial.print("\r\nReleased all relays\r\n");
        break;
      }
      case '\r':
      case '\n':
        break;
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
    sprintf(outstr, "Link E relay changed to %s\r\n", link_e == 1 ? "Operated" : "Released");
    link_updated = 0;
    Serial.print(outstr);
  }
}

/* Process low to high transitions on the MT8870 StD input. */
void DTMF_isr(void) {

  if (digitalRead(DTMF_StD) == 1) {
    LED_ON();
    dtmf_digit = VPORTA.IN & 0x0F;
  }
}

/* Process high to low transitions on the LINK_E line. */
void link_isr(void) {
  bool link_prev = link_e;
  if (digitalRead(LINK_E) == 0) {
    link_e = true;
  } else {
    link_e = false;
  }

  if (link_prev != link_e) {
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
    LED_ON();
  } else {
    LED_OFF();
  }
#endif /* BLINK_LED */

  switch(dtmf_dial_state) {
    case DIAL_STATE_IDLE:
      /* Waiting for DTMF from Link. */
      LED_OFF();

      if (digitalRead(LINK_E) == 0) {
        /* When LINK E relay is active, the called station has answered,
         * do not allow dialing until the called party hangs up.
         */
        dtmf_digit = 0;
        break;
      }
      if (dtmf_digit > 0) {
          dtmf_dial_state = DIAL_STATE_WAIT_DTMF_DONE;
          dtmf_digits[digits_collected] = dtmf_digit;
          if (digits_collected == 0) {
            switch (dtmf_digit) {
              case 3:
                /* Operate T3. */
                digitalWrite(SSR_T3, LOW);
              case 10:
              case 2:
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
      }
      break;
    case DIAL_STATE_WAIT_DTMF_DONE:
      /* Wait in this state until DTMFx_StD is de-asserted. */

      if (digitalRead(DTMF_StD) == 0) {
        digits_collected++;
        LED_ON();
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
          dtmf_dial_state = DIAL_STATE_OPERATE_C1;
        }
      }
      break;
    case DIAL_STATE_OPERATE_C1:
      dtmf_delay++;
      if (dtmf_digits[1] > 0) {
        /* Operate C1 relay */
        digitalWrite(SSR_C1, LOW);
      }

      if (dtmf_delay == C1_DELAY_COUNT) {
        dtmf_delay = 0;
        dtmf_dial_state = DIAL_STATE_OPERATE_RELAYS;
      }
      break;
    case DIAL_STATE_OPERATE_RELAYS:
      dtmf_delay++;
      if (dtmf_delay == POST_DIAL_COUNT) {
        dtmf_delay = 0;
        dtmf_updated = 1;

        /* Operate the 755A Tens Relays T2, T3 */
        switch (dtmf_digits[0]) {
          case 3: /* 3x */
            digitalWrite(SSR_T3, LOW);
            /* Fall through */
          case 2: /* 2x */
            digitalWrite(SSR_T2, LOW);
            break;
          default:
            break;
        }

        if (dtmf_digits[1] > 0) {
          /* Operate Units relays */
          VPORTD.OUT &= ~(units_relay_table[dtmf_digits[1]]);
        }
        dtmf_dial_state = DIAL_STATE_DIALING_COMPLETE;
      }
      break;
    case DIAL_STATE_DIALING_COMPLETE:
      dtmf_delay++;
      if (dtmf_delay == RELAY_HOLD_COUNT) {
        dtmf_delay = 0;

        /* Release T2 and T3 relays */
        digitalWrite(SSR_T2, HIGH);
        digitalWrite(SSR_T3, HIGH);

        /* Release units relays */
        VPORTD.OUT |= units_relay_table[dtmf_digits[1]];

        /* Release C1 relay */
        digitalWrite(SSR_C1, HIGH);

        /* Block dialing until the called station answers, and then goes back on-hook. */
        dtmf_dial_state = DIAL_STATE_DIALING_BLOCKED;
      }
      break;
    case DIAL_STATE_DIALING_BLOCKED:
      /* Unblock dialing when the called station answers. */
      if (digitalRead(LINK_E) == 0) {
        digits_collected = 0;
        dtmf_digits[0] = 0;
        dtmf_digits[1] = 0;
        dtmf_dial_state = DIAL_STATE_IDLE;
      }
      break;
    default: /* Invalid state, go back to idle. */
      dtmf_dial_state = DIAL_STATE_IDLE;
      break;
  }
}
