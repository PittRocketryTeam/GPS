#include <SPI.h> // patrick - tentatively remove
#include <RH_RF95.h>

#define RFM95_CS 4 // Change 'RFM95' to 'LoRa' - Rachel
#define RFM95_RST 2
#define RFM95_INT 3

#define TLED 8    // remove -rachel
#define GREEN 24
#define RED 25

char header[5] = "U_UP";  // make this constant -rachel
// add define for flight header - rachel

#define RF95_FREQ 433.0 // Change 'RFM95' to 'LoRa' - Rachel

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT); // Change 'RFM95' to 'LoRa' - Rachel

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

  Serial.println("Arduino LoRa TX Test!"); // go away -rachel

  // Manually reset the LoRa module
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Wait for LoRa module to be ready
  while (!rf95.init())
  {
    Serial.println("LoRa radio init failed");
    while (1); // hang  // Add delay before trying to reinit -Rachel
  }
  Serial.println("LoRa radio init OK!");

  // Set frequency
  // Defaults after init are 433.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) // make this a while loop -rachel
  {
    Serial.println("setFrequency failed");
    while (1); // hang  // Add delay before trying to reinit -Rachel
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // Set transmitter powers to 23 dBm:
  rf95.setTxPower(23, false); // how high can this go? -Patrick

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
  uint8_t len = sizeof(buf);            // Just use RH_RF95_MAX_MESSAGE_LEN -Rachel

  // Wait for data from the flight computer
  while (!rf95.available())
  {
//    Serial.println("no data received");
//     signal no data received
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
  }
  if (rf95.recv(buf, &len)) // Don't pass by reference, just use RH_RF95_MAX_MESSAGE_LEN -Rachel
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
    if (!str.substring(0,4).equals("HEYY"))
    {
      Serial.println("FATAL! BAD HEADER");
      // indicate bad header
      digitalWrite(RED, HIGH);
      digitalWrite(GREEN, HIGH);
      delay(1000);
      return; // break out of loop function // how does this behave???? -rachel
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

    delay(500); // Breathing room for the flight computer

    /**
     * Instead of redefining the buffers every cycle, I'd rather
     * define the buffers once, and then call either
     * memset(packet, 0, sizeof(packet)); or set the first index of the array
     * to '\0' (null terminator)
     * every cycle.
     **/

    // Define response buffer
    char packet[20] = ""; // move packet definition out of loop(), define 20 as constant
    strcat(packet, header); // Add header
    strcat(packet, " CMD other stuff"); // Add dummy command
    packet[19] = '\0'; // Null terminate // use constant -rachel
    Serial.print("SENDING PACKET... ");
    //Serial.println((char*)packet);
    rf95.send((uint8_t*)packet, 20); // TODO change the 20 to sizeof(packet) // use constant -rachel
    rf95.waitPacketSent(); // Wait until finished sending
    Serial.println("DONE!");

    // Report RSSI
    Serial.print("RSSI: ");
    Serial.println(rf95.lastRssi());
    Serial.println("<----------  END TRANSMISSION  ---------->");

    delay(100); 
  }
}
