//библиотеки
#include <Wire.h> //
#include <LiquidCrystal_I2C.h> // дисплей
#include "cactus_io_BME280_I2C.h"//датчик температуры и т.д.
#include "RTClib.h" // модуль времени
#include "GyverButton.h" //кнопка
#include "GyverTimer.h"  //таймеры
#include <MQ135.h> // датчик СО2

RTC_DS3231 rtc; //создаем объект для модуля времени
BME280_I2C bme; //создаем объект для датчика температуры
MQ135 gasSensor = MQ135(A1); //создаем объект для датчика СО2

GButton btn1 (4, LOW_PULL, NORM_OPEN); //пин кнопки
//-------------------задержки отрисовки(для обновления страниц)----------
GTimer_ms page1(5000); 
GTimer_ms page2(5000);
GTimer_ms page3(5000);


LiquidCrystal_I2C lcd(0x27,20,4); //создаем объект для дисплея

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"}; //названия дней недели
int aFlag=0; // флаг для alarm 
int plot_array[15]; // массиви для графика

//------------------значки для 1 страницы---------------
byte prs[] = {0x4,0x2,0x2,0x4,0x15,0xe,0x4,0x1f}; //давление
byte tem[] = {0x4,0xa,0xa,0xa,0x11,0x1f,0xe,0x0}; //температура
byte hum[] = {0x3,0x6,0xe,0x1f,0x17,0x13,0xe,0x0}; //влажность
byte timer[] = {  B00000, B00000, B00110, B01010, B10001, B11111, B00000, B00000}; //СО2

//--------------------различные счетчики--------------
long counter = 0;  //номер страницы
long counter_set = 0; // отрисовка страницы
bool dop = 1; //костыль(помогает отрисовать страницу, если счетчик для страницы не готов)
int counterPlot = 0; //для графика

//------------------------насторйки кнопки-------------------------
#define hold 1000           // время (мс), после которого кнопка считается зажатой
#define debounce 80        // (мс), антидребезг
unsigned long button1_timer; // таймер последнего нажатия кнопки

//---------------------------------НАСТРОЙКИ--------------------

void setup()
{
    
    Serial.begin(9600); // вывод в порт(для поиска ошибок)

    //если проблемы с датчиком температуры
    if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
    }
      
    bme.setTempCal(-5);// калибровка датчика

    //если проблемы с модулем времени
    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      while (1);
    }
    
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //ОТЛАДКА ВРЕМЕНИ(ЕСЛИ СБИЛОСЬ ВРЕМЯ)

    //если модуль времени потерял питание
    if (rtc.lostPower()) {
      Serial.println("RTC lost power, lets set the time!");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    } 
    
    lcd.init(); //создание дисплея                      
    lcd.backlight(); //включение подсветки 
}
//---------------------------------ГЛАВНАЯ--------------------
void loop()
{ 
  btn1.tick(); //ловим нажатия на кнопку 
  
    switch (counter){
      case 0:
      firstPage();  //если счетчик равен 0, то идет отрисовка 1 страницы    
      dop = 0; //костыль(для начальной отрисовки)
      break;
      case 1:
      secondPage();//если счетчик равен 1, то идет отрисовка 2 страницы   
      dop = 0;
      break;
      case 2:
      thirdPage();//если счетчик равен 2, то идет отрисовка 3 страницы   
      dop = 0;
      break;
    }

  if (counter > 2) //обнуление счетчика
    counter = 0;
  
  if (btn1.isClick()){ //добавляет счетчику 1, если кнопак была нажата
  counter++;
  lcd.clear(); // очищение дисплея при переходе на другую страницу
  dop = 1;
  }
  
  if(btn1.isHolded()){ //возврат на 1 страницу при удержании кнопки
  counter = 0;
  lcd.clear();
  dop = 1;
  }  
  
}

//----------------------символы для чисиков(нужны для отрисовки времени)--------------
uint8_t LT[8] = {B00111, B01111, B11111, B11111, B11111, B11111, B11111, B11111};
uint8_t UB[8] = {B11111, B11111, B11111, B00000, B00000, B00000, B00000, B00000};
uint8_t RT[8] = {B11100, B11110, B11111, B11111, B11111, B11111, B11111, B11111};
uint8_t LL[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B01111, B00111};
uint8_t LB[8] = {B00000, B00000, B00000, B00000, B00000, B11111, B11111, B11111};
uint8_t LR[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11110, B11100};
uint8_t MB[8] = {B11111, B11111, B11111, B00000, B00000, B00000, B11111, B11111};
uint8_t block[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111};

// ------------------------------------ЧАСИКИ-------------------------

