#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "SSD1306.h"
#include "fonts.h"  //Open_Sans_Hebrew_Condensed_14, Open_Sans_Hebrew_Condensed_18, Open_Sans_Hebrew_Condensed_24

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)

#define BAND  915E6  // 433E6, 868E6, 915E6
#define TXPOW 20 // 2~20 -> PA_OUTPUT_RF0_PIN, 0~14-> PA_OUTPUT_BOOST_PIN
#define SF 7 // Spreading Factor: 6~12, default 7
#define SBW 125E3 // Signal Bandwidth: 7.83E, 10.4E3, 15.6E3, 20.8E3, 41.7E3, 62.5E3, 125.E3, 250E3, default 125E3
#define CR 8 // Coding Rate: 5~8, default 5


WiFiMulti WiFiMulti;
HTTPClient http;
SSD1306 display(0x3c, 21, 22);

void printInfo();
int filter_okay(float val1, float val2, float newVal);


void setup() {
	Serial.begin(115200);
	Serial.println("\n\nDevice - LoRa Gateway");

	pinMode(16, OUTPUT);  // OLED reset pin
	pinMode(2, OUTPUT);  //LED
	digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
	delay(50);
	digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

	display.init();
	display.flipScreenVertically();
	display.clear();
	display.setFont(Open_Sans_Hebrew_Condensed_24);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(0, 0, "LoRa Gateway");
	printInfo();
	display.display();

	WiFi.mode(WIFI_STA);
	WiFiMulti.addAP("DimiFi 2G1", "newdimigo");
	WiFiMulti.addAP("DimiFi 2G2", "newdimigo");
	WiFiMulti.addAP("DimiFi-2G", "newdimigo");
	WiFiMulti.addAP("DimiFi_2G", "newdimigo");
	WiFiMulti.addAP("DimiFi-1", "newdimigo");
	Serial.print("Establishing WiFI connection");
	uint16_t wifiCount = 0;
	while(WiFiMulti.run() != WL_CONNECTED) {
		// ESP.wdtFeed();  // uneccesary for ESP32
		Serial.print(".");
		delay(250);
		wifiCount++;
		if(wifiCount>60){
			ESP.restart();  // WiFi connection takes too long
			return;
		}
	}
	Serial.print("\nWiFi Connected: ");
	Serial.println(WiFi.SSID());
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	WiFi.printDiag(Serial);
	Serial.println();

	display.clear();
	display.setFont(Open_Sans_Hebrew_Condensed_24);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(0, 0, "LoRa Gateway");
	display.setFont(Open_Sans_Hebrew_Condensed_18);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(0, 26, "WiFi connected");
	printInfo();
	display.display();

	SPI.begin(SCK,MISO,MOSI,SS);
	LoRa.setPins(SS,RST,DI0);
	LoRa.setTxPower(TXPOW);
	LoRa.setSpreadingFactor(SF);
	LoRa.setSignalBandwidth(SBW);
	LoRa.setCodingRate4(CR);
	if (LoRa.begin(BAND)) {
		Serial.println("LoRa Initialized");
	}else{
		Serial.println("LoRa failed to start");
		while (1){
			delay(1000);
		}
	}
	LoRa.receive();

  Serial.println("Init complete\n");
	delay(200);
}


String packet = "";
int packetSize = 0;
String pac_history="";

uint32_t count_packnum = 0;
uint32_t count_packnum_a[6] = {0};
uint8_t pac_NodeNum = 0, pac_okay = 1;
float pac_dust, pac_temp, pac_hum, pac_pres = 0;

float filt_dust[6][2] = {0.0};
float filt_temp[6][2] = {0.0};
float filt_hum[6][2] = {0.0};
float filt_pres[6][2] = {0.0};

float httpData[6][4] = {0.0};

