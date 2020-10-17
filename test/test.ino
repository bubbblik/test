#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "cactus_io_BME280_I2C.h"
#include "RTClib.h"
#include "GyverButton.h"
#include "GyverTimer.h" 
#include <MQ135.h>

RTC_DS3231 rtc;
BME280_I2C bme;
MQ135 gasSensor = MQ135(A1);

GButton btn1 (4, LOW_PULL, NORM_OPEN);
GTimer_ms page1(5000);
GTimer_ms page2(5000);
GTimer_ms page3(5000);

LiquidCrystal_I2C lcd(0x27,20,4); 

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int aFlag=0; // флаг для alarm 
int plot_array[15];


byte prs[] = {0x4,0x2,0x2,0x4,0x15,0xe,0x4,0x1f};
byte tem[] = {0x4,0xa,0xa,0xa,0x11,0x1f,0xe,0x0};
byte hum[] = {0x3,0x6,0xe,0x1f,0x17,0x13,0xe,0x0};
byte timer[] = {  B00000, B00000, B00110, B01010, B10001, B11111, B00000, B00000};

long counter = 0;  
long counter_set = 0;
bool dop = 1; //костыль
int counterPlot = 0;

#define hold 1000           // время (мс), после которого кнопка считается зажатой
#define debounce 80        // (мс), антидребезг
unsigned long button1_timer; // таймер последнего нажатия кнопки

//---------------------------------НАСТРОЙКИ--------------------

void setup()
{
    
    Serial.begin(9600);

    if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
    }
      
    bme.setTempCal(-5);// Temp was reading high so subtract 5 degree
    
    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      while (1);
    }
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //ОТЛАДКА ВРЕМЕНИ(ЕСЛИ СБИЛОСЬ ВРЕМЯ)
  
    if (rtc.lostPower()) {
      Serial.println("RTC lost power, lets set the time!");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }  
    lcd.init();                      
    lcd.backlight(); 
}
//---------------------------------ГЛАВНАЯ--------------------
void loop()
{ 
  btn1.tick(); 
  
    switch (counter){
      case 0:
      firstPage();      
      dop = 0;
      break;
      case 1:
      secondPage();
      dop = 0;
      break;
      case 2:
      thirdPage();
      dop = 0;
      break;
    }

  if (counter > 2)
    counter = 0;
  
  if (btn1.isClick()){
  counter++;
  lcd.clear();
  dop = 1;
  }
  
  if(btn1.isHolded()){
  counter = 0;
  lcd.clear();
  dop = 1;
  }  
  
}


uint8_t LT[8] = {B00111, B01111, B11111, B11111, B11111, B11111, B11111, B11111};
uint8_t UB[8] = {B11111, B11111, B11111, B00000, B00000, B00000, B00000, B00000};
uint8_t RT[8] = {B11100, B11110, B11111, B11111, B11111, B11111, B11111, B11111};
uint8_t LL[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B01111, B00111};
uint8_t LB[8] = {B00000, B00000, B00000, B00000, B00000, B11111, B11111, B11111};
uint8_t LR[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11110, B11100};
uint8_t MB[8] = {B11111, B11111, B11111, B00000, B00000, B00000, B11111, B11111};
uint8_t block[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111};

// ------------------------------------ЧАСИКИ-------------------------
void custom0(int x)
{ // uses segments to build the number 0

  lcd.setCursor(x,0); // set cursor to column 0, line 0 (first row)
  lcd.write(0);  // call each segment to create
  lcd.write(1);  // top half of the number
  lcd.write(2);
  lcd.setCursor(x, 1); // set cursor to colum 0, line 1 (second row)
  lcd.write(3);  // call each segment to create
  lcd.write(4);  // bottom half of the number
  lcd.write(5);
}

void custom1(int x)
{
  lcd.setCursor(x,0);
  lcd.write(1);
  lcd.write(2);
  lcd.print(" ");
  lcd.setCursor(x,1);
  lcd.write(4);
  lcd.write(7);
  lcd.write(4);
}

void custom2(int x)
{
  lcd.setCursor(x,0);
  lcd.write(6);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.write(3);
  lcd.write(4);
  lcd.write(4);
}

void custom3(int x)
{
  lcd.setCursor(x,0);
  lcd.write(6);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.write(4);
  lcd.write(4);
  lcd.write(5); 
}

void custom4(int x)
{
  lcd.setCursor(x,0);
  lcd.write(3);
  lcd.write(4);
  lcd.write(7);
  lcd.setCursor(x, 1);
  lcd.print(" ");
  lcd.print(" ");
  lcd.write(7);
}

