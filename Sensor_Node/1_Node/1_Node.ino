#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <BME280I2C.h>  // https://github.com/finitespace/BME280
#include "SSD1306.h"
#include "fonts.h"  //Open_Sans_Hebrew_Condensed_14, Open_Sans_Hebrew_Condensed_18, Open_Sans_Hebrew_Condensed_24

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)

#define BAND  868E6  // 433E6, 868E6, 915E6
#define TXPOW 18 // 2~20 -> PA_OUTPUT_RF0_PIN, 0~14-> PA_OUTPUT_BOOST_PIN
#define SF 7 // Spreading Factor: 6~12, default 7
#define SBW 125E3 // Signal Bandwidth: 7.83E, 10.4E3, 15.6E3, 20.8E3, 41.7E3, 62.5E3, 125.E3, 250E3, default 125E3
#define CR 5 // Coding Rate: 5~8, default 5

#define DEBUG 0  // 0: normal state, 1: radom/fixed sensor values
const uint8_t GP2Y10 = 34;
const uint8_t dipPin1 = 15;
const uint8_t dipPin2 = 2;  // LED pin - should be replaced
const uint8_t dipPin3 = 4;
const uint8_t dipPin4 = 0;  // CLX1 pin - should be replaced

BME280I2C::Settings settings(
	BME280::OSR_X1,  // temp oversampling
	BME280::OSR_X1,  // humidity oversampling
	BME280::OSR_X1,  // pressure oversampling
	BME280::Mode_Forced,  // operation mode
	BME280::StandbyTime_1000ms,  // standby time
	BME280::Filter_2,  // filter
	BME280::SpiEnable_False,  // spi interface
	BME280I2C::I2CAddr_0x76 // I2C address. I2C specific.
);
BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
BME280::PresUnit presUnit(BME280::PresUnit_Pa);

SSD1306 display(0x3c, 21, 22);
BME280I2C bme(settings);


uint8_t dip1, dip2, dip3, dip4, NodeNum = 0;

void setup() {
	delay(200);
	Serial.begin(115200);
	Serial.println("\n\nDevice - LoRa Node");
	if(DEBUG == 1){
		Serial.println("#########################################");
		Serial.println("#  Debug mode on - random sensor values  #");
		Serial.println("#########################################");
	}

	Wire.begin();

  pinMode(GP2Y10, INPUT);
	pinMode(16, OUTPUT);  // OLED reset pin
	digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
	delay(50);
	digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

	pinMode(dipPin1, INPUT);  // CLX1 pin, seems to have special function - 2.72V on LOW
	pinMode(dipPin2, INPUT_PULLDOWN);  //all connected to 3V3
	pinMode(dipPin3, INPUT_PULLDOWN);
	pinMode(dipPin4, INPUT_PULLDOWN);

	SPI.begin(SCK,MISO,MOSI,SS);
	LoRa.setPins(SS,RST,DI0);
	LoRa.setTxPower(TXPOW);
	LoRa.setSpreadingFactor(SF);
	LoRa.setSignalBandwidth(SBW);
	LoRa.setCodingRate4(CR);
	if (LoRa.begin(BAND)) {
		Serial.println("LoRa Initialized.");
	}else{
		Serial.println("LoRa failed to start.");
		while (1){
			delay(1000);
		}
	}
	LoRa.receive();
	
	if(DEBUG == 0){
		uint8_t bmeInitCount = 0;
		while(!bme.begin()){
			Serial.println("BEM280 Not Found.");
			delay(1000);

			bmeInitCount++;
			if(bmeInitCount>4) {	break;	}
		}
		switch(bme.chipModel()){
			case BME280::ChipModel_BME280:
				Serial.println("BME280 Initialized.");
				break;
			case BME280::ChipModel_BMP280:
				Serial.println("BMP280 Initialized. No Humidity available.");
				break;
			default:
				Serial.println("BME280 Initialization FAILED!");
		}
	}

	display.init();
	display.flipScreenVertically();

	dip1 = digitalRead(dipPin1);  // read node number from external DIP switch
	dip2 = digitalRead(dipPin2);
	dip3 = digitalRead(dipPin3);
	dip4 = analogRead(dipPin4)>4000?1:0;  // designated pin CLX1: seems to have special function, disconnect before upload
	NodeNum = dip1 + dip2*2 + dip3*4 + dip4*8;

	Serial.print("\nDIP1: "); Serial.println(dip1);
	Serial.print("DIP2: "); Serial.println(dip2);
	Serial.print("DIP3: "); Serial.println(dip3);
	Serial.print("DIP4: "); Serial.println(dip4);
	Serial.print("\nNodeNum:"); Serial.println(NodeNum);

  Serial.println("Init complete\n");
	delay(1200);
}

