/*************************************************************************\
 *                  Copyright (C) Ed Cole 2018.                            *
 *                 colege@gmail.com                         *
 *                                                                         *
 * This program is free software. You may use, modify, and redistribute it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation, either version 3 or (at your option) any      *
 * later version. This program is distributed without any warranty.  See   *
 * the file COPYING.gpl-v3 for details.                                    *
 *                                                                         *
 \*************************************************************************/

/*********
 * neoprobe
 *
 *********/


#ifndef DEBUG
#define DEBUG 0 // dont change this - pass it via -DDEBUG=1 at compile
#endif

#include <iostream>
#include </usr/local/include/json/json.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MYNAME "neoprobe"
#define VERSION "1.0.0"

#define D_SERVER_NAME "neohub"
#define D_PORT 4242
#define SOC_BUFFER_SZ 4096
#define NO_OF_DEVICES 64

#include "neohub.h"

char myname[] = MYNAME;
char version[] = VERSION;

// default server
static char d_server_name[] = D_SERVER_NAME;
static int port_no = D_PORT;
char *server_name=NULL;

int buffer_sz = 65536;      //  for dump from neohub JSON
int debug = DEBUG;

struct neohub devices[NO_OF_DEVICES]; // see neohub.h

//************   getValueBool
bool getValueBool(char *tok){
    // "KEY":value => true | false
    for(;*tok!=':';tok++);
    tok++;
    if(*tok=='f') return(false);
    return(true);
}//getValueBool

//************   getValueInt
int getValueInt(char *tok){
    // "KEY":value => value string converted to int
    for(;*tok!=':';tok++);
    tok++;
    return(atoi(tok));
}//getValueInt

//************   getValueFloat
float getValueFloat(char *tok){
    // "KEY":value => value string converted to float
    // value is in the form "xx.x" so just skip open quote and
    // NULL the closing quote
    // TODO -- what is the temp is less than 10 degC
    for(;*tok!=':';tok++);
    tok++;
    tok++;    // skip over opening quote
    tok[4]='\0'; // NULL the closing quote
    return(atof(tok));
}//getValueFloat


//************   getValueString
char *getValueString(char *tok){
    
    char *value = NULL;
    int len;
    
    // "KEY":value => value string
    for(;*tok!=':';tok++);
    tok++;  // skip over the :
    tok++;  // skip over opening quote
    len = (int)strlen(tok);
    value = (char *)malloc(len);
    strncpy(value,tok,len);
    value[len-1] = '\0'; //  NULL out closing quote
    return(value);
}//getValueString


//************   errorUsage
void
errorUsage()
{
    printf("USAGE: %s [-v] [-s server] [-p port] \n",myname);
    printf("    -v           - Prints version number and exits\n");
    
}//errorUsage

//************   error
void
error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}//error


//************   createNeohub
int createNeohub(char *buffer, struct neohub *table){
    
    // Populates the devices array of struct neohub from
    // the memory buffer dump from {"INFO":0} command
    
    int device = 0;
    
    buffer = strchr(buffer,'['); // scan to start of device table
    buffer++; // skip over [
    buffer = strtok(buffer,"{},"); // start token parse
    while(*buffer!=']'){  // devices end with ]
        
        // device - string
        if(strncmp(buffer,"\"device",7)==0){
            devices[device].device=getValueString(buffer);
            device++;
            // set up some baseline for next device
            devices[device].stat_mode.thermostat=false;
            devices[device].device=NULL; // next device
        }
        
        // thermostat - bool
        if(strncmp(buffer,"\"THERMOSTAT",11)==0){
            devices[device].stat_mode.thermostat=getValueBool(buffer);
        }
        
        // Current Temperature - float
        if(strncmp(buffer,"\"CURRENT_TEMP",13)==0){
            devices[device].current_temperature
            =getValueFloat(buffer);
        }
        
        buffer = strtok(NULL,"{},"); // next token
        
    }//while buffer
    return(0);
}

//************   indent       - used by parse_buffer
void indent(int i){
    
    while( i-- > 0 ) putc('\t',stdout);
}//indent


