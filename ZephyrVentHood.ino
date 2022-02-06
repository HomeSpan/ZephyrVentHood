
#include "HomeSpan.h"

#include "DEV_Identify.h"
#include "DEV_Zephyr.h"

#define SKETCH_VERSION  "2.0"

void setup() {

  Serial.begin(115200);

  homeSpan.setLogLevel(1);
  homeSpan.enableOTA();
  homeSpan.setSketchVersion(SKETCH_VERSION);
  
  homeSpan.begin(Category::Fans,"Zephyr Vent Hood");

  new SpanAccessory();
    new DEV_Identify("Vent Hood","HomeSpan","ZVH-1","RF-Control",SKETCH_VERSION,0);
    new Service::HAPProtocolInformation();
      new Characteristic::Version("1.1.0");
    (new DEV_ZephyrFan(0x51388,0x61398,18))->setPrimary();
    new DEV_ZephyrLight(0x51390,0x61398,19);

}

//////////////////////////////////////

void loop(){
  homeSpan.poll();
}
