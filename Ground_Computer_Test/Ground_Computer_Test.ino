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

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 433.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// blink our pretty test led
void testBlink(int n, int d)
{
  if (n <=0) return;
  if (d <= 0) return;

  for (int i=0; i<n; i++)
  {
    digitalWrite(TLED, HIGH);
    delay(d);
    digitalWrite(TLED, LOW);
    delay(d);
  }
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
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);

  digitalWrite(GREEN, HIGH);
  digitalWrite(RED, HIGH);
  
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // wait for serial
  while (!Serial);
  Serial.begin(9600);
  delay(100);

  Serial.println("Arduino LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) 
  {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) 
  {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);
}

void loop()
{ 
  Serial.println("Sending to rf95_server");
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
      blinkLed(GREEN, 3, 100);
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
      blinkLed(RED, 2, 300);
      
    }
  }
  else
  {
    Serial.println("No reply, is there a listener around?");
    blinkLed(RED, 3, 100);
  }
  delay(1000);
}
