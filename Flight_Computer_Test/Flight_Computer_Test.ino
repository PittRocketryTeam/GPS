#include <SPI.h>
#include <RH_RF95.h>
#include <SoftwareSerial.h>

// miso 12
// mosi 11
// sck 13
#define RFM95_CS  4
#define RFM95_RST 2
#define RFM95_INT 3

#define TLED 8

#define GPS_TX 7
#define GPS_RX 8
 
#define RF95_FREQ 433.0
 
// radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

#define GREEN 24
#define RED 25

#define PACKET_SIZE 82
char packet[PACKET_SIZE];
String gpsBuffer;
char byteIn;
bool transmitReady;

// gps serial
SoftwareSerial GPS(GPS_TX, GPS_RX);

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
  
  // initialize pins
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  
  // wait for serial
  while (!Serial);
  Serial.begin(9600); // start serial
  delay(100);

  GPS.begin(9600); // start gps
  delay(100);
 
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // init and check radio systems

  while (!rf95.init()) 
  {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
 
  if (!rf95.setFrequency(RF95_FREQ)) 
  {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
 
  // Set transmit power to 23 dBm
  rf95.setTxPower(23, false);

  transmitReady = false;
  gpsBuffer.reserve(82); // allocate memory
  GPS.listen(); // start listening

  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);
}

void loop()
{
  // GPS Reading
  while (GPS.available()) 
  {
    byteIn = GPS.read(); // read 1 byte
    gpsBuffer += char(byteIn); // add byte to buffer
    if (byteIn == '\n') 
    { // end of line
      transmitReady = true; // ready to transmit
    }
  }

  // Transmitting
  if (transmitReady) 
  {
    if (gpsBuffer.startsWith("$GPGGA")) // only transmit what's needed
    {
      gpsBuffer.toCharArray(packet, PACKET_SIZE); // copy gps data to packet
      packet[PACKET_SIZE - 1] = 0; // null terminate the packet
  
      Serial.print("PACKET: ");
      Serial.println(packet);
      rf95.send((uint8_t*)packet, PACKET_SIZE); // cast pointer to unsigned character and send
      rf95.waitPacketSent(); // wait for send to finish
      
      // check for a response
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
          if (str.startsWith("CMD")) {
            
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
    } 
    else 
    {
      Serial.println("none");
    }

    delay(1000); // breathing room
    gpsBuffer = ""; // clear gps buffer
    transmitReady = false; // read more gps data
  }
}
