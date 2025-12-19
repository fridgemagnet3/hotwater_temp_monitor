#include <config.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <time.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <sstream>
#include <MQTT.h>

// board: ESP-32 S3 supermini

// Instantiate the one-wire bus for the temperature sensors
static OneWire OneWire(ONE_WIRE_BUS) ;

// Instantiate the temperature sensors
static DallasTemperature Sensors(&OneWire);
// no of sensors on the bus
static int NumOfSensors ;
// MQTT client instance
static MQTTClient MQclient;
static WiFiClient Net;

static void GetNtpTime(void)
{
  const long GmOffsetSec = 0;
  const int  DaylightOffsetSec = 3600;

  // once initially set, this should periodically re-sync to the server
  configTime(GmOffsetSec, DaylightOffsetSec, NTP_SERVER);
}

static void BrokerConnect(void)
{
  // connect to MQTT broker
  Serial.println("Connecting to broker");
  MQclient.begin(MQTT_BROKER,Net) ;
  while(!MQclient.connect("hotwater-pub"))
  {
    delay(500);
    Serial.print(F("."));
  }
}

void setup() 
{
  IPAddress LocalIp ;

  // turn on status led, indicate we're alive
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);
  
  Serial.begin(115200);
  Serial.println("Starting up...");
  Sensors.begin();

  NumOfSensors = Sensors.getDeviceCount();
  Serial.printf("Detected %d sensors\n",NumOfSensors);

  // connect to Wifi
  Serial.println("Connecting to WiFi");
  //WiFi.mode(WIFI_STA);
  WiFi.begin(SSID1, PWD1);
  // workaround https://forum.arduino.cc/t/no-wifi-connect-with-esp32-c3-super-mini/1324046/12
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(F("."));
  }
  LocalIp = WiFi.localIP();
  Serial.println(F("WiFi connected"));
  Serial.println(LocalIp);

  // fetch current time
  GetNtpTime() ;

  BrokerConnect() ;

  Serial.println("Setup done");
}

void loop() 
{
  DeviceAddress SensorAddr ;
  std::stringstream SensorBuf ;
  time_t TimeStamp = time(NULL);

  // blink the status LED every time around the loop to indicate we're alive
  if ( digitalRead(STATUS_LED) == LOW )
    digitalWrite(STATUS_LED, HIGH);
  else
    digitalWrite(STATUS_LED, LOW);

  if ( !MQclient.connected())
  {
    Serial.println("Lost connection to broker, re-connecting") ;
    BrokerConnect() ;
  }

  // build up sensor string for UDP packet
  SensorBuf.precision(1) ;
  // add in timestamp
  SensorBuf << TimeStamp << std::fixed ;

  Sensors.requestTemperatures(); 
  for(int i=0 ; i < NumOfSensors ; i++)
  {
    if ( Sensors.getAddress(SensorAddr,i))
    {
      float temperatureC = Sensors.getTempC(SensorAddr);
      String TempBuf(temperatureC,1) ;
      String TopicBuf("hotwater/temp") ;

      TopicBuf+=String(i) ;

      Serial.printf("Sensor %d: %f ºC\n",i,temperatureC);

      // publishh individual sensor readings
      MQclient.publish(TopicBuf,TempBuf) ;

      // append to the sensor buffer
      SensorBuf << " " << temperatureC ;
    }
  }
  Serial.println(SensorBuf.str().c_str());

  // publish everything, retained so we can always grab the latest
  MQclient.publish("hotwater/data",SensorBuf.str().c_str(),true,0) ;

  delay(5000);
}