void loop() {
	packetSize = LoRa.parsePacket();
	if (packetSize){  // if(non-zero) -> true
		Serial.println("\n\n=====");

		packet ="";
		for (int i = 0; i < packetSize; i++){
			packet.concat((char) LoRa.read());
		}

		packet.trim();

		Serial.print("count_packnum: ");  Serial.println(count_packnum);
		Serial.print("Packet: ");  Serial.println(packet);
		Serial.print("RSSI: ");  Serial.println(String(LoRa.packetRssi(), DEC));

		if(packet.startsWith("$") && packet.endsWith("$")){
			pac_okay = 1;

			String data="";
			packet.replace("$", "");
			data = packet.substring(0, packet.indexOf("#"));
			pac_NodeNum = data.toInt();
			packet = packet.substring(packet.indexOf("#")+1);
			data = packet.substring(0, packet.indexOf("#"));
			pac_dust = data.toFloat();
			packet = packet.substring(packet.indexOf("#")+1);
			data = packet.substring(0, packet.indexOf("#"));
			pac_temp = data.toFloat();
			packet = packet.substring(packet.indexOf("#")+1);
			data = packet.substring(0, packet.indexOf("#"));
			pac_hum = data.toFloat();
			packet = packet.substring(packet.indexOf("#")+1);
			data = packet.substring(0, packet.indexOf("#"));
			pac_pres = data.toFloat();
			// data before filtering
			Serial.print("pac_NodeNum: "); Serial.println(pac_NodeNum);
			Serial.print("pac_dust: "); Serial.print(pac_dust); Serial.println("ug/m3");
			Serial.print("pac_temperature: "); Serial.print(pac_temp); Serial.println("°C");
			Serial.print("pac_humidity: "); Serial.print(pac_hum); Serial.println("% RH");
			Serial.print("pac_pressure: "); Serial.print(pac_pres); Serial.println("Pa");
			
			// data filtering
			if(pac_NodeNum<0 || pac_NodeNum>16 || pac_dust<0 || pac_temp<=0 || pac_hum<=0 || pac_pres<=0){
				Serial.println("!!!!! abnormal packet data - dropping !!!!!");
				pac_okay=0;
			}

			count_packnum_a[pac_NodeNum-1]++;

			if(count_packnum>3 && count_packnum_a[pac_NodeNum-1]>3 && pac_okay == 1){
				if(filter_okay(filt_temp[pac_NodeNum-1][0], filt_temp[pac_NodeNum-1][1], pac_temp)!=0 ) {  pac_okay = 0; Serial.println("INVALID temp, FILTER_OKAY!=0");  }
				if(filter_okay(filt_hum[pac_NodeNum-1][0], filt_hum[pac_NodeNum-1][1], pac_hum)!=0 ) {  pac_okay = 0; Serial.println("INVALID hum, FILTER_OKAY!=0");  }
				if(filter_okay(filt_pres[pac_NodeNum-1][0], filt_pres[pac_NodeNum-1][1], pac_pres)!=0 ) {  pac_okay = 0; Serial.println("INVALID pres, FILTER_OKAY!=0");  }
			}

			if(pac_okay == 1){
				filt_dust[pac_NodeNum-1][0] = filt_dust[pac_NodeNum-1][1];
				filt_dust[pac_NodeNum-1][1] = pac_dust;
				filt_temp[pac_NodeNum-1][0] = filt_temp[pac_NodeNum-1][1];
				filt_temp[pac_NodeNum-1][1] = pac_temp;
				filt_hum[pac_NodeNum-1][0] = filt_hum[pac_NodeNum-1][1];
				filt_hum[pac_NodeNum-1][1] = pac_hum;
				filt_pres[pac_NodeNum-1][0] = filt_pres[pac_NodeNum-1][1];
				filt_pres[pac_NodeNum-1][1] = pac_pres;
				count_packnum ++;

				String temp = "";
				temp = pac_NodeNum;
				temp.concat(pac_history);
				pac_history = temp;

				if(pac_history.length()>16) {  pac_history = pac_history.substring(0, 16);  }
				Serial.print("pac_history: "); Serial.println(pac_history);

				display.clear();
				display.setFont(Open_Sans_Hebrew_Condensed_18);
				display.setTextAlignment(TEXT_ALIGN_RIGHT);
				display.drawString(128, 0, "RSSI:" + String(LoRa.packetRssi(), DEC));
				display.setFont(Open_Sans_Hebrew_Condensed_24);
				display.setTextAlignment(TEXT_ALIGN_LEFT);
				display.drawStringMaxWidth(0, 0, 64, String(count_packnum));
				display.setFont(Open_Sans_Hebrew_Condensed_18);
				display.setTextAlignment(TEXT_ALIGN_LEFT);
				display.drawStringMaxWidth(0, 26, 128, pac_history);
				printInfo();
				display.display();

				const char* webLink = "https://oongyi.com/lora/get/?num=<NodeNum>&dust=<DUST>&temp=<TEMP>&humid=<HUM>&pres=<PRESS>&status=set&co2=<CO2>&batt=<BATT>";
				String conLink = webLink;
				conLink.replace("<NodeNum>", String(pac_NodeNum));
				conLink.replace("<DUST>", String(pac_dust));
				conLink.replace("<TEMP>", String(pac_temp));
				conLink.replace("<HUM>", String(pac_hum));
				conLink.replace("<PRESS>", String(pac_pres));
				conLink.replace("&co2=<CO2>", "");
				conLink.replace("&batt=<BATT>", "");
				conLink.replace("<STATUS>", "200");

				http.begin(conLink);
				if (http.GET() == HTTP_CODE_OK) {
					String payload = http.getString();
					Serial.print("HTTP connection success: "); Serial.println(payload);
				}else{
					Serial.println("HTTP Connection Error");
					Serial.flush();
					delay(50);
					http.end();
					return;
					// ESP.restart();
				}
				http.end();
			}
		}
		Serial.println("=====\n");
	}
	
	Serial.flush();
	// delay(50);
}

void printInfo(){
	display.setFont(Open_Sans_Hebrew_Condensed_14);
	display.setTextAlignment(TEXT_ALIGN_RIGHT);

	switch( String(BAND).substring(0, 3).toInt() ) {
		case 433:
			display.drawString(128, 48, "TXPOW: " + String(TXPOW) + ", 433Mhz, SF " + String(SF));
			break;

		case 868:
			display.drawString(128, 48, "TXPOW: " + String(TXPOW) + ", 868Mhz, SF " + String(SF));
			break;

		case 915:
			display.drawString(128, 48, "TXPOW: " + String(TXPOW) + ", 915Mhz, SF " + String(SF));
			break;
		
		default:
			break;
	}
}

int filter_okay(float val1, float val2, float newVal){
	val1 = val1<0?val1*(-1):val1;
	val2 = val2<0?val2*(-1):val2;
	newVal = newVal<0?newVal*(-1):newVal;

	float val_mean = (val1 + val2)/2;
	val_mean = val_mean<0?val_mean*(-1):val_mean;

	if(val_mean*(0.5) >= newVal){
		Serial.println("Catch1");
		return 10;  // not okay
	}else if(val_mean*(1.5) <= newVal){
		Serial.println("Catch2");
		return 10;
	}else{
		return 0;  // okay
	}
}
