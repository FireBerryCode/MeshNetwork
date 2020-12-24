#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

RF24 radio(9, 10);                 
RF24Network network(radio);
 
const uint16_t this_node = 03;        
const uint16_t other_node = 00;       
  
 
struct payload_t {    
  char id[3];   
  float temperature;           
};

void setup(void)
{
  Serial.begin(115200); 
  SPI.begin();
  radio.begin();
  radio.setRetries(15, 15);
  radio.setPALevel(RF24_PA_MIN);
  network.begin(120, this_node);
}
 
void loop() {
  
  network.update();
  unsigned long now = millis();  
  Serial.print("Sending...");
  payload_t payload = {"03", 15.57773354365312};
  RF24NetworkHeader header(other_node);
  bool ok = network.write(header,&payload,sizeof(payload));
  Serial.println(ok);
  delay(20000);
}
