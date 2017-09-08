
/*
  CS: pin 7
  MOSI: pin 11
  MISO: pin 12
  SCK: pin 13
*/
#include <SPI.h>
#include "VN100.h"
#include <math.h>
#include <Time.h>

const int chipSelectPin = 7;
const int errorPin = 5;
unsigned long start_time_ms = 0;
unsigned long elapsed_time_ms = 0;


float old_acceleration[3] = {0.0, 0.0, -9.81};
float velocity[3] = {0.0, 0.0, 0.0};
float location[3] = {0.0, 0.0, 0.0};
float old_time = 0;

// struct for recieved data
typedef struct {
  byte zeroByte;
  byte ComID;
  byte RegNum;
  byte ErrorCode;
  unsigned long Data[15]; //Not sure what the best number is for this or if it can be left blank
} RecPacket;

unsigned long Rate = 2000000; // Sets the transfer rate.  It is upto 16Mhz, but the arduino limit is 2Mhz
//This is not the rate that data is updated (Max 800Hz) - only the rate of transfer

void setup() {
  Serial.begin(Rate);

  // start the SPI library:
  SPI.begin();

  // initalize the  data ready and chip select pins:
  pinMode(chipSelectPin, OUTPUT);
  pinMode(errorPin, OUTPUT);

  digitalWrite(errorPin, LOW); //Turn off error LED

  delay(100);

   Tare();
  //   writeTag("DynRoboticsLab2017");
  start_time_ms = millis();
  Serial.println("Setup Complete.");
  //setBaudRate(115200);
  old_time = millis();
}

void loop() {
  displacement();
  delay(500);
  // getTag();
  //  Serial.println("Getting Serial...");
  // getSerial();
  //  Serial.println("Getting Model...");
  //  getModel();
  /*
    getTag();
    getModel();
    getHWRev(); //Correct??
    getSerial();
    getFWVer(); //Not working, I think
    getADOR(); //Correct??
    getADOF(); //Correct??
    getQTN();
    getQTNMag();
    getQTNAcc();
  */
  // getQTNMagAccRates();
  //getYPRMagAccRates();
  // getDCM();
  // getRates();
  // getBaud();
  //    getYPR();
  // speedTest();
  //getAcc();

  //  delay(1000);

}


