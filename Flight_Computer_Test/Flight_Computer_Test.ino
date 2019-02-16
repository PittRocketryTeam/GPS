// GENERAL COMMENT: make more small, descriptive functions -Rachel

#include <SPI.h> // patrick - tentatively remove
#include <RH_RF95.h>
#include <SoftwareSerial.h>
#include <TeensyID.h>

// PCB UUID = 4e453614-8016-4000-8016-04e9e507f491
// Prototype UUID = 4e453614-8016-4000-8020-04e9e507f41f

//const char* UUID = teensyUUID();

// TODO: Fix this to set conditionally based on UUID -- Rachel
// Flight Configuration
#define RFM95_CS  9 // Change 'RFM95' to 'LoRa' - Rachel
#define RFM95_RST 24
#define RFM95_INT 5
#define GPS_TX 0
#define GPS_RX 1
#define RF95_FREQ 433.0
RH_RF95 rf95(RFM95_CS, RFM95_INT);// radio driver

// Prototype Configuration
//#define RFM95_CS  4
//#define RFM95_RST 2
//#define RFM95_INT 3
//#define GPS_TX 7
//#define GPS_RX 8
//#define RF95_FREQ 433.0
//RH_RF95 rf95(RFM95_CS, RFM95_INT);// radio driver

// Reorganize define statements -Rachel
#define PACKET_SIZE 86
char packet[PACKET_SIZE];
char VEHICLE_HEADER[4] = {'H','E','Y','Y'}; // make this constant -Rachel
// ^ make this a string literal -Patrick
String GS_HEADER = "U_UP";    // make this constant -Rachel
String gpsBuffer;           // Move to above loop() -Rachel
char byteIn;              // Move to above loop() -Rachel
bool transmitReady;// Move to above loop() -Rachel

// gps serial
SoftwareSerial GPS(GPS_TX, GPS_RX);

void setup()
{
  // initialize pins
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH); //turning on LoRa

  // start serial
  Serial.begin(9600);
  delay(100); // Check this -Rachel

  // start gps
  pinMode(GPS_RX, INPUT);
  pinMode(GPS_TX, OUTPUT);
  GPS.begin(9600); // start gps
  delay(100); // And check this -Patrick

  // manual reset LoRa
  // move this up to LoRa init -Patrick
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // init and check radio systems
  // move this up to LoRa init also -Patrick
  while (!rf95.init())
  {
    Serial.println("LoRa radio init failed"); // Keep for 5/17, use LED indicators for new PCB and get rid of this -Rachel
    while (1);          // Add delay before trying to reinit -Rachel
  }

  Serial.println("LoRa radio init OK!");

  if (!rf95.setFrequency(RF95_FREQ))    // Make this a while loop -Rachel
  {
    Serial.println("setFrequency failed");
    while (1);        // Add delay before trying to reinit -Rachel
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Set transmit power to 23 dBm
  rf95.setTxPower(23, false); // how high can this go? -Patrick

  transmitReady = false;

  // this statement is kind of cryptic -Patrick
  // Address this by defining value as a constant -Rachel
  gpsBuffer.reserve(PACKET_SIZE - sizeof(VEHICLE_HEADER)); // allocate memory

  // Move this up so GPS gets a fix sooner -Rachel
  GPS.listen(); // start listening
}

void loop()
{
  // Move println to inside while loop below (one less call to available()) -Rachel
   Serial.println("GPS available? "); // println to print to consolidate to 1 line? -Patrick
   Serial.println(GPS.available());
   // Move transmitReady = false to here -Rachel

  // GPS Reading
  while (GPS.available())
  {
    byteIn = GPS.read();        // read 1 byte
    gpsBuffer += char(byteIn);  // add byte to buffer // fix casting syntax -Rachel
    if (byteIn == '\n')         // end of line
    {
      transmitReady = true;     // ready to transmit
    }
  }

  // Transmitting
  if (transmitReady)
  {
    if (gpsBuffer.startsWith("$GPGGA")) // only transmit what's needed
    {
      // Populate packet with header and GPS payload
      // Copying information from the buffer into a char array
      // Consider copying GPS data straight into char array to eliminate these two lines -Rachel
      char payload[PACKET_SIZE - sizeof(VEHICLE_HEADER)];// Address this by defining value as a constant -Rachel
      gpsBuffer.toCharArray(payload, PACKET_SIZE - sizeof(VEHICLE_HEADER)); // copy gps data to packet

      packet[PACKET_SIZE - 1] = 0; // null terminate the packet

      // Copy in vehicle header
      // Use strcat here -Rachel
      // BRACES!!!!!1! -Patrick
      for (int i = 0; i < sizeof(VEHICLE_HEADER); i++)
        packet[i] = VEHICLE_HEADER[i];

      // Copying info being transmitted into final transmittion packet
      // Fix this with function to copy in payload/content -Rachel
      for (int i = sizeof(VEHICLE_HEADER); i < PACKET_SIZE; i++)
        packet[i] = payload[i - sizeof(VEHICLE_HEADER)];

      // cast pointer to unsigned character and send
      rf95.send((uint8_t*)packet, PACKET_SIZE);

      // wait for send to finish
      rf95.waitPacketSent();

      // check for a response
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);          // Just use RH_RF95_MAX_MESSAGE_LEN -Rachel
      Serial.println("Waiting for reply...");
      delay(10);
      if (rf95.waitAvailableTimeout(1000))
      {
        // Should be a reply message for us now. If something in the buffer, proceed
        if (rf95.recv(buf, &len)) // Don't pass by reference, just use RH_RF95_MAX_MESSAGE_LEN -Rachel
        {
          Serial.println("Received something, checking header...");
          String str((char*)buf);

          // Message has valid header
          if (str.substring(0,4).equals(GS_HEADER))
          {
            Serial.print("Valid header. Got reply: ");
            Serial.println((char*)buf);
            if (str.substring(5,8).equals("CMD"))
            {
              Serial.println("Received a command");
              if (str.substring(9,15).equals("RELEASE"))
              {
                // Stepper motor code goes here -Rachel
              }
            }
          }
          else
          {
            Serial.println("Error: Message has invalid header.");
          }
          Serial.print("RSSI: ");
          Serial.println(rf95.lastRssi(), DEC);
        }
        else
        {
          Serial.println("Receive failed");
        }
      }
      else
      {
        Serial.println("No reply, is there a listener around?");
      }
    }
    else
    {
      Serial.println("Error: GPS problem.");
    }

    delay(1000); // breathing room
    gpsBuffer = ""; // clear gps buffer
    transmitReady = false; // read more gps data //remember to put this at the top - Matt
  }
}
