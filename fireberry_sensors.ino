#include <SimpleDHT.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

 
// DEFINICIONES
#define prt(S) Serial.print(S)
#define prtl(S) Serial.println(S)
#define pinDHT22 2 // Pin digital para el DHT22
#define pinIR A3 
#define pinMQ2 A1
#define pinMQ7 A0
#define pinTSL2591 A4
#define freqChannel 120
#define antennaIntensity RF24_PA_MIN
#define numRetries 15
#define msRetries 15



// CLASES PARA LOS OBJETOS
SimpleDHT22 dht22(pinDHT22);
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
RF24 radio(9, 10);  
RF24Network network(radio);      



// IDENTIFICADORES EN LA TOPOLOGIA
const uint16_t this_node = 01;        
const uint16_t other_node = 00;   



// ESTRUCTURAS DE DATOS
struct dht22Struct {
  float temperature;
  float humidity;
};

struct tslStruct {
  uint16_t visible;
  uint16_t ir;
  float lux;
};

struct payloadRF24 {    
  char id[3];   
  float temperature;  
  float humidity;
  int ir;
  int mq2;
  int mq7;
  int tsl;
};
 

void setup(){
  Serial.begin(9600);
  prtl("============================");
  prtl("Iniciando Fireberry Brick ...");
  prtl("============================");

  prtl("============================");
  prtl("Iniciando NF24L01 ...");
  prtl("============================");
  initRF24();

  prtl("============================");
  prtl("Configurando sensores ...");
  prtl("============================");
  configureTSL();

  
}

void loop(){

  prtl("============================");
  prtl("Iniciando medidas de sensores...");
  
  // Medicion de DHT22
  struct dht22Struct dht22Measures = readDHT22();
  prt(dht22Measures.temperature);
  prt(" *C, ");
  prt(dht22Measures.humidity);
  prtl(" RH%");

  
  // Medicion de TSL2591
  struct tslStruct tslMeasures = readTSL2591Sensor();
  prt("Luz visible: ");
  prt(tslMeasures.visible);
  prt(" Luz infrarrojo: ");
  prt(tslMeasures.ir);
  prt(" Lux: ");
  prt(tslMeasures.lux);
  prtl(" ");

  // Medicion de sensor infrarrojo
  int irMeasures = readIRSensor();
  prt("Flame detection: ");
  prtl(irMeasures);

  // Medicion de MQ2
  int mq2Measures = readMQ2Sensor();
  prt("MQ2 reading: ");
  prtl(mq2Measures);

  // Medicion de MQ7
  int mq7Measures = readMQ7Sensor();
  prt("MQ7 reading: ");
  prtl(mq7Measures);

  // Envio de datos al Nodo Raiz
  // prtl("Enviando datos al nodo raiz ....");
  // payloadRF24 payload = {"01", dht22Measures.temperature, dht22Measures.humidity, irMeasures, mq2Measures, mq7Measures, tslMeasures};
  // bool ok = sendRF24(payload);

  // if (ok == true){
  //  prtl("Medida enviada correctamente ...");
  // } else {
  //  prtl("No se ha recibido payload de respuesta ...");
  // }
  
  delay(2000);
  
}

// FUNCIONES AUXILIARES DE CONFIGURACION DE SENSORES

void configureTSL(void)
{

  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println(F(" lux"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println(F(" lux"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution, 4); Serial.println(F(" lux"));  
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  delay(500);
  
  // Definimos la ganancia. Hay tres posibilidades
  // tsl.setGain(TSL2591_GAIN_LOW);    // 1x ganancia
  // tsl.setGain(TSL2591_GAIN_MED);      // 25x ganancia
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x ganancia
  
  tsl.setGain(TSL2591_GAIN_MED);      

  // Definimos el tiempo de exposicion. Hay seis posibilidades.
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  

  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Ganancia:         "));
  tsl2591Gain_t gain = tsl.getGain();
  switch(gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Baja)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Media)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (Alta)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
  Serial.print  (F("Timing:       "));
  Serial.print((tsl.getTiming() + 1) * 100, DEC); 
  Serial.println(F(" ms"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  
}

// FUNCIONES AUXILIARES PARA LECTURA EN CADA SENSOR

struct dht22Struct readDHT22(){
  struct dht22Struct measures;
  int err = SimpleDHTErrSuccess;

  if ((err = dht22.read2(&measures.temperature, &measures.humidity, NULL)) != SimpleDHTErrSuccess){
    prt("Lectura del DHT22 fallida, err=");
    prtl(err);
    delay(2000);
    measures.temperature = -3.4028235E+38;
    measures.humidity = -3.4028235E+38;
    return measures;
  }
  
  measures.temperature = (float)measures.temperature;
  measures.humidity = (float)measures.humidity;
  return measures;
}


struct tslStruct readTSL2591Sensor(){
  struct tslStruct measures;
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full, visible;
  float lux;
  
  ir = lum >> 16;
  full = lum & 0xFFFF;
  visible = full - ir;  
  lux = tsl.calculateLux(full, ir);

  measures.visible = visible;
  measures.ir = ir;
  measures.lux = (float)lux;
  return measures;
}


int readIRSensor(){
   int sensorReading = analogRead(pinIR);
   return sensorReading;
}

int readMQ2Sensor(){
   int sensorReading = analogRead(pinMQ2);
   return sensorReading;
}

int readMQ7Sensor(){
   int sensorReading = analogRead(pinMQ7);
   return sensorReading;
}



// FUNCIONES AUXILIARES DEL NRF24L01

void initRF24(){
  SPI.begin();
  radio.begin();
  radio.setChannel(freqChannel);
  radio.setRetries(numRetries, msRetries);
  radio.setPALevel(antennaIntensity);
  network.begin(freqChannel, this_node);
}


bool sendRF24(struct payloadRF24 payload){
  network.update();
  RF24NetworkHeader header(other_node);
  bool ok = network.write(header,&payload,sizeof(payload));
  return ok;
}
