//
//  dsp_settings.c
//  SDtestC
//
//  Created by Matt Webb on 1/1/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include "dsp_settings.h"

float dsp_read_value(ApiRead* read) {
    // lookup the value corresponding to the attributes of the read object
    //aka channel, channel i/o, feature, param
    //return the corresponding value as a float
    //may need to cast as float just to be safe!
    return (float) 14578.51;
}

uint8_t dsp_write_value(ApiWrite* write) {
    //figure out what parameter you are writing
    //use channel, channel i/o, feature, param
    //if write succeeds, return 1
    //if write fails, return 0
    
    return 1;
}