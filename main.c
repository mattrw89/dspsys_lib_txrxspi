//
//  main.c
//  SDtestC
//
//  Created by Matt Webb on 11/23/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h> //not needed in CCS


#include <stdint.h>
#include "APICommand.h"
#include "channel.h"
#include "ApiHandler.h"

void notif_callback(ApiNot* notif);

void callbackFunction(void* api_ptr, float b) {
    ApiRead* a = (ApiRead*) api_ptr;
    printf("in callback: b=%lf , cmd_count:%d\n\n", b, (a->super).cmd_count);
}

void notif_callback(ApiNot* notif) {
    printf("\n\nHey!  this is my notifications callback\n");
    printf("value of message: %d\n\n", notif->message);
    free(notif);
}


int main (int argc, const char * argv[])
{
    ApiRead  r1, r2;
    ApiWrite w1, w2;
    ApiAck   a1, a2;
    ApiNot   n1, n2;
    

    //TEST API READ CONSTRUCTOR AND RE-CONSTRUCTOR
    ApiRead_ctor(&r1, 1, OUTPUT, COMP, THRESHOLD);
    char read_format[3];
    char read_format2[3];
    
    printf("ApiRead Constructor Results:\n");    
    ApiRead_inspect(&r1);
    ApiRead_frmtr(&r1, read_format);

    printf("%X %X %X\n\n", read_format[0], read_format[1], read_format[2]);
    printf("Reconstructed\n");
    
    ApiRead_rector(&r2, read_format);
    ApiRead_frmtr(&r2, read_format2);
    ApiRead_inspect(&r2);
    
    printf("%X %X %X\n\n", read_format2[0], read_format2[1], read_format2[2]);
    
    
    //TEST API WRITE CONSTRUCTOR AND RE-CONSTRUCTOR
    ApiWrite_ctor(&w1, 3, OUTPUT, COMP, THRESHOLD, 7567.557);
    char write_format[7];
    char write_format2[7];

    
    printf("ApiWrite Constructor Results:\n");
    ApiWrite_inspect(&w1);
    ApiWrite_frmtr(&w1, write_format);

    printf("%X %X %X\n", write_format[0], write_format[1], write_format[2]);
    printf("value float to 4 bytes: %X %X %X %X\n\n", 
           write_format[3] & 0x000000FF, write_format[4] & 0x000000FF, 
           write_format[5] & 0x000000FF, write_format[6] & 0x000000FF);
    
    printf("Reconstructed\n");
    ApiWrite_rector(&w2, write_format);
    ApiWrite_frmtr(&w2, write_format2);
    ApiWrite_inspect(&w2);
    
    printf("%X %X %X\n", write_format2[0], write_format2[1], write_format2[2]);
    printf("value float to 4 bytes: %X %X %X %X\n\n", 
           write_format2[3] & 0x000000FF, write_format2[4] & 0x000000FF, 
           write_format2[5] & 0x000000FF, write_format2[6] & 0x000000FF);
    
    Type_enum *recovered_type = (Type_enum *) &w1;
    
    printf("\n\nrecovered type: %d\n\n", *recovered_type);
    
    
    //TEST API ACKNOWLEDGE CONSTRUCTOR AND RE-RECONSTRUCTOR
    ApiAck_ctor(&a1, 99, 7567.557);
    char ack_format[6];
    char ack_format2[6];

    printf("ApiAck Constructor Results:\n");
    ApiAck_inspect(&a1);
    ApiAck_frmtr(&a1, ack_format);
    
    printf("%X %X\n", ack_format[0], ack_format[1]);
    printf("value float to 4 bytes: %X %X %X %X\n\n", 
           ack_format[2] & 0x000000FF, ack_format[3] & 0x000000FF, 
           ack_format[4] & 0x000000FF, ack_format[5] & 0x000000FF);
    
    printf("Reconstructed\n");
    ApiAck_rector(&a2, ack_format);
    ApiAck_frmtr(&a2, ack_format2);
    ApiAck_inspect(&a2);
    
    printf("%X %X\n", ack_format2[0], ack_format2[1]);
    printf("value float to 4 bytes: %X %X %X %X\n\n", 
           ack_format2[2] & 0x000000FF, ack_format2[3] & 0x000000FF, 
           ack_format2[4] & 0x000000FF, ack_format2[5] & 0x000000FF); 
    
    Type_enum *recovered_type2 = (Type_enum *) &a1;
    printf("\n\nrecovered type: %d\n\n", *recovered_type2);
    
    //TEST API NOTIFICATION CONSTRUCTOR AND RE-CONSTRUCTOR
    ApiNot_ctor(&n1, 1, OUTPUT, COMP, 4);
    char not_format[2];
    char not_format2[2];

    printf("ApiNot Constructor Results:\n");
    ApiNot_inspect(&n1);    
    ApiNot_frmtr(&n1, not_format);

    printf("%X %X\n\n", not_format[0] & 0x000000FF, not_format[1] & 0x000000FF);
    printf("Reconstructed\n");
    
    ApiNot_rector(&n2, not_format);
    ApiNot_frmtr(&n2, not_format2);
    ApiNot_inspect(&n2);
    printf("%X %X\n\n", not_format2[0] & 0x000000FF, not_format2[1] & 0x000000FF);
    
    Type_enum *recovered_type3 = (Type_enum *) &n1;
    printf("\n\nrecovered type: %d\n\n", *recovered_type3);
    
    //END API TESTING
    
    //START CHANNEL TESTING
    printf("All API Command Tests Pass!\n");
    
    printf("Start Channel Testing\n\n");
    
    Channel ch1;

    Channel_ctor(&ch1, 1, "Ch1", ACTIVE, INPUT);
    
    //get_name test
    char* ptr = channel_get_name(&ch1);
    printf("ch1 name:  %s\n\n", ptr);
    
    //set_name test
    channel_set_name(&ch1, "CH1!!");
    
    char* ptr2 = channel_get_name(&ch1);
    printf("ch1 name:  %s\n\n", ptr2);
    
    channel_inspect_full(&ch1);
    
    //test interaction with eqband
    EqBand* eq = channel_get_eqband(&ch1, 2);
    eqband_enable(eq);
    eqband_set_type(eq, HPF);
    eqband_set_band_num(eq, 5);
    eqband_set_bw(eq, 10.0);
    eqband_set_freq(eq, 1357);
    eqband_set_gain(eq, 4.0);
    
    eqband_inspect(channel_get_eqband(&ch1, 2));
    
    Enable_enum en = eqband_is_enabled(eq);
    if(en) printf("enum values usable as boolean logic\n");
    
    //test setting parameters in compressor
    Comp *comp = channel_get_comp(&ch1);
    comp_enable(comp);
    comp_set_threshold(comp, 10.0);
    comp_set_ratio(comp, 1.5);
    comp_set_attack(comp, 10);
    comp_set_release(comp, 500);
    comp_set_gain(comp, 2.0);
    
    comp_inspect(comp);
    
    //Now let's use malloc to allocate memory for a channel
    
    Channel *ch2 = malloc(sizeof *ch2);
    Channel_ctor(ch2, 2, "Ch2", ACTIVE, INPUT);
    channel_inspect_basic(ch2);
    
    free(ch2);
    
    Channel *ch01 = malloc(sizeof *ch01);
    Channel *ch02 = malloc(sizeof *ch02);
    Channel *ch03 = malloc(sizeof *ch03);
    Channel *ch04 = malloc(sizeof *ch04);
    Channel *ch05 = malloc(sizeof *ch05);
    Channel *ch06 = malloc(sizeof *ch06);
    Channel *ch07 = malloc(sizeof *ch07);
    Channel *ch08 = malloc(sizeof *ch08);
    Channel *ch09 = malloc(sizeof *ch09);
    Channel *ch10 = malloc(sizeof *ch10);
    Channel *ch11 = malloc(sizeof *ch11);
    Channel *ch12 = malloc(sizeof *ch12);
    Channel *ch13 = malloc(sizeof *ch13);
    Channel *ch14 = malloc(sizeof *ch14);
    
    Channel* ch_array[14] = {ch01, ch02, ch03, ch04, ch05, ch06, ch07, ch08, ch09, ch10, ch11, ch12, ch13, ch14};
    
    Channel_ctor(ch02, 2, "Ch2!", ACTIVE, INPUT); 
    
    channel_inspect_basic(ch_array[1]);
    channel_inspect_basic(ch02);
    
    for(int i = 0; i < 13; i++) {
        free (ch_array[i]);
    }
    
    printf("size of ch_array: %lu bytes\n", sizeof(ch_array));
    printf("size of a channel: %lu bytes\n", sizeof(Channel));
    printf("size of 14 channels: %lu bytes\n", sizeof(Channel) * 14);
    
    
    //TEST TRANSMISSION & RECEPTION HANDLERS
    printf("\n\nTx&Rx Handler Testing\n\n");
    
    //initialize the handler variables
    struct ApiHandlerVars handler_vars;
    struct ApiCmdNode* tx2_stack = malloc(sizeof(struct ApiCmdNode));
    
    Api_init_handler_vars(&handler_vars, tx2_stack);
    
    handler_vars.cmd_counter = 17;
    
    //let's transmit the read_1 api command
    //have it call callbackFunction upon reception of the corresponding value
    
    Api_tx_all(&r1, &handler_vars, callbackFunction);
    
    
    //Let's transmit a Notification
    
    Api_register_notif_callback(&handler_vars, notif_callback);
    Api_tx_all(&n1, &handler_vars, NULL);
    
    //Let's transmit a Write
    Api_tx_all(&w1, &handler_vars, callbackFunction);
    
    
    //Now, Let's test out that stack list
    struct ApiCmdNode* tx_stack = malloc(sizeof(struct ApiCmdNode));
    tx_stack->cmd_count = 5;
    tx_stack->next = NULL;
    
    Api_tx_stack_push(&tx_stack, &r1, r1.super.cmd_count);
    Api_tx_stack_push(&tx_stack, &a1, a1.super.cmd_count);
    Api_tx_stack_append(&tx_stack, &w1, w1.super.cmd_count);
    
    printf("cmd counts: %d %d %d\n", r2.super.cmd_count, a1.super.cmd_count, w1.super.cmd_count);
    printf("length of stack: %d\n\n", Api_tx_stack_length(tx_stack));
    printf("next: %d %d %d\n", (int) tx_stack->next, a1.super.cmd_count, w1.super.cmd_count);
    
    struct ApiCmdNode* match = Api_tx_stack_locate(&tx_stack, r1.super.cmd_count);
    
    if (match != NULL) {
        ApiAck* matching = (ApiAck*) match->api_ptr;
        printf("\nmatching list type: %d\n", (Type_enum) ((matching->super).type));
    } else {
        printf("\nNo match found\n");
    }
    

    tx_stack = Api_tx_stack_delete(tx_stack, w1.super.cmd_count);
    printf("length of stack: %d\n\n", Api_tx_stack_length(tx_stack));
    
    printf("All done!");

    return 0;
}