void speedTest() {
  unsigned long buffer[4];

  buffer[0] = 0x00001201;

  SPI.beginTransaction(SPISettings(Rate, MSBFIRST, SPI_MODE3)); //Settings explained in header
  digitalWrite(chipSelectPin, LOW); //Selects the correct slave (only IMU now)
  SPI.transfer(buffer, 16); //Transfers data
  digitalWrite(chipSelectPin, HIGH); //Deselects the slave
  SPI.endTransaction();

  elapsed_time_ms = millis() - start_time_ms;
  Serial.print(elapsed_time_ms);
  Serial.print("x Rate:");
  Serial.println(buffer[1]);
  Serial.print("y Rate:");
  Serial.println(buffer[2]);
  Serial.print("z Rate:");
  Serial.println(buffer[3]);


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
void spiRead(RecPacket* packet, byte reg, byte Size) {

  unsigned long buffer[Size]; //Buffer to transmit and recieve the data from the IMU

  Transfer(buffer, reg, 1, Size);

  // Parsing the data into the RecPacket data type
  packet->zeroByte = (byte)((buffer[0]) & 0x00000FF);
  packet->ComID = (byte)((buffer[0] >> 8) & 0x00000FF);
  packet->RegNum = (byte)((buffer[0] >> 16) & 0x00000FF);
  packet->ErrorCode = (byte)((buffer[0] >> 24) & 0x00000FF);
  for (int x = 0; x < Size; x++) {
    packet->Data[x] =  buffer[x + 1];
  }
  checkError(packet->ErrorCode);
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


void spiWrite(RecPacket* packet, unsigned long* buffer, byte reg, byte Size) {


  //Combining the commands in to a 4 byte long
  unsigned long temp1 = (unsigned long)(((0 << 8) | (0 << 0)));
  unsigned long temp2 = (unsigned long)(((reg << 8) | (2 << 0)) & 0x0000FFFF);
  temp1 = temp1 * 65536;
  unsigned long combined = temp1 + temp2;

  buffer[0] = combined;
  //  for (int y = 2; y < Size; y++) {
  //   buffer[y] = 0;
  //  }

  // The transfer
  SPI.beginTransaction(SPISettings(Rate, MSBFIRST, SPI_MODE3));
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(buffer, Size * 4);
  digitalWrite(chipSelectPin, HIGH);
  SPI.endTransaction();

  delay(20);
  SPI.beginTransaction(SPISettings(Rate, MSBFIRST, SPI_MODE3));
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(buffer, Size * 4);
  digitalWrite(chipSelectPin, HIGH);
  SPI.endTransaction();

  // Parsing the data into the RecPacket data type
  packet->zeroByte = (byte)((buffer[0]) & 0x00000FF);
  packet->ComID = (byte)((buffer[0] >> 8) & 0x00000FF);
  packet->RegNum = (byte)((buffer[0] >> 16) & 0x00000FF);
  packet->ErrorCode = (byte)((buffer[0] >> 24) & 0x00000FF);

  //  for (int x = 0; x <= 15; x++) {
  //    packet->Data[x] =  buffer[x + 1];
  //  }
  checkError(packet->ErrorCode);

  Serial.print("Zero: ");
  Serial.println(packet->zeroByte);
  Serial.print("Command: ");
  Serial.println(packet->ComID);
  Serial.print("Register: ");
  Serial.println(packet->RegNum);
  Serial.print("Error Code: ");
  Serial.println(packet->ErrorCode);

}


void writeTag(char temptag[20]) {

  char tag[20] = "";

  int x = 0;

  while (temptag[x] != 0) {
    tag[x] = temptag[x];
    x++;
  }

  unsigned long tempbuffer[5];

  for (int x = 0; x < 5; x++) {
    tempbuffer[x] = (((unsigned long)tag[3 + x * 4]) * 16777216) | ((unsigned long)tag[2 + x * 4]) << 16 | ((unsigned long)tag[1 + x * 4]) << 8 | ((unsigned long)tag[0 + x * 4]) << 0;
    // Serial.println(tempbuffer[x+1],HEX);
  }

  //Combining the commands in to a 4 byte long
  unsigned long temp1 = (unsigned long)(((0 << 8) | (0 << 0)));
  unsigned long temp2 = (unsigned long)(((0 << 8) | (2 << 0)) & 0x0000FFFF);
  temp1 = temp1 * 65536;
  unsigned long combined = temp1 + temp2;
  unsigned long buffer[6];
  buffer[0] = combined;
  for (int y = 0; y < 5; y++) {
    buffer[y + 1] = tempbuffer[y];
  }
  RecPacket packet;
  // The transfer
  SPI.beginTransaction(SPISettings(Rate, MSBFIRST, SPI_MODE3));
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(buffer, 6 * 4);
  digitalWrite(chipSelectPin, HIGH);
  SPI.endTransaction();


  // Parsing the data into the RecPacket data type
  packet.zeroByte = (byte)((buffer[0]) & 0x00000FF);
  packet.ComID = (byte)((buffer[0] >> 8) & 0x00000FF);
  packet.RegNum = (byte)((buffer[0] >> 16) & 0x00000FF);
  packet.ErrorCode = (byte)((buffer[0] >> 24) & 0x00000FF);
  for (int x = 0; x < 5; x++) {
    packet.Data[x] =  buffer[x + 1];
  }

  checkError(packet.ErrorCode);

  Serial.print("Zero: ");
  Serial.println(packet.zeroByte);
  Serial.print("Command: ");
  Serial.println(packet.ComID);
  Serial.print("Register: ");
  Serial.println(packet.RegNum);
  Serial.print("Error Code: ");
  Serial.println(packet.ErrorCode);
  Serial.print("Written: ");
  Serial.println(packet.Data[0]);


}


void getTag() {
  RecPacket packet;

  spiRead(&packet, 0, 6);

  // Parse the returned data into ascii characters
  char tag[20];
  for (int c = 0; c < 20; c++) {
    tag[c] = (char)((packet.Data[int(c / 4)] >> (c % 4 * 8)) & 0x000000FF);
  }

  Serial.print("Tag: ");
  Serial.println(tag);
}



/*
   This function returns the model number.
*/
void getModel() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_MODEL, 4);

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


void getHWRev() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_HWREV, 2);

  Serial.print("Hardware Revision Number: ");
  Serial.println(packet.Data[0]);

}



