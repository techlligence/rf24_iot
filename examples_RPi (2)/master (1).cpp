 
 
 /** RF24Mesh_Example_Master.ino by TMRh20
  * 
  * Note: This sketch only functions on -Arduino Due-
  *
  * This example sketch shows how to manually configure a node via RF24Mesh as a master node, which
  * will receive all data from sensor nodes.
  *
  * The nodes can change physical or logical position in the network, and reconnect through different
  * routing nodes as required. The master node manages the address assignments for the individual nodes
  * in a manner similar to DHCP.
  *
  */
  
#include "RF24Mesh/RF24Mesh.h"  
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <math.h>

// MY LIBRARIES
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <thread>


RF24 radio(25,0);  
RF24Network network(radio);
RF24Mesh mesh(radio,network);


unsigned long testMessage = 12345;
unsigned long sendToggleMessage = 8844;
uint32_t displayTimer = 0;

uint8_t slaveNode = 2;

uint16_t currentAddress;


struct NodeData{
	int active;
	int nodeID;
	float currentLevel;
	int relayStatus;
	int smokeStatus;
	unsigned long lastReceived = 0;
	
};

struct sensor_information {
	float currentStatus = 0.0;
	int relayStatus = 0;
	int smokeStatus = 0;
};

// Lets say we can store 10 nodes for now
struct NodeData allNodeData[10];

using namespace std;

string fifoFormat(struct NodeData allNodeData[]){
	string format = "";
	for(int i = 0; i < 10; i++){
		if(allNodeData[i].active == 1){
			string nodeID = to_string(allNodeData[i].nodeID);
			string currentLevel = to_string(allNodeData[i].currentLevel);
			string relayLevel = to_string(allNodeData[i].relayStatus);
			string smokeLevel = to_string(allNodeData[i].smokeStatus);
			format += nodeID + " " + currentLevel + " " + relayLevel + " " + smokeLevel + " # ";
		}
		// We now we reached the end of the line
	} 
	format += "@";

	return format;	
}

void stringToChar(char* charMessage, string message){
	strcpy(charMessage, message.c_str());
}

void readToggleFIFO(int toggleFIFOfile){
	while(1){
		char toggleMessage[1024];
		read(toggleFIFOfile, toggleMessage, 1024);
		std::string stringMessage(toggleMessage);
		printf("Node toggled: %s\n",toggleMessage);
	}

}

int toggleFIFO;

bool sendStatus = false;
int toToggleNodeID = 0;

