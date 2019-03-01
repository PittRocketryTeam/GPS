//#include <SPI.h> // patrick - tentatively remove // done
#include <RH_RF95.h>

#define TIMEOUT 1000

// temporary pin values for the debug leds on the pcb
/*#define DEBUG_TX 12
#define DEBUG_RX 11
#define DEBUG_OK 10
#define DEBUG_ER 9
#define MODE_TX 0
#define MODE_RX 1
#define MODE_IN 2
#define STATUS_OK 0
#define STATUS_ER 1

void debug_init()
{
  pinMode(DEBUG_TX, OUTPUT);
  pinMode(DEBUG_RX, OUTPUT);
  pinMode(DEBUG_ER, OUTPUT);
  pinMode(DEBUG_OK, OUTPUT);
}

void debug_setmode(uint8_t mode)
{
  if (mode > 1)
  {
    digitalWrite(DEBUG_TX, HIGH);
    digitalWrite(DEBUG_RX, HIGH);
  }
  digitalWrite(DEBUG_TX, !mode);
  digitalWrite(DEBUG_RX, mode);
}

void debug_setstatus(uint8_t stat)
{
  digitalWrite(DEBUG_ER, !stat);
  digitalWrite(DEBUG_ER, stat);
}*/

#define LORA_CS 4 // Change 'RFM95' to 'LoRa' - Rachel
#define LORA_RST 2
#define LORA_INT 3

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

RH_RF95 lora(LORA_CS, LORA_INT); // Change 'RFM95' to 'LoRa' - Rachel // done - patrick

/*void encodeErrorMsg(uint8_t code)
{
    // the state of the debug leds corresponds to the binary bits set to 1 in the error code argument
    // eg. code = 5 -> 0b00000101 in binary
    // the 3 debug leds would display ON OFF ON, or 5 in binary
    digitalWrite(DEBUG_0, (code >> 0) & 1); // first led (rightmost)
    digitalWrite(DEBUG_1, (code >> 1) & 1); // second led
    digitalWrite(DEBUG_2, (code >> 2) & 1); // third led (leftmost)
}*/

void blinkLed(int pin, int blinks, int duration)
{
  if (blinks <= 0)
  {
      return;
  }

  if (duration <= 0)
  {
      return;
  }

  for (int i=0; i<blinks; i++)
  {
    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW);
    delay(duration);
  }
}

String recv_packet()
{
  String out;
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = RH_RF95_MAX_MESSAGE_LEN;

  if (lora.recv(buf, &len))
  {
    out = String((char*)buf);
  }
  else
  {
    out = "";
  }

  return out;
}

String send_and_listen(String data)
{
  String out;
  String packet = "";
  packet.concat(header);
  packet.concat(data);
  
  lora.send((uint8_t*)packet.c_str(), PACKET_SIZE);
  lora.waitPacketSent();

  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = RH_RF95_MAX_MESSAGE_LEN;

  if (lora.waitAvailableTimeout(TIMEOUT))
  {
    if (lora.recv(buf, &len))
    {
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

String get_command()
{
  String cmd = "";
  while (Serial.available())
  {
    cmd.concat((char)Serial.read());
  }

  return cmd;
}

void setup()
{
  //debug_init();
  //debug_setmode(MODE_IN);

  // Init debug LEDs
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);

  digitalWrite(GREEN, HIGH);
  digitalWrite(RED, HIGH);
  delay(1000);

  // Start serial communications
  Serial.begin(9600);
  delay(100);

  // Init LoRa reset pin
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, HIGH);

  // Manually reset the LoRa module
  digitalWrite(LORA_RST, LOW);
  delay(10);
  digitalWrite(LORA_RST, HIGH);
  delay(10);

  // Wait for LoRa module to be ready
  while (!lora.init())
  {
    //debug_setstatus(STATUS_ER);
    Serial.println("LoRa radio init failed");
    blinkLed(RED, 5, 200);
    //while (1); // hang  // Add delay before trying to reinit -Rachel
    delay(1000);
  }
  //debug_setstatus(STATUS_OK);
  Serial.println("LoRa radio init OK!");

  // Set frequency
  // Defaults after init are 433.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  //if (!lora.setFrequency(LORA_FREQ)) // make this a while loop -rachel
  while (!lora.setFrequency(LORA_FREQ))
  {
    Serial.println("setFrequency failed");
    //while (1); // hang  // Add delay before trying to reinit -Rachel
    delay(1000);
    //debug_setstatus(STATUS_ER);
  }
  //debug_setstatus(STATUS_OK);
  Serial.print("Set Freq to: "); Serial.println(LORA_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // Set transmitter powers to 23 dBm:
  lora.setTxPower(23, false); // how high can this go? -Patrick

  // Indicate successful init
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);
  delay(1000);
}

void loop()
{
  // Define recv buffers
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = RH_RF95_MAX_MESSAGE_LEN;

  // Wait for data from the flight computer
  Serial.println("wait");
  while (!lora.available())
  {
  //    Serial.println("no data received");
  //     signal no data received
    //digitalWrite(RED, HIGH);
    blinkLed(RED, 1, 100);
    digitalWrite(GREEN, LOW);
  }

  String str = recv_packet();
  if (str != "")
  {
    // Get rid of printlns; GUI works -Rachel
    Serial.println("");
    Serial.println("<---------- BEGIN TRANSMISSION ---------->");
    // Signal data received
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, HIGH);
    Serial.print("GOT REPLY: ");
    Serial.println(str);

    // convert recv buffer into Arduino String
    //String str((char*)buf);

    // Validate header
    if (!str.substring(0,4).equals(flight_header))
    {
      Serial.println("FATAL! BAD HEADER");
      // indicate bad header
      blinkLed(RED, 2, 300);
      delay(1000);
      return;
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
    else if (str.substring(4).startsWith("LOADED") | str.substring(4).startsWith("RELEASED"))
    {
      String reply = " ";
      while (!(reply.substring(4).startsWith("$GPGGA")))
      {
          reply = send_and_listen("CONFIRMED");
      }
      Serial.println("EXECUTION CONFIRMED");
    }

    delay(100); // Breathing room for the flight computer  // fails without this delay

    String cmd = get_command();
    send_and_listen(cmd);
    Serial.println("DONE!");

    // Report RSSI
    Serial.print("RSSI: ");
    Serial.println(lora.lastRssi());
    Serial.println("<----------  END TRANSMISSION  ---------->");

    //delay(100);
  }
}
