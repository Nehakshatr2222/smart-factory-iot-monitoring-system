//include required libraries
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <DHT.h>

#include "config.h"
#include "SensorManager.h"
#include "actuator.h"
#include "network.h"
#include "telemetry.h"
#include "rpc.h"
//#define SHARED_BUF_SIZE 200
//uint8_t sharedBuf[SHARED_BUF_SIZE];
static char sharedBuf[200];
  


//create object of type ethernet pubsubclient
static EthernetClient ethClient;
static PubSubClient     mqttClient(ethClient);

unsigned long lastTelemetry =0;

static SensorManager    sensors;


void setup() 
{
  Serial.begin(9600);
  Serial.print("Node2 is booting...");

  // put your setup code here, to run once:
   // initilase sensor 
   sensor_begin();
   sensor_init(&sensors);
   //initialise actutors
   actuators_begin();

    
   telemetry_init(sharedBuf, sizeof(sharedBuf));
   rpc_init(&mqttClient,sharedBuf, sizeof(sharedBuf) );

   // connect board to internet and mqtt
   network_begin(&mqttClient);

   
}


void loop() 
{
  // put your main code here, to run repeatedly:
  // keep checking if board is connected to Things board 
  network_maintain();

  //read data from the sensors 
  SensorData data;
  sensor_read( &data, &sensors);   // humidity,ldr,temp,door status,motion status

  //if humidity is more than 85 turn on the realy if realy is off
  if(data.humidity >= HUMIDITY_CRIT && !actuators_getRelayState())
  {
    actuators_setRelay("true");
  }


  
  //publish to the cloud 
  // check if network available then push data to things board
  if (network_isConnected())
  {  unsigned long now = millis();
     //publish data to things board for every 5 secs
     if( now - lastTelemetry >= TELEMETRY_INTERVAL) // if 5secs is over
       {
          lastTelemetry = now;
          telemetry_publishTelemetry( &data, actuators_getRelayState()); // converting sensor into json and publish
       }
  }

   actuators_updateStatusLEDs(network_isConnected(), data.sensorError);

}
