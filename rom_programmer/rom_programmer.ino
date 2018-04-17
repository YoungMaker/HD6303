#define REG_STROBE 9 //latches data into reg on HIGH pulse
#define REG_DATA 8 
#define REG_CLK 7

#define ROM_WE 10 //ROM write enable

#define ROM_D0 6
#define ROM_D1 5
#define ROM_D2 4
#define ROM_D3 3
#define ROM_D4 2
#define ROM_D5 11
#define ROM_D6 12
#define ROM_D7 13

#define OE_ACTIVE 0x00
#define OE_INACTIVE 0x80

#define WE_INACTIVE HIGH
#define WE_ACTIVE LOW

#include <Arduino.h>

void setAddress(int address, boolean outputEnable)
{
	shiftOut(REG_DATA, REG_CLK, MSBFIRST, (address >> 8) | (outputEnable ? OE_ACTIVE : OE_INACTIVE)); //top 8 bits
	shiftOut(REG_DATA, REG_CLK, MSBFIRST, (address)); //bottom 8 bits
	
	digitalWrite(REG_STROBE, HIGH); //flicks the REG_STROBE so that the data is latched in the internal register
	delayMicroseconds(1);
	digitalWrite(REG_STROBE, LOW); //reset REG_STROBE so that new data will not be latched. 
}

int readData(int address) {
  clearDataPins();
  setDataPinsInput();
  setAddress(address, true);
 
  int data = 0x00; 
  data = (data << 1) + digitalRead(ROM_D7);
  data = (data << 1) + digitalRead(ROM_D6);
  data = (data << 1) + digitalRead(ROM_D5);
  data = (data << 1) + digitalRead(ROM_D4);
  data = (data << 1) + digitalRead(ROM_D3);
  data = (data << 1) + digitalRead(ROM_D2);
  data = (data << 1) + digitalRead(ROM_D1);
  data = (data << 1) + digitalRead(ROM_D0);
  return data;
}
void clearDataPins(){
  digitalWrite(ROM_D0, LOW);
  digitalWrite(ROM_D1, LOW);
  digitalWrite(ROM_D2, LOW);
  digitalWrite(ROM_D3, LOW);
  digitalWrite(ROM_D4, LOW);
  digitalWrite(ROM_D5, LOW);
  digitalWrite(ROM_D6, LOW);
  digitalWrite(ROM_D7, LOW);
}

void setDataPinsInput(){
	pinMode(ROM_D0, INPUT);
	pinMode(ROM_D1, INPUT);
	pinMode(ROM_D2, INPUT);
	pinMode(ROM_D3, INPUT);
	pinMode(ROM_D4, INPUT);
	pinMode(ROM_D5, INPUT);
	pinMode(ROM_D6, INPUT);
	pinMode(ROM_D7, INPUT);
}

void setDataPinsOutput(){
	pinMode(ROM_D0, OUTPUT);
	pinMode(ROM_D1, OUTPUT);
	pinMode(ROM_D2, OUTPUT);
	pinMode(ROM_D3, OUTPUT);
	pinMode(ROM_D4, OUTPUT);
	pinMode(ROM_D5, OUTPUT);
	pinMode(ROM_D6, OUTPUT);
	pinMode(ROM_D7, OUTPUT);
}

void writeData(int address, int data){ //can't use loops since the pins are not in order
 setAddress(address, false); // this must be done first so that the OE is set LOW and there isn't bus contention
 clearDataPins();
 setDataPinsOutput();
 digitalWrite(ROM_D0, data &1); 
 data = data >> 1; //shift right one to get the next bit of data to the end
 digitalWrite(ROM_D1, data &1); //& 1 applies the last bit of data. by anding the last bit with 0x01 
 data = data >> 1;
 digitalWrite(ROM_D2, data &1);
  data = data >> 1;
 digitalWrite(ROM_D3, data &1);
  data = data >> 1;
 digitalWrite(ROM_D4, data &1);
  data = data >> 1;
 digitalWrite(ROM_D5, data &1);
  data = data >> 1;
 digitalWrite(ROM_D6, data &1);
  data = data >> 1;
 digitalWrite(ROM_D7, data &1);

 digitalWrite(ROM_WE, WE_ACTIVE); //flick the write enable pin low for 200us
 delayMicroseconds(1); // 1000ns pulse for write enable
 digitalWrite(ROM_WE, WE_INACTIVE);
 delay(2); //write cycle time 

 clearDataPins();
 setDataPinsInput();
}

void printDataBlock(int block_start, int block_end){
   for(int b=block_start; b<block_end; b+=16) {
    Serial.print(b, HEX);
    Serial.print(": ");
    for(int i=0; i<16; i++){
      Serial.print(readData(b+i), HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void printResetVector(){
  Serial.print("HD6303 reset vector: ");
  Serial.print(readData(0x7FE), HEX);
  Serial.println(readData(0x7FF), HEX);
}

void erase(int block_start, int block_end){
  for(int i=block_start; i<block_end; i++){
    writeData(i, 0xFF);
  }
}

void setup() {
  // put your setup code here, to run once:
	pinMode(REG_STROBE, OUTPUT);
	pinMode(REG_DATA, OUTPUT);
	pinMode(REG_CLK, OUTPUT);
	pinMode(ROM_WE, OUTPUT);
  setDataPinsInput();
	Serial.begin(9600);
  
	digitalWrite(ROM_WE, WE_INACTIVE); //set ROM to read mode
	digitalWrite(REG_STROBE, LOW); //close the shift register output/internal register off
  delay(1);
  Serial.println("ERASING");
  erase(0, 0x050);
  Serial.println("WRITING");
  //write reset vector
  writeData(0x7FE, 0xE0); //chip select ROM
  writeData(0x7FF, 0x00); //ROM addres 00
  //write program- this should configure & turn on port 23 continously, jumping back to E000 each time
  writeData(0x00, 0x86); //ldaa
  writeData(0x01, 0x18); //0x18 (load 18 into reg a)
  writeData(0x02, 0x97); //staa (store a at mem address)
  writeData(0x03, 0x01); //PORT2_DDR LOW (internal port data direction register address). Config pin 23 for output
  writeData(0x04, 0x86); //ldaa
  writeData(0x05, 0x10); // 0x08 = 0b01000
  writeData(0x06, 0x97); //staa
  writeData(0x07, 0x03); //PORT2_DATA LOW (internal port data register address)
  writeData(0x08, 0x7E); // jmp offset address X  (current address) + d (the next byte, negative 0x0b, 0xF5)
  writeData(0x09, 0xE0); //HIGH byte of jump, select ROM
  writeData(0x0A, 0x04); //Jump to ROM loc 0x04

  printResetVector();
  printDataBlock(0x00, 0x100);
  Serial.flush();
}

void loop() {
  // put your main code here, to run repeatedly:
  while(!Serial.available() > 0){
    if(Serial.read() == 'r'){
      printDataBlock(0, 256);
      Serial.flush();
    }
  }
}
