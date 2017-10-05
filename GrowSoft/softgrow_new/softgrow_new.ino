#include "dht.h"
#include <TimedAction.h>
#include <MenuBackend.h>
#include<LiquidCrystal.h>
#include <SoftwareSerial.h>
////////////////////////////////////////////////////////////////////////////////////////////////
//PIN DEFINITIONS
//INPUTS
dht DHT;
#define dht_dpin A0
#define soil A1
#define buttonPinUp A2      // pin for the Up button
#define buttonPinDown  A3    // pin for the Down button
#define buttonPinEsc  A4     // pin for the Esc button
#define buttonPinEnter  A5 
//OUTPUTS
#define fan 9
#define heater 10
#define spray 11
#define pump 12
#define dehumidifier 13
///////////////////////////////////////////////////////////////////////////////////////////////
//OTHER DEFINITIONS
#define MAX_TEMP 25
#define MIN_TEMP 18
#define MAX_HUM 50
#define MIN_HUM 40
#define MIN_SOIL_HUM 50
///////////////////////////////////////////////////////////////////////////////////////////////
//CUSTOM VARIABLES
int max_temp = MAX_TEMP;
int min_temp = MIN_TEMP;
int max_hum = MAX_HUM;
int min_hum = MIN_HUM;
int min_soil_hum = MIN_SOIL_HUM;
byte degree[8] =
              {
                0b00011,
                0b00011,
                0b00000,
                0b00000,
                0b00000,
                0b00000,
                0b00000,
                0b00000
              };
///////////////////////////////////////////////////////////////////////////////////////////////

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

///////////////////////////////////////////////////////////////////////////////////////////////
//DEBOUNCE VARIABLES
int lastButtonPushed = 0;
int lastButtonPushedSub = 0;

int lastButtonEnterState = LOW;  // the previous reading from the Enter input pin
int lastButtonEscState = LOW;    // the previous reading from the Esc input pin
int lastButtonUpState = LOW;     // the previous reading from the UP input pin
int lastButtonDownState = LOW;   // the previous reading from the DOWN input pin

long lastEnterDebounceTime = 0;  // the last time the enter pin was toggled
long lastEscDebounceTime = 0;    // the last time the escape pin was toggled
long lastUpDebounceTime = 0;     // the last time the up pin was toggled
long lastDownDebounceTime = 0;   // the last time the down pin was toggled
long debounceDelay = 500;        // the debounce time
/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
//MENU VARIABLES
void menuChanged(MenuChangeEvent changed);
void menuUsed(MenuUseEvent event);
MenuBackend menu = MenuBackend(menuUsed,menuChanged);
MenuItem Item1 = MenuItem("CREDITS");

MenuItem Item2 = MenuItem("INSPECT GROWROOM");

MenuItem Item3 = MenuItem("SET CONDITIONS");
MenuItem Item3sub1 = MenuItem("SET TEMPERATURE");
MenuItem Item3sub2 = MenuItem("SET HUMIDITY");
MenuItem Item3sub3 = MenuItem("SET SOIL HUMIDITY");

