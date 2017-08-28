
/*
  CS: pin 7
  MOSI: pin 11
  MISO: pin 12
  SCK: pin 13
*/
#include <SPI.h>
#include "VN100.h"
#include <math.h>

const int chipSelectPin = 7;


// struct for recieved data
typedef struct {
  byte zeroByte;
  byte ComID;
  byte RegNum;
  byte ErrorCode;
  unsigned long Data[15]; //Not sure what the best number is for this or if it can be left blank
} RecPacket;



void setup() {
  Serial.begin(9600);

  // start the SPI library:
  SPI.begin();

  // initalize the  data ready and chip select pins:
  pinMode(chipSelectPin, OUTPUT);

  delay(100);
  getTare();
  delay(100);
  getTare();
  delay(4000);
  Serial.println("Tare Complete");
 // writeTag();
}

void loop() {

 // getModel();
  getQTN();
 //getSerial();
 // getBaud();
   getYPR();
// getTag();
  delay(1000);
}

/*
   This function is the heart of the communication with the IMU.
   It takes 4 byte-length commands in the form of:
   Command: To read a register this ia always 1
   Register: The register number to be read
   Two zeroes (no idea why.  Maybe just to make it 4 bytes long
   These 4 bytes are combined to create an unsigned long and placed in an array of
   unsigned longs called "buffer".  This array is transfered to the IMU
   and the the rsponse is returned in the same buffer.  This buffer is then parsed
   into the RecPacket structure.  The first entry is broken up into individual
   bytes which contain information (including error codes).  Additional
   data is placed in a data array.
*/
void spiRead(RecPacket* packet, byte reg) {

  /*
    CPOL = 1 (idle high)
    CPHA = 1 (data sampled on the clock rising edge and sent on the falling edge)
    This means SPI_MODE3
    It appears to be MSBFIRST
  */
  unsigned long buffer[16]; //Buffer to transmit and recieve the data from the IMU

Transfer(buffer, reg, 1, 16);

  // Parsing the data into the RecPacket data type
  packet->zeroByte = (byte)((buffer[0]) & 0x00000FF);
  packet->ComID = (byte)((buffer[0] >> 8) & 0x00000FF);
  packet->RegNum = (byte)((buffer[0] >> 16) & 0x00000FF);
  packet->ErrorCode = (byte)((buffer[0] >> 24) & 0x00000FF);
  for (int x = 0; x < 15; x++) {
    packet->Data[x] =  buffer[x + 1];
  }

  /*
    Serial.print("Zero: ");
    Serial.println(packet->zeroByte);
    Serial.print("Command: ");
    Serial.println(packet->ComID);
    Serial.print("Register: ");
    Serial.println(packet->RegNum);
    Serial.print("Error Code: ");
    Serial.println(packet->ErrorCode);
  */
}


void spiWrite(RecPacket* packet, byte reg) {


  unsigned long buffer[16]; //Buffer to transmit and recieve the data from the IMU

Transfer(buffer, reg, 2, 16);

  // Parsing the data into the RecPacket data type
  packet->zeroByte = (byte)((buffer[0]) & 0x00000FF);
  packet->ComID = (byte)((buffer[0] >> 8) & 0x00000FF);
  packet->RegNum = (byte)((buffer[0] >> 16) & 0x00000FF);
  packet->ErrorCode = (byte)((buffer[0] >> 24) & 0x00000FF);
  for (int x = 0; x < 15; x++) {
    packet->Data[x] =  buffer[x + 1];
  }

  
    Serial.print("Zero: ");
    Serial.println(packet->zeroByte);
    Serial.print("Command: ");
    Serial.println(packet->ComID);
    Serial.print("Register: ");
    Serial.println(packet->RegNum);
    Serial.print("Error Code: ");
    Serial.println(packet->ErrorCode);
  
}


void writeTag() {
  RecPacket packet;

  spiWrite(&packet, 0);
  spiWrite(&packet, 0);
  //spiRead(&packet, 0);

  Serial.println("Write");
 /// Serial.println(packet.Data[0]);

}


void getTag() {
  RecPacket packet;

  spiRead(&packet, 0);
  spiRead(&packet, 0);

  Serial.print("Tag: ");
  Serial.println(packet.Data[0]);

}



/*
   This function returns the model number.
*/
void getModel() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_MODEL);
  spiRead(&packet, VN100_REG_MODEL); //Second call is for synchronization
  char model[12];
  // It requires some rearrangemnet to turn a long into char.  Everything is backwards
  // otherwise.
  for (int c = 0; c < 4; c++) {
    model[c] = (packet.Data[0] >> (c * 8)) & 0x000000FF;
    model[c + 4] = (packet.Data[1] >> (c * 8)) & 0x000000FF;
    model[c + 8] = (packet.Data[2] >> (c * 8)) & 0x000000FF;
  }

  Serial.print("Model Number: ");
  for (int x = 0; x < 12; x++) {
    Serial.print(model[x]);
  }
  Serial.println(" "); //Advance the line

}

/*
   The function returns the serial number
*/

void getSerial() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_SN);
  spiRead(&packet, VN100_REG_SN);

  Serial.print("Serial Number: ");
  Serial.println(packet.Data[0]);

}

/*
   This function returns the Baud rate
*/

void getBaud() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_SBAUD);
  spiRead(&packet, VN100_REG_SBAUD);

  Serial.print("Baud Rate: ");
  Serial.println(packet.Data[0]);

}

/*
   This function returns the Yaw Pitch Roll
*/

