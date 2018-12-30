/* 
 HTU21D Humidity Sensor Library
 By: Nathan Seidle
 SparkFun Electronics
 Date: September 22nd, 2013
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 Get humidity and temperature from the HTU21D sensor.
 
 This same library should work for the other similar sensors including the Si
 
 */
 
#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>

#define HTU21D_ADDRESS 0x40  //Unshifted 7-bit I2C address for the sensor

#define ERROR_I2C_TIMEOUT 	998
#define ERROR_BAD_CRC		999

#define TRIGGER_TEMP_MEASURE_HOLD  0xE3
#define TRIGGER_HUMD_MEASURE_HOLD  0xE5
#define TRIGGER_TEMP_MEASURE_NOHOLD  0xF3
#define TRIGGER_HUMD_MEASURE_NOHOLD  0xF5
#define WRITE_USER_REG  0xE6
#define READ_USER_REG  0xE7
#define SOFT_RESET  0xFE

#define USER_REGISTER_RESOLUTION_MASK 0x81
#define USER_REGISTER_RESOLUTION_RH12_TEMP14 0x00
#define USER_REGISTER_RESOLUTION_RH8_TEMP12 0x01
#define USER_REGISTER_RESOLUTION_RH10_TEMP13 0x80
#define USER_REGISTER_RESOLUTION_RH11_TEMP11 0x81

#define USER_REGISTER_END_OF_BATTERY 0x40
#define USER_REGISTER_HEATER_ENABLED 0x04
#define USER_REGISTER_DISABLE_OTP_RELOAD 0x02

class HTU21D {

public:
  HTU21D(int sda=SDA, int scl=SCL,TwoWire &wirePort = Wire);

  //Public Functions
//  bool begin( ); //If user doesn't specificy then Wire will be used
  bool readHumidityAsync(float &hum, uint16_t maxmillis=0);
  float readHumidity(void);
  bool readTemperatureAsync(float&temp, uint16_t maxmillis=0);
  float readTemperature(void);
  void setResolution(byte resBits);

  byte readUserRegister(void);
  void writeUserRegister(byte val);


  void startReadValue(byte cmd);
  void checkValue();
  uint16_t readResultValue();

  bool tempStarted,tempChanged;
  int tchangecount;
  float temperature,humidity;
  bool readTimeout , humChanged ;
  uint32_t readts,tempts,humts,checkts,humdelay,tempdelay;
  bool valueReady,humStarted;

  bool init=false,error=false;
  byte sda,scl;

  bool begin();
  bool reset();
private:
  //Private Functions
  TwoWire *_i2cPort; //The generic connection to user's chosen I2C hardware
  byte currentCommand;



  byte checkCRC(uint16_t message_from_sensor, uint8_t check_value_from_sensor);
  uint16_t readValue(byte cmd);

  byte I2CRead(byte address);
  byte I2CRead();
  void I2CWrite(byte address,byte command);
  void I2CWrite(byte command);
  //Private Variables


};


HTU21D::HTU21D(int sda0, int scl0, TwoWire &wirePort)
{
	//Set initial values for private vars
	tempStarted=false;
	humStarted=false;
	readTimeout=false;
	readts=0;
	tempts=0;
	humts=0;
	tempChanged=false;
	humChanged=false;
	_i2cPort = &wirePort; //Grab which port the user wants us to use
	sda=sda0;
	scl=scl0;
}

void HTU21D::I2CWrite(byte address,byte command){
	_i2cPort->beginTransmission(address);
	_i2cPort->write(command);
	_i2cPort->endTransmission();
}
void HTU21D::I2CWrite(byte command){
	I2CWrite(HTU21D_ADDRESS,command);
}
byte HTU21D::I2CRead(byte address){
	_i2cPort->requestFrom((byte)address,(byte) 1);
	return _i2cPort->read();
}
byte HTU21D::I2CRead(){
	return I2CRead(HTU21D_ADDRESS);
}
//Begin
/*******************************************************************************************/
//Start I2C communication
bool HTU21D::begin()
{
	_i2cPort->begin(sda,scl);

	I2CWrite(SOFT_RESET);

	I2CWrite(READ_USER_REG);
	return (I2CRead()==0x2); // after reset should be 0x2

}
bool HTU21D::reset(void) {
	//I2CWrite(HTU21D_ADDRESS,SOFT_RESET);
//	_i2cPort->reset();
//TWCR = 0; // reset TwoWire Control Register to default, inactive state
	I2CWrite(SOFT_RESET);
	// delay(15);
	I2CWrite(READ_USER_REG);
	return (I2CRead()==0x2); // after reset should be 0x2
}