/////////////////////////////////////////////////////////////////////////////////////////////////
//***********************************************************************************************************************************************************************
void setup() {
  Serial.begin(9600);   
  /////////////////////////////
  //define INPUTS
  //Serial.print("!!!");
  pinMode(dht_dpin, INPUT);
  pinMode(soil, INPUT);
  pinMode(buttonPinUp, INPUT);
  pinMode(buttonPinDown, INPUT);
  pinMode(buttonPinEsc, INPUT);
  pinMode(buttonPinEnter, INPUT);
  ////////////////////////////
  //define OUTPUTS
  pinMode(heater, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(spray, OUTPUT);
  pinMode(pump, OUTPUT);
  pinMode(dehumidifier, OUTPUT);
  ////////////////////////////
  lcd.begin(16,2);
  lcd.createChar(1, degree);
  ///////////////////////////////////////////////////////////////////////////////////////////////
  //MENU CONFIGURATION
  menu.getRoot().add(Item2);
  Item1.addRight(Item2);
  Item2.addRight(Item3);
  Item3.addRight(Item1);
  Item1.addLeft(Item3);
  Item3.add(Item3sub1);
  Item3sub1.addRight(Item3sub2);
  Item3sub2.addRight(Item3sub3);
  Item3sub3.addRight(Item3sub1);
  Item3sub1.addLeft(Item3sub3);
  menu.toRoot();
  /////////////////////////////////////////////////////////////////////////////////////////////////
}
//***********************************************************************************************************************************************************************
//***********************************************************************************************************************************************************************
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //TIMED ACTIONS
TimedAction MainProcess = TimedAction(5000,mainProcess);
TimedAction RefreshMenu = TimedAction(20000,refreshMenu);
TimedAction ReadConditions = TimedAction(5000,readConditions);
  /////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  MainProcess.check();
  RefreshMenu.check();
  ReadConditions.check();
  readButtons(true);  
  navigateMenus();
}
//***********************************************************************************************************************************************************************
//***********************************************************************************************************************************************************************
void  readButtons(bool recordLast){  //read buttons status  
  int reading;
  int buttonEnterState=LOW;             
  int buttonEscState=LOW;             
  int buttonUpState=LOW;             
  int buttonDownState=LOW;             
  /*
   *DEBOUNCING TECHNIQUE 
   *1)check the state of the pin
   *2)if pin has different value that its previous value then we reset the debounce timer
   *3)if the time set time of debounce has passed then store the new state
  */
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //ENTER
  reading = digitalRead(buttonPinEnter); 
  if (reading != lastButtonEnterState) { 
    lastEnterDebounceTime = millis();
  }                 
  if ((millis() - lastEnterDebounceTime) > debounceDelay) { 
    buttonEnterState=reading;
    lastEnterDebounceTime=millis();
  }
  lastButtonEnterState = reading;
  /////////////////////////////////////////////////////////////////////////////////////////////////
  
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //ESCAPE
  reading = digitalRead(buttonPinEsc);
  if (reading != lastButtonEscState) {
    lastEscDebounceTime = millis();
  } 
  if ((millis() - lastEscDebounceTime) > debounceDelay) {
    buttonEscState = reading;
    lastEscDebounceTime=millis();
  }
  lastButtonEscState = reading; 
  /////////////////////////////////////////////////////////////////////////////////////////////////
  
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //UP
  reading = digitalRead(buttonPinUp);
  if (reading != lastButtonUpState) {
    lastUpDebounceTime = millis();
  } 
  if ((millis() - lastUpDebounceTime) > debounceDelay) {
    buttonUpState = reading;
    lastUpDebounceTime =millis();
  }
  lastButtonUpState = reading;                          
  /////////////////////////////////////////////////////////////////////////////////////////////////
  
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //DOWN
  reading = digitalRead(buttonPinDown);
  if (reading != lastButtonDownState) {
    lastDownDebounceTime = millis();
  } 
  if ((millis() - lastDownDebounceTime) > debounceDelay) {
    buttonDownState = reading;
    lastDownDebounceTime=millis();;
  }
  lastButtonDownState = reading; 
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //CHECK WHICH BUTTON WAS PRESSED LAST
  if(recordLast){ //this is used from menu
    if (buttonEnterState == HIGH){
      lastButtonPushed = buttonPinEnter;
    }
    else if(buttonEscState == HIGH){
      lastButtonPushed = buttonPinEsc;
    }
    else if(buttonDownState == HIGH){
      lastButtonPushed = buttonPinDown;
    }
    else if(buttonUpState == HIGH){
      lastButtonPushed = buttonPinUp;
    }
    else{
      lastButtonPushed=0;
    }
  } 
  else{ //this is used from selectValue()
    if (buttonEnterState == HIGH){
      lastButtonPushedSub = buttonPinEnter;
    }
    else if(buttonEscState == HIGH){
      lastButtonPushedSub = buttonPinEsc;
    }
    else if(buttonDownState == HIGH){
      lastButtonPushedSub = buttonPinDown;
    }
    else if(buttonUpState == HIGH){
      lastButtonPushedSub = buttonPinUp;
    }
    else{
      lastButtonPushedSub=0;
    }
  }              
  //////////////////////////////////////////////////////////////////////////////////////////////////   
}
//***********************************************************************************************************************************************************************
//***********************************************************************************************************************************************************************
void navigateMenus() {
  MenuItem currentMenu=menu.getCurrent();  
  switch (lastButtonPushed){
    case buttonPinEnter:
      if(!(currentMenu.moveDown())){  
        menu.use();
      }
      else{
        menu.moveDown();
      } 
      break;
    case buttonPinEsc:
      menu.toRoot();
      break;
    case buttonPinDown:
      menu.moveRight();
      
      break;      
    case buttonPinUp:
      menu.moveLeft();
      break;      
  }
  lastButtonPushed=0;
}

