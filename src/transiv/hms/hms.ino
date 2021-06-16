//transiv
///библилтеки для сенсоров
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>

#define DHTPIN_out 7
#define DHTPIN_in 6
#define NORMAL_LEVEL_PRESSURE_HPA 734  //норма давления
///bmp на I2C
/*
  I2C
  SCL  A5
  SDA  A4
*/ 
     
#define MQ5PIN A1        //датчик газа
#define MQ135PIN A2
#define MQ7PIN A3
#define CH4  180       //предел газов

#define POTOPPIN A6      //датчик дождя
#define LEVELRAIN 550     //порог дождя

#define SOUNDPIN 3

#include <SPI.h>
/*SPI
  SS 10
  MOSI 11
  MISO 12
  SCLK 13
*/
#include <LiquidCrystal.h>     // подключаем стандартную библиотеку LiquidCrystal

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

DHT dht_out(DHTPIN_out, DHT22); 
DHT dht_in(DHTPIN_in, DHT22);
Adafruit_BMP280 bmp;

// инициализируем объект-экран, передаём использованные
// для подключения контакты на Arduino в порядке:
// RS, E, DB4, DB5, DB6, DB7
//  LiquidCrystal lcd(A1, 8, 5, 4, 3, 2);

void setup()
{

  Serial.begin(9600); 
  
  dht_in.begin();
  dht_out.begin();
  bmp.begin(0x76); //адресс датчика давления
  
  // устанавливаем размер (количество столбцов и строк) экрана
  // lcd.begin(16, 2);
  pinMode(VENTPIN, OUTPUT);
  pinMode(POMPAPIN, OUTPUT);
  pinMode(POTOPPIN, INPUT);
  pinMode(MQ5PIN, INPUT);
  pinMode(MQ135PIN, INPUT);
  pinMode(MQ7PIN, INPUT);
  pinMode(SOUNDPIN, OUTPUT);
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  radio.begin();
  //radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);       //(время между попыткой достучаться, число попыток)
  //radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  //radio.setPayloadSize(32);     //размер пакета, в байтах
  radio.openWritingPipe(pipe[0]); //мы - труба 0, открываем канал для передачи данных
  //radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)
  //radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  //radio.setDataRate (RF24_2MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS должна быть одинакова на приёмнике и передатчике!
  //radio.powerUp(); //начать работу
  radio.stopListening();  //не слушаем радиоэфир, мы передатчик
  //radio.openReadingPipe(1,pipe);
  //radio.startListening();
  //radio.printDetails();
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
}


float h_out, h_in, t_out, t_in, absH_in, absH_out;
float tBMP, pBMP;                            //данные с датчиков BMP
float rain;                                  //днные с датчика дождя
boolean ventOn = 0;
float data[8];                              //массив для передачи данных
int MQ5, MQ135, MQ7;

void loop()
{ 
  // Задержка 1 секунду между измерениями
  delay(1000); 
  MQ5 = analogRead(MQ5PIN);
  //MQ135 = analogRead(MQ135PIN);
  //MQ7 = analogRead(MQ7PIN);
  //Считываем влажность
  h_out = dht_out.readHumidity();
  h_in = dht_in.readHumidity(); 
  // Считываем температуру
  t_out = dht_out.readTemperature(); 
  t_in = dht_in.readTemperature();
  //получаем влажность
  absH_in = calcAbsH(t_in, h_in);
  absH_out = calcAbsH(t_out, h_out);
  
  //читаем данные с датчика BMP
  tBMP = bmp.readTemperature();                   
  pBMP = bmp.readPressure() / 133.322F + 2;


  //////////////////////////////////////////////////////////////////////
  /////////управление помпой
  rain = analogRead(POTOPPIN);
  //boolean POTOP = digitalRead(POTOPPIN);
  //boolean POTOP = (rain>550)? 1 : 0; 
  //////////////////////////////////////////////////////////////////////
  //Serial.print("Влажность: " + h + " %\t" + "Температура: " + t + " *C ");
  print_serial();
  
  //digitalWrite(VENTPIN, ventOn);
  
  /////////////////////////////////////////////////////////////
  //передача данных
  if (millis() / 1000 % 2 == 0) { //раз в 2 секунды
    //предаем температуру с датчиков dht
    data[0] = t_out;    
    data[1] = t_in;
    //передаем влажность с датчиков dht 
	  data[2] = h_in;
    data[3] = h_out;
	/*
    data[2] = absH_in;
    data[3] = absH_out;*/
    //предаем данные датчика BMP
    data[4] = tBMP;
    data[5] = pBMP;
    //данные датчика дождя
    data[6] = rain;
    //газы
    data[7] = MQ5;
    
    //тревожная сигнализация уровень газов 
    if (CH4 < MQ5) {      
      tone(SOUNDPIN,2000);
    }
    else {
      noTone(SOUNDPIN);
    }
    

    bool ok = radio.write( &data, sizeof (data) );
    if (ok)
      printf("ok\n\r");
    else
      printf("failed\n\r");
  }
  /////////////////////////////////////////////////////////////



}
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
  
  
  
void print_serial() {
/*  Serial.print("Т_in:");
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
  //*/
  
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
/*
  Serial.print("MQ135= "); 
  Serial.print(MQ135);
  Serial.println(); 

  Serial.print("MQ7= ");
  Serial.print(MQ7);
  Serial.println();
  Serial.println();
*/
}


float  calcAbsH(float t, float h) { // t - Температура, h - Относительная влажность %
  float tmp;
  float absHumid;
  tmp = pow(2.718281828, (17.67 * t) / (t + 243.5));
  absHumid = (6.112 * tmp * h * 2.1674) / (273.15 + t);
  //console.log('Abs Humidity is ', absHumid.toFixed(2), "g/m3");
  return absHumid; // Это граммы воды в дном кубометре воздуха.
}
