
////////////////////////////////////
//    DEVICE-SPECIFIC SERVICE     //
////////////////////////////////////

#include "extras/RFControl.h"

#define RF433_PIN 22            // pin used for 433MHz transmitter

// Zephyr Vent Hood

// Frequency:   433 MHz
// Encoding:    Fixed pulse duration of 850 usec
// 0-Bit:       230 HIGH / 620 LOW
// 1-Bit:       620 HIGH / 230 LOW
// N-Bits:      20
// N-Cycles:    8
// Cycle Gap:   4000 usec

void transmitZephyr(uint32_t code);
boolean resetLight=false;
boolean resetFan=false;
RFControl RF433(RF433_PIN);

//////////////////////////////////

struct DEV_ZephyrLight : Service::LightBulb {

  uint32_t lightCode;
  uint32_t powerCode;
  SpanCharacteristic *power;
  int lightPin;
  int state=0;                        // 0=off, 1=dim, 2=medium, 3=bright

  DEV_ZephyrLight(uint32_t lightCode, uint32_t powerCode, int lightPin) : Service::LightBulb(){

    power=new Characteristic::On();
    new Characteristic::Name("Vent Hood Light");
    this->lightCode=lightCode;
    this->powerCode=powerCode;
    this->lightPin=lightPin;

    Serial.print("Configuring Zephyr Vent Hood Light 433MHz Transmitter with light code: ");
    Serial.print(lightCode,HEX);
    Serial.print("  power code: ");
    Serial.print(powerCode,HEX);
    Serial.print("\n");

    new SpanButton(lightPin);
  }

  boolean update(){

    if(!power->getVal() && power->getNewVal()){      // only transmit code to turn on light if power is off
      LOG1("Zephyr Vent Hood Light: Power On\n");
      transmitZephyr(lightCode);
      state=3;                                       // light always turns on to brightest setting
    } else 
    
    if(power->getVal() && !power->getNewVal()){      // only transmit code to turn off light if power is on
      LOG1("Zephyr Vent Hood Light: Power Off\n");
      transmitZephyr(powerCode);
      state=0;
      resetFan=true;                                 // if fan is on, we need to tell HomeKit it is now off
    }    

    return(true);
      
  } // update

  void button(int pin, int pressType) override {

    if(pressType==SpanButton::SINGLE){
      LOG1("Zephyr Vent Hood Light SINGLE Button Press: Brightness Change\n");
      transmitZephyr(lightCode);
      
      state--;                      // decrement state
      if(state==0){                 // if reached zero, set power to OFF
        power->setVal(false);
      } else
      
      if(state==-1){                // if state was already zero, reset to 3 and set power to ON
        state=3;
        power->setVal(true);
      }
    } else

    if(pressType==SpanButton::DOUBLE){
      LOG1("Zephyr Vent Hood Light DOUBLE Button Press: IGNORED\n");      
    } else
    
    if(power->getVal()){
      LOG1("Zephyr Vent Hood Light LONG Button Press: Powering off Vent Hood\n");
      transmitZephyr(powerCode);

      state=0;
      power->setVal(false);      
      resetFan=true;
    } else {
      
      LOG1("Zephyr Vent Hood Light LONG Button Press: Power is already off!\n");      
    }
    
  } // button

  void loop(){

    if(resetLight){             
      resetLight=false;
      if(power->getVal()){
        LOG1("Zephyr Vent Hood Light: Resetting Power Off\n");      
        power->setVal(false);
        state=0;
      }
    }
    
  } // loop
    
};
      
//////////////////////////////////

struct DEV_ZephyrFan : Service::Fan {

  uint32_t fanCode;
  uint32_t powerCode;
  SpanCharacteristic *power;
  int fanPin;

  DEV_ZephyrFan(uint32_t fanCode, uint32_t powerCode, int fanPin) : Service::Fan(){

    power=new Characteristic::Active();
    new Characteristic::Name("Vent Hood Fan");
    this->fanCode=fanCode;
    this->powerCode=powerCode;
    this->fanPin=fanPin;
    
    Serial.print("Configuring Zephyr Vent Hood Fan 433MHz Transmitter with fan code: ");
    Serial.print(fanCode,HEX);
    Serial.print("  power code: ");
    Serial.print(powerCode,HEX);
    Serial.print("\n");

    new SpanButton(fanPin);
  }

  boolean update(){

    if(!power->getVal() && power->getNewVal()){       // only transmit code to turn on fan if power is off
      LOG1("Zephyr Vent Hood Fan: Power On\n");
      transmitZephyr(fanCode);
    } else 
    
    if(power->getVal() && !power->getNewVal()){       // only transmit code to turn off fan if power is on
      LOG1("Zephyr Vent Hood Fan: Power Off\n");
      transmitZephyr(powerCode);
      resetLight=true;
    }

    return(true);
    
  } // update

  void button(int pin, int pressType) override {

    if(pressType==SpanButton::SINGLE){
      LOG1("Zephyr Vent Hood Fan SINGLE Button Press: Speed Change\n");
      transmitZephyr(fanCode);
      if(!power->getVal())
        power->setVal(true);
    } else

    if(pressType==SpanButton::DOUBLE){
      LOG1("Zephyr Vent Hood Fan DOUBLE Button Press: IGNORED\n");      
    } else
        
    if(power->getVal()){
      LOG1("Zephyr Vent Hood Fan LONG Button Press: Powering off Vent Hood\n");
      transmitZephyr(powerCode);
      power->setVal(false);      
      resetLight=true;
    } else {
      
      LOG1("Zephyr Vent Hood Fan LONG Button Press: Power is already off!\n");      
    }

  } // button
    
  void loop(){

    if(resetFan){             
      resetFan=false;
      if(power->getVal()){
        LOG1("Zephyr Vent Hood Fan: Resetting Power Off\n");      
        power->setVal(false);
      }
    }
    
  } // loop

};
            
//////////////////////////////////

void transmitZephyr(uint32_t code){
  char c[32];
  sprintf(c,"Transmitting code: %lx\n",code);
  LOG1(c);
  
  RF433.clear();
  
  for(int b=19;b>0;b--){
    if(code&(1<<b))
      RF433.add(620,230);
    else
      RF433.add(230,620);
  }
      
  if(code&1)
    RF433.add(620,4230);
  else
    RF433.add(230,4620);
    
  RF433.start(8,1);

  delay(200);               // wait 200 ms before returning to ensure vent has sufficient time to process request
  
}
