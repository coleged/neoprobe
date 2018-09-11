//
//  file.cpp
//  neoprobe
//
//  Created by Ed Cole on 10/09/2018.
//  Copyright © 2018 Ed Cole. All rights reserved.
//

#include "file.hpp"

//*************** readJson()

bool readJson(char *file, char *buffer){
    
    FILE *jsonfd;
    int n =0;
    
    if( file == nullptr){
        jsonfd = stdin;
    }else{
        if( (jsonfd = fopen(file, "r")) == nullptr){
            fprintf(stderr,"ERROR: Unable to open file %s\n",file);
            return false;
        };
    }
    while ( (buffer[n++] = fgetc(jsonfd)) != EOF){
        
    }
    
    return true;
}//END***********readJson