void custom5(int x)
{
  lcd.setCursor(x,0);
  lcd.write(3);
  lcd.write(6);
  lcd.write(6);
  lcd.setCursor(x, 1);
  lcd.write(4);
  lcd.write(4);
  lcd.write(5);
}

void custom6(int x)
{
  lcd.setCursor(x,0);
  lcd.write(0);
  lcd.write(6);
  lcd.write(6);
  lcd.setCursor(x, 1);
  lcd.write(3);
  lcd.write(4);
  lcd.write(5);
}

void custom7(int x)
{
  lcd.setCursor(x,0);
  lcd.write(1);
  lcd.write(1);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.print(" ");
  lcd.print(" ");
  lcd.write(7);
}

void custom8(int x)
{
  lcd.setCursor(x,0);
  lcd.write(0);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.write(3);
  lcd.write(4);
  lcd.write(5);
}

void custom9(int x)
{
  lcd.setCursor(x,0);
  lcd.write(0);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.print(" ");
  lcd.print(" ");
  lcd.write(7);

}

void dot(int x){ 
  lcd.setCursor(x, 0);
  lcd.write(4);
  lcd.setCursor(x, 1);
  lcd.write(1);
}

void print1(int digits, int x){
  lcd.createChar(0,LT);
  lcd.createChar(1,UB);
  lcd.createChar(2,RT);
  lcd.createChar(3,LL);
  lcd.createChar(4,LB);
  lcd.createChar(5,LR);
  lcd.createChar(6,MB);
  lcd.createChar(7,block);
  // utility function for digital clock display: prints preceding colon and leading 0

  switch (digits) {
  case 0:  
    custom0(x);
    break;
  case 1:  
    custom1(x);
    break;
  case 2:  
    custom2(x);
    break;
  case 3:  
    custom3(x);
    break;
  case 4:  
    custom4(x);
    break;
  case 5:  
    custom5(x);
    break;
  case 6:  
    custom6(x);
    break;
  case 7:  
    custom7(x);
    break;
  case 8:  
    custom8(x);
    break;
  case 9:  
    custom9(x);
    break;
  }

}

// ------------------------------------ПЕРВАЯ СТРАНИЦА-------------------------
void firstPage(){
  Serial.print("1");
  if(millis()-counterPlot>=24000){
    //redrawPlot(12);
    
    counterPlot=millis();
  }
  
  if (millis()-counter_set>=5000 || dop == 1){;
          
          DateTime now = rtc.now();
          lcd.createChar(0, hum);
          lcd.createChar(1, tem);
          lcd.createChar(2, prs);
          lcd.createChar(3, timer);
          
          bme.readSensor();
          float prs = (bme.getPressure_MB()*100)/133.3; 
              
          lcd.setCursor(0,0);
          lcd.write(1); 
          lcd.print(" Tem: ");
          lcd.print(bme.getTemperature_C());
          lcd.print(" C");        
            
          lcd.setCursor(0,1); 
          lcd.write(0);             
          lcd.print(" Hum: ");
          lcd.print(bme.getHumidity());
          lcd.print(" %");
            
          lcd.setCursor(0,2);
          lcd.write(2); 
          lcd.print(" Prs: ");
          lcd.print(prs); 
          lcd.print(" mmHg"); 

          lcd.setCursor(0,3);
          lcd.write(3);
          lcd.print(" CO2: ");
          lcd.print(gasSensor.getPPM());
          lcd.print(" ppm");

          if(gasSensor.getPPM()>750 && aFlag==0){
            alert(1, 3);
            aFlag = 1;
          }
      
          if(gasSensor.getPPM()<=750 && aFlag==1){
            disalert(1, 3);
            aFlag = 0;
          }
          
          counter_set = millis();
          
  }
}

// ------------------------------------ВТОРАЯ СТРАНИЦА-------------------------

void secondPage(){ 
  if(millis()-counterPlot>=24000){
    //redrawPlot(12);
    counterPlot=millis();
  }
  
  if (millis()-counter_set>=5000 || dop == 1){
    
    int gasV = gasSensor.getPPM();
    
    digitalClockDisplay();
    printDay();
    bmePrint();

    if(gasV>750 && aFlag==0){
      alert(0, 3);
      aFlag = 1;
    }

    if(gasV<=750 && aFlag==1){
      disalert(0, 3);
      aFlag = 0;
    }
          
    counter_set = millis();
  }
}

// ------------------------------------АЛЕРТЫ-------------------------
void alert(int x, int y) { 
  lcd.setCursor(x, y);
  lcd.write(7);
}

