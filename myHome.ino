#include <PubSubClient.h>// MQTT library

#include<Ticker.h>// Requried for watchdog

//Including libraries for WiFi manager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>        // https://github.com/tzapu/WiFiManager


//Defining variables

#define ledPin D5
bool ledpinStatus;
unsigned long timeNow;
unsigned long lastpublishTime= 0;
long publishInterval=60000;


//--------------------ISR for implementing watchdog-------------------//

Ticker secondTick;

volatile int watchdogCount=0;

void ISRwatchdog(){

  watchdogCount++;

  if(watchdogCount==60){

  Serial.println();

  Serial.print("The watch dog bites......");

   //ESP.reset();

  ESP.restart();

  }}


const char* mqtt_server = "182.163.112.207";//Broker address

WiFiClient espClient;

PubSubClient client(espClient);


//Main Setup

void setup() {

  Serial.begin(115200);

  WiFiManager wifiManager;

 //wifiManager.resetSettings();

  wifiManager.autoConnect("myHome", "admin1234");

  Serial.println("Connected.");


  secondTick.attach(1,ISRwatchdog);

  client.setServer(mqtt_server, 1883);

  client.setCallback(callback);

  pinMode(ledPin,OUTPUT);
}


//Main loop

void loop(){
  
  watchdogCount=0;
 
  if (!client.connected()) {

    reconnect();

}

  client.loop();

//checking led status

  ledpinStatus=digitalRead(ledPin);
  
  //Serial.println(ledpinStatus);
  //delay(1000);

  timeNow=millis();

  if(timeNow-lastpublishTime>publishInterval)

  {

  if(ledpinStatus==0)client.publish("home/ledStatus","0");
  if(ledpinStatus==1)client.publish("home/ledStatus","1");
  
  lastpublishTime=timeNow;  
  
  }

}

//----------------------reconnect()--------------------------//

void reconnect() {

  // Loop until we're reconnected

  while (!client.connected()) 

  {

    Serial.print("Attempting MQTT connection...");

    // Create a random client ID

    String clientId = "ESP8266-";

    clientId += String(random(0xffff), HEX);

    // Attempt to connect

    //if your MQTT broker has clientID,username and password

    //please change following line to    if (client.connect(clientId,userName,passWord))

    if (client.connect(clientId.c_str()))

    {

      Serial.println("connected");

     //once connected to MQTT broker, subscribe command if any

    //----------------------Subscribing to required topics-----------------------//

      client.subscribe("myHome/led");

      Serial.println("Subsribed to topic: myHome/led");

      client.subscribe("myHome/reset");

      Serial.println("Subsribed to topic:myHome/reset");

   } 
   else 
   {
     Serial.print("failed, rc=");
     Serial.print(client.state());
     Serial.println(" try again in 5 seconds");

     // Wait 6 seconds before retrying

     delay(5000);

    }

  }

} //end reconnect()



//---------------------------Callback funtion-------------------------------------//


void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

//------------------------user_input for manual load control-----------------//

  if(strcmp(topic, "myHome/led") == 0){

  Serial.print("Message:");

  for (int i = 0; i < length; i++) {

    Serial.print((char)payload[i]);

    char data=payload[i];

    if (data=='0')

    {

      digitalWrite(ledPin,LOW);//light off

      Serial.println("Light turned off through user app");
      
      //publishing led status
      client.publish("home/ledStatus","0");

      //Update ligth status (if required)
      //Publish light status

      }

      if (data=='1')

    {

      digitalWrite(ledPin,HIGH);//light on
      Serial.println("Ligth turned off through user app");

      //Update ligth status (if required)
      //publishing led status
      client.publish("home/ledStatus","1");


      }}}



//----------------------Restarting the board from Engineer's end----------------------------//

  
  if(strcmp(topic, "myHome/reset") == 0){

  Serial.print("Message:");

  for (int i = 0; i < length; i++) {

  Serial.print((char)payload[i]);

  char data2=payload[i];

    if (data2=='1')

    {

      Serial.println("Resetting Device.........");

       ESP.restart();
       //ESP.reset();

      }}}
}//end of callback
