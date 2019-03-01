// GENERAL COMMENT: make more small, descriptive functions -Rachel

#include <SPI.h> // patrick - tentatively remove
#include <RH_RF95.h>
#include <SoftwareSerial.h>
//#include <TeensyID.h>
#include <Arduino.h>
#include "BasicStepperDriver.h"


// PCB UUID = 4e453614-8016-4000-8016-04e9e507f491
// Prototype UUID = 4e453614-8016-4000-8020-04e9e507f41f

//const char* UUID = teensyUUID();

// TODO: Fix this to set conditionally based on UUID -- Rachel

#define MOTOR_STEPS 1800
#define RPM 80
#define MICROSTEPS 1
#define DIR 32
#define STEP 31

// Flight Configuration
#define RFM95_CS  9 // Change 'RFM95' to 'LoRa' - Rachel
#define RFM95_RST 24
#define RFM95_INT 5
#define GPS_TX 0
#define GPS_RX 1
#define RF95_FREQ 433.0

// Prototype Configuration
//#define RFM95_CS  4
//#define RFM95_RST 2
//#define RFM95_INT 3
//#define GPS_TX 7
//#define GPS_RX 8
//#define RF95_FREQ 433.0

BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);
RH_RF95 rf95(RFM95_CS, RFM95_INT);// radio driver

#define PACKET_SIZE 86

String packet;
const String header = "HEYY";
const String gs_header = "U_UP";    
String gpsBuffer;           
char byteIn;             
bool transmitReady;

int greenPin = 8;

// gps serial
SoftwareSerial GPS(GPS_TX, GPS_RX);

String send_and_listen(String message)
{
  String out;
  packet = "";
  packet.concat(header);
  packet.concat(message);

  // cast pointer to unsigned character and send
  rf95.send((uint8_t*)packet.c_str(), PACKET_SIZE);

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
      out = String((char*)buf);
    }
    else
    {
      out = "ERR_NO_REPLY";
    }
  }
  else
  {
    out = "ERR_NO_REPLY";
  }

  return out;
}


void setup()
{
  stepper.begin(RPM, MICROSTEPS);

  pinMode(greenPin, OUTPUT);
  digitalWrite(greenPin, LOW);
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
  //gpsBuffer.reserve(PACKET_SIZE - sizeof(VEHICLE_HEADER)); // allocate memory

  // Move this up so GPS gets a fix sooner -Rachel
  GPS.listen(); // start listening
}

void loop()
{
  gpsBuffer = ""; // clear gps buffer
  transmitReady = false; // read more gps data //remember to put this at the top - Matt
  // Move println to inside while loop below (one less call to available()) -Rachel
  Serial.print("GPS available? "); // println to print to consolidate to 1 line? -Patrick
  Serial.println(GPS.available());
   // Move transmitReady = false to here -Rachel


  // GPS Reading
  while (GPS.available())
  {
    digitalWrite(greenPin, HIGH);
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
    //if (1)
    {
      String str = send_and_listen(gpsBuffer);
      if (str.substring(0,4).equals(gs_header))
      {
        Serial.print("Valid header. Got reply: ");
        Serial.println((char*)str.c_str());

        if (str.substring(0,14).equals("U_UPCMDRELEASE"))
        {
          Serial.println("RECEIVED RELEASE COMMAND");
          stepper.rotate(7000);  // deploy rover


          //Added confirmation to prevent ground control from incorrectly assuming rover is released
          String reply=" ";
          while (!(reply.substring(0,13).equals("U_UPCONFIRMED")))
          {
            reply = send_and_listen("RELEASED");
          }

          
        }
        if (str.substring(0,18).equals("U_UPCMDLOADPAYLOAD"))
        {
          Serial.println("RECEIVED LOAD PAYLOAD COMMAND");
          stepper.rotate(-5400);  // deploy rover

          //Added confirmation to prevent ground control from incorrectly assuming rover loaded
         String reply= " ";
          while (!(reply.substring(0,13).equals("U_UPCONFIRMED")))
          {
            reply = send_and_listen("LOADED");
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
      Serial.println("GPS: no GPGGA header");
      String str = send_and_listen("NONE");
      
    }

    delay(1000); // breathing room
  }
}