/*
   The function returns the serial number
*/

void getSerial() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_SN, 2);

  Serial.print(" Serial Number: ");
  Serial.println(packet.Data[0]);

}

void getFWVer() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_FWVER, 2);

  Serial.print("Firmware Version: ");
  Serial.println(packet.Data[0]);
}

/*
   This function returns the Baud rate
*/

void getBaud() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_SBAUD, 2);

  Serial.print("Baud Rate: ");
  Serial.println(packet.Data[0]);

}


void setBaudRate(unsigned long rate) {
  RecPacket* packet;
  unsigned long buffer[2];
  buffer[1] = rate;
  spiWrite(packet, buffer, VN100_REG_SBAUD, 2);

}

void getADOR() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_ADOR, 2);

  Serial.print("ADOR: ");
  Serial.println(packet.Data[0]);
}


void setADOR() {

}


void getADOF() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_ADOF, 2);

  Serial.print("ADOF: ");
  Serial.println(packet.Data[0]);
}


void setADOF() {

}


/*
   This function returns the Yaw Pitch Roll
*/

void getYPR() {

  RecPacket packet;

  spiRead(&packet, VN100_REG_YPR, 4);

  Serial.print("Yaw:");
  Serial.println(long2float(packet.Data[0]));
  Serial.print("Pitch:");
  Serial.println(long2float(packet.Data[1]));
  Serial.print("Roll:");
  Serial.println(long2float(packet.Data[2]));

}



/*
   This function returns Quaternions
*/
void getQTN() {

  RecPacket packet;

  spiRead(&packet, VN100_REG_QTN, 5);


  Serial.print("q0:");
  Serial.println(long2float(packet.Data[0]));
  Serial.print("q1:");
  Serial.println(long2float(packet.Data[1]));
  Serial.print("q2:");
  Serial.println(long2float(packet.Data[2]));
  Serial.print("q3:");
  Serial.println(long2float(packet.Data[3]));


  float q0 = long2float(packet.Data[0]);
  float q1 = long2float(packet.Data[1]);
  float q2 = long2float(packet.Data[2]);
  float q3 = long2float(packet.Data[3]);

  //  float YAW = atan((2.0*(q0*q1+q3*q2))/(q3*q3 - q2*q2 - q1*q1 + q0*q0));
  float YAW = atan2((q3 * q3 - q2 * q2 - q1 * q1 + q0 * q0), (2.0 * (q0 * q1 + q3 * q2)));
  YAW = YAW * 180.0 / 3.14;
  //  Serial.print("Yaw (Quat):");
  //  Serial.println(YAW);
  float PITCH = asin(-2.0 * (q0 * q2 - q1 * q3));
  PITCH = PITCH * 180.0 / 3.14;
  // Serial.print("Pitch (Quat):");
  // Serial.println(PITCH);
  float ROLL = atan((2.0 * (q1 * q2 + q0 * q3)) / (q3 * q3 + q2 * q2 - q1 * q1 - q0 * q0));
  ROLL = ROLL * 180.0 / 3.14;
  //  Serial.print("Roll (Quat):");
  //  Serial.println(ROLL);

}

void getQTNMag() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_QTM, 8);


  Serial.print("q0:");
  Serial.println(long2float(packet.Data[0]));
  Serial.print("q1:");
  Serial.println(long2float(packet.Data[1]));
  Serial.print("q2:");
  Serial.println(long2float(packet.Data[2]));
  Serial.print("q3:");
  Serial.println(long2float(packet.Data[3]));
  Serial.print("One:");
  Serial.println(long2float(packet.Data[4]));
  Serial.print("Two:");
  Serial.println(long2float(packet.Data[5]));
  Serial.print("Three:");
  Serial.println(long2float(packet.Data[6]));





}

