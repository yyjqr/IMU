
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
}

void loop() {

  getModel();
  //getQTN();
   getSerial();
   getBaud();
  // getYPR();
  delay(1000);
}


void spiRead(RecPacket* packet, byte reg) {

  /*
    CPOL = 1 (idle high)
    CPHA = 1 (data sampled on the clock rising edge and sent on the falling edge)
    This means SPI_MODE3
    It appears to be MSBFIRST
  */
  unsigned long buffer[16]; //Buffer to transmit and recieve the data from the IMU

  unsigned long temp1 = (unsigned long)(((0 << 8) | (0 << 0)));
  unsigned long temp2 = (unsigned long)(((reg << 8) | (1 << 0)) & 0x0000FFFF);
  temp1 = temp1 * 65536;
  unsigned long combined = temp1 + temp2;


  SPI.beginTransaction(SPISettings(9600, MSBFIRST, SPI_MODE3));
  digitalWrite(chipSelectPin, LOW);
  buffer[0] = combined;
  buffer[1] = 0;
  buffer[2] = 0;
  buffer[3] = 0;
  for (int y = 4; y < 16; y++) {
    buffer[y] = 0;
  }
  SPI.transfer(buffer, 16);
  digitalWrite(chipSelectPin, HIGH);
  SPI.endTransaction();

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

void getModel() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_MODEL);
  spiRead(&packet, VN100_REG_MODEL);
  char serial[12];

  for (int c = 0; c < 4; c++) {
    serial[c] = (packet.Data[0] >> (c * 8)) & 0x000000FF;
    serial[c + 4] = (packet.Data[1] >> (c * 8)) & 0x000000FF;
    serial[c + 8] = (packet.Data[2] >> (c * 8)) & 0x000000FF;
  }

  Serial.print("Model Number: ");
  for (int x = 0; x < 12; x++) {
    Serial.print(serial[x]);
  }
  Serial.println(" ");

}


void getSerial() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_SN);
  spiRead(&packet, VN100_REG_SN);

  Serial.print("Serial Number: ");
  Serial.println(packet.Data[0]);

}


void getBaud() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_SBAUD);
  spiRead(&packet, VN100_REG_SBAUD);
  
  Serial.print("Baud Rate: ");
  Serial.println(packet.Data[0]);

}


void getYPR() {

  RecPacket packet;

  spiRead(&packet, VN100_REG_YPR);
  spiRead(&packet, VN100_REG_YPR);
  Serial.print("Yaw:");
  Serial.println(long2float(packet.Data[0]));
  Serial.print("Pitch:");
  Serial.println(long2float(packet.Data[1]));
  Serial.print("Roll:");
  Serial.println(long2float(packet.Data[2]));

}


void getQTN() {

  RecPacket packet;

  spiRead(&packet, VN100_REG_QTN);

}



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


void getTare() {
  byte buffer[24];
  SPI.beginTransaction(SPISettings(9600, MSBFIRST, SPI_MODE3));
  digitalWrite(chipSelectPin, LOW);
  //byte buffer[24];
  buffer[0] = 5;
  buffer[1] = 0;
  buffer[2] = 0;
  buffer[3] = 0;
  for (int y = 4; y < 24; y++) {
    buffer[y] = 0;
  }
  SPI.transfer(buffer, 24);
  digitalWrite(chipSelectPin, HIGH);
  SPI.endTransaction();
  delay(4000);

  //Second Read cycle synchronizes the data (Not sure if there is a better way)
  //Not doing this will result in current read recieving the previous read's reply
  SPI.beginTransaction(SPISettings(9600, MSBFIRST, SPI_MODE3));
  digitalWrite(chipSelectPin, LOW);
  //byte buffer[24];
  buffer[0] = 5;
  buffer[1] = 0;
  buffer[2] = 0;
  buffer[3] = 0;
  for (int y = 4; y < 24; y++) {
    buffer[y] = 0;
  }

  SPI.transfer(buffer, 24);
  digitalWrite(chipSelectPin, HIGH);
  SPI.endTransaction();
  delay(4000);
  Serial.println("Tare");

}
/*
   This function converts the output which is an
   unsigned long to a float.  It is done because casting it
   as a float doesn't work.
*/

float long2float(long value) {

  byte Sign = (value >> 31) & 1;
  long Exponent = (value >> 23) & 0x000000FF;
  long Fraction = (value) & 0x007FFFFF;
  //Serial.println(Sign,BIN);
  //Serial.println(Exponent);
  //Serial.println(Fraction,BIN);
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