void getYPR() {

  RecPacket packet;

  spiRead(&packet, VN100_REG_YPR);
  spiRead(&packet, VN100_REG_YPR);
//Serial.print("Yaw:");
 // Serial.println(long2float(packet.Data[0]));
  Serial.print("Pitch:");
  Serial.println(long2float(packet.Data[1]));
 // Serial.print("Roll:");
 // Serial.println(long2float(packet.Data[2]));

}

/*
   This function returns Quaternions
*/
void getQTN() {

  RecPacket packet;

  spiRead(&packet, VN100_REG_QTN);
  spiRead(&packet, VN100_REG_QTN);

  
  Serial.print("One:");
  Serial.println(long2float(packet.Data[0]));
  Serial.print("Two:");
  Serial.println(long2float(packet.Data[1]));
  Serial.print("Three:");
  Serial.println(long2float(packet.Data[2]));
  Serial.print("Four:");
  Serial.println(long2float(packet.Data[3]));


float q0 = long2float(packet.Data[0]);
float q1 = long2float(packet.Data[1]);
float q2 = long2float(packet.Data[2]);
float q3 = long2float(packet.Data[3]);

//  float YAW = atan((2.0*(q0*q1+q3*q2))/(q3*q3 - q2*q2 - q1*q1 + q0*q0));
  float YAW = atan2((q3*q3 - q2*q2 - q1*q1 + q0*q0),(2.0*(q0*q1+q3*q2)));
  YAW = YAW*180.0/3.14;
//  Serial.print("Yaw (Quat):");
//  Serial.println(YAW);
float PITCH = asin(-2.0*(q0*q2-q1*q3));
PITCH = PITCH*180.0/3.14;
  Serial.print("Pitch (Quat):");
  Serial.println(PITCH);
float ROLL = atan((2.0*(q1*q2+q0*q3))/(q3*q3 + q2*q2 - q1*q1 - q0*q0));
ROLL = ROLL*180.0/3.14;
//  Serial.print("Roll (Quat):");
//  Serial.println(ROLL);
  
}

/*
   This function returns the accelleration rate (m/s^2)
*/

void getAcc() {

  byte buffer[24];

  // spiRead(buffer, VN100_REG_ACC, 3);

  unsigned long temp1 = (unsigned long)(((buffer[11] << 8) | (buffer[10] << 0)));
  unsigned long temp2 = (unsigned long)(((buffer[9] << 8) | (buffer[8] << 0)) & 0x0000FFFF);
  temp1 = (temp1 * 65536);
  float combined = (float)(temp1 + temp2);
  Serial.print("Acc: ");
  Serial.println(combined);

  long temp1l = (long)(((buffer[11] << 8) | (buffer[10] << 0)));
  long temp2l = (long)(((buffer[9] << 8) | (buffer[8] << 0)) & 0x0000FFFF);
  temp1l = (temp1l * 65536);
  long combinedl = (long)(temp1l + temp2l);
  Serial.print("long Acc: ");
  Serial.println(combinedl);

}

void getAccGain() {

}

/*
   This function tares the device
   (It needs to be fixed to work like spiRead)
*/

void getTare() {

  unsigned long buffer[1]; //Buffer to transmit and recieve the data from the IMU

  //Combining the commands in to a 4 byte long
  unsigned long temp1 = (unsigned long)(((0 << 8) | (0 << 0)));
  unsigned long temp2 = (unsigned long)(((0 << 8) | (5 << 0)) & 0x0000FFFF);
  temp1 = temp1 * 65536;
  unsigned long combined = temp1 + temp2;

  // The transfer
  SPI.beginTransaction(SPISettings(9600, MSBFIRST, SPI_MODE3));
  digitalWrite(chipSelectPin, LOW);
  buffer[0] = combined;

  SPI.transfer(buffer, 1);
  digitalWrite(chipSelectPin, HIGH);
  SPI.endTransaction();

}

/*
   This function converts the output which is an
   unsigned long to a float.  It is done because casting it
   as a float doesn't work.
   Explanation of a float:
   https://www3.ntu.edu.sg/home/ehchua/programming/java/datarepresentation.html
*/

float long2float(long value) {

  byte Sign = (value >> 31) & 1;
  long Exponent = (value >> 23) & 0x000000FF;
  long Fraction = (value) & 0x007FFFFF;
  float Frac = 1.0;
  for (int i = 0; i < 24; i++) {
    byte temp = (Fraction >> i) & 1;
    Frac = Frac + temp * pow(2, -(i + 1));
  }
  long Exp = Exponent - 127;
  long Sign2;
  if (Sign == 0) {
    Sign2 = 1;
  } else {
    Sign2 = -1;
  }

  return Sign2 * Frac * pow(2, Exp);

}


void Transfer(unsigned long* buffer, byte reg, byte Comm, byte Size){
    /*
    CPOL = 1 (idle high)
    CPHA = 1 (data sampled on the clock rising edge and sent on the falling edge)
    This means SPI_MODE3
    It appears to be MSBFIRST
  */
//Combining the commands in to a 4 byte long
  unsigned long temp1 = (unsigned long)(((0 << 8) | (0 << 0)));
  unsigned long temp2 = (unsigned long)(((reg << 8) | (Comm << 0)) & 0x0000FFFF);
  temp1 = temp1 * 65536;
  unsigned long combined = temp1 + temp2;

  // The transfer
  SPI.beginTransaction(SPISettings(9600, MSBFIRST, SPI_MODE3));
  digitalWrite(chipSelectPin, LOW);
  buffer[0] = combined;
  for (int y = 1; y < 16; y++) {
    buffer[y] = 65;
  }
  SPI.transfer(buffer, 16);
  digitalWrite(chipSelectPin, HIGH);
  SPI.endTransaction();

  delay(20);
  
}

