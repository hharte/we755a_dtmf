# 755A PBX DTMF Converter


# Introduction

The Western Electric 755A PBX was designed in 1938 and supports pulse dialing only  This DTMF converter adds support for Touch-Tone dialing.  One DTMF Converter board needs to be installed in each of the three links.  The converter recognizes DTMF digits, and sets the pulse counting relays in the link to the terminal count for the number dialed.  This is not a tone to pulse converter, and operates much faster than one.

The dial plan for the 755A PBX consists of local station extensions 20-29 and 30-39.  Although this PBX supports four trunks, they are not directly accessible by stations; instead, key stations with individual trunk pickup keys are used to select the trunks.

Here is an [introductory video of the 755A PBX in operation](https://www.youtube.com/watch?v=Fxt21lnzOqg).

This project builds upon the [OKI AC125A DTMF Converter](https://github.com/hharte/ac125a_dtmf) designed several years ago.  While the OKI is an electromechanical crossbar PBX similar in some ways to the 755A, the OKI is about 40 years more modern.


# Status

Three boards have been assembled and installed in each of the 755A’s links.  They work as intended for station to station and conference calls.

![alt_text](https://raw.githubusercontent.com/hharte/we755a_dtmf/main/photos/755A_DTMF_Closeup.jpg "image_tooltip")

![alt_text](https://raw.githubusercontent.com/hharte/we755a_dtmf/main/photos/755A_DTMF_Overview.jpg "image_tooltip")

![alt_text](https://github.com/hharte/we755a_dtmf/blob/main/photos/755A_DTMF_Wiring.jpg "image_tooltip")

# Translation

The DTMF converter employs digit translation as follows:


<table>
  <tr>
   <td>First Dialed Digit
   </td>
   <td>755A Extension
   </td>
   <td>Description
   </td>
  </tr>
  <tr>
   <td>0
   </td>
   <td>20
   </td>
   <td>Operator
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>-
   </td>
   <td>No operation
   </td>
  </tr>
  <tr>
   <td>2
   </td>
   <td>2x
   </td>
   <td>Direct-dialed 755A stations.
   </td>
  </tr>
  <tr>
   <td>3
   </td>
   <td>3x
   </td>
   <td>Direct-dialed 755A stations.
   </td>
  </tr>
  <tr>
   <td>4, 5, 6, 7
   </td>
   <td>-
   </td>
   <td>No operation
   </td>
  </tr>
  <tr>
   <td>8
   </td>
   <td>26
   </td>
   <td>Ringdown extensions 26, 27.
   </td>
  </tr>
  <tr>
   <td>9
   </td>
   <td>36
   </td>
   <td>Ringdown extensions 36, 37
   </td>
  </tr>
</table>



# 755A PBX Connector


<table>
  <tr>
   <td>Pin
   </td>
   <td>Description
   </td>
   <td>Ribbon Cable Color
   </td>
   <td>Cloth Wire Color
   </td>
   <td>755A Link Connection
   </td>
  </tr>
  <tr>
   <td>1,2
   </td>
   <td>GRD
   </td>
   <td>Brown/Red
   </td>
   <td>Black
   </td>
   <td>Connect to the B relay coil so the converter powers up when the link is active.
   </td>
  </tr>
  <tr>
   <td>3
   </td>
   <td>LINK_SPARE
   </td>
   <td>Orange
   </td>
   <td>
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>4
   </td>
   <td>LINK_C1
   </td>
   <td>Yellow
   </td>
   <td>Or/Sl
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>5
   </td>
   <td>LINK_6
   </td>
   <td>Green
   </td>
   <td>Sl/W/R
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>6
   </td>
   <td>LINK_5-6
   </td>
   <td>Blue
   </td>
   <td>SL/W
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>7
   </td>
   <td>LINK_4-0
   </td>
   <td>Violet
   </td>
   <td>Br/R
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>8
   </td>
   <td>LINK_3-9
   </td>
   <td>Slate
   </td>
   <td>Br
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>9
   </td>
   <td>LINK_2-8
   </td>
   <td>White
   </td>
   <td>Gr/R
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>10
   </td>
   <td>LINK_1-7
   </td>
   <td>Black
   </td>
   <td>Gr
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>11
   </td>
   <td>LINK_T3
   </td>
   <td>Brown
   </td>
   <td>Or/R
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>12
   </td>
   <td>LINK_T2
   </td>
   <td>Red
   </td>
   <td>Or
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>13
   </td>
   <td>N/C
   </td>
   <td>Orange
   </td>
   <td>
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>14
   </td>
   <td>N/C
   </td>
   <td>Yellow
   </td>
   <td>
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>15
   </td>
   <td>N/C
   </td>
   <td>Green
   </td>
   <td>
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>16
   </td>
   <td>LINK_E
   </td>
   <td>Blue
   </td>
   <td>Blue
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>17
   </td>
   <td>Tip
   </td>
   <td>Violet
   </td>
   <td>O/GR
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>18
   </td>
   <td>Ring
   </td>
   <td>Slate
   </td>
   <td>O/GR-R
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>19,20
   </td>
   <td>-24VDC
   </td>
   <td>White/Black
   </td>
   <td>Red
   </td>
   <td>-24VDC (Red)
   </td>
  </tr>
</table>



# Connection Notes

From SD-66611-01:

-BAT: Pick up from P1 relay red lead.

GRD: Pick up from B relay coil.

LINK_B: (Blue) Pick up from B relay coil (Upper left terminal.)

Tip: (Violet): Pick up T from 2mF capacitors on lower right corner (looking at from rear):

Ring: (Slate): Pick up T from 2mF capacitors on lower right corner (looking at from rear):

LINK_T2: (Red) Pick up from T2 relay coil.

LINK_T3: (Brown) Pick up from T3 relay coil.

LINK_C1: (Yellow) Pick up from C1 relay coil.


# Technical Details


## Theory of Operation

The 755A DTMF converter’s ground supply is connected to the B relay coil in the link so that the DTMF converter is only powered on when the link is active.  This is necessary so that the first and second digits dialed can be properly identified if the calling station hangs up after dialing only a single digit.

When a station goes off hook, the link allotter selects an idle link, operating the link’s B relay.  This powers up the DTMF converter, which waits for digit(s) to be dialed.  If the first dialed digit is 2 or 3, the converter waits for a second digit to be dialed and then activates the tens and units counting relays to their terminal count.  If the first digit is a digit other than 2 or 3 is dialed, the digit is handled as follows:



* 1, 4, 5, 6, or 7: Ignored
* 0, 8, 9: Translated to a 2-digit extension that is dialed immediately.

After dialing is complete, further dialing is disabled until the called station answers and hangs up again.  This is needed to prevent operation of the pulse counting relays during an established call, while also supporting conference calling.

Conference calling with the 755A works as follows:



1. The originating station calls an extension.
2. The extension answers the call, activating the E relay.
3. The originating station asks the answering station to hang up momentarily so that a conference call can be established.
4. After the called station hangs up, the originating station dials the number of an additional station to join the conference.
5. After the first digit of the called number is dialed, 


## Pulse Dialing Terminal Count Relays


### Tens Digit (2 or 3)


<table>
  <tr>
   <td>Digit
   </td>
   <td>Relays Activated
   </td>
   <td>Note
   </td>
  </tr>
  <tr>
   <td>2
   </td>
   <td>T2
   </td>
   <td>T2 activated stops dialtone.
   </td>
  </tr>
  <tr>
   <td>3
   </td>
   <td>T2 and T3
   </td>
   <td>
   </td>
  </tr>
</table>



### Units Digit (0-9)


<table>
  <tr>
   <td>Digit
   </td>
   <td>Relays Activated
   </td>
   <td>To finish dialing
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>C1, 1-7
   </td>
   <td>Release C1
   </td>
  </tr>
  <tr>
   <td>2
   </td>
   <td>C1, 2-8
   </td>
   <td>Release C1
   </td>
  </tr>
  <tr>
   <td>3
   </td>
   <td>C1, 3-9
   </td>
   <td>Release C1
   </td>
  </tr>
  <tr>
   <td>4
   </td>
   <td>C1, 4-0
   </td>
   <td>Release C1
   </td>
  </tr>
  <tr>
   <td>5
   </td>
   <td>C1, 5-6
   </td>
   <td>Release C1
   </td>
  </tr>
  <tr>
   <td>6
   </td>
   <td>C1, 5-6, 6
   </td>
   <td>Release C1
   </td>
  </tr>
  <tr>
   <td>7
   </td>
   <td>C1, 1-7, 6
   </td>
   <td>Release C1
   </td>
  </tr>
  <tr>
   <td>8
   </td>
   <td>C1, 2-8, 6
   </td>
   <td>Release C1
   </td>
  </tr>
  <tr>
   <td>9
   </td>
   <td>C1, 3-9, 6
   </td>
   <td>Release C1
   </td>
  </tr>
  <tr>
   <td>0
   </td>
   <td>C1, 4-0, 6
   </td>
   <td>Release C1
   </td>
  </tr>
</table>



## Inputs from 755A


<table>
  <tr>
   <td>T
   </td>
   <td colspan="3" >Tip
   </td>
  </tr>
  <tr>
   <td>R
   </td>
   <td colspan="3" >Ring
   </td>
  </tr>
  <tr>
   <td>E
   </td>
   <td colspan="3" >Call Active (Interrupt the microcontroller.)
   </td>
  </tr>
</table>



## Outputs to 755A


<table>
  <tr>
   <td>T2
   </td>
   <td colspan="3" >Tens Digit 2 (Breaks dial tone.)
   </td>
  </tr>
  <tr>
   <td>T3
   </td>
   <td colspan="3" >Tens Digit 3 (Also requires T2 activated.)
   </td>
  </tr>
  <tr>
   <td>1-7, 2-8, 3-9, 4-0, 5-6, 6
   </td>
   <td colspan="3" >Units Digit Selection
   </td>
  </tr>
  <tr>
   <td>C1
   </td>
   <td colspan="3" >Activated before Units selection relays are set, released when the digit is complete.
   </td>
  </tr>
</table>



## MT8870 DTMF Converter Outputs


<table>
  <tr>
   <td>Q3:0
   </td>
   <td colspan="3" >DTMF Digit Selection
   </td>
  </tr>
  <tr>
   <td>StB
   </td>
   <td colspan="3" >DTMF Valid Strobe (Interrupt to microcontroller.)
   </td>
  </tr>
</table>



## Bill of Materials

Online [DigiKey BOM](https://www.digikey.com/en/mylists/list/MP3JW7CHBF).


<table>
  <tr>
   <td>Quantity
   </td>
   <td>Reference Designator
   </td>
   <td>Value
   </td>
  </tr>
  <tr>
   <td>4
   </td>
   <td>C1, C2, C4, C7
   </td>
   <td>0.1uF
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>C3
   </td>
   <td>0.01uF
   </td>
  </tr>
  <tr>
   <td>2
   </td>
   <td>C5, C8
   </td>
   <td>0.01uF/630V
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>C6
   </td>
   <td>10uF 25V
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>D1
   </td>
   <td>1N4148
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>D2
   </td>
   <td>LED
   </td>
  </tr>
  <tr>
   <td>2
   </td>
   <td>D3, D4
   </td>
   <td>1N4734A
   </td>
  </tr>
  <tr>
   <td>2
   </td>
   <td>F1, F2
   </td>
   <td>Fuse 630mA
   </td>
  </tr>
  <tr>
   <td>2
   </td>
   <td>H1, H2
   </td>
   <td>MountingHole
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>J1
   </td>
   <td>Conn_01x06_Male right angle
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>J2
   </td>
   <td>UPDI (2x3 pin header) Straight
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>J3
   </td>
   <td>Conn_02x10_Odd_Even
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>PS1
   </td>
   <td>SPBW06
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>R1
   </td>
   <td>100K
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>R2
   </td>
   <td>330K
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>R3
   </td>
   <td>220
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>R4
   </td>
   <td>52.3K 1%
   </td>
  </tr>
  <tr>
   <td>4
   </td>
   <td>R5, R6, R8, R12
   </td>
   <td>110K 1%
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>R7
   </td>
   <td>4.7K
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>R9
   </td>
   <td>220K 1%
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>R10
   </td>
   <td>10K
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>R11
   </td>
   <td>68.1K 1%
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>R13
   </td>
   <td>1.5K
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>RN1
   </td>
   <td>220
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>U1
   </td>
   <td>AVR128DA28-I/SP or AVR128DA28-E/SP
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>U2
   </td>
   <td>MT8870
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>U3
   </td>
   <td>H11AA1
   </td>
  </tr>
  <tr>
   <td>5
   </td>
   <td>U4, U5, U6, U7, U8
   </td>
   <td>LAA108 (or LAA110L or CLA170)
   </td>
  </tr>
  <tr>
   <td>1
   </td>
   <td>Y2
   </td>
   <td>3.579545MHZ
   </td>
  </tr>
</table>