#define MAX_WAIT 1000
#define DELAY_INTERVAL 10
#define MAX_COUNTER (MAX_WAIT/DELAY_INTERVAL)

void HTU21D::startReadValue(byte cmd)
{//Serial.println("HTU21D::startReadValue");
	//if(!init) {if(!begin()) {error=true;return ;}else init=true;}
	//begin();
	currentCommand=cmd;
	I2CWrite(cmd);
	readts=millis();
	valueReady=false;
	checkts=0;
}

void HTU21D::checkValue()
{  byte b=_i2cPort->requestFrom((byte)HTU21D_ADDRESS,(byte) 3);
//	println(std::string()+"HTU21D::checkValue:"+to_string(b));
	valueReady = (3 ==b);
	checkts=millis();
}

uint16_t HTU21D::readResultValue()
{//Serial.println("HTU21D::readResultValue");
	byte msb, lsb, checksum;
	msb = _i2cPort->read();
	lsb = _i2cPort->read();
	checksum = _i2cPort->read();

	uint16_t rawValue = ((uint16_t) msb << 8) | (uint16_t) lsb;

	if (checkCRC(rawValue, checksum) != 0) return (ERROR_BAD_CRC); //Error out

	return rawValue & 0xFFFC; // Zero out the status bits
}


//Given a command, reads a given 2-byte value with CRC from the HTU21D
uint16_t HTU21D::readValue(byte cmd)
{
	currentCommand=cmd;
	I2CWrite(cmd);

	//Hang out while measurement is taken. datasheet says 50ms, practice may call for more
	bool validResult;
	byte counter;
	for (counter = 0, validResult = 0 ; counter < MAX_COUNTER && !validResult ; counter++)
	{
		delay(DELAY_INTERVAL);

		//Comes back in three bytes, data(MSB) / data(LSB) / Checksum
		validResult = (3 == _i2cPort->requestFrom(HTU21D_ADDRESS, 3));
	}

	if (!validResult) return (ERROR_I2C_TIMEOUT); //Error out

	byte msb, lsb, checksum;
	msb = _i2cPort->read();
	lsb = _i2cPort->read();
	checksum = _i2cPort->read();

	uint16_t rawValue = ((uint16_t) msb << 8) | (uint16_t) lsb;

	if (checkCRC(rawValue, checksum) != 0) return (ERROR_BAD_CRC); //Error out

	return rawValue & 0xFFFC; // Zero out the status bits
}

//Read the humidity
/*******************************************************************************************/
//Calc humidity and return it to the user
//Returns 998 if I2C timed out
//Returns 999 if CRC is wrong
float HTU21D::readHumidity(void)
{
	uint16_t rawHumidity = readValue(TRIGGER_HUMD_MEASURE_NOHOLD);

	if(rawHumidity == ERROR_I2C_TIMEOUT || rawHumidity == ERROR_BAD_CRC) return(rawHumidity);

	//Given the raw humidity data, calculate the actual relative humidity
	float tempRH = rawHumidity * (125.0 / 65536.0); //2^16 = 65536
	float rh = tempRH - 6.0; //From page 14

	return (rh);
}

