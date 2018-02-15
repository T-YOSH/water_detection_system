/*******************************************************************************
// SWITCHSCIENCE wiki -- http://trac.switch-science.com/
// PCAL9555APW Arduino Sample
*******************************************************************************/
#include <Wire.h>
#include <ST7032.h>

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

// BUZZER
#define BUZZER_PIN 23       // ブザーを鳴らすためのピン
#define MAGNET_PIN 12       // 磁石スイッチのHIGHを出力するピン
#define JUDGE_PIN 34         // 磁石スイッチの判断をするピン

#define BEAT 150            // 一つの音を鳴らす時間
#define LEDC_CHANNEL 0      // チャンネル
#define LEDC_TIMER_BIT 13
#define LEDC_BASE_FREQ 5000

//OUTPUT GPIO
#define OUTPUT_GPIO_NUMBER 8


//LCD
ST7032 lcd;
bool isLcdOn = false;

void datasend(int,int,int *,int);
void dataread(int,int,int *,int);

void setup()
{
  Serial.begin(115200); 
  Wire.begin(21,22);
  Wire.setClock(10000);

  // BUZZER
  ledcSetup(LEDC_CHANNEL, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcAttachPin(BUZZER_PIN, LEDC_CHANNEL);

  //LCD
  lcd.begin(8, 2);
  lcd.setContrast(30);            // コントラスト設定
  lcd.clear();

  // setup for OUTPUT GPIO
  Wire.beginTransmission(PCAL9555APW_ADDR);
  Wire.write(CONFIG_REG);
  Wire.write(0xFF);
  Wire.write(0xFE);
  Wire.endTransmission();
}

void loop()
{
  boolean isWaterDetect = false;
  String strDetectedSensors = " No"; 

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
    if(value==0){
      isWaterDetect=true;
      strDetectedSensors = strDetectedSensors + String(".") + String(i);
    }
  }
  Serial.print("\tport1:");
  Serial.println(port_input[1], BIN);
  for(int i =0; i < (NUM_OF_PORT/2) ; i++){
    int value = (port_input[1]>>i) & 1;
    Serial.print("\t\t\GPIO ");
    Serial.print(i+8);
    Serial.print(" value : ");
    Serial.println(value);
    if(value==0){
      if( (i+8) != OUTPUT_GPIO_NUMBER){
        isWaterDetect=true;
        strDetectedSensors = strDetectedSensors + String(".") + String(i+8);
      }
    }
  }

  if((strDetectedSensors.length()-1) < 9){
    for (int thisChar = 0; thisChar < (9 -(strDetectedSensors.length()-1)); thisChar++) {
      strDetectedSensors = strDetectedSensors + String(" ");
    }
  }

  // show message to the LCD
  if(isWaterDetect){
    Serial.println("WARNING");
    Serial.println(strDetectedSensors);

    ///////////
    //LCD
    ///////////
    lcd.setCursor(0, 0);
    lcd.print("WARNING!!");
  
    // print from 0 to 10:
    lcd.setCursor(0, 1);
    lcd.print(strDetectedSensors);

    if(strDetectedSensors.length()>8){
        Serial.println("LONG LENGTH");
        Serial.println(strDetectedSensors.length());

        lcd.autoscroll();
        for (int thisChar = 0; thisChar < (strDetectedSensors.length()-8); thisChar++) {
         lcd.setCursor(strDetectedSensors.length(), 1);
         lcd.print(".");
         lcd.noAutoscroll();
         lcd.setCursor(thisChar+1, 0);
         lcd.print("WARNING!!");
         lcd.autoscroll();
         delay(500);
        }
        lcd.noAutoscroll();
        lcd.noDisplay();
        lcd.setCursor(0, 1);
        lcd.print("");

        for (int thisChar = 0; thisChar < (strDetectedSensors.length()-8); thisChar++) {
           lcd.scrollDisplayRight();
        }
        lcd.setCursor(0, 0);
        lcd.print("WARNING!!");
      
        // Turn on the display:
        lcd.display();

    }else{
        Serial.println("LONG LENGTH");
        Serial.println(strDetectedSensors.length());
    }
    
    ///////////
    // buzzer start
    ///////////
    callBuzerSound();

    //別ラインのブザースイッチをオンにする。
    static int buzzerCountDown =3;
    buzzerCountDown--;
    if( (buzzerCountDown%3)==0){
      Serial.println("set gpio ON");
      int port_output[2] = {0x00, 0x00}; //入力状態を取得する変数
      datasend(PCAL9555APW_ADDR, OUTPUT_REG, port_output, 2);  
      buzzerCountDown=3;
    }else{
      Serial.println("NOT set gpio ON");
      int port_output[2] = {0x00, 0x01}; //入力状態を取得する変数
      datasend(PCAL9555APW_ADDR, OUTPUT_REG, port_output, 2);  
    }
   
  }else{
    lcd.clear();
    Serial.println("No Problem");
    Serial.println(strDetectedSensors);
    //LCD
      lcd.setCursor(0, 1);
      lcd.print("StatusOK");
      lcd.setCursor(0, 0);
    if(isLcdOn){
      lcd.print("Monitor.");
    }else{
      lcd.print("Monitor");
    }
    isLcdOn = !isLcdOn;

    //別ラインのブザースイッチをオフにする。
    Serial.println("set gpio OFF");
    int port_output[2] = {0x00, 0x01}; //入力状態を取得する変数
    datasend(PCAL9555APW_ADDR, OUTPUT_REG, port_output, 2);  

  }
  delay(1000);
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

 /**
 * ブザー音
 */
void callBuzerSound() {
  ledcWriteTone(LEDC_CHANNEL, 3136); // ソ
  delay(BEAT);
  ledcWriteTone(LEDC_CHANNEL, 2489); // ソ
  delay(BEAT);
  ledcWriteTone(LEDC_CHANNEL, 3136); // ソ
  delay(BEAT);
  delay(BEAT);
  ledcWriteTone(LEDC_CHANNEL, 0);    // 音を止める
}
