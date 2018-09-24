//
//  neoprobe.cpp
//  neoprobe
//
//  Created by Ed Cole on 19/05/2018.
//  Copyright © 2018 Ed Cole. All rights reserved.
///

#include "neohub.h"
#include "neoprobe.h"

#define JSON_FILE "/Users/ecole/src/neoprobe/neoprobe/neohub.json"
#define _NEO_CONNECTED    // uncomment this when neohub available

extern bool quite,verbose,debug_flag;

// default server
char d_server_name[] = D_SERVER_NAME;
int port_no = D_PORT;
char *server_name=NULL;

//************ stripString()
char *stripString(char *str){
    // strip out unquoted whitespace and new lines from a string.
    bool quote = false;
    char *f = str;
    char *r = str;
    
    while( *f != '\0'){
        if( *f == '"'){
            quote = !quote;
        }
        if( isspace(*f) == 0 ){ // not a space
            *r = *f;
            r++;
        }else{ // it is a space
            if( quote ){
                *r = *f;
                r++;
            }
        }
        f++;
    }
    *r = '\0';
    return(str);
}
//END************* stripString()

//********** getNeoHub()
char *getNeohub(char *cmd,char *buffer,int buffer_sz){
    // connects to neohub issues string in cmd and returns response in buffer.

#ifdef _NEO_CONNECTED
    int  sockfd, port;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char *b_point;
    int bytes_in;
    
    struct timeval tv;    // used for timeout in recv on socket
    tv.tv_sec = 4;
    tv.tv_usec = 0;
    
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
    if (verbose) printf("connecting to %s:%i\n",server_name,port_no);
    
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        error("ERROR connecting");
    }
    // Set recv timeout as neohub doesn't close socket. It
    // just stops taking
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    if (verbose) printf("\n%s\n",stripString(cmd));
    write(sockfd,stripString(cmd),strlen(stripString(cmd)));
    write(sockfd,"\0\n",2);         // JSON needs \0 terminated string
    bzero(buffer,buffer_sz);        // Probably not necessary
    b_point=buffer;
    while((bytes_in=(int)recv(sockfd,b_point,SOC_BUFFER_SZ,0))>0){
        b_point=b_point+bytes_in;
    }
    close(sockfd);
   
#else // read JSON from a file
    if(debug_flag) printf("reading JSON from file\n");
    char json_file_name[] = JSON_FILE;
    readJson(json_file_name, buffer);
#endif
    
    return(buffer);
}

