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
#define JUDGE_PIN 14        // 磁石スイッチの判断をするピン

#define BEAT 150            // 一つの音を鳴らす時間
#define LEDC_CHANNEL 0      // チャンネル
#define LEDC_TIMER_BIT 13
#define LEDC_BASE_FREQ 5000

//LCD
ST7032 lcd;
bool isLcdOn = false;

void datasend(int,int,int *,int);
void dataread(int,int,int *,int);

void setup()
{
  Serial.begin(115200); 
  Wire.begin(21,22);

  // BUZZER
  ledcSetup(LEDC_CHANNEL, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcAttachPin(BUZZER_PIN, LEDC_CHANNEL);

  //LCD
  lcd.begin(8, 2);
  lcd.setContrast(30);            // コントラスト設定
  lcd.clear();

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
      isWaterDetect=true;
      strDetectedSensors = strDetectedSensors + String(".") + String(i+8);
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
        lcd.print("                                ");
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
    callZeldaSound();   // ゼルダの謎解き音を鳴らす

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

/**
 * ゼルダの謎解き音
 */
void callZeldaSound() {
  ledcWriteTone(LEDC_CHANNEL, 3136); // ソ
  delay(BEAT);
  ledcWriteTone(LEDC_CHANNEL, 2960); // ♯ファ
  delay(BEAT);
  ledcWriteTone(LEDC_CHANNEL, 2489); // ♯レ
  delay(BEAT);
  ledcWriteTone(LEDC_CHANNEL, 1760); // ラ
  delay(BEAT);
  ledcWriteTone(LEDC_CHANNEL, 1661); // ♯ソ
  delay(BEAT);
  ledcWriteTone(LEDC_CHANNEL, 2637); // ミ
  delay(BEAT);
  ledcWriteTone(LEDC_CHANNEL, 3322); // ♯ソ
  delay(BEAT);
  ledcWriteTone(LEDC_CHANNEL, 4186); // ド
  delay(BEAT);
  ledcWriteTone(LEDC_CHANNEL, 0);    // 音を止める
}      

