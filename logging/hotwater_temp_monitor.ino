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

// board: ESP-32 S3 supermini

// Instantiate the one-wire bus for the temperature sensors
static OneWire OneWire(ONE_WIRE_BUS) ;

// Instantiate the temperature sensors
static DallasTemperature Sensors(&OneWire);
// no of sensors on the bus
static int NumOfSensors ;

// UDP broadcast stuff
static int sFd = -1 ;
static struct sockaddr_in BroadcastAddr ; 

static void GetNtpTime(void)
{
  const long GmOffsetSec = 0;
  const int  DaylightOffsetSec = 3600;

  // once initially set, this should periodically re-sync to the server
  configTime(GmOffsetSec, DaylightOffsetSec, NTP_SERVER);
}

void setup() 
{
  IPAddress LocalIp ;
  int EnBroadcast = 1 ;

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

  // create the UDP socket & configure for broadcasts
  sFd = socket(PF_INET,SOCK_DGRAM,0) ;
  if ( sFd < 0 )
    Serial.println("Failed to create UDP socket");
  else
  {
    if ( setsockopt(sFd, SOL_SOCKET, SO_BROADCAST, (char*)&EnBroadcast, sizeof(EnBroadcast)) < 0 )
      Serial.println("Failed set broadcast option on UDP socket");

    memset((void*)&BroadcastAddr, 0, sizeof(struct sockaddr_in));
    BroadcastAddr.sin_family = PF_INET;
    BroadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    BroadcastAddr.sin_port = htons(52004);
  }
  Serial.println("Setup done");
}

void loop() 
{
  static unsigned long LastStatusUpdate = 0 ;
  const unsigned long StatusUpdateInterval = 1000 ;
  unsigned long TimeNow ;
  DeviceAddress SensorAddr ;
  std::stringstream SensorBuf ;
  time_t TimeStamp = time(NULL);

  TimeNow = millis() ;
  // blink the status LED every second to provide indication of alive
  if ( (TimeNow - LastStatusUpdate) > StatusUpdateInterval )
  {
    LastStatusUpdate = TimeNow ;
    if ( digitalRead(STATUS_LED) == LOW )
      digitalWrite(STATUS_LED, HIGH);
    else
      digitalWrite(STATUS_LED, LOW);
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
      Serial.printf("Sensor %d: %f ºC\n",i,temperatureC);
      // append to the sensor buffer
      SensorBuf << " " << temperatureC ;
    }
  }
  Serial.println(SensorBuf.str().c_str());

  // transmit
  if ( sendto(sFd, SensorBuf.str().c_str(), SensorBuf.str().size(), 0, (struct sockaddr*) &BroadcastAddr, sizeof(struct sockaddr_in)) < 0 )
    Serial.println("Failed to send broadcast packet") ;

  delay(5000);
}