//***********************************************************************************************************************************************************************
//***********************************************************************************************************************************************************************
void menuChanged(MenuChangeEvent changed){
  
  MenuItem newMenuItem=changed.to; //get the destination menu
  lcd.clear();
  if(newMenuItem.getName()==menu.getRoot()){
      //DHT.read11(dht_dpin);
      lcd.setCursor(0,0);
      lcd.print("  Temp: ");
      lcd.setCursor(7,0);
      lcd.print(String(DHT.temperature));
      lcd.setCursor(11,0);
      lcd.write(1);
      lcd.setCursor(12,0);
      lcd.print("C ");
      
      lcd.setCursor(0,1);
      lcd.print("Hum:");
      lcd.setCursor(4,1);
      lcd.print(String(DHT.humidity));
      lcd.setCursor(6,1);
      lcd.print("% ");
      lcd.setCursor(8,1);
      lcd.print("Soil:");
      lcd.setCursor(13,1);
      lcd.print(String(map(analogRead(soil), 0, 1023, 100, 0)));
      lcd.setCursor(15,1);
      lcd.print("%");
  }
  if(newMenuItem==Item1){
      lcd.setCursor(0,0);
      lcd.print("    CREDITS     ");
  }
  else if(newMenuItem==Item2){
      lcd.setCursor(0,0);
      lcd.print(" INSPECT MIN-MAX");
      lcd.setCursor(0,1);
      lcd.print("    CONDINTION  ");
  }
  else if(newMenuItem==Item3){
      lcd.setCursor(0,0);
      lcd.print("      SET       ");
      lcd.setCursor(0,1);
      lcd.print("   CONDITIONS   ");
  }
  else if(newMenuItem==Item3sub1){
      lcd.setCursor(0,0);
      lcd.print("SET MAX AND MIN ");
      lcd.setCursor(0,1);
      lcd.print("  TEMPERATURE   ");
  }
  else if(newMenuItem==Item3sub2){
      lcd.setCursor(0,0);
      lcd.print("SET MAX AND MIN ");
      lcd.setCursor(0,1);
      lcd.print("    HUMIDITY    ");
  }
  else if(newMenuItem==Item3sub3){
      lcd.setCursor(0,0);
      lcd.print("  SET MINIMUM   ");
      lcd.setCursor(0,1);
      lcd.print(" SOIL MOISTURE  ");
  }
}

//***********************************************************************************************************************************************************************
//***********************************************************************************************************************************************************************

