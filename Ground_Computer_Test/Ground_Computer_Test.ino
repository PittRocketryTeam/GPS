//#include <SPI.h> // patrick - tentatively remove // done
#include <RH_RF95.h>

// temporary pin values for the debug leds on the pcb
#define DEBUG_0 0
#define DEBUG_1 0
#define DEBUG_2 0

#define RFM95_CS 4 // Change 'RFM95' to 'LoRa' - Rachel
#define RFM95_RST 2
#define RFM95_INT 3

//#define TLED 8    // remove -rachel // done
#define GREEN 24
#define RED 25

#define PACKET_SIZE 20

/*const char header[5] = "U_UP";  // make this constant -rachel // done
const char flight_header[5] = "HEYY";
char packet[PACKET_SIZE];*/

String packet = "";
const String header = "U_UP";
const String flight_header = "HEYY";
// add define for flight header - rachel

#define LORA_FREQ 433.0 // Change 'RFM95' to 'LoRa' - Rachel // done - patrick

RH_RF95 lora(RFM95_CS, RFM95_INT); // Change 'RFM95' to 'LoRa' - Rachel // done - patrick

void encodeErrorMsg(uint8_t code)
{
    // the state of the debug leds corresponds to the binary bits set to 1 in the error code argument
    // eg. code = 5 -> 0b00000101 in binary
    // the 3 debug leds would display ON OFF ON, or 5 in binary
    digitalWrite(DEBUG_0, (code >> 0) & 1); // first led (rightmost)
    digitalWrite(DEBUG_1, (code >> 1) & 1); // second led
    digitalWrite(DEBUG_2, (code >> 2) & 1); // third led (leftmost)
}

void blinkLed(int pin, int blinks, int duration)
{
  if (blinks <= 0) return;
  if (duration <= 0) return;

  for (int i=0; i<blinks; i++)
  {
    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW);
    delay(duration);
  }
}

void setup()
{
  // Init debug LEDs
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);

  // Start serial communications
  Serial.begin(9600);
  delay(100);

  // Signal init start
  digitalWrite(GREEN, HIGH);
  digitalWrite(RED, HIGH);

  // Init LoRa reset pin
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  //Serial.println("Arduino LoRa TX Test!"); // go away -rachel // done -

  // Manually reset the LoRa module
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Wait for LoRa module to be ready
  while (!lora.init())
  {
    Serial.println("LoRa radio init failed");
    //while (1); // hang  // Add delay before trying to reinit -Rachel
    delay(100);
  }
  Serial.println("LoRa radio init OK!");

  // Set frequency
  // Defaults after init are 433.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  //if (!lora.setFrequency(LORA_FREQ)) // make this a while loop -rachel
  while (!lora.setFrequency(LORA_FREQ))
  {
    Serial.println("setFrequency failed");
    //while (1); // hang  // Add delay before trying to reinit -Rachel
    delay(1000);
  }
  Serial.print("Set Freq to: "); Serial.println(LORA_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // Set transmitter powers to 23 dBm:
  lora.setTxPower(23, false); // how high can this go? -Patrick

  // Indicate successful init
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);
}

void loop()
{
  /**
   * Instead of redefining the buffers every cycle, I'd rather
   * define the buffers once, and then call either
   * memset(buf, 0, sizeof(buf)); or set the first index of the array
   * to '\0' (null terminator)
   * every cycle.
   **/
  // Define recv buffers
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  //uint8_t len = sizeof(buf);            // Just use RH_RF95_MAX_MESSAGE_LEN -Rachel
  uint8_t len = RH_RF95_MAX_MESSAGE_LEN;

  // Wait for data from the flight computer
  while (!lora.available())
  {
  //    Serial.println("no data received");
  //     signal no data received
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
  }

  if (lora.recv(buf, &len)) // Don't pass by reference, just use RH_RF95_MAX_MESSAGE_LEN -Rachel // turns out, the code on github is different len tells the lora how much to receive
  {
    // Get rid of printlns; GUI works -Rachel
    Serial.println("");
    Serial.println("<---------- BEGIN TRANSMISSION ---------->");
    // Signal data received
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, HIGH);
    Serial.print("GOT REPLY: ");
    Serial.println((char*)buf);

    // convert recv buffer into Arduino String
    String str((char*)buf);

    // Validate header
    if (!str.substring(0,4).equals(flight_header))
    {
      Serial.println("FATAL! BAD HEADER");
      // indicate bad header
      digitalWrite(RED, HIGH);
      digitalWrite(GREEN, HIGH);
      delay(1000);
      return; // break out of loop function // how does this behave???? -rachel // behaves as expected - patrick
    }
    else
    {
      Serial.println("HEADER CONFIRMED");
    }


    // Validate NMEA sentence
    if (str.substring(4).startsWith("$GPGGA"))  // move this to inside valid header loop -rachel
    {
      Serial.println("PACKET CONTAINS NMEA DATA");
    }

    delay(100); // Breathing room for the flight computer  // fails without this delay

    /**
     * Instead of redefining the buffers every cycle, I'd rather
     * define the buffers once, and then call either
     * memset(packet, 0, sizeof(packet)); or set the first index of the array
     * to '\0' (null terminator)
     * every cycle.
     **/

    // Define response buffer
    //char packet[PACKET_SIZE] = ""; // move packet definition out of loop(), define 20 as constant
    //packet[0] = '\0'; // "clear" the buffer
    //strcat(packet, header); // Add header
    //strcat(packet, " CMD other stuff"); // Add dummy command
    //packet[PACKET_SIZE-1] = '\0'; // Null terminate // use constant -rachel
    packet = "";
    packet.concat(header);
    packet.concat("CMD other stuff");
    Serial.print("SENDING PACKET... ");
    //Serial.println((char*)packet);
    lora.send((uint8_t*)packet.c_str(), PACKET_SIZE); // TODO change the 20 to sizeof(packet) // use constant -rachel
    lora.waitPacketSent(); // Wait until finished sending
    Serial.println("DONE!");

    // Report RSSI
    Serial.print("RSSI: ");
    Serial.println(lora.lastRssi());
    Serial.println("<----------  END TRANSMISSION  ---------->");

    //delay(100);
  }
}