void disalert(int x, int y){
  lcd.setCursor(x, y);
  lcd.print(" ");
}

// ------------------------------------ТРЕТЬЯ СТРАНИЦА------------------------

void thirdPage(){
  if (millis()-counter_set>=24000 || dop == 1){
          initPlot(); 
          
          int temp = bme.getTemperature_C();
          int valueT = map(temp, 0, 60, 10, 80);
          drawPlot(1, 3, 12, 4, 10, 80, valueT); 

          lcd.setCursor(13,0);
          lcd.print("Tem C");
          lcd.setCursor(13,2);
          lcd.print("Tem Now:");
          lcd.setCursor(13,3);
          lcd.print(bme.getTemperature_C());
          lcd.print(" C");
                
          counter_set = millis();
  }
  lcd.setCursor(0,1);
  lcd.print(" ");
}

// ------------------------------------ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ-------------------------


void initPlot() {
  // необходимые символы для работы
  // создано в http://maxpromer.github.io/LCD-Character-Creator/
  byte row8[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
  byte row7[8] = {0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
  byte row6[8] = {0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
  byte row5[8] = {0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
  byte row4[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111};
  byte row3[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};
  byte row2[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};
  byte row1[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111};
  lcd.createChar(0, row8);
  lcd.createChar(1, row1);
  lcd.createChar(2, row2);
  lcd.createChar(3, row3);
  lcd.createChar(4, row4);
  lcd.createChar(5, row5);
  lcd.createChar(6, row6);
  lcd.createChar(7, row7);
}

void drawPlot(byte pos, byte row, byte width, byte height, int min_val, int max_val, int fill_val) {
    
    if(millis()-counterPlot>=24000){
      for (byte i = 0; i < width; i++) {
        plot_array[i] = plot_array[i + 1];
      }
      counterPlot=millis();
    }
    
    fill_val = constrain(fill_val, min_val, max_val);
    plot_array[width] = fill_val;
    for (byte i = 0; i < width; i++) {                  // каждый столбец параметров
      byte infill, fract;
      // найти количество целых блоков с учётом минимума и максимума для отображения на графике
      infill = floor((float)(plot_array[i] - min_val) / (max_val - min_val) * height * 10);
      fract = (infill % 10) * 8 / 10;                   // найти количество оставшихся полосок
      infill = infill / 10;
      for (byte n = 0; n < height; n++) {     // для всех строк графика
        if (n < infill && infill > 0) {       // пока мы ниже уровня
          lcd.setCursor(pos + i, (row - n));        // заполняем полными ячейками
          lcd.write(0);
        }
        if (n >= infill) {                    // если достигли уровня
          lcd.setCursor(pos + i, (row - n));
          if (fract > 0) lcd.write(fract);          // заполняем дробные ячейки
          else lcd.write(16);                       // если дробные == 0, заливаем пустой
          for (byte k = n + 1; k < height; k++) {   // всё что сверху заливаем пустыми
            lcd.setCursor(pos + i, (row - k));
            lcd.write(16);
          }
          break;
        }
      }
    }
}

void redrawPlot(int widthPlot){
    for (byte i = 0; i < widthPlot; i++) {
      plot_array[i] = plot_array[i + 1];
    }
    
    int temp = bme.getTemperature_C();
    int valueT = map(temp, 0, 60, 10, 80);
    
   plot_array[widthPlot] = valueT;
}

void bmePrint(){
  bme.readSensor();
  
  lcd.setCursor(1, 3);
  lcd.print(int(gasSensor.getPPM()));
  lcd.print("ppm");
  
  lcd.setCursor(10, 3);
  lcd.print(int(bme.getTemperature_C()));
  lcd.print("C");
  
  lcd.setCursor(15, 3);
  lcd.print(int(bme.getHumidity()));
  lcd.print("%");
}

void digitalClockDisplay(){
  // digital clock display of the time
  DateTime now = rtc.now();

  int hour01 = now.hour()/10;
  int hour11 = now.hour()%10;  

  print1(hour01,1); 
  print1(hour11,5); 

  dot(9);

  print1(now.minute()/10,11);
  print1(now.minute()%10,15);
}

void printDay(){

  DateTime now = rtc.now(); 
  
  lcd.setCursor(1, 2);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.setCursor(10, 2);
  lcd.print(now.day(), DEC);
  lcd.print('/');
  lcd.print(now.month(), DEC);
  lcd.print('/');
  lcd.print(now.year()-2000, DEC);
}
