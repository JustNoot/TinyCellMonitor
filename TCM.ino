#include <EEPROM.h>
#include <FastCRC.h>

#define VOLT_PIN A1
#define TEMP_PIN A3
#define DMOS_PIN 4

FastCRC8 CRC8;

int VOLT_MAX = 616; // 4150 mV @ 2500 mV ref
int VOLT_OK = 638; // 4010 mV @ 2500 mV ref

byte data[6] = {0};
byte check = 0;
byte addr = 0;
int temp = 0;
int volt = 0;

void setup()
{
  pinMode(VOLT_PIN, INPUT);
  pinMode(TEMP_PIN, INPUT);
  pinMode(DMOS_PIN, OUTPUT);

  uint8_t cal = EEPROM.read(0);
  if (cal < 0x7F)
    OSCCAL = cal;
  addr = EEPROM.read(1);

  Serial.begin();
}

void loop()
{
  volt = analogRead(VOLT_PIN) + analogRead(VOLT_PIN) + analogRead(VOLT_PIN) + analogRead(VOLT_PIN);
  temp = analogRead(TEMP_PIN) + analogRead(TEMP_PIN) + analogRead(TEMP_PIN) + analogRead(TEMP_PIN);

  if (volt / 4 < VOLT_MAX) digitalWrite(DMOS_PIN, HIGH);
  if (volt / 4 > VOLT_OK) digitalWrite(DMOS_PIN, LOW);

  Serial.read(data, sizeof(data));
  check = data[5];
  data[5] = 0;

  if ((addr == data[0]) && (check == CRC8.smbus(data, sizeof(data))))
  {
    if ((data[1] != 0) && (data[2] != 0)) VOLT_MAX = data[1] << 8 | data[2];
    if ((data[3] != 0) && (data[4] != 0)) VOLT_OK = data[3] << 8 | data[4];

    data[1] = highByte(volt);
    data[2] = lowByte(volt);
    data[3] = highByte(temp);
    data[4] = lowByte(temp);

    data[5] = CRC8.smbus(data, sizeof(data));

    Serial.write(data, sizeof(data));
  }
  else if (data[0] != 0)
  {
    Serial.write(data, sizeof(data));
  }

  data[0] = 0;
}
