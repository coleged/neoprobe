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

#define _MAC_OS     // using this to include jsoncpp code that I currently cant get to work with Ubuntu

#include "neohub.h"
#include </usr/local/include/json/json.h>

char myname[] = MYNAME;
char version[] = VERSION;

bool quiet,verbose;


int buffer_sz = 65536;      //  for dump from neohub JSON
int debug = DEBUG;
bool debug_flag = false;

struct neohub devices[NO_OF_DEVICES]; // see neohub.h

char *timestamp(){ // for log line output
    static char timestamp[] = "dd-mm-yyyy,hh:mm";
    // current date/time based on current system
    time_t now = time(0);
    tm *ltm = localtime(&now);
    sprintf(timestamp,"%02d-%02d-%4d,%02d:%02d",
            ltm->tm_mday,
            1 + ltm->tm_mon,
            1900 + ltm->tm_year,
            ltm->tm_hour,
            ltm->tm_min);
    
    return(timestamp);
    
}

//************   error()
void
error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}//************  error()


//************   errorUsage
void
errorUsage()
{
    printf("USAGE: %s [-v] [-s server] [-p port] \n",myname);
    printf("    -t           - Prints tempratures and state of all stats and timers\n");
    printf("    -l           - Prints tempratures and state of all stats in one log line\n");
    printf("    -D           - Prints debug detail of all connected devices\n");
    printf("    -V           - Prints version number and exits\n");
    
}//errorUsage


#ifdef _MAC_OS

//************   examineElement()
void examineElement(int dev, Json::Value element){
    
    // recursively parse object element. Prints out what it finds.
    static int depth = 1;
    
    for( Json:: Value member : element.getMemberNames()){
        if(debug_flag) std::cout << "{" << depth << "}" << member << " : ";
        switch (element[member.asString()].type())
        {
            case Json::booleanValue:
                if(debug_flag) std::cout << "(bool) ";
                if(element[member.asString()].asBool()){
                    if(debug_flag) std::cout << "= true ";
                }else{
                    if(debug_flag) std::cout << "= false ";
                };
                if(member.asString() == "THERMOSTAT"){
                    devices[dev].stat_mode.thermostat=element[member.asString()].asBool();
                    if(debug_flag) std::cout << "loaded into struct";
                }
                if(member.asString() == "TIMECLOCK"){
                    devices[dev].stat_mode.timeclock=element[member.asString()].asBool();
                    if(debug_flag) std::cout << "loaded into struct";
                }
                if(member.asString() == "TIMER"){
                    devices[dev].timer=element[member.asString()].asBool();
                    if(debug_flag) std::cout << "loaded into struct";
                }
                if(member.asString() == "HEATING"){
                    devices[dev].heating=element[member.asString()].asBool();
                    if(debug_flag) std::cout << "loaded into struct";
                }
                
                if(debug_flag) std::cout << std::endl;
                break;
            case Json::realValue:
                if(debug_flag) std::cout << "(real) ";
                if(debug_flag) std::cout  << element[member.asString()].asFloat();
                if(debug_flag) std::cout << std::endl;
                break;
            case Json::uintValue:
                if(debug_flag) std::cout << "(u_int) ";
                if(debug_flag) std::cout  << element[member.asString()].asInt();
                if(debug_flag) std::cout << std::endl;
                break;
            case Json::stringValue:
                if(debug_flag) std::cout << "(string) \"";
                if(debug_flag) std::cout  << element[member.asString()].asString();
                if(debug_flag) std::cout << "\"";
               
                if(member.asString() == "CURRENT_TEMPERATURE"){
                    devices[dev].current_temperature=
                                stof(element[member.asString()].asString(),nullptr);
                    if(debug_flag) std::cout << "loaded into struct as float";
                }
                if(debug_flag) std::cout << std::endl;
                
                break;
            case Json::intValue:
                if(debug_flag) std::cout << "(int) ";
                if(debug_flag) std::cout  << element[member.asString()].asInt();
                if(debug_flag) std::cout << std::endl;
                //... need to save **membername** and **value**
                break;
            case Json::nullValue:
                if(debug_flag) std::cout << "(null)";
                if(debug_flag) std::cout << std::endl;
                break;
            case Json::arrayValue:
                if(debug_flag) std::cout << "(array))";
                if(debug_flag) std::cout <<  std::endl;
                //... parse the array by recursively calling myself for
                // each array member
                depth++;
                for(Json::Value s_element : element[member.asString()]){ //
                    if( s_element.type() == Json::stringValue){ // array of strings
                        if(debug_flag) std::cout << "{" << depth << "}" << "(string) \"";
                        if(debug_flag) std::cout  << s_element.asString();
                        if(debug_flag) std::cout << "\"" << std::endl;
                    }else{ // array of objects
                        examineElement(dev,s_element);
                    }
                }//for
                if(debug_flag) std::cout << "{" << depth << "}" << "(END-array)";
                if(debug_flag) std::cout << std::endl;
                depth--;
                break;
            case Json::objectValue:
                if(debug_flag) std::cout << "(object)";
                if(debug_flag) std::cout <<  std::endl;
                //... parse the object by calling myself
                depth++;
                examineElement(dev,element[member.asString()]);
                if(debug_flag) std::cout << "{" << depth << "}" << "(END-object)";
                if(debug_flag) std::cout << std::endl;
                depth--;
                break;
        }//switch
        
    }//for
}//END*********** examineElement()