//Read the temperature
bool HTU21D::readTemperatureAsync(float &temp, uint16_t maxmillis)
{readTimeout=false;
	tchangecount=0;
	tempChanged=false;
/*	Serial.println("HTU21D::readTemperatureAsync 0");
	Serial.print(String()+"HTU21D::readTemperatureAsync maxmillis:");
	Serial.print(maxmillis);
	Serial.print(", tempts:");
	Serial.print(tempts);
	Serial.print(", millis:");
	Serial.println(millis());
*/
	if(maxmillis && (millis()-tempts)<maxmillis) {
		temp=temperature;
		return true;}
//	Serial.println("HTU21D::readTemperatureAsync");
	if((tempStarted || humStarted) && (millis()-checkts)<DELAY_INTERVAL) return false; // we should wait more between checks
//	Serial.println("HTU21D::readTemperatureAsync 2");
	if(humStarted) {readHumidityAsync(humidity,0);return false;} //busy
//	Serial.println("HTU21D::readTemperatureAsync 3");
	if(!tempStarted) {
		startReadValue(TRIGGER_TEMP_MEASURE_NOHOLD);
		//Serial.println("HTU21D::readTemperatureAsync tempStarted");
		tempStarted=true;return false;}
	if((millis()-readts)>MAX_WAIT) {tempStarted=false;}//readTimeout=true;return false;}//timeout
//	Serial.println("HTU21D::readTemperatureAsync precheck");
	checkValue();

	if(valueReady) {//Serial.println("HTU21D::readTemperatureAsync value ready");
		uint16_t rawTemperature = readResultValue();;
	//	Serial.println(String()+"HTU21D::readTemperatureAsync temperature raw "+String(rawTemperature));
		if(rawTemperature == ERROR_I2C_TIMEOUT || rawTemperature == ERROR_BAD_CRC || rawTemperature==0) {readTimeout=true;tempStarted=false;return false;}
	//	Serial.println(String()+"HTU21D::readTemperatureAsync temperature raw after ");
		//Given the raw temperature data, calculate the actual temperature
		float tempTemperature = rawTemperature * (175.72 / 65536.0); //2^16 = 65536
		temp = temperature= tempTemperature - 46.85; //From page 14
		tempts=millis();
		tempdelay=tempts-readts;
		tempStarted=false;
		tempChanged=true;
		tchangecount=3;
	//	Serial.println(String()+"HTU21D::readTemperatureAsync tempChanged:"+String((int)tempChanged));
		return true;
	}
//	Serial.println("HTU21D::readTemperatureAsync end");
	return false;
}



bool HTU21D::readHumidityAsync(float &hum, uint16_t maxmillis)
{
	readTimeout=false;
	humChanged=false;
//	 Serial.println(String()+"HTU21D::readHumidityAsync maxmillis:"+String(maxmillis)+", humts:"+String(humts)+", millis:"+millis());
	if(maxmillis && (millis()-humts)<maxmillis) {
		hum=humidity;
		return true;}

	if((tempStarted || humStarted) && (millis()-checkts)<DELAY_INTERVAL) return false; // we should wait more between checks
	//Serial.println("HTU21D::readTemperatureAsync");
	if(tempStarted) {readTemperatureAsync(temperature,0);return false;} //busy
	if(!humStarted) {startReadValue(TRIGGER_HUMD_MEASURE_NOHOLD);humStarted=true;return false;}

	if((millis()-readts)>MAX_WAIT) {tempStarted=false;}//readTimeout=true;return false;}//timeout

	checkValue();

	if(valueReady) {//Serial.println("HTU21D::readTemperatureAsync value ready");
		uint16_t rawHumidity = readResultValue();;
	//	Serial.println(String()+"HTU21D::readTemperatureAsync rawHumidity "+rawHumidity+" "+(rawHumidity&0x03));//status bits ?  https://github.com/Porokhnya/GreenhouseProject/blob/master/UniversalSensorsModule/HTU21D.cpp#L253

		if(rawHumidity == ERROR_I2C_TIMEOUT || rawHumidity == ERROR_BAD_CRC) {readTimeout=true;humStarted=false;return false;}
//		Serial.println(String()+"HTU21D::readTemperatureAsync rawHumidity after");
		//Given the raw humidity data, calculate the actual relative humidity
		float tempRH = rawHumidity * (125.0 / 65536.0); //2^16 = 65536
		hum = humidity = tempRH - 6.0; //From page 14
		humts=millis();
		humdelay=humts-readts;
		humStarted=false;
		humChanged=true;

		return true;
	}
	return false;
}

