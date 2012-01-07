//
//  dsp_settings.h
//  SDtestC
//
//  Created by Matt Webb on 1/1/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#ifndef SDtestC_dsp_settings_h
#define SDtestC_dsp_settings_h

#include <stdint.h>
#include "common.h"
#include "APICommand.h"

float dsp_read_value(ApiRead* read);
uint8_t dsp_write_value(ApiWrite* write);

#endif
