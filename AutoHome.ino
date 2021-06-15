#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

const char* ssid        = "PTCL_BB";
const char* password    = "F4D428A4";
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

#define DHTPIN 2     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT11  
DHT_Unified dht(DHTPIN, DHTTYPE);
#define ACPin 14

void MQTT_connect();
uint32_t x=0;
static int cc = 0;
static bool AUTO = true;
static bool AC = true;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(strstr(topic,"AC"))
  {
      AC = (int) payload[0] %48;
  }
  if(strstr(topic,"AUTO"))
  {
      AUTO = (int) payload[0] %48;
  }
  Serial.println(AUTO);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESPAns121";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("ans/room/AUTO");
      client.subscribe("ans/room/AC");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  pinMode(ACPin,OUTPUT);
  // Initialize device.
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  // Connect to WiFi access point.
 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); 
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float RoomTemp  = event.temperature;
  if (!isnan(RoomTemp)) {
    Serial.print(F("Temperature: "));
    Serial.print(RoomTemp);
    Serial.println(F("°C"));
  }
  dht.humidity().getEvent(&event);
  int   RoomHumi  = event.relative_humidity;

  char stemp[5] = {0};
  char shumi[5] = {0};

  char sAC[3]    = {0};
  char sAUTO[3]  = {0};
  


  if (!isnan(RoomHumi)) {
    Serial.print(F("Humidity: "));
    Serial.print(RoomHumi);
    Serial.println(F("%"));
  }


  if(AUTO)
  {
    if(event.relative_humidity < 50 && event.temperature < 25)
    {
          Serial.println("AUTOOO 0000");
      digitalWrite(ACPin,0);
    }
    
    if(event.relative_humidity > 50 && event.temperature > 28)
    {
      Serial.println("AUTOOO 1111");
      digitalWrite(ACPin,1);
    }  
  }
  else
  {
    Serial.println("AUTOOO ELSE");
    if(!AC)
    {
      digitalWrite(ACPin,1);
    }
    else
    {
      digitalWrite(ACPin,0);
    }  
  }
  sprintf(stemp,"%f",RoomTemp);
  sprintf(shumi,"%d",RoomHumi);
  sprintf(sAUTO,"%d",AUTO);
  sprintf(sAC,"%d",digitalRead(ACPin));
  Serial.println(sAC);
  client.publish("ans/room/temp",stemp);
  client.publish("ans/room/humi",shumi);
  client.publish("ans/room/AUTOs",sAUTO);
  client.publish("ans/room/ACs"  ,sAC);
  client.loop();
  delay(1000);
}
