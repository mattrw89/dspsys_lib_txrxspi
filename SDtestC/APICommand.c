/*
 * APICommand.c
 *
 *  Created on: Dec 19, 2011
 *      Author: ParallelsWin7
 */
#include <stdio.h> //not needed in CCS


#include "APICommand.h"


/*////////////////////////////////////////////////////////////////////////
////////////                 API CONSTRUCTORS               //////////////
////////////////////////////////////////////////////////////////////////*/

void ApiCmd_ctor(ApiCmd *cmd, Type_enum type, uint8_t channel, Io_enum io, Feature_enum feature, Param_enum param) {
	cmd->type = type;
	cmd->channel = channel;
	cmd->chan_io = io;
	cmd->feature = feature;
	cmd->param = param;
}

void ApiRead_ctor(ApiRead *read, uint8_t channel, Io_enum io, Feature_enum feature, Param_enum param) {
	ApiCmd_ctor(&read->super, READ, channel, io, feature, param);
    (read->super).cmd_count = 64;
}

void ApiWrite_ctor(ApiWrite *write, uint8_t channel, Io_enum io, Feature_enum feature, Param_enum param, float value) {
	ApiCmd_ctor(&write->super, WRITE, channel, io, feature, param);
	write->value = value;
    (write->super).cmd_count = 72;
}

void ApiAck_ctor(ApiAck *ack, uint8_t count, float value) {
	//Maybe go fetch the previous instruction record and populate that data here? TODO
	ApiCmd_ctor(&ack->super, ACK, 0, INPUT, NOF, NOP);

	(ack->super).cmd_count = count;
	ack->value = value;
}

void ApiNot_ctor(ApiNot *not, uint8_t channel, Io_enum io, Feature_enum feature, uint8_t message) {
	ApiCmd_ctor(&not->super, NOT, channel, io, feature, NOP);
    not->message = message;
}

/*////////////////////////////////////////////////////////////////////////
////////////                  API GETTERS                   //////////////
////////////////////////////////////////////////////////////////////////*/

void* Api_get_callback(ApiCmd *cmd) {
    return cmd->callback;
}

uint8_t Api_get_cmd_count(ApiCmd *cmd) {
    return cmd->cmd_count;
}

/*////////////////////////////////////////////////////////////////////////
////////////                  API SETTERS                   //////////////
////////////////////////////////////////////////////////////////////////*/
void Api_set_callback(ApiCmd *cmd, void (*callback)(void*, float)) {
    cmd->callback = callback;
}

void Api_set_cmd_count(ApiCmd *cmd, uint8_t cmd_count) {
    cmd->cmd_count = cmd_count;
}



/*////////////////////////////////////////////////////////////////////////
////////////                 API FORMATTERS                 //////////////
////////////////////////////////////////////////////////////////////////*/

//Format an ApiRead object into 3 bytes
void ApiRead_frmtr(ApiRead *cmd, char *formatted) {
    
    //first byte contains type, channel #, and i/o status
    formatted[0] = Api_common_front(&(cmd->super));
    
    //get the feature and parameter numbers in 8bit integer
    uint8_t feat = (int) ((cmd->super).feature);
    uint8_t param = (int) ((cmd->super).param);
    
    //shift the feature to the left by 4 and add param to make 2nd char (byte)
    formatted[1] = (char) ((feat << 4) + param);
    
    //command count is 3rd char (byte)
    formatted[2] = (char) (cmd->super).cmd_count;
}


//Format an ApiWrite object into 7 bytes
void ApiWrite_frmtr(ApiWrite *cmd, char *formatted) {
    //first byte contains type, channel #, and i/o status
    formatted[0] = Api_common_front(&(cmd->super));
    
    //get the feature and parameter numbers in 8bit integer
    uint8_t feat = (int) ((cmd->super).feature);
    uint8_t param = (int) ((cmd->super).param);
    
    //shift the feature to the left by 4 and add param to make 2nd char (byte)
    formatted[1] = (char) ((feat << 4) + param);

    //command count is 3rd char (byte)
    formatted[2] = (char) (cmd->super).cmd_count;
    
    volatile CONVERTER Converter;
    Converter.value = cmd->value;

    //bytes 4-6 contain the value to write (MSB in byte 4, LSB at end of byte 6)
    formatted[3] = Converter.stored[3];
    formatted[4] = Converter.stored[2];
    formatted[5] = Converter.stored[1];
    formatted[6] = Converter.stored[0];
}

//Format an ApiAck object into 6 bytes
void ApiAck_frmtr(ApiAck *cmd, char *formatted) {
    
    //first byte contains type, channel #, and i/o status
    formatted[0] = 0x80;
    
    //command count is 2nd char (byte)
    formatted[1] = (char) (cmd->super).cmd_count;
    
    volatile CONVERTER Converter;
    Converter.value = cmd->value;
    
    //bytes 4-6 contain the value to write (MSB in byte 4, LSB at end of byte 6)
    formatted[2] = Converter.stored[3];
    formatted[3] = Converter.stored[2];
    formatted[4] = Converter.stored[1];
    formatted[5] = Converter.stored[0];
}

//Format an ApiNot object into 2 bytes
void ApiNot_frmtr(ApiNot *cmd, char *formatted) {
    
    //first byte contains type, channel #, and i/o status
    formatted[0] = Api_common_front(&(cmd->super));
    
    //get the feature and parameter numbers in 8bit integer
    uint8_t feat = (int) ((cmd->super).feature);
    uint8_t message = (int) (cmd->message);
    
    //shift the feature to the left by 4 and add param to make 2nd char (byte)
    formatted[1] = (char) ((feat << 4) + message);
}


