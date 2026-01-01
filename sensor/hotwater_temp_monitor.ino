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
#include <esp_sntp.h>

// board: ESP-32 C6 supermini

// Instantiate the one-wire bus for the temperature sensors
static OneWire OneWire(ONE_WIRE_BUS) ;

// Instantiate the temperature sensors
static DallasTemperature Sensors(&OneWire);
// No of boots performed - used to determine when to resync NTP
RTC_DATA_ATTR int BootCount = 0;

static void GetNtpTime(void)
{
  const long GmOffsetSec = 0;
  const int  DaylightOffsetSec = 3600;
  uint32_t Retries = 30 ;

  esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, NTP_SERVER);
  esp_sntp_init();

  // allow up to 30s for sync to occur
  Serial.println("Wait for NTP sync...") ;
  while(sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED)
  {
    Serial.print(F("."));
    Retries-- ;
    delay(1000) ;
  }
  if ( Retries )
    Serial.println("done.") ;
  else
    Serial.println("timed out.") ;
}

void setup() 
{
  IPAddress LocalIp ;
  uint32_t Retries = 10 ;
  bool Connected = false ;
  MQTTClient MQclient;
  WiFiClient Net;
  const uint32_t SleepTime = 15*60 ; // expressed in seconds
  const uint32_t NtpSyncInterval = 24*60*60 / SleepTime ;  // resync NTP every 24 hours

  // turn on status led, indicate we're alive
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);
  
  Serial.begin(115200);
  Serial.println("Starting up...");

  // connect to Wifi
  WiFi.disconnect(true);
  Serial.println("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID1, PWD1);
  // don't spin indefinately here, if we fail to connect after 10 tries, give up for this session
  while ((WiFi.status() != WL_CONNECTED) && Retries)
  {
    Serial.print(F("."));
    Retries-- ;
    delay(1000);
  }

  if ( !Retries )
    Serial.println("Failed to connect to Wifi");
  else
  {
    LocalIp = WiFi.localIP();
    Serial.println(F("WiFi connected"));
    Serial.println(LocalIp);

    Serial.println("Connecting to broker");
    MQclient.begin(MQTT_BROKER,Net) ;
    Connected = MQclient.connect("hotwater-pub") ;

    if ( !Connected )
      Serial.println("Failed to connect to MQTT broker");
  }

  if ( Connected )
  {
    // update NTP time every 24 hours
    if ( !(BootCount % NtpSyncInterval))
      GetNtpTime() ;

    BootCount++ ;

    Sensors.begin();
    int NumOfSensors = Sensors.getDeviceCount();
    Serial.printf("Detected %d sensors\n",NumOfSensors);

    DeviceAddress SensorAddr ;
    std::stringstream SensorBuf ;
    time_t TimeStamp = time(NULL);

    // build up sensor string for MQTT data packet
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

    // report the current battery voltage level
    int AnalogMv = analogReadMilliVolts(BATTERY_ADC);
    AnalogMv = (AnalogMv*1515)/1000 ;  // 1515 = 5/3.3v voltage divider

    SensorBuf.str("") ;
    SensorBuf << ((double)AnalogMv/1000.0) ;
    Serial.println(SensorBuf.str().c_str());
    MQclient.publish("hotwater/battery",SensorBuf.str().c_str(),true,0) ;

    MQclient.disconnect() ;
  }

  Serial.println("Entering deep sleep mode");

  if ( Connected )
  {
    // time to allow any pending traffic to flush through
    // pulse LED to indicate successful update
    for(uint32_t i=0 ; i < 3 ; i++ )
    {
      digitalWrite(STATUS_LED, LOW);
      delay(400);
      digitalWrite(STATUS_LED, HIGH);
      delay(400);
    }
  }
  else
    delay(2000) ;
  
  digitalWrite(STATUS_LED, LOW);
  esp_sleep_enable_timer_wakeup(SleepTime*1000*1000) ; // expressed in microseconds
  esp_deep_sleep_start();
}

void loop() 
{
  // does nothing (we can never get here)
  delay(1000) ;
}
