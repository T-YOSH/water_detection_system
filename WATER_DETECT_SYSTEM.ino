/*******************************************************************************
// SWITCHSCIENCE wiki -- http://trac.switch-science.com/
// PCAL9555APW Arduino Sample
*******************************************************************************/
#include <Wire.h>

#define PCAL9555APW_ADDR 0x20 // in 7bit for Arduino

#define INPUT_REG     0x00
#define OUTPUT_REG    0x02
#define INVERS_REG    0x04
#define CONFIG_REG    0x06
#define OUTPUT_DRIVE  0x40
#define INPUT_LATCH   0x44
#define PULLUP_EN     0x46
#define PULLUP_SEL    0x48
#define INTRRPT_MASK  0x4A
#define INTRRPT_STAT  0x4C
#define OUTPUT_CONFIG 0x4F

#define NUM_OF_PORT 16

void datasend(int,int,int *,int);
void dataread(int,int,int *,int);

void setup()
{
  Serial.begin(115200); 
  Wire.begin(21,22);
}

void loop()
{

  int port_input[2]; //入力状態を取得する変数
  dataread(PCAL9555APW_ADDR, INPUT_REG, port_input, 2);
  Serial.print("port0:");
  Serial.println(port_input[0], BIN);
  for(int i =0; i < (NUM_OF_PORT/2) ; i++){
    int value = (port_input[0]>>i) & 1;
    Serial.print("\tGPIO ");
    Serial.print(i);
    Serial.print(" value : ");
    Serial.println(value);
  }
  Serial.print("\tport1:");
  Serial.println(port_input[1], BIN);
  for(int i =0; i < (NUM_OF_PORT/2) ; i++){
    int value = (port_input[1]>>i) & 1;
    Serial.print("\t\t\GPIO ");
    Serial.print(i+8);
    Serial.print(" value : ");
    Serial.println(value);
  }

  
  delay(500);
}

void datasend(int id,int reg,int *data,int datasize)
{
  Wire.beginTransmission(id);
  Wire.write(reg);
  for(int i=0;i<datasize;i++)
  {
    Wire.write(data[i]);
  }
  Wire.endTransmission();
}

void dataread(int id,int reg,int *data,int datasize)
{
  Wire.beginTransmission(id);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(id, datasize, false);
  for(int i=0;i<datasize;i++)
  {
    data[i] = Wire.read();
  }
  Wire.endTransmission(true);
}