//******************************************************
//************   main
//*****************************************************
int
main(int argc, char *argv[])
{
    int  i,sockfd, port;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int opt;
    char buffer[buffer_sz];
    char mbuffer[buffer_sz];
    char *b_point;
    char *key;
    int bytes_in;
    
    static FILE *jsfd;   // json file descriptor for off-line dev hack
    
    struct timeval tv;    // used for timeout in recv on socket
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    while ((opt = getopt(argc, argv, "vp:s:")) != -1){
        switch (opt){
                
            case 's': // Alternate server
                server_name=(char *)malloc(sizeof(optarg));
                strcpy(server_name,optarg);
                break;
                
            case 'p': // Alternate listening port
                port_no = atoi(optarg);
                break;
                
            case 'v':
                printf("%s: Version %s\n",myname,version);
                exit(EXIT_SUCCESS);
                break;
                
            default:
                errorUsage();
                exit(EXIT_FAILURE);
                
        }//switch
    }//while
    
    /*   - connecing to neohub code commented out for offline development
     
    if (server_name==NULL) server_name=d_server_name;
    
    server = gethostbyname(server_name);
    port = port_no;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR creating socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(  (char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(port);
    //printf("connecting to %s:%i\n",server_name,port_no);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        error("ERROR connecting");
    }
    // Set recv timeout as neohub doesn't close socket. It
    // just stops taking
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
    char cmd[] = "{\"INFO\":0}";     // JSON command to extract data
    write(sockfd,cmd,strlen(cmd));
    write(sockfd,"\0\n",2);        // JSON needs \0 terminated string
    bzero(buffer,buffer_sz);    // Not necessary
    b_point=buffer;
    while((bytes_in=(int)recv(sockfd,b_point,SOC_BUFFER_SZ,0))>0){
        b_point=b_point+bytes_in;
    }
    close(sockfd);
    
    */
    
    // off-line test hack simulating neohub dump
    
    // open json file
    if ((jsfd = fopen("/Users/ecole/src/neoprobe/neoprobe/neohub.json","r")) == nullptr){
        fprintf(stderr,"cant open file\n");
        exit(EXIT_FAILURE);
    };
    
    // read contents into buffer
    int n=0;
    while( (buffer[n++] = fgetc(jsfd) ) != EOF  ){
    
    }
    
    // print buffer to stdout
    b_point = buffer;
    while( *b_point != '\0'){
        putchar(*b_point);
        ++b_point;
    }
    putchar('\n');
    
    // copy buffer -> mbuffer as createNeohub() modifies buffer
    memcpy(mbuffer, buffer, buffer_sz);
    
    createNeohub(buffer,devices); // parse buffer into devices array
    
    // Test loop
    i = 0;
    while(devices[i].device != NULL){
        printf("%s : ",devices[i].device);
        if(devices[i].stat_mode.thermostat==true){ // thermostat
            printf("%2.2f\n",devices[i].current_temperature);
        }else{ // not a thermostat
            devices[i].current_temperature=0;
            printf("N/A\n");
        }
        i++;
    }
    
    // jsoncpp playpen
    
    //Json::Value root;
    //Json::CharReaderBuilder reader;
    //std::string errs;
    
    //Json::parseFromStream(reader, &mbuffer, root, &errs);
    
    Json::Value root;   // contains the root value after parsing.
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( mbuffer, root );
    if ( !parsingSuccessful )
    {
        // report to the user the failure and their locations in the document.
        std::cout  << "Failed to parse neohub JSON\n"
        << reader.getFormattedErrorMessages();
        
    }
    
    //auto devicesArray = root["devices"];
    
    std::cout << "***** JSONcpp stuff *****\n";
    
    for(Json::Value element : root["devices"]){ // cycle thru array elements
       
        for( Json:: Value member : element.getMemberNames()){
            std::cout << member << " : ";
            
            switch (element[member.asString()].type())
            {
                case Json::booleanValue:
                    std::cout << "BOOL.....";
                    if(element[member.asString()].asBool()){
                        std::cout << " TRUE ";
                    }else{
                        std::cout << " FALSE ";
                    };
                    //... need to save **membername** and **value**
                    break;
                case Json::realValue:
                    std::cout << "I'm REAL";
                    //... need to save **membername** and **value**
                    break;
                case Json::uintValue:
                    std::cout << "I'm UINT";
                    //... need to save **membername** and **value**
                    break;
                case Json::stringValue:
                    std::cout << "string.....";
                    std::cout  << element[member.asString()].asString();
                    //... need to save **membername** and **value**
                    break;
                case Json::intValue:
                    std::cout << "int.....";
                    std::cout  << element[member.asString()].asInt();
                    //... need to save **membername** and **value**
                    break;
                case Json::nullValue:
                    std::cout << "I'm null";
                    break;
                case Json::arrayValue:
                    std::cout << "Array ......";
                    //... code to parse an array (with nested sure) ...
                    //... need to save
                    break;
                case Json::objectValue:
                    std::cout << "Object ......";
                    //... code to parse an object (with nested sure) ...
                    //... need to save
                    break;
            }//switch
            std::cout << std::endl;
            
        }// for
        
        /*
        for(std::string key : keys){
    
            std::cout <<  key << ":" << element[key].asString() << "\n\t";
           
        }// for
        std::cout << "\n";
         */
        
        
    }// for
    
    
    
    
    exit(0);
}//main


