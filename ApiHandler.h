//
//  ApiHandler.h
//  SDtestC
//
//  Created by Matt Webb on 12/23/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef SDtestC_ApiHandler_h
#define SDtestC_ApiHandler_h

#include "../dspsys_lib_channel/channel.h"
#include "APICommand.h"
#include "../dspsys_lib_channel/common.h"
#include "../dspsys_lib_txrxspi/dsp_settings.h"
#include <stdint.h>
#include <stdlib.h>

struct ApiCmdNode {
    void* api_ptr;
    uint8_t cmd_count;
    struct ApiCmdNode* next;
};

struct ApiHandlerVars {
    uint8_t cmd_counter;
    uint16_t reset_counter;
    void (*notif_callback) (ApiNot*);  //Pointer to function to handle notification callbacks
    struct ApiCmdNode *head;
    uint16_t failed_tx_counter;
};




//Getters

uint8_t Api_get_cmd_counter(struct ApiHandlerVars* vars);
uint8_t Api_get_inc_cmd_counter(struct ApiHandlerVars* vars);
void* Api_get_notif_callback (struct ApiHandlerVars* vars);

//Setters
void Api_set_cmd_counter(struct ApiHandlerVars* vars, uint8_t value);
void Api_inc_cmd_counter(struct ApiHandlerVars* vars);
void Api_init_handler_vars(struct ApiHandlerVars* vars, struct ApiCmdNode* head); 
void Api_register_notif_callback(struct ApiHandlerVars* vars, void (*notif_callback)(ApiNot*));
void Api_inc_failed_tx_counter(struct ApiHandlerVars* vars);


//Api Command Stack Management Functions

void Api_tx_stack_push(struct ApiCmdNode** head, void* api_ptr, uint8_t cmd_count);
void Api_tx_stack_append(struct ApiCmdNode** head, void* api_ptr, uint8_t cmd_count);
struct ApiCmdNode* Api_tx_stack_delete(struct ApiCmdNode* currP, uint8_t value);
struct ApiCmdNode* Api_tx_stack_locate(struct ApiCmdNode** head, uint8_t cmd_count);
void* Api_tx_stack_locate_api_ptr(struct ApiCmdNode** head, uint8_t cmd_count);
uint16_t Api_tx_stack_length(struct ApiCmdNode* head);


//Receive Functions

//Take in up to 8 bytes from SPI RX buffer
//Construct the appropriate ApiCmd type (Read/Write/Not/Ack)
//Check to ensure that it is the right number of bytes in length according to type
//Check that all values are within bounds
//
uint8_t Api_rx_all(char* chars, struct ApiHandlerVars* vars);



//Transmit Functions

//Takes in an ApiCmd based object & function pointer (callback)
//output 1 (transmit done) or 0 (transmit failed)
uint8_t Api_tx_all(void* api_ptr, struct ApiHandlerVars* vars, void(*callback)(void*, float));

uint8_t spi_transmit(char* formatted, struct ApiHandlerVars* vars);


void callbackFunction(void* aa, float bb);  // can be deleted after testing (used in main.c)

#endif