void getQTNAcc() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_QTA, 8);


  Serial.print("q0:");
  Serial.println(long2float(packet.Data[0]));
  Serial.print("q1:");
  Serial.println(long2float(packet.Data[1]));
  Serial.print("q2:");
  Serial.println(long2float(packet.Data[2]));
  Serial.print("q3:");
  Serial.println(long2float(packet.Data[3]));
  Serial.print("Acc One:");
  Serial.println(long2float(packet.Data[4]));
  Serial.print("Acc Two:");
  Serial.println(long2float(packet.Data[5]));
  Serial.print("Acc Three:");
  Serial.println(long2float(packet.Data[6]));
}

void getQTNRates() {

}


void getQTNMagAcc() {

}


void getQTNAccRates() {

}

void getQTNMagAccRates() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_QMR, 14);


  Serial.print("q0:");
  Serial.println(long2float(packet.Data[0]));
  Serial.print("q1:");
  Serial.println(long2float(packet.Data[1]));
  Serial.print("q2:");
  Serial.println(long2float(packet.Data[2]));
  Serial.print("q3:");
  Serial.println(long2float(packet.Data[3]));
  Serial.print("Mag One:");
  Serial.println(long2float(packet.Data[4]));
  Serial.print("Mag Two:");
  Serial.println(long2float(packet.Data[5]));
  Serial.print("Mag Three:");
  Serial.println(long2float(packet.Data[6]));
  Serial.print("Acc One:");
  Serial.println(long2float(packet.Data[7]));
  Serial.print("Acc Two:");
  Serial.println(long2float(packet.Data[8]));
  Serial.print("Acc Three:");
  Serial.println(long2float(packet.Data[9]));
  Serial.print("Rates One:");
  Serial.println(long2float(packet.Data[10]));
  Serial.print("Rates Two:");
  Serial.println(long2float(packet.Data[11]));
  Serial.print("Rates Three:");
  Serial.println(long2float(packet.Data[12]));
}

void getYPRMagAccRates() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_YMR, 13);

  Serial.print("Yaw:");
  Serial.println(long2float(packet.Data[0]));
  Serial.print("Pitch:");
  Serial.println(long2float(packet.Data[1]));
  Serial.print("Roll:");
  Serial.println(long2float(packet.Data[2]));
  Serial.print("Mag One:");
  Serial.println(long2float(packet.Data[3]));
  Serial.print("Mag Two:");
  Serial.println(long2float(packet.Data[4]));
  Serial.print("Mag Three:");
  Serial.println(long2float(packet.Data[5]));
  Serial.print("Acc One:");
  Serial.println(long2float(packet.Data[6]));
  Serial.print("Acc Two:");
  Serial.println(long2float(packet.Data[7]));
  Serial.print("Acc Three:");
  Serial.println(long2float(packet.Data[8]));
  Serial.print("Rates One:");
  Serial.println(long2float(packet.Data[9]));
  Serial.print("Rates Two:");
  Serial.println(long2float(packet.Data[10]));
  Serial.print("Rates Three:");
  Serial.println(long2float(packet.Data[11]));

}

void getDCM() {
  RecPacket packet;

  spiRead(&packet, VN100_REG_DCM, 10);

  Serial.println("DCM:");
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      Serial.print(long2float(packet.Data[(i * 3) + j]));
      Serial.print("\t");
    }
    Serial.print("\n");
  }

}


void getMag() {

}

/*
   This function returns the accelleration rate (m/s^2)
*/

void getAcc() {

  RecPacket packet;

  spiRead(&packet, VN100_REG_ACC, 4);

  Serial.print("x rate:");
  Serial.println(long2float(packet.Data[0]));
  Serial.print("y rate:");
  Serial.println(long2float(packet.Data[1]));
  Serial.print("z rate:");
  Serial.println(long2float(packet.Data[2]));


}



void getRates() {

  RecPacket packet;

  spiRead(&packet, VN100_REG_GYR, 4);
  elapsed_time_ms = millis() - start_time_ms;
  Serial.print(elapsed_time_ms);
  Serial.print("Yaw Rate:");
  Serial.println(long2float(packet.Data[0]));
  Serial.print("Pitch Rate:");
  Serial.println(long2float(packet.Data[1]));
  Serial.print("Roll Rate:");
  Serial.println(long2float(packet.Data[2]));

}


void getMagAccRates() {

}

void getMagAccRef() {

}

void setMagAccRef() {

}