void json_parse(char *buffer){ //  Alternate version using CharReader
   
        //
        int device = 0;
    
        Json::Value root;   // contains the root value after parsing.
        
        Json::CharReaderBuilder builder;
        Json::CharReader * reader = builder.newCharReader();
        
        std::string errors;
        
        bool parsingSuccessful = reader->parse( buffer,
                                               buffer+strlen(buffer),
                                               &root,
                                               &errors );
        
        if ( !parsingSuccessful )
        {
            
            std::cout  << "Failed to parse JSON\n"
            << errors << std::endl;
            
        }
        
        if(debug_flag) std::cout << "***** JSONcpp stuff *****\n";
        
        for(Json::Value element : root["devices"]){ // cycle thru devices
            
            devices[device].device=element["device"].asString();
            if(debug_flag) std::cout << element["device"] << std::endl;
            examineElement(device,element);
            device++;
            
        }// for
 
}


#endif



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
    
    bool log_flag = false;

    quiet=verbose=false;
    
    while ((opt = getopt(argc, argv, "DVltvp:s:")) != -1){
        switch (opt){
                
            case 's': // Alternate server (neohub)
                server_name=(char *)malloc(sizeof(optarg));
                strcpy(server_name,optarg);
                break;
                
            case 'D': //
                debug_flag = true;
                break;
                
            case 'l': // log output
                log_flag = true;
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
    // TODO can probably use buffer now as not using createNeohub, but parsing
    // JSON with jsoncpp rather than home grown parser
    
    memcpy(mbuffer, buffer, buffer_sz);
    
    
#ifdef _MAC_OS
    
    json_parse(mbuffer); // uses CharReader method
    
#endif
    
    if(temp_flag){
        i = 0;
        while(devices[i].device != ""){
            std::cout << devices[i].device;
            if(devices[i].stat_mode.thermostat){ // thermostat
                
                printf(" %2.2f",devices[i].current_temperature);
                if(devices[i].heating){
                    printf(" HEATING");
                }else{
                    //printf(" NOT HEATING");
                }
            }else{ // not a thermostat
                devices[i].current_temperature=0;
                //printf("N/A\n");
            }
            if(devices[i].stat_mode.timeclock){ // timer
                if(devices[i].timer){
                    printf(" ON");
                }else{
                    printf(" OFF");
                }
            }
            printf("\n");
            i++;
        }// while
    } // if temp_flag
    if(log_flag){
        i = 0;
        printf("%s",timestamp());
        while(devices[i].device != ""){
            if(devices[i].stat_mode.thermostat){ // thermostat
                printf(",%2.2f",devices[i].current_temperature);
            }
            i++;
        }// while
        printf("\n");
    }
    
    exit(0);
}//main