void menuUsed(MenuUseEvent event){
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //CREDITS SUBMENU
  if( event.item == Item1 ){ 
    lcd.setCursor(0,0);
    lcd.print("Created by:");
    lcd.setCursor(0,1);
    lcd.print("Kolias Electr.");
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //INSPECT CONDITIONS
  else if( event.item == Item2 ){ 
    lcd.setCursor(0,0);
    lcd.print("T:");
    lcd.setCursor(2,0);
    lcd.print(String(min_temp));
    lcd.setCursor(4,0);
    lcd.print("-");
    lcd.setCursor(5,0);
    lcd.print(String(max_temp));

    lcd.setCursor(7,0);
    lcd.print("  S:");
    lcd.setCursor(11,0);
    lcd.print(String(min_soil_hum));
    lcd.setCursor(13,0);
    lcd.print("   ");
    
    lcd.setCursor(0,1);
    lcd.print("    H:");
    lcd.setCursor(6,1);
    lcd.print(String(min_hum));
    lcd.setCursor(8,1);
    lcd.print("-");
    lcd.setCursor(9,1);
    lcd.print(String(max_hum));
    lcd.setCursor(11,1);
    lcd.print("    ");
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //TEMPERATURE
  else if( event.item == Item3sub1 ){
    min_temp = selectValue("SET MIN TEMP",min_temp,100,0);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("    ********    ");
        delay(1000);
    lcd.clear();
    if(min_temp != -1){
      max_temp = selectValue("SET MAX TEMP",max_temp,100,0);
    }
    if((max_temp>=min_temp) && max_temp != -1 && min_temp != -1){
      lcd.print("    SUCCESS     ");
    }
    else{
      max_temp = MAX_TEMP;
      min_temp = MIN_TEMP;
      lcd.print("    FAILURE     ");
    }  
    delay(3000);
    lcd.clear();
    menu.toRoot();
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //HUMIDITY
  else if( event.item == Item3sub2 ){
    min_hum = selectValue("SET MIN HUM",min_hum,100,0);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("    ********    ");
    delay(1000);
    lcd.clear();
    if(min_hum != -1 ) { 
      max_hum = selectValue("SET MAX HUM:",max_hum,100,0);
    }
    if(max_hum>=min_hum && max_hum != -1 && min_hum != -1){
      lcd.print("    SUCCESS     ");
    }
    else{
      max_hum = MAX_HUM;
      min_hum = MIN_HUM;
      lcd.print("    FAILURE     ");
    }
    delay(3000);
    lcd.clear();
    menu.toRoot();
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //SOIL HUMIDITY
  else if( event.item == Item3sub3 ){
    min_soil_hum = selectValue("MIN SOIL HUM",min_soil_hum,100,0);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("    *******    ");
    delay(1000);
    lcd.clear();
    if( min_soil_hum != -1){
      lcd.print("    SUCCESS     ");
    }
    else{
      min_soil_hum = MIN_SOIL_HUM;
      lcd.print("    FAILURE     ");
    }
    delay(3000);
    lcd.clear();
    menu.toRoot();
  }
 
}

//***********************************************************************************************************************************************************************
//***********************************************************************************************************************************************************************
int selectValue(String message, int value,int upperBound,int lowerBound){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(message);
  
  lastButtonPushedSub = 0;
  readButtons(false);
  while(lastButtonPushedSub!=buttonPinEnter){
    readButtons(false);
    if(lastButtonPushedSub == buttonPinEsc){
      value = -1;
      break;
    } 
    if(lastButtonPushedSub==buttonPinUp && value <= upperBound){
      value++;
    }
    if(lastButtonPushedSub==buttonPinDown && value >= lowerBound){
      value--;
    }
    lcd.setCursor(0,1);
   lcd.print(String(value));
  }
  lcd.clear();
  return value;
}
//***********************************************************************************************************************************************************************
//***********************************************************************************************************************************************************************

void refreshMenu(){
  if(menu.getCurrent() == menu.getRoot()){
    menu.toRoot();
  }
}

void readConditions(){
   DHT.read22(dht_dpin);
}

void mainProcess(){
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //VARIABLES AND INITIALIZATION
  int cur_temp = DHT.temperature;
  int cur_hum = DHT.humidity;
  Serial.println(DHT.temperature);
  int cur_soil_hum = map(analogRead(soil), 0, 1023, 100, 0);
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////   
  int select = 0;
  if(cur_temp>=max_temp){
      digitalWrite(fan, HIGH);
      digitalWrite(heater, LOW);
  }
  if(cur_temp<=min_temp){
      digitalWrite(fan, LOW);
      digitalWrite(heater, HIGH);
  }
  if(cur_temp>min_temp && cur_temp<max_temp){
    digitalWrite(fan, LOW);
    digitalWrite(heater, LOW);
  }
  if(cur_soil_hum<=min_soil_hum){
    digitalWrite(pump, HIGH);
  }
  else{
    digitalWrite(pump, LOW);
  }
  if(cur_hum<=min_hum){
    digitalWrite(spray, LOW);
    digitalWrite(dehumidifier, HIGH);
  }
  if(cur_hum>=max_hum){
    digitalWrite(spray, LOW);
    digitalWrite(dehumidifier, HIGH);
  }
  if(cur_hum>min_hum && cur_hum<max_hum){
    digitalWrite(spray, LOW);
    digitalWrite(dehumidifier, LOW);
  }        
  //////////////////////////////////////////////////////////////////////////////////////////////////////
}

//***********************************************************************************************************************************************************************
//***********************************************************************************************************************************************************************
