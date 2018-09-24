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

#define _MAC_OS

#include "neohub.h"
#include </usr/local/include/json/json.h>

char myname[] = MYNAME;
char version[] = VERSION;

bool quiet,verbose;


int buffer_sz = 65536;      //  for dump from neohub JSON
int debug = DEBUG;
bool debug_flag = false;

struct neohub devices[NO_OF_DEVICES]; // see neohub.h

// ************   timestamp()
char *timestamp(){ // for log line output
    // returns pointer to static string containing current date & time
    static char timestamp[] = "dd-mm-yyyy,hh:mm";
    time_t now = time(0);
    tm *ltm = localtime(&now);
    sprintf(timestamp,"%02d-%02d-%4d,%02d:%02d",
            ltm->tm_mday,
            1 + ltm->tm_mon,
            1900 + ltm->tm_year,
            ltm->tm_hour,
            ltm->tm_min);
    return(timestamp);
}// ************   timestamp()

// ************   error()
void
error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}// ************  error()


// ************   errorUsage
void
errorUsage()
{
    // DVltvp:s:
    printf("USAGE: %s [-VvtlD] [-s server] [-p port] \n",myname);
    printf("    -t           - Prints temperatures and state of all stats and timers\n");
    printf("    -l           - Prints temperatures all stats in one log line\n");
    printf("    -D           - Debug. Prints debug detail\n");
    printf("    -v           - Verbose. Prints more detail\n");
    printf("    -V           - Prints version number and exits\n");
}// ************   errorUsage


#ifdef _MAC_OS

/* now depreciated but left here for reference
// ************   examineElement()
void examineElement(int dev, Json::Value element, bool verbose){
    // recursively parse object element. Populating devices structure array
    static int depth = 1;
    for( Json:: Value member : element.getMemberNames()){
        if(verbose) std::cout << "{" << depth << "}" << member << " : ";
        switch (element[member.asString()].type())
        {
            case Json::booleanValue:
                if(verbose) std::cout << "(bool) ";
                if(element[member.asString()].asBool()){
                    if(verbose) std::cout << "= true ";
                }else{
                    if(verbose) std::cout << "= false ";
                };
                if(member.asString() == "THERMOSTAT"){
                    devices[dev].stat_mode.thermostat=element[member.asString()].asBool();
                    if(verbose) std::cout << "loaded into struct";
                }
                if(member.asString() == "TIMECLOCK"){
                    devices[dev].stat_mode.timeclock=element[member.asString()].asBool();
                    if(verbose) std::cout << "loaded into struct";
                }
                if(member.asString() == "TIMER"){
                    devices[dev].timer=element[member.asString()].asBool();
                    if(verbose) std::cout << "loaded into struct";
                }
                if(member.asString() == "HEATING"){
                    devices[dev].heating=element[member.asString()].asBool();
                    if(verbose) std::cout << "loaded into struct";
                }
                if(verbose) std::cout << std::endl;
                break;
            case Json::realValue:
                if(verbose) std::cout << "(real) ";
                if(verbose) std::cout  << element[member.asString()].asFloat();
                if(verbose) std::cout << std::endl;
                break;
            case Json::uintValue:
                if(verbose) std::cout << "(u_int) ";
                if(verbose) std::cout  << element[member.asString()].asInt();
                if(verbose) std::cout << std::endl;
                break;
            case Json::stringValue:
                if(verbose) std::cout << "(string) \"";
                if(verbose) std::cout  << element[member.asString()].asString();
                if(verbose) std::cout << "\"";
                if(member.asString() == "CURRENT_TEMPERATURE"){
                    devices[dev].current_temperature=
                                stof(element[member.asString()].asString(),nullptr);
                    if(verbose) std::cout << "loaded into struct as float";
                }
                if(verbose) std::cout << std::endl;
                break;
            case Json::intValue:
                if(verbose) std::cout << "(int) ";
                if(verbose) std::cout  << element[member.asString()].asInt();
                if(verbose) std::cout << std::endl;
                //... need to save **membername** and **value**
                break;
            case Json::nullValue:
                if(verbose) std::cout << "(null)";
                if(verbose) std::cout << std::endl;
                break;
            case Json::arrayValue:
                if(verbose) std::cout << "(array))";
                if(verbose) std::cout <<  std::endl;
                //... parse the array by recursively calling myself for
                // each array member
                depth++;
                for(Json::Value s_element : element[member.asString()]){ //
                    if( s_element.type() == Json::stringValue){ // array of strings
                        if(verbose) std::cout << "{" << depth << "}" << "(string) \"";
                        if(verbose) std::cout  << s_element.asString();
                        if(verbose) std::cout << "\"" << std::endl;
                    }else{ // array of objects
                        examineElement(dev,s_element,verbose);
                    }
                }//for
                if(verbose) std::cout << "{" << depth << "}" << "(END-array)";
                if(verbose) std::cout << std::endl;
                depth--;
                break;
            case Json::objectValue:
                if(verbose) std::cout << "(object)";
                if(verbose) std::cout <<  std::endl;
                //... parse the object by calling myself
                depth++;
                examineElement(dev,element[member.asString()],verbose);
                if(verbose) std::cout << "{" << depth << "}" << "(END-object)";
                if(verbose) std::cout << std::endl;
                depth--;
                break;
        }//switch
    }//for
}//END*********** examineElement()
 */

void parseInfo(Json::Value root, bool verbose){
    // parses the INFO:0 command
    int device = 0;
    Json::Value stat;
    
    for(Json::Value element : root["devices"]){ // cycle thru devices
        devices[device].device=element["device"].asString();
        devices[device].current_temperature=stof(element["CURRENT_TEMPERATURE"].asString(),nullptr);
        devices[device].heating=element["HEATING"].asBool();
        devices[device].timer=element["TIMER"].asBool();
        stat = element["STAT_MODE"];
            devices[device].stat_mode.thermostat=stat["THERMOSTAT"].asBool();
            devices[device].stat_mode.timeclock=stat["TIMECLOCK"].asBool();
        device++;
        
    }// for
}

void json_parse(char *buffer, bool verbose){ //  Alternate version using CharReader
   
        Json::Value root;   // contains the root value after parsing.
        Json::CharReaderBuilder builder;
        Json::CharReader * reader = builder.newCharReader();
        Json::Value param; // object to hold nested object
        
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
    
    /*
     // depricated code
        for(Json::Value element : root["devices"]){ // cycle thru devices
            devices[device].device=element["device"].asString();
            if(verbose) std::cout << element["device"] << std::endl;
            examineElement(device,element,verbose);
            device++;
        }// for
    */
    parseInfo(root,verbose);
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
    
#ifdef _MAC_OS
    
    json_parse(buffer,verbose); // uses CharReader method
    
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