String packet = "";
float bmetemp(NAN), bmehum(NAN), bmepres(NAN);
float dust = 0.0;

void loop() {
	dip1 = digitalRead(dipPin1);  // check for update on DIP switch
	dip2 = digitalRead(dipPin2);
	dip3 = digitalRead(dipPin3);
	dip4 = analogRead(dipPin4)>4000?1:0;  // designated pin CLX1: seems to have special function, pull low before upload
	NodeNum = dip1 + dip2*2 + dip3*4 + dip4*8;

	if(DEBUG == 0){
  	bme.read(bmepres, bmetemp, bmehum, tempUnit, presUnit);
		Serial.print("\nNodeNum:"); Serial.println(NodeNum);
		Serial.print("Temp: "); Serial.print(bmetemp); Serial.print("°"+ String(tempUnit==BME280::TempUnit_Celsius?"C":"F"));
		Serial.print("    Humidity: "); Serial.print(bmehum); Serial.print("% RH");
		Serial.print("    Pressure: "); Serial.print(bmepres); Serial.println("Pa");

		// printBME280Data(&Serial);
		dust = pulse2ugm3(pulseIn(GP2Y10, LOW, 20000));
  	Serial.print("Dust: "); Serial.print(dust, 3); Serial.println(" ug/m3\n");
	}else{
		bmetemp = 22.0 + random(0, 1000)/100.0;  // 22~32
		bmehum = 25.0 + random(0, 5000)/100.0;  // 25~75
		bmepres = 1010000 + random(0, 8000000)/100.0;  // 101000 ~ 109000
		Serial.print("Temp: "); Serial.print(bmetemp); Serial.print("°"+ String(tempUnit==BME280::TempUnit_Celsius?"C":"F"));
		Serial.print("    Humidity: "); Serial.print(bmehum); Serial.print("% RH");
		Serial.print("    Pressure: "); Serial.print(bmepres); Serial.println("Pa");

		dust = 10.0 + random(0, 1000)/100.0; // 10~20
  	Serial.print("Dust: "); Serial.print(dust, 3); Serial.println(" ug/m3\n");
	}

	packet = "";  // NodeNum, dust, temp, hum
	packet.concat("$");
	packet.concat(String(NodeNum));
	packet.concat("#");
	packet.concat(String(dust, 2));
	packet.concat("#");
	packet.concat(bmetemp);
	packet.concat("#");
	packet.concat(bmehum);
	packet.concat("$  ");
	
	LoRa.beginPacket();
	LoRa.print(packet);
	LoRa.endPacket();

	delay(1000);
}


float pulse2ugm3(uint16_t pulse){
  float value = (pulse-1400)/14.0;
  if(value > 300){
    value = 0;
  }
  return value;
}

void printBME280Data(Stream* client){
  bme.read(bmepres, bmetemp, bmehum, tempUnit, presUnit);

  client->print("Temp: ");
  client->print(bmetemp);
  client->print("°"+ String(tempUnit == BME280::TempUnit_Celsius ? 'C' :'F'));
  client->print("    Humidity: ");
  client->print(bmehum);
  client->print("% RH");
  client->print("    Pressure: ");
  client->print(bmepres);
  client->println("Pa");
}
