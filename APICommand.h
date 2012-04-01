/*
 * APICommand.h
 *
 *  Created on: Dec 19, 2011
 *      Author: ParallelsWin7
 */

#ifndef APICOMMAND_H_
#define APICOMMAND_H_
#include <stdint.h>
#include "../dspsys_lib_channel/common.h"
#include <stdlib.h>


typedef enum Type_enum {
	READ  = 0,
	WRITE = 1,
	ACK   = 2,
	NOT   = 3
} Type_enum;

typedef enum Feature_enum {
	NOF  = 0,
	EQB1 = 1,
	EQB2 = 2,
	EQB3 = 3,
	EQB4 = 4,
	COMP = 5,
	LIM  = 6,
	MAT  = 9
} Feature_enum;

typedef enum Param_enum {
	NOP       = 0,
	EN        = 1,
	GAIN      = 2,
	BW        = 3,
	FREQ      = 4,
	TYPE      = 5,
	THRESHOLD = 11,
	RATIO     = 12,
	ATTACK    = 13,
	RELEASE   = 14
} Param_enum;

typedef struct ApiCommandType {
	Type_enum type;
    void (*callback)(void* _api, float value);
	uint8_t channel;
	Io_enum chan_io;
	Feature_enum feature;
	Param_enum param;
	uint8_t cmd_count;
    uint8_t retry_count;
} ApiCmd;

typedef struct ApiReadValueType {
	ApiCmd super;
} ApiRead;

typedef struct ApiWriteValueType {
	ApiCmd super;
    float value;
    
} ApiWrite;

typedef struct ApiAckType {
	ApiCmd super;
	float value;
} ApiAck;

typedef struct ApiNotificationType {
	ApiCmd super;
	uint8_t message;
} ApiNot;

//API CONSTRUCTORS
void ApiCmd_ctor(ApiCmd *cmd, Type_enum type, uint8_t channel, Io_enum io, Feature_enum feature, Param_enum param);
void ApiRead_ctor(ApiRead *read, uint8_t channel, Io_enum io, Feature_enum feature, Param_enum param);
void ApiWrite_ctor(ApiWrite *write, uint8_t channel, Io_enum io, Feature_enum feature, Param_enum param, float value);
void ApiAck_ctor(ApiAck *ack, uint8_t count, float value);
void ApiNot_ctor(ApiNot *not, uint8_t channel, Io_enum io, Feature_enum feature, uint8_t message);

//API GETTERS
void* Api_get_callback(ApiCmd *cmd);
uint8_t Api_get_cmd_count(ApiCmd *cmd);


//API SETTERS
void Api_set_callback(ApiCmd *cmd, void (*callback)(void*, float));
void Api_set_cmd_count(ApiCmd *cmd, uint8_t cmd_count);
void Api_set_retry_count(ApiCmd *cmd, uint8_t retry_count);
void Api_inc_retry_count(ApiCmd *cmd);
void Api_reset_retry_count(ApiCmd *cmd);

//API FORMATTERS
//Format Api Command styles for transmission
//Char array (2nd function param) is modified to contain the bytes first-to-last in transmission order
void ApiRead_frmtr(ApiRead *cmd, char *formatted);
void ApiWrite_frmtr(ApiWrite *cmd, char *formatted);
void ApiAck_frmtr(ApiAck *cmd, char *formatted);
void ApiNot_frmtr(ApiNot *cmd, char *formatted);

//API RECONSTRUCTORS
//Re-constructors upon reception
void ApiRead_rector(ApiRead *cmd, char *formatted);
void ApiWrite_rector(ApiWrite *cmd, char *formatted);
void ApiAck_rector(ApiAck *cmd, char *formatted);
void ApiNot_rector(ApiNot *cmd, char *formatted);

//API ENCODERS
char Api_common_front(ApiCmd *cmd);

//API BYTE DECODERS
uint8_t Api_decode_channel(char x);
Type_enum Api_decode_type(char x);
Io_enum Api_decode_io(char x);
Feature_enum Api_decode_feature(char x);
Param_enum Api_decode_param(char x);

//API OBJECT INSPECTION TOOLS
void ApiCmd_inspect(ApiCmd *cmd);
void ApiRead_inspect(ApiRead *cmd);
void ApiWrite_inspect(ApiWrite *cmd);
void ApiAck_inspect(ApiAck *cmd);
void ApiNot_inspect(ApiNot *cmd);

#endif /* APICOMMAND_H_ */
