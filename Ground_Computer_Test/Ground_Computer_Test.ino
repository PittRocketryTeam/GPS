// LoRa 9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example LoRa9x_RX

#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 4
#define RFM95_RST 2
#define RFM95_INT 3

#define TLED 8
#define GREEN 24
#define RED 25

char header[5] = "U_UP";

#define RF95_FREQ 433.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

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

  Serial.println("Arduino LoRa TX Test!");

  // Manually reset the LoRa module
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Wait for LoRa module to be ready
  while (!rf95.init())
  {
    Serial.println("LoRa radio init failed");
    while (1); // hang
  }
  Serial.println("LoRa radio init OK!");

  // Set frequency
  // Defaults after init are 433.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ))
  {
    Serial.println("setFrequency failed");
    while (1); // hang
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // Set transmitter powers to 23 dBm:
  rf95.setTxPower(23, false);

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
  uint8_t len = sizeof(buf);

  // Wait for data from the flight computer
  while (!rf95.available())
  {
    // signal no data received
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
  }
  if (rf95.recv(buf, &len))
  {
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
      return; // break out of loop function
    }
    else
    {
      Serial.println("HEADER CONFIRMED");
    }

    // Validate NMEA sentence
    if (str.substring(4).startsWith("$GPGGA"))
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
    char packet[20] = "";
    strcat(packet, header); // Add header
    strcat(packet, " CMD other stuff"); // Add dummy command
    packet[19] = '\0'; // Null terminate
    Serial.print("SENDING PACKET... ");
    //Serial.println((char*)packet);
    rf95.send((uint8_t*)packet, 20); // TODO change the 20 to sizeof(packet)
    rf95.waitPacketSent(); // Wait until finished sending
    Serial.println("DONE!");

    // Report RSSI
    Serial.print("RSSI: ");
    Serial.println(rf95.lastRssi());
    Serial.println("<----------  END TRANSMISSION  ---------->");

    delay(100);
  }

  /**
   * Everything below is old code and should not be used
   */

  /*Serial.println("Sending to rf95_server");
  // Send a message to rf95_server

  char radiopacket[20] = "CMD other stuff    ";
  Serial.print("Sending "); Serial.println(radiopacket);
  radiopacket[19] = 0;

  Serial.println("Sending..."); delay(10);
  rf95.send((uint8_t *)radiopacket, 20);

  Serial.println("Waiting for packet to complete..."); delay(10);
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply..."); delay(10);
  if (rf95.waitAvailableTimeout(1000))
  {
    // Should be a reply message for us now
    if (rf95.recv(buf, &len))
    {
      blinkLed(GREEN, 2, 200);
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      String str((char*)buf);
      if (str.startsWith("$GPGGA")) {

      }
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
    }
    else
    {
      Serial.println("Receive failed");
      blinkLed(RED, 2, 1000);

    }
  }
  else
  {
    Serial.println("No reply, is there a listener around?");
    blinkLed(RED, 2, 200);
  }
  delay(1000);*/
  //delay(1000);
}
