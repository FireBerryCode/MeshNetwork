#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#define button 2
#define led 3
RF24 radio(10, 9);               // nRF24L01 (CE,CSN)
RF24Network network(radio);      // Include the radio in the network
const uint16_t this_node = 02;   // Address of this node in Octal format ( 04,031, etc)
const uint16_t other_node = 00;       

struct payload_t {    
  char id[3];   
  float temperature;           
};

void setup() {
  SPI.begin();
  radio.begin();
  radio.setRetries(15, 15);
  radio.setPALevel(RF24_PA_MIN);
  network.begin(120, this_node); 
}
void loop() {
  network.update();
  payload_t payload;
  bool messageReceived = 0;
  
  while ( network.available() ) {     
    RF24NetworkHeader header;
    network.read(header, &payload, sizeof(payload));
    messageReceived = 1;
    
    Serial.println("Data received");
    Serial.println(payload.id); 
    Serial.print(payload.temperature);
  }

  if (messageReceived) {
    RF24NetworkHeader header2(other_node); 
    bool ok = network.write(header2, &payload, sizeof(payload)); 
  }
}
