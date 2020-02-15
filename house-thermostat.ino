#include <OneWire.h> 
#include <DallasTemperature.h>
#include "U8glib.h"

#define ONE_WIRE_BUS 10
#define SLOTS 100
#define HOURS_TO_SHOW 24
#define DEFAULT_MIN 1000.0
#define DEFAULT_MAX -1000.0
//#define MILLIS_PER_COLUMN 675000 //(24h*60*60*1000/128)
#define MILLIS_PER_COLUMN 337500 //(12h*60*60*1000/128)
//#define MILLIS_PER_COLUMN 5000
#define DISPLAY_WIDTH_PX 128
#define VERTICAL_EXAGGERATION 8

const int xOffset = DISPLAY_WIDTH_PX - SLOTS;
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI 

float temps[SLOTS];
float minTemp = DEFAULT_MIN;
float maxTemp = DEFAULT_MAX;

float readTemp(){
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}

void updateMinMax(float result){
  for (int i=0; i < SLOTS; i++){
    if (temps[i] == 0.0){
      continue;
    }
    
    if (temps[i] < minTemp){
      minTemp = temps[i];
    } else if (temps[i] > maxTemp){
      maxTemp = temps[i];
    }
  }

  if (minTemp == DEFAULT_MIN){
    minTemp = result;
  }

  if (maxTemp == DEFAULT_MAX){
    maxTemp = result;
  }

  if (result > maxTemp){
    maxTemp = result;
  } else if (result < minTemp){
    minTemp = result;
  }
}

extern volatile unsigned long timer0_millis;
void resetMillis(){
  noInterrupts ();
  timer0_millis = 0;
  interrupts ();
}

void setup(void) {  
  Serial.begin(9600); 
  sensors.begin(); 
  u8g.setColorIndex(1); // pixel on
}

void drawScreen(String line1){

  char* l1 = line1.c_str();
  String smint = String(minTemp, 1);
  char* mint = smint.c_str();
  String smaxt = String(maxTemp, 1);
  char* maxt = smaxt.c_str();

  float range = maxTemp - minTemp;
  int factor;
  if (range == 0.0){
    factor = 1;
  } else {
    factor = 32/range;
  }

  /*Serial.print("maxTemp: ");
  Serial.println(maxTemp);
  Serial.print("minTemp: ");
  Serial.println(minTemp);
  Serial.print("range: ");
  Serial.println(range);
  Serial.print("factor: ");
  Serial.println(factor);(*/
    
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_unifont);
    u8g.drawStr(0, 10, l1);
    u8g.setFont(u8g_font_helvR08);
    u8g.drawStr(0, 40, maxt);
    u8g.drawStr(0, 63, mint);
    
    for (int i = 0; i < SLOTS; i++){
      int y2 = 63 - factor * (temps[i] - minTemp);
      int x = i + xOffset;
      
      u8g.drawLine(x, 63, x, y2);
    }
    
  } while( u8g.nextPage() );
}

void shuffleLeft(){
  for (int i = 0; i < SLOTS-1; i++){
    temps[i] = temps[i+1];
  }
}

unsigned long lastUpdated;
float temp;
bool startup = true;

void loop(void) 
{ 
  unsigned long now = millis();

  long duration = now - lastUpdated;

  temp = readTemp();
  
  if (startup || duration < 0 || duration > MILLIS_PER_COLUMN){
    startup = false;
    shuffleLeft();  
    temps[SLOTS - 1] = temp;
    lastUpdated = now;

    for (int i=0; i < SLOTS; i++){
      Serial.print(temps[i]);
      Serial.print(" ");
    }
    Serial.println();
  }

  updateMinMax(temp);

  String line1 = "Now: " + String(temp,1) + "C";
  drawScreen(line1);
}
