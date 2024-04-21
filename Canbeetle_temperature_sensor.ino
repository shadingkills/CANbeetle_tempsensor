#include <OneWire.h>
#include <DallasTemperature.h>
#include <mcp2515_can.h>
#include <mcp_can.h>

//#define DEBUG_PRINT

#include <DShot.h>

#include <SPI.h>

#include <EEPROM.h>

#define ONE_WIRE_BUS 11

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DShot Esc(DShot::Mode::DSHOT300);

const int SPI_CS_PIN = 10;
mcp2515_can CAN(SPI_CS_PIN);

unsigned char len;
unsigned char msgBuf[8];

byte ID;
byte recID;
int sendID;
byte filtClass;

int PWM;
int controllerInput;
int center = 0;

void inline debugPrint(String data)
{
  #ifdef DEBUG_PRINT
  Serial.print(data);
  #endif
}

void inline debugPrintInt(int data) 
{
  #ifdef DEBUG_PRINT
  Serial.print(data);
  #endif
}

void inline debugPrintHex(int data) 
{
  #ifdef DEBUG_PRINT
  Serial.print(data,HEX);
  #endif
}

void inline space() 
{
  #ifdef DEBUG_PRINT
  Serial.print(" ");
  #endif
}

void inline newLine() 
{
  #ifdef DEBUG_PRINT
  Serial.println();
  #endif
}


void setup() 
{
  Serial.begin(9600);
  sensors.begin();
  sensors.setResolution(11);
 
  while (CAN_OK != CAN.begin(CAN_250KBPS, MCP_8MHz))
  {
    debugPrint("CanBeetle initialization failed!!");
    delay(100); 
  }

  
  ID = EEPROM.read(0);
  sendID = ID | 0x100;
  filtClass = (ID & 0xF0) + 0xF;
  
  CAN.init_Mask(0, 0, 0x1FF);
  CAN.init_Mask(1, 0, 0x1FF);
  CAN.init_Filt(0, 0, 0xFF);
  CAN.init_Filt(1, 0, filtClass);
  CAN.init_Filt(2, 0, ID);

  unsigned char msg[8];
  CAN.MCP_CAN::sendMsgBuf(sendID,0,0,msg);
  debugPrint("Connected!");
  newLine();

}

// void loop() {
//   sensors.requestTemperatures();
//   float temperature = sensors.getTempCByIndex(0);
//   Serial.print("Temperature: ");
//   Serial.println(temperature);
//   delay(1000); // Delay between temperature readings
// }


void loop() {
  sensors.requestTemperatures();
     float Celsius = sensors.getTempCByIndex(0);
     int Celint = int(Celsius); // Convert float to integer
     Serial.print(Celsius);

     float decimalValue = Celsius - Celint; // Extract the decimal value
     int decimalint = decimalValue*100;

     Serial.print("int Value");
     Serial.println(Celint);

     Serial.print("Decimal Value: ");
     Serial.println(decimalValue); // Print the decimal value with desired precisio
   if(CAN_MSGAVAIL == CAN.checkReceive())
  {
    CAN.readMsgBuf(&len, msgBuf);
    recID = CAN.getCanId();

    if (msgBuf[0] == 0xAA) //roleCall command
    {
      unsigned char msg[8];
      debugPrint("0xAA roleCall: ");
      debugPrint("sendID: ");
      debugPrint("0x");
      debugPrintHex(sendID);
      newLine();
      newLine();
      CAN.MCP_CAN::sendMsgBuf(sendID,0,0,msg);
      
    }
    else if (msgBuf[0] == 0x10) // read data command
    {
     unsigned char msg[8];
     debugPrint("0x10 dataRead:");

     msg[0] = Celint >> 8;
     msg[1] = Celint;
     debugPrintInt(Celint);
     msg[2] = decimalint >> 8;
     msg[3] = decimalint;
     //debugPrintInt(decimalint);


     newLine();
     newLine();
     CAN.MCP_CAN::sendMsgBuf(sendID,0,4,msg);
    
    
    
    } 
    else if (msgBuf[0] == 0xCE and recID == ID) //changeID command
    {
      debugPrint("0xCE: ");
      
      EEPROM.put(0, msgBuf[1]);
      ID = msgBuf[1];
      sendID = ID | 0x100;

      filtClass = (ID & 0xF0) + 0xF;
      
      CAN.init_Filt(1,0, filtClass);
      CAN.init_Filt(2, 0, ID);

      debugPrint("filtClass: ");
      debugPrint("0x");
      debugPrintHex(filtClass);
      debugPrint(" ID: ");
      debugPrint(" 0x");
      debugPrintHex(ID);
      newLine();
      newLine();
    }
  }
}