//функции для отрисовки различных цифр для часов(только внешний вид)
void custom0(int x)
{ // цифра 0

  lcd.setCursor(x,0); // установка курсора на х-овый столбец, первой строки
  lcd.write(0);  // отрисовка первого элемента(начинается с 0) 
  lcd.write(1);  // отрисовка второго элемента
  lcd.write(2);
  lcd.setCursor(x, 1); // установка курсора на х-овый столбец, второй строки
  lcd.write(3);  // отрисовка четвертого элемента
  lcd.write(4);  // отрисовка пятого элемента
  lcd.write(5);
}
void custom1(int x)
{
  // цифра 1
  lcd.setCursor(x,0);
  lcd.write(1);
  lcd.write(2);
  lcd.print(" ");
  lcd.setCursor(x,1);
  lcd.write(4);
  lcd.write(7);
  lcd.write(4);
}
// цифра 2
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
// цифра 3
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
// цифра 4
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
// цифра 5
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
// цифра 6
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
// цифра 7
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
// цифра 8
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
// цифра 9
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
//точки
void dot(int x){ 
  lcd.setCursor(x, 0);
  lcd.write(4);
  lcd.setCursor(x, 1);
  lcd.write(1);
}
//создаем символы для часиков
void print1(int digits, int x){
  lcd.createChar(0,LT);
  lcd.createChar(1,UB);
  lcd.createChar(2,RT);
  lcd.createChar(3,LL);
  lcd.createChar(4,LB);
  lcd.createChar(5,LR);
  lcd.createChar(6,MB);
  lcd.createChar(7,block);
//-------------------------ловим значения от модуля времени------------------
  switch (digits) {
  case 0:  //если на часах есть 0, то идет отрисовка красивого 0
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
  
  if(millis()-counterPlot>=24000){ //задержка для графиков
    redrawPlot(12); //перерисовка графиков(криво работает)
    
    counterPlot=millis();
  }
  //отрисовка для первой страницы 
  if (millis()-counter_set>=5000 || dop == 1){; //millis() - функция, которая считает время от запуска программы. 
                                                //Если мы будем сравнивать переменную, которая периодически будет приравниваться к millis, 
                                                //с этой функцией, то можно получить определенную задержку исполнения команд
                                                //в данном случае задержка 5000 милисекунд.
                                                //dop - костыль(т.к. изначально условие (millis()-counter_set>=5000) будет ложно,
                                                //т.к. от запуска программы просто не пройдет столько времени), то для отрисовки пришлось
                                                //ввести этот костыль, благодаря которому страница всегда будет отрисовываться(даже если условие ложно)
          
          DateTime now = rtc.now(); //достаем значения от модуля времени
          // отрисовка значков
          lcd.createChar(0, hum);
          lcd.createChar(1, tem);
          lcd.createChar(2, prs);
          lcd.createChar(3, timer);
          
          bme.readSensor(); //достаем значения от датчика
          float prs = (bme.getPressure_MB()*100)/133.3; // переводим давление на человеческий язык

          // отрисовка температуры
          lcd.setCursor(0,0);
          lcd.write(1); 
          lcd.print(" Tem: ");
          lcd.print(bme.getTemperature_C());//принимаем значение от датчика температуры
          lcd.print(" C");  
                
          // отрисовка влажности
          lcd.setCursor(0,1); 
          lcd.write(0);             
          lcd.print(" Hum: ");
          lcd.print(bme.getHumidity());//принимаем значение от датчика влажности
          lcd.print(" %");
          
          // отрисовка давления
          lcd.setCursor(0,2);
          lcd.write(2); 
          lcd.print(" Prs: ");
          lcd.print(prs); 
          lcd.print(" mmHg"); 
          
          // отрисовка СО2
          lcd.setCursor(0,3);
          lcd.write(3);
          lcd.print(" CO2: ");
          lcd.print(gasSensor.getPPM());//принимаем значение от датчика СО2
          lcd.print(" ppm");

          //подаем сигнал, если значения СО2>750 
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
    redrawPlot(12);
    counterPlot=millis();
  }
  
  if (millis()-counter_set>=5000 || dop == 1){
    
    int gasV = gasSensor.getPPM(); //достаем данные с датчика СО2
    
    digitalClockDisplay();//отрисовка часиков
    printDay();//отрисовка даты
    bmePrint();//отрисовка данных с датиков

    //подаем сигнал, если значения СО2>750 
    if(gasV>750 && aFlag==0){
      alert(0, 3);
      aFlag = 1;
    }
    //убираем сигнал, если значения СО2<750 
    if(gasV<=750 && aFlag==1){
      disalert(0, 3);
      aFlag = 0;
    }
          
    counter_set = millis();
  }
}

// ---------------------------------ПРЕДУПРЕЖДЕНИЕ О СО2-------------------------
void alert(int x, int y) { 
  lcd.setCursor(x, y);//устанавливает курсор на стобце х, строке у
  lcd.write(7);//выводит прямоугольник
}

void disalert(int x, int y){
  lcd.setCursor(x, y);//устанавливает курсор на стобце х, строке у
  lcd.print(" ");//очищает место
}

//----------------------ОТРИСОВКА ДАТЧИКОВ-----------------
//функция для отрисовки данных с датчика для 2 страницы
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
//функция для отрисовки часиков для 2 страницы
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
//функция для отрисовки даты для 2 страницы
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

// ------------------------------------ТРЕТЬЯ СТРАНИЦА(ГРАФИКИ)------------------------
//--------------------------------__этот код еще сырой, пропусти его__------------------

void thirdPage(){ //графики. Я тут сам хз
  if (millis()-counter_set>=24000 || dop == 1){
          initPlot(); 
          
          int temp = bme.getTemperature_C();
          int valueT = map(temp, 0, 60, 10, 80);
          drawPlot(1, 3, 12, 4, 10, 80, valueT); // вызываем функцию для построения графиков

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

// функция создания символов для графиков
void initPlot() { 
  // необходимые символы для работы графиков
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


//тут вообще без комментариев(просто Ctrl+c и Ctrl+v)
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
//функция для пересчета данных для графика(работает криво)
void redrawPlot(int widthPlot){
    for (byte i = 0; i < widthPlot; i++) {
      plot_array[i] = plot_array[i + 1];
    }
    
    int temp = bme.getTemperature_C();
    int valueT = map(temp, 0, 60, 10, 80);
    
   plot_array[widthPlot] = valueT;
}