void getFiltMeasVar() {

}

void setFiltMeasVar() {

}

void getHardSoftIronComp() {

}

void setHardSoftIronComp() {

}

void getFiltActTuning() {

}

void setFiltActTuning() {

}


void getAccComp() {

}

void setAccComp() {

}


void getRefFrameRot() {

}

void setRefFrameRot() {

}


void getAccGain() {

}

void setAccGain() {

}


void Restore() {

}


/*
   This function tares the device
   (It needs to be fixed to work like spiRead)
*/

void Tare() {

  unsigned long buffer[1]; //Buffer to transmit and recieve the data from the IMU

  //Combining the commands in to a 4 byte long
  unsigned long temp1 = (unsigned long)(((0 << 8) | (0 << 0)));
  unsigned long temp2 = (unsigned long)(((0 << 8) | (5 << 0)) & 0x0000FFFF);
  temp1 = temp1 * 65536;
  unsigned long combined = temp1 + temp2;
  buffer[0] = combined;

  // The first transfer - we ignore this data see Transfer function for more information
  SPI.beginTransaction(SPISettings(Rate, MSBFIRST, SPI_MODE3));
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer(buffer, 1);
  digitalWrite(chipSelectPin, HIGH);
  SPI.endTransaction();

  delay(1); //It takes 50us for the IMU to package the data

  // The second transfer - we keep this data see Transfer function for more information
  SPI.beginTransaction(SPISettings(Rate, MSBFIRST, SPI_MODE3));
  digitalWrite(chipSelectPin, LOW);
  buffer[0] = combined;
  SPI.transfer(buffer, 4);
  digitalWrite(chipSelectPin, HIGH);
  SPI.endTransaction();

  Serial.println("Tareing - Don't move the IMU!");
  delay(4000);
  Serial.println("Tare Complete!");

  //  // Parsing the data into the RecPacket data type
  //  byte zeroByte = (byte)((buffer[0]) & 0x00000FF);
  //  byte ComID = (byte)((buffer[0] >> 8) & 0x00000FF);
  //  byte RegNum = (byte)((buffer[0] >> 16) & 0x00000FF);
  //  byte ErrorCode = (byte)((buffer[0] >> 24) & 0x00000FF);
  //
  //  Serial.print("Zero: ");
  //  Serial.println(zeroByte);
  //  Serial.print("Command: ");
  //  Serial.println(ComID);
  //  Serial.print("Register: ");
  //  Serial.println(RegNum);
  //  Serial.print("Error Code: ");
  //  Serial.println(ErrorCode);

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

/*
   This important function is where the actual communication with the LMU takes place.
   As noted in the code, it sends data two times.  This is because the device only
   responds on the next transfer.  If you don't do this, the response will not be
   synchronized with the read or write made.
   unsigned long* buffer is an empty buffer which will be used to transfer
   the command and data to the device and to hold the returned data
   byte reg is the register number that you wish to access.  Details are in VN100.h
   byte Comm is the command that you wish to do (read/write/etc). Details are in VN100.h
   byte Size is the sixe of the buffer (in 4 byte words).  This can be very important for
   some commands
   The function receives the data and makes  a command out of the register number and command.
   It then transfers this twice to synchronize the data

   SPI Transfer settings are based on the following (from manual):
    CPOL = 1 (idle high)
    CPHA = 1 (data sampled on the clock rising edge and sent on the falling edge)
    It appears to be MSBFIRST, This means SPI_MODE3
*/

void Transfer(unsigned long* buffer, byte reg, byte Comm, byte Size) {

  //Combining the commands in to a 4 byte long
  //  unsigned long temp1 = (unsigned long)(((0 << 8) | (0 << 0)));
  //  unsigned long temp2 = (unsigned long)(((reg << 8) | (Comm << 0)) & 0x0000FFFF);
  //  temp1 = temp1 * 65536;
  //  unsigned long combined = temp1 + temp2;

  unsigned long combined = (unsigned long)((0 << 24) | (0 << 16) | (reg << 8) | (Comm << 0));

  buffer[0] = combined;
  for (int y = 1; y < Size; y++) {
    buffer[y] = 0;
  }
  /*
     We will make two transfers to get the required data/action.  The Response to the
     first transfer occurs during the second transfer.  This isn't an issue when you
     repeatedly call for teh same information, but it does matter when you call for different
     data squentially or during single calls in setup.
  */
  // The first transfer - this is the data we will get
  SPI.beginTransaction(SPISettings(Rate, MSBFIRST, SPI_MODE3)); //Settings explained in header
  digitalWrite(chipSelectPin, LOW); //Selects the correct slave (only IMU now)
  SPI.transfer(buffer, Size * 4); //Transfers data
  digitalWrite(chipSelectPin, HIGH); //Deselects the slave
  SPI.endTransaction();

  delay(1); //Need to wait atleast 50us for the IMU to package the data

  // The second tansfer - We receieve the data from the previous transfer and ignore the data that returns.
  SPI.beginTransaction(SPISettings(Rate, MSBFIRST, SPI_MODE3));
  digitalWrite(chipSelectPin, LOW);
  buffer[0] = combined;
  SPI.transfer(buffer, Size * 4);
  digitalWrite(chipSelectPin, HIGH);
  SPI.endTransaction();

}

/*
   This function looks at the error code returned by the IMU when a command is given
   and halts the program for any value besides 0 (no error).  It also lights up the LED
   on the board.  It halts the program by entering a while loop with no escape.
   The LED is turned off when setup is executed.
*/
void checkError(byte errorCode) {
  switch (errorCode) {
    case VN100_Error_None:
      //No error so do nothing
      break;
    case VN100_Error_HardFaultException:
      Serial.print("Error! Hard Fault Exception");
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
    case VN100_Error_InputBufferOverflow:
      Serial.print("Error! Input Buffer Overflow");
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
    case VN100_Error_InvalidChecksum:
      Serial.print("Error! Invalid Checksum");
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
    case VN100_Error_InvalidCommand:
      Serial.print("Error! Invalid Command");
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
    case VN100_Error_NotEnoughParameters:
      Serial.print("Error! Not Enough Parameters");
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
    case VN100_Error_TooManyParameters:
      Serial.print("Error! Too Many Parameters");
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
    case VN100_Error_InvalidParameter:
      Serial.print("Error! Invalid Parameter");
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
    case VN100_Error_InvalidRegister:
      Serial.print("Error! Invalid Register");
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
    case VN100_Error_UnauthorizedAccess:
      Serial.print("Error! Unauthorized Access");
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
    case VN100_Error_WatchdogReset:
      Serial.print("Error! Watch Dog Reset");
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
    case VN100_Error_OutputBufferOverflow:
      Serial.print("Error! Output Buffer Overflow");
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
    case VN100_Error_InsufficientBandwidth:
      Serial.print("Error! Insufficient Bandwidth");
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
    default:
      Serial.println("Error! Cause Unknown"); //This would indicate data might be being parsed incorrectly
      Serial.print(errorCode);
      while (1) {
        digitalWrite(errorPin, HIGH);
      }
      break;
  }
}
void displacement() {

  RecPacket packet;

  spiRead(&packet, VN100_REG_ACC, 4);



  float acceleration[3]={0.0, 0.0, 0.0};
float temp;

Serial.print("Accel (x, y, z): ");
  for (int x = 0; x < 3; x++) {
    temp = long2float(packet.Data[x]);
    if(fabs(temp)>0.05){
    acceleration[x] = (temp + old_acceleration[x]) / 2;
    }else{
      acceleration[x] = old_acceleration[x];
    }
    old_acceleration[x] = acceleration[x];
    Serial.print(acceleration[x]);
Serial.print("\t");
  }
Serial.print("\n");
  
  float Now = millis();
  float elapsed = (Now - old_time)/1000.0;
  old_time = Now;
  Serial.print("Vel (x, y, z): ");
  for (int x = 0; x < 3; x++) {
    velocity[x] = velocity[x] + elapsed * acceleration[x];
    Serial.print(velocity[x]);
Serial.print("\t");
  }
Serial.print("\n");

Serial.print("Location (x, y, z): ");
  for (int x = 0; x < 3; x++) {
    location[x] = location[x] + elapsed * velocity[x];
    Serial.print(location[x]);
Serial.print("\t");
  }
Serial.print("\n");

}

