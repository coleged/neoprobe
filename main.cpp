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

#include "neohub.h"
#include </usr/local/include/json/json.h>

char myname[] = MYNAME;
char version[] = VERSION;

bool quiet,verbose;


int buffer_sz = 65536;      //  for dump from neohub JSON
int debug = DEBUG;

struct neohub devices[NO_OF_DEVICES]; // see neohub.h



//************   error()
void
error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}//************  error()


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
        
        // heating - bool if true stat is calling for heat
        if(strncmp(buffer,"\"HEATING",8)==0){
            devices[device].heating=getValueBool(buffer);
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


//************   examineElement()
void examineElement(Json::Value element){
    
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
                std::cout << "Object ......" << std::endl;
                //... code to parse an object (with nested sure) ...
                //... need to save
                    std::cout <<  std::endl;
                    examineElement(element[member.asString()]);
                    std::cout << "END ... Object ......" << std::endl;
                break;
        }//switch
        std::cout << std::endl;
    }//for
    std::cout << std::endl;
}//examineElement()


//******************************************************
//************   main
//*****************************************************
int
main(int argc, char *argv[])
{
    int  i;
    int opt;
    char buffer[buffer_sz];
    char mbuffer[buffer_sz];
    extern char *server_name;
    extern int port_no;
    bool temp_flag = false;
    bool debug_flag = false;

    quiet=verbose=false;
    
    while ((opt = getopt(argc, argv, "DVtvp:s:")) != -1){
        switch (opt){
                
            case 's': // Alternate server (neohub)
                server_name=(char *)malloc(sizeof(optarg));
                strcpy(server_name,optarg);
                break;
                
            case 'D': // Alternate server (neohub)
                debug_flag = true;
                break;
                
            case 'p': // Alternate neohub port number
                port_no = atoi(optarg);
                break;
                
            case 't': // Temperatures
                temp_flag = true;
                break;
                
            case 'v': // verbose
                verbose = true;
                break;
                
            case 'q': // quiet
                quiet = true;
                break;
        
            case 'V':
                printf("%s: Version %s\n",myname,version);
                exit(EXIT_SUCCESS);
                break;
                
            default:
                errorUsage();
                exit(EXIT_FAILURE);
                
        }//switch
    }//while
    
    char cmd[]="{\"INFO\":0}";
    
    getNeohub(cmd,buffer,buffer_sz);  // get the JSON from the neohub
    
    // copy buffer -> mbuffer as createNeohub() modifies buffer
    memcpy(mbuffer, buffer, buffer_sz);
    
    createNeohub(buffer,devices); // parse buffer into devices array
    
    if(temp_flag){
        i = 0;
        while(devices[i].device != NULL){
            if(devices[i].stat_mode.thermostat==true){ // thermostat
                printf("%s : ",devices[i].device);
                printf("%2.2f ",devices[i].current_temperature);
                if(devices[i].heating){
                    printf("HEATING\n");
                }else{
                    printf("\n");
                }
            }else{ // not a thermostat
                devices[i].current_temperature=0;
                //printf("N/A\n");
            }
            i++;
        }
    }
    
    // jsoncpp playpen
    
    if (debug_flag){
        
        // 
    
        Json::Value root;   // contains the root value after parsing.
        Json::Reader reader;
        bool parsingSuccessful = reader.parse( mbuffer, root );
        if ( !parsingSuccessful )
        {
            std::cout  << "Failed to parse neohub JSON\n"
            << reader.getFormattedErrorMessages();
            
        }
        
        std::cout << "***** JSONcpp stuff *****\n";
        
        for(Json::Value element : root["devices"]){ // cycle thru array elements
           
                examineElement(element);
            
        }// for
    }//if
    
    exit(0);
}//main


