//receiver
//#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN_out 6
#define DHTPIN_in 7
#define NORMAL_LEVEL_PRESSURE_HPA 734             //норма давления
#define MAX_PRESSURE  812
#define MIN_PRESSURE  654

#define CH4  180                                  //предел газов
#define LEVELRAIN 550                             //порог дождя

#define VENTPIN A0

#define LED_RED 8
#define LED_GREEN 7
#define LED_BLUE 6

#include <Wire.h> 
/*I2C
SDA A4
SCL A5
*/
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4); 

#include <SPI.h>
/*SPI
  SS 10
  MOSI 11
  MISO 12
  SCLK 13
*/
// подключаем стандартную библиотеку LiquidCrystal
#include <LiquidCrystal.h>

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
#include <nRF24L01.h>                                  //////
#include <printf.h>                                    //////
#include <RF24.h>                                      //////
#include <RF24_config.h>                               //////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
#define NRF_CE 9
#define NRF_CSN 10

byte pipe[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб
RF24 radio(NRF_CE, NRF_CSN);

//DHT dht_out(DHTPIN_out, DHT22); 
//DHT dht_in(DHTPIN_in, DHT22);

/*
// инициализируем объект-экран, передаём использованные
// для подключения контакты на Arduino в порядке:
// RS, E, DB4, DB5, DB6, DB7
LiquidCrystal lcd(A1, 8, 5, 4, 3, 2);
*/
void setup()
{
  Serial.begin(9600); 
  
  // устанавливаем размер (количество столбцов и строк) экрана
  //lcd.begin(16, 2);

  lcd.init();
  lcd.backlight(); 
  
  pinMode(VENTPIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  radio.begin(); //активировать модуль
  //radio.setAutoAck(1);           //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);         //(время между попыткой достучаться, число попыток)
  //radio.enableAckPayload();      //разрешить отсылку данных в ответ на входящий сигнал
  //radio.setPayloadSize(32);       //размер пакета, в байтах
  radio.openReadingPipe(1, pipe[0]); //хотим слушать трубу 0
  //radio.setChannel(0x60);           //выбираем канал (в котором нет шумов!)
  //radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  //radio.setDataRate (RF24_2MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS  должна быть одинакова на приёмнике и передатчике!
  //radio.powerUp(); //начать работу
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
  //radio.openWritingPipe(pipe);
  //radio.stopListening();
  //radio.printDetails();
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
}


float h_out, h_in, t_out, t_in, absH_in, absH_out;
float tBMP, pBMP;                            //данные с датчиков BMP
float rain;
int MQ5;

boolean ventOn = 0;
boolean isRain = 0;
boolean isGas = 0;

float data[8];                                 //массив для получения данных
int screen;                                    //номер экрана


void loop()
{ 
  // Задержка 2 секунды между измерениями
  delay(1000); 

  //Serial.print("Влажность: " + h + " %\t" + "Температура: " + t + " *C ");

  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  //прием данных
  if ( radio.available() ) 
  {
    // while (radio.available())
    {
      radio.read ( &data, sizeof (data) );
    }
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  //получаем температуру с датчиков dht
  t_out = data[0];
  t_in  = data[1];
  //получаем влажность с датчиков dht 
  h_in = data[2];
  h_out = data[3] ;
  //получаем данные датчика BMP
  tBMP = data[4];
  pBMP = data[5];
  //датчик дождя, газа
  rain = data[6];
  MQ5 = data[7];
  //выводим информацию
  print();
    
  //идет дождь
  isRain = (rain > LEVELRAIN)? 1 : 0;
  //высокий уровень газов
  isGas = (MQ5 > CH4)? 1 : 0;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
///Метод управляющий RGB диодом
void lightDiode()
{
    digitalWrite(LED_RED,!isGas);
    digitalWrite(LED_GREEN,1);
    digitalWrite(LED_BLUE,!isRain);  
}
///Метод отправлящий отладочную информацию в монитор порта
void print_serial() 
{
 /* Serial.print("Т_in:");
  Serial.print(t_in);
  Serial.print("*C Влажность: ");
  Serial.print(h_in);
  Serial.println();
  Serial.print("Т_out:");
  Serial.print(t_out);
  Serial.print("*C Влажность: ");
  Serial.print(h_out);
  Serial.println();
  Serial.println();
  */
  Serial.print("Т_bmp:");
  Serial.print(tBMP);
  Serial.print(" Давление мм: ");
  Serial.print(pBMP);
  Serial.println();
    
  Serial.print("MQ5= ");
  Serial.print(MQ5); 
  Serial.println();
  Serial.println();
  
  Serial.print("rain= ");
  Serial.print(rain);
  Serial.println();
  Serial.println();
}

///выводим инормацию на дисплей 
void print_lcd(String str1, String str2)
{	 
	//очистка экрана
	lcd.clear();
	//первая строка
	lcd.print(str1);
	// устанавливаем курсор в колонку 0, строку 1
	// на самом деле это вторая строка, т.к. нумерация начинается с нуля
	lcd.setCursor(0, 1);
	//вторая строка
	lcd.print(str2);
} 

///метод выводящий набор информации 
void print_lcd(int screen)
{
	String str1, str2;
	switch (screen)
	{
		case 0:	
			//screen0
			// собираем первую строку
			str1 = "o t:" + String(t_out) + " h:" + String(h_out);
			// собираем вторую строку
			str2 = "i t:" + String(t_in) + " h:" + String(h_in);
			break;
		case 1:
			//screen1
			str1 = "p:"+String(pBMP);
			str2 = "rain: "+String(precipitation(pBMP, h_out))+"%";
			break;
	}
	print_lcd(str1, str2);
}

///метод для вывода информации
void print()
{
	//меняем экран раз в несколько секунд
	if (screen != millis() / 1000 /4 % 2) 
	{
		screen = millis() / 1000 /4 % 2;
		print_lcd(screen);
    lightDiode();
	}
		
	//debag
	print_serial();
}

//вычисления вероятности осадков
int precipitation(float pBMP, float h)
{
	int result;
	int value = pBMP;
	//dp = NORMAL_LEVEL_PRESSURE_HPA-pBMP;
	if (h>90)
		result = map(value, MAX_PRESSURE, MIN_PRESSURE, 0, 100);
	else
		result = map(value+10, MAX_PRESSURE, MIN_PRESSURE, 0, 100);
	return result;
}

//вычисление количества воды в воздухе
float  calcAbsH(float t, float h) { // t - Температура, h - Относительная влажность %
  float tmp;
  float absHumid;
  tmp = pow(2.718281828, (17.67 * t) / (t + 243.5));
  absHumid = (6.112 * tmp * h * 2.1674) / (273.15 + t);
  //console.log('Abs Humidity is ', absHumid.toFixed(2), "g/m3");
  return absHumid; // Это граммы воды в дном кубометре воздуха.
}
