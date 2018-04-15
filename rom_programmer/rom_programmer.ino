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



void setAddress(int address, boolean outputEnable)
{
	shiftOut(REG_DATA, REG_CLK, MSBFIRST, (address >> 8) | (outputEnable ? OE_ACTIVE : OE_INACTIVE)); //top 8 bits
	shiftOut(REG_DATA, REG_CLK, MSBFIRST, (address)); //bottom 8 bits
	
	digitalWrite(REG_STROBE, HIGH); //flicks the REG_STROBE so that the data is latched in the internal register
	delayMicroseconds(1);
	digitalWrite(REG_STROBE, LOW); //reset REG_STROBE so that new data will not be latched. 
}

int readData(int address) {
  setAddress(address, true);
  setDataPinsInput();
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

void writeData(int address, int data){ //can't use loops since the pins are not in order
 setAddress(address, false); // this must be done first so that the OE is set LOW and there isn't bus contention
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
 delay(1); //write cycle time 
}

void readDataBlock(){
   for(int b=0; b<=256; b+=16) {
    Serial.print(b, HEX);
    Serial.print(": ");
    for(int i=0; i<16; i++){
      Serial.print(readData(b+i), HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void setup() {
  // put your setup code here, to run once:
	pinMode(REG_STROBE, OUTPUT);
	pinMode(REG_DATA, OUTPUT);
	pinMode(REG_CLK, OUTPUT);
	pinMode(ROM_WE, OUTPUT);
	Serial.begin(9600);
  
	digitalWrite(ROM_WE, WE_INACTIVE); //set ROM to read mode
	digitalWrite(REG_STROBE, LOW); //close the shift register output/internal register off
  writeData(0x00, 0x55);  
}

void loop() {
  // put your main code here, to run repeatedly:
  while(!Serial.available()> 0){
    if(Serial.read() == 'r'){
      readDataBlock();
    }
  }
}