//Read the temperature
/*******************************************************************************************/
//Calc temperature and return it to the user
//Returns 998 if I2C timed out
//Returns 999 if CRC is wrong
float HTU21D::readTemperature()
{

	uint16_t rawTemperature = readValue(TRIGGER_TEMP_MEASURE_NOHOLD);

	if(rawTemperature == ERROR_I2C_TIMEOUT || rawTemperature == ERROR_BAD_CRC) return(rawTemperature);

	//Given the raw temperature data, calculate the actual temperature
	float tempTemperature = rawTemperature * (175.72 / 65536.0); //2^16 = 65536
	float realTemperature = tempTemperature - 46.85; //From page 14

	return (realTemperature);
}

//Set sensor resolution
/*******************************************************************************************/
//Sets the sensor resolution to one of four levels
//Page 12:
// 0/0 = 12bit RH, 14bit Temp
// 0/1 = 8bit RH, 12bit Temp
// 1/0 = 10bit RH, 13bit Temp
// 1/1 = 11bit RH, 11bit Temp
//Power on default is 0/0

void HTU21D::setResolution(byte resolution)
{
	byte userRegister = readUserRegister(); //Go get the current register state
	userRegister &= B01111110; //Turn off the resolution bits
	resolution &= B10000001; //Turn off all other bits but resolution bits
	userRegister |= resolution; //Mask in the requested resolution bits

	//Request a write to user register
	writeUserRegister(userRegister);
}

//Read the user register
byte HTU21D::readUserRegister(void)
{
	byte userRegister;

	//Request the user register
	_i2cPort->beginTransmission(HTU21D_ADDRESS);
	_i2cPort->write(READ_USER_REG); //Read the user register
	_i2cPort->endTransmission();

	//Read result
	_i2cPort->requestFrom(HTU21D_ADDRESS, 1);

	userRegister = _i2cPort->read();

	return (userRegister);
}

void HTU21D::writeUserRegister(byte val)
{
	_i2cPort->beginTransmission(HTU21D_ADDRESS);
	_i2cPort->write(WRITE_USER_REG); //Write to the user register
	_i2cPort->write(val); //Write the new resolution bits
	_i2cPort->endTransmission();
}







//Give this function the 2 byte message (measurement) and the check_value byte from the HTU21D
//If it returns 0, then the transmission was good
//If it returns something other than 0, then the communication was corrupted
//From: http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
//POLYNOMIAL = 0x0131 = x^8 + x^5 + x^4 + 1 : http://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
#define SHIFTED_DIVISOR 0x988000 //This is the 0x0131 polynomial shifted to farthest left of three bytes

byte HTU21D::checkCRC(uint16_t message_from_sensor, uint8_t check_value_from_sensor)
{
	//Test cases from datasheet:
	//message = 0xDC, checkvalue is 0x79
	//message = 0x683A, checkvalue is 0x7C
	//message = 0x4E85, checkvalue is 0x6B

	uint32_t remainder = (uint32_t)message_from_sensor << 8; //Pad with 8 bits because we have to add in the check value
	remainder |= check_value_from_sensor; //Add on the check value

	uint32_t divsor = (uint32_t)SHIFTED_DIVISOR;

	for (int i = 0 ; i < 16 ; i++) //Operate on only 16 positions of max 24. The remaining 8 are our remainder and should be zero when we're done.
	{
		//Serial.print("remainder: ");
		//Serial.println(remainder, BIN);
		//Serial.print("divsor:    ");
		//Serial.println(divsor, BIN);
		//Serial.println();

		if ( remainder & (uint32_t)1 << (23 - i) ) //Check if there is a one in the left position
			remainder ^= divsor;

		divsor >>= 1; //Rotate the divsor max 16 times so that we have 8 bits left of a remainder
	}

	return (byte)remainder;
}