/*////////////////////////////////////////////////////////////////////////
////////////               API RECONSTRUCTORS               //////////////
////////////////////////////////////////////////////////////////////////*/

//Reconstruct an ApiRead object from 3 bytes
void ApiRead_rector(ApiRead *cmd, char *formatted) {
    //use first char to get channel number and io status with Api_decode_x functions
    //get feature and param from 2nd byte
    Feature_enum feat = (formatted[1] & 0xF0) >> 4;
    Param_enum param  = formatted[1] & 0x0F;
    
    ApiRead_ctor(cmd, Api_decode_channel(formatted[0]), Api_decode_io(formatted[0]), feat, param);
    
    //command count comes from 3rd byte
    (cmd->super).cmd_count = formatted[2];
}

//Reconstruct an ApiWrite object from 7 bytes
void ApiWrite_rector(ApiWrite *cmd, char *formatted) {
    //use first char to get channel number and io status with Api_decode_x functions
    //get feature and param from 2nd byte
    Feature_enum feat = (formatted[1] & 0xF0) >> 4;
    Param_enum param  = formatted[1] & 0x0F;
    
    //bytes 3 through 6 contain the floating point value
    volatile CONVERTER Converter;
    Converter.stored[0] = formatted[6];
    Converter.stored[1] = formatted[5];
    Converter.stored[2] = formatted[4];
    Converter.stored[3] = formatted[3];
    
    ApiWrite_ctor(cmd, Api_decode_channel(formatted[0]), Api_decode_io(formatted[0]), feat, param, Converter.value);
    
    (cmd->super).cmd_count = formatted[2];
}

//Reconstruct an ApiWrite object from 7 bytes
void ApiAck_rector(ApiAck *cmd, char *formatted) {
        
    //bytes 3 through 6 contain the floating point value
    volatile CONVERTER Converter;
    Converter.stored[0] = formatted[5];
    Converter.stored[1] = formatted[4];
    Converter.stored[2] = formatted[3];
    Converter.stored[3] = formatted[2];
    
    ApiAck_ctor(cmd, formatted[1], Converter.value);
}

//Reconstruct an ApiRead object from 3 bytes
void ApiNot_rector(ApiNot *cmd, char *formatted) {
    //use first char to get channel number and io status with Api_decode_x functions
    //get feature and param from 2nd byte
    Feature_enum feat = (formatted[1] & 0xF0) >> 4;
    uint8_t message  = formatted[1] & 0x0F;
    
    ApiNot_ctor(cmd, Api_decode_channel(formatted[0]), Api_decode_io(formatted[0]), feat, message);
}


/*////////////////////////////////////////////////////////////////////////
////////////                 API ENCODERS                   //////////////
////////////////////////////////////////////////////////////////////////*/

//Output a single byte containing Api command type, channel number, and io status
char Api_common_front(ApiCmd *cmd) {
	uint8_t type, chan, io;

    //get type and shift it 6 digits left
	type = ((int) cmd->type) << 6;
    //get channel number, subtract one to force binary and shift left by 2
    chan = (cmd->channel - 1) << 2;
    //shift left io flag. don't care about the 0 carried in.
    io = ((int) cmd->chan_io) << 1;
    //add it all up and return
    return (char) (type+chan+io);
}


/*/////////////////////////////////////////////////////////////////////////
////////////               API BYTE DECODERS                //////////////
////////////////////////////////////////////////////////////////////////*/

//Return channel from Api_common_front style byte
uint8_t Api_decode_channel(char x) {
    return ((x & 0x3C) >> 2) + 1;;
}

//Return message type from Api_common_front style byte
Type_enum Api_decode_type(char x) {
    return (Type_enum) (x & 0xC0) >> 6;
}

//Return io status from Api_common_front style byte
Io_enum Api_decode_io(char x) {
    return (Io_enum) (x & 0x02) >> 1;
}

Feature_enum Api_decode_feature(char x) {
    return (Feature_enum) ((x & 0xF0) >> 4);
}

Param_enum Api_decode_param(char x) {
    return (Param_enum) (x & 0x0F);
}




/*/////////////////////////////////////////////////////////////////////////
////////////               API INSPECTION TOOLS              //////////////
/////////////////////////////////////////////////////////////////////////*/

void ApiCmd_inspect(ApiCmd *cmd) {
    printf("type: %d\n", (int) cmd->type);
    printf("channel: %d\n", cmd->channel);
    printf("channel io:%d \n", (int) cmd->chan_io);
    printf("feature: %d\n", (int) cmd->feature);
    printf("parameter: %d\n", (int) cmd->param);
    printf("cmd_count: %d\n", cmd->cmd_count);
}

void ApiRead_inspect(ApiRead *cmd) {
    ApiCmd_inspect(&cmd->super);
    printf("\n");
}

void ApiWrite_inspect(ApiWrite *cmd) {
    ApiCmd_inspect(&cmd->super);
    printf("value: %lf\n\n", cmd->value);
}

void ApiAck_inspect(ApiAck *cmd) {
    ApiCmd_inspect(&cmd->super);
    printf("value: %lf\n\n", cmd->value);
}

void ApiNot_inspect(ApiNot *cmd) {
    ApiCmd_inspect(&cmd->super);
    printf("message: %d\n\n", cmd->message);
}
