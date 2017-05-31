/*
Code used to program 28C16 EEPROM chips using an Arduino Nano
Based on code from Ben Eater: https://www.youtube.com/watch?v=K88pgWhEb1M&t=26s
Timings are based around the Catalyst CAT28C16A
Datasheet available from: http://pdf1.alldatasheet.com/datasheet-pdf/view/57386/CATALYST/CAT28C16AP-20.html
*/

#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

void setAddress(int address, bool outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);

}

// Reads 1 byte of data from the supplied address and returns a byte value
byte readEEPROM(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin +=1) {
    pinMode(pin, INPUT);
  }

  setAddress(address, /*outputEnable*/ true);
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -=1) {
    data = (data << 1) + digitalRead(pin);
  }
  return data;
}

void writeEEPROM(int address, byte data) {

  // Set the Address and Output Enable Pins through the Shift Registers
  setAddress(address, /*outputEnable*/ false);
  delayMicroseconds(1); // Output Enable and Address needs 15 ns setup time;
  // Put data on the I/O Pins ready to be latched
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin +=1) {
    pinMode(pin, OUTPUT);
  }
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    digitalWrite(pin, data & 1); // bitwise AND operator
    data = data >> 1; // bitwise SHIFT operaator
  }
  // Bring Write Enable Low to send a write pulse for at least 150ns
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  // Bring Write Enable High to latch data - requires 50ns
  digitalWrite(WRITE_EN, HIGH);
  // data hold time ... Write Cycle total is up to 10 ms
  delay(10);
}

void printContents() {
  // 239 + 16 = 255 ... so this reads the first 256 bytes of data, 16 bytes at a time
  for (int base = 0; base <=239; base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset += 1) {
      data[offset] = readEEPROM(base + offset);
    }

    char buf[54];
    sprintf(buf, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
      base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], 
      data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    Serial.println(buf); 
  }  
}

byte data[] = {0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B, 0x77, 0x1F, 0x4E, 0x3D, 0x4F, 0x47};

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);
  //  Chip is write proteccted for between 5ms and 20ms at power on
  delay(20);
  Serial.begin(57600);

//  // Erase entire EEPROM
//  for (int address = 0; address <= 2047; address +=1) {
//    writeEEPROM(address, 0xFF);
//  }
//

  // Program 16 bytes
  for (int address = 0; address <= 15; address +=1) {
    writeEEPROM(address, data[address]);
  }
  
  printContents();
}

void loop() {

}