int main(int argc, char** argv) {
/////////////////////////////////////
//testing
///////////////////////////////////

  // OPEN THE TOGGLE FIFO 
  //int fd = open("../toggleFIFO", O_RDONLY);
  //if(fd == -1){
//	  printf("FIFO failed with -1 exit code\n");
//	  return 1;
 // }
 // //////////////////////////////////
 // Writing the CURRENTLEVEL FIFO
 // //////////////////////////////////
  int currentLevelFIFO = open("../currentLevelFIFO", O_WRONLY); 
  if(currentLevelFIFO == -1){
	  printf("Couln't open the current level fifo\n");
	  return 1;
  }

  // /////////////////////////////////
  // Reading the toggleFIFO
  // /////////////////////////////////
  toggleFIFO = open("../toggleFIFO", O_RDONLY | O_NONBLOCK);
  if(toggleFIFO == -1){
	  printf("Could not open toggle fifo\n");
	  return 1;
  }

  // Set the nodeID to 0 for the master node
  mesh.setNodeID(0);
  // Connect to the mesh
  printf("start\n");
  uint8_t channel = 125;
  mesh.begin(channel);
  radio.printDetails();

  // Start the toggle fifo thread
  //std::thread toggleThread(readToggleFIFO, toggleFIFO);

while(1)
{

  //int fifoMessage;
  //read(fd, &fifoMessage, sizeof(fifoMessage));
  //printf("FIFO message:");
  //printf("%u\n", (unsigned int)fifoMessage);
  
  // Call network.update as usual to keep the network updated
  mesh.update();

  // In addition, keep the 'DHCP service' running on the master node so addresses will
  // be assigned to the sensor nodes
  mesh.DHCP();
  
  
  // Check for incoming data from the sensors
  while(network.available()){
//    printf("rcv\n");
    RF24NetworkHeader header;
    network.peek(header);
    
    uint32_t dat=0;

    //double incomingData = 0;
    struct sensor_information incomingData;

    uint16_t currentNodeID = 0;

    switch(header.type){
      // Display the incoming millis() values from the sensor nodes
      case 'M': network.read(header,&incomingData,sizeof(incomingData)); 

    		currentNodeID = mesh.getNodeID(header.from_node);
                //printf("Rcv %u from address 0%o\n",incomingData,header.from_node);
	  	//printf("From NODEID: %u\n", (unsigned int) currentNodeID);
		if(allNodeData[currentNodeID].active == 0){
			// make the array element active
			allNodeData[currentNodeID].active = 1;
			allNodeData[currentNodeID].nodeID = currentNodeID;
		}
		// Set the new current level	
		allNodeData[currentNodeID].currentLevel = (float)((int)(incomingData.currentStatus*100+.5))/100;
		allNodeData[currentNodeID].relayStatus = incomingData.relayStatus;
		allNodeData[currentNodeID].smokeStatus = incomingData.smokeStatus;
		allNodeData[currentNodeID].lastReceived = millis();

                break;
      default:  network.read(header,0,0); 
                printf("Rcv bad type %d from 0%o\n",header.type,header.from_node); 
                break;
    }
  }

  for(int i = 0; i < 10; i++){
	if(allNodeData[i].active == 1){
		unsigned long currentDelay = millis() - allNodeData[i].lastReceived;
		if(currentDelay > 5000){
			allNodeData[i].active = 0;
		}
	}
  }

  /////////////////////////////////////////
  // testing
  /////////////////////////////////////////
  //for(int i = 0; i < 10; i++){
//	  if(allNodeData[i].active == 1){
//		  printf("Node %u has current level %u\n", allNodeData[i].nodeID, allNodeData[i].currentLevel);
//	  }
 // }
 // //////////////////////////////
 // WRITING THE CURRENTLEVEL FIFO
 // //////////////////////////////
  string currentStatus = fifoFormat(allNodeData);
  char charCurrentStatus [currentStatus.length() + 1];
  stringToChar(charCurrentStatus, currentStatus);

  if(write(currentLevelFIFO, &charCurrentStatus, sizeof(charCurrentStatus)) == -1){
	  printf("Couldn't write the current level status to fifo\n");
	  return 2;
  }


  printf("%s\n", charCurrentStatus);

  //printf("Address List:\n");
  //for(int i = 0; i < mesh.addrListTop; i++){
//	  uint16_t testAddress = mesh.addrList[i].address;
//	  uint16_t testNodeID = mesh.getNodeID(testAddress);
//	  printf("NodeID: %u\n", (unsigned int) testNodeID);
 // }
 //
 // //////////////////////////////////////////
 // Reading the toggle fifo
 // //////////////////////////////////////////
 char toggleMessage[1024];
 if(read(toggleFIFO,toggleMessage,1024)!=-1 && read(toggleFIFO,toggleMessage,1024)!=0){
	 printf("Received: %s\n");
	 toToggleNodeID = atoi(toggleMessage);
	 printf("Digit: %d\n", toToggleNodeID);
	 sendStatus = true;
 }
	 
 //read(toggleFIFO, toggleMessage, 1024);
 //printf("Toggle FIFO: %s\n", toggleMessage);



  if(millis() - displayTimer > 1000){

	  displayTimer = millis();
	  //printf("Nodes connected : \n");
	  if(sendStatus == true){
		  uint16_t currentAddress =  mesh.getAddress(toToggleNodeID);
  	  	  RF24NetworkHeader header(currentAddress);
  	          int x = network.write(header, &sendToggleMessage, sizeof(sendToggleMessage));
		  printf("Sent toggle message to the node\n");
		  sendStatus = false;
	  }
	  
	  //for(int i = 0; i < mesh.addrListTop; i++){
	  	  //printf("Node : ");
	  //	  currentAddress = mesh.addrList[i].address;
	  	  //printf("%u\n", (unsigned int)currentAddress);
  	  //	  RF24NetworkHeader header(currentAddress);
  	   //       int x = network.write(header, &testMessage, sizeof(testMessage));
	  //}
  }
  }
return 0;
}
