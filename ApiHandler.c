//
//  ApiHandler.c
//  SDtestC
//
//  Created by Matt Webb on 12/23/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>

#include "ApiHandler.h"

//////////////////////////////////////////////////////////////////////////
////////////                HANDLER GETTERS                 //////////////
//////////////////////////////////////////////////////////////////////////

//Return the next available cmd_counter
uint8_t Api_get_cmd_counter(struct ApiHandlerVars* vars) {
    return vars->cmd_counter;
}

//Return the next available cmd_counter & increment counter
uint8_t Api_get_inc_cmd_counter(struct ApiHandlerVars* vars) {
    Api_inc_cmd_counter(vars);
    return ((vars->cmd_counter) - 1);
}

void* Api_get_notif_callback (struct ApiHandlerVars* vars) {
    return vars->notif_callback;
}


//////////////////////////////////////////////////////////////////////////
////////////                HANDLER SETTERS                 //////////////
//////////////////////////////////////////////////////////////////////////

//Set the cmd_counter to a particular integral value
void Api_set_cmd_counter(struct ApiHandlerVars* vars, uint8_t value) {
    vars->cmd_counter = value;
}


//Increment the cmd_counter
void Api_inc_cmd_counter(struct ApiHandlerVars* vars) {
    (vars->cmd_counter)++;
}

//Initialize the handle ApiHandlerVars struct
void Api_init_handler_vars(struct ApiHandlerVars* vars, struct ApiCmdNode* head) {
    vars->cmd_counter = 0;
    vars->reset_counter = 0;
    vars->head = head;
    vars->failed_tx_counter = 0;
}

void Api_register_notif_callback(struct ApiHandlerVars* vars, void (*notif_callback)(ApiNot*)) {
    vars->notif_callback = notif_callback;
}

void Api_inc_failed_tx_counter(struct ApiHandlerVars* vars) {
    (vars->failed_tx_counter++);
}

//////////////////////////////////////////////////////////////////////////
////////////            HANDLER STACK MANAGEMENT            //////////////
//////////////////////////////////////////////////////////////////////////

//push a new value to the front of the 
void Api_tx_stack_push(struct ApiCmdNode** head, 
                                     void* api_ptr, uint8_t cmd_count) {
    struct ApiCmdNode* newNode = malloc(sizeof(struct ApiCmdNode));
    newNode->cmd_count = cmd_count;
    newNode->api_ptr = api_ptr;
    newNode->next = *head;
    *head = newNode;
}

void Api_tx_stack_append(struct ApiCmdNode** head,
                                       void* api_ptr, uint8_t cmd_count) {
    struct ApiCmdNode* current = *head;
    //special case for empty list
    if (current == NULL) {
        Api_tx_stack_push(head, api_ptr, cmd_count);
    } else {
        while (current->next != NULL) {
            current = current->next;
        }
        Api_tx_stack_push(&(current->next), api_ptr, cmd_count);
    }
}


struct ApiCmdNode* Api_tx_stack_delete(struct ApiCmdNode* currP, uint8_t value) {
    /* See if we are at end of list. */
    if (currP == NULL) {
        return NULL;
    } else {
        
        if(currP->cmd_count == value) {
            struct ApiCmdNode *tempNextP;
        
        /* Save the next pointer in the node. */
            tempNextP = currP->next;
        
        /* Deallocate the node. */
            free(currP);
        
        /*
         * Return the NEW pointer to where we
         * were called from.  I.e., the pointer
         * the previous call will use to "skip
         * over" the removed node.
         */
            return tempNextP;
        } else {
    
    /*
     * Check the rest of the list, fixing the next
     * pointer in case the next node is the one
     * removed.
     */
            currP->next = Api_tx_stack_delete(currP->next, value);
    
    
    /*
     * Return the pointer to where we were called
     * from.  Since we did not remove this node it
     * will be the same.
     */
            return currP;
        }
    }
}



//locate a command node with a specified cmd_count
//returns a pointer to the located node if found
//upon no match, returns NULL;
struct ApiCmdNode* Api_tx_stack_locate(struct ApiCmdNode** head, uint8_t cmd_count) {
    struct ApiCmdNode* current = *head;

    //loop over all nodes
    while(current != NULL) {
        //if the command counts don't match, point to next node
        if (current->cmd_count != cmd_count) {
            current = current->next;
        } else { //found a match.  return pointer to matching node
            return current;
        }
    }
    
    //case where the whole list is iterated over and no result is found
    return NULL;
}


//locate an api cmd with a specified cmd_count
//returns a pointer to the api cmd if found
//upon no match, returns NULL;
void* Api_tx_stack_locate_api_ptr(struct ApiCmdNode** head, uint8_t cmd_count) {
    struct ApiCmdNode* node = Api_tx_stack_locate(head, cmd_count);
    if(node != NULL) {
        return node->api_ptr;
    } else {
        return NULL;
    }

}


//calculate the length of the stack
//returns the length as a 16-bit unsigned integer (uint16_t)
uint16_t Api_tx_stack_length(struct ApiCmdNode* head) {
    uint16_t count = 0;
    struct ApiCmdNode* current = head;
    while( current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}



//////////////////////////////////////////////////////////////////////////
////////////               HANDLER RECEIVERS                //////////////
//////////////////////////////////////////////////////////////////////////


uint8_t Api_rx_all(char* chars){ //, struct ApiHandlerVars* vars) {
    struct ApiHandlerVars* vars = global_api_handler(0);
    char rx_chars[8];

    rx_chars[0] = chars[0];
    rx_chars[1] = chars[1];
    rx_chars[2] = chars[2];
    rx_chars[3] = chars[3];
    rx_chars[4] = chars[4];
    rx_chars[5] = chars[5];
    rx_chars[6] = chars[6];
    rx_chars[7] = 0;
    
    Type_enum type = Api_decode_type(rx_chars[0]);
    
    switch (type) {
        case READ: {
            ApiRead read;
            ApiRead_rector(&read, rx_chars);
            
            float return_value = dsp_read_value(&read);
            
            ApiAck ack;
            ApiAck_ctor(&ack, Api_get_cmd_count(&(read.super)) , return_value);
            Api_tx_all(&ack);
            break;   
        }
            
        case WRITE: {
            float return_value;
            ApiWrite write;
            ApiWrite_rector(&write, rx_chars);
            if(dsp_write_value(&write)) {
                return_value = write.value;
            } else {
                return_value = 99999.99;
            }
            
            ApiAck ack;
            ApiAck_ctor(&ack, Api_get_cmd_count(&(write.super)) , return_value);
            Api_tx_all(&ack);
            break;
        }
            
        case NOT: {
            ApiNot *notif = malloc(sizeof *notif);
            ApiNot_rector(notif, rx_chars);
            
            //get the registered notification callback function, then call it
            void (*notif_callback)(ApiNot*) = Api_get_notif_callback(vars);
            if(notif_callback != NULL) {
                (*notif_callback)(notif);
            } else {
                printf("whoops!  no callback set!");
                return 0;
            }
            break;
        }
            
        case ACK: {
            ApiAck ack;
            ApiAck_rector(&ack, rx_chars);
            void* api_ptr = Api_tx_stack_locate_api_ptr(&(vars->head), Api_get_cmd_count(&(ack.super)));
            Type_enum* api_cmd_type = (Type_enum*) api_ptr;
            
            if ( *api_cmd_type == READ ) {
                ApiRead* read_match = (ApiRead*) api_ptr;
                void (*callback)(void*, float) = Api_get_callback(&(read_match->super));
                //remove the read from the tx stack
                Api_tx_stack_delete(vars->head, Api_get_cmd_count(&(read_match->super)));
                (*callback)(read_match, ack.value);
                //TODO where do I free it?
                free(read_match);
                
            } else if ( *api_cmd_type == WRITE ) {
                ApiWrite* write_match = (ApiWrite*) api_ptr;
                void (*callback)(void*, float) = Api_get_callback(&(write_match->super));
                if ( (write_match->value) == (ack.value) ) {
                    //remove the write from the tx stack
                    Api_tx_stack_delete(vars->head, Api_get_cmd_count(&(write_match->super)));
                    
                    //call the callback!  Set float param to 1 for success
                    (*callback)(write_match, 1);
                    //free it because we're done with it.  TODO: is this the right place to free?
                    //free(write_match);
                } else {
                    //oh dear.  Command didn't write correctly to slave
                    //So, transmit again.  TODO:  Can I recursively call this OK?
                    //TODO:  does anything else need to be free'd here?
                    //let's just retry 5 times. If still failing after 5, increment our tx failure counter
                    if((write_match->super.retry_count) < 5) {
                        Api_inc_retry_count(&(write_match->super));
                        Api_tx_all(api_ptr);//, callback); TODO: rewrite callback
                    } else {
                        Api_inc_failed_tx_counter(vars);
                    }

                }
                
            } else {
                //crap.  no match to a type that matters  TODO
            }

            break;
        }
            
        default:
            break;
    }
    
    return 1;
}


/////////////////////////////////////////////////////////////////////////
////////////             HANDLER TRANSMITTERS               //////////////
//////////////////////////////////////////////////////////////////////////

uint8_t Api_tx_all(void* api_ptr) { //, struct ApiHandlerVars* vars){//, void(*callback)(void*, float)) {
    struct ApiHandlerVars* vars = global_api_handler(0);
    
    //First, need to identify the type of api command being passed in
    Type_enum* api_type = (Type_enum*) api_ptr;
    
    //create a variable to place the formatted api_cmd type in
    char formatted[8] = {0,0,0,0,0,0,0,0};
    
    switch (*api_type) {
        case READ: {
            //recover the ApiRead object from a pointer
            ApiRead* read = (ApiRead*) api_ptr;
            //setup the function to be called when response is received
         //Api_set_callback(&(read->super), callback);
            //Set the cmd_count var & increment it
            Api_set_cmd_count(&(read->super), Api_get_inc_cmd_counter(vars));
            //convert ApiRead object to char array for transmission
            ApiRead_frmtr(read, formatted);
            
            //push transmitted api cmd onto stack
            Api_tx_stack_push(&(vars->head), read, (read->super).cmd_count);
            
            //check for successful transmission (if spi_transmit=1)
            if( !uart_transmit(formatted, 8) ) {  
                //Condition where TX is NOT successful  TODO
            }
            
            break;        
        }
            
        case WRITE: {
            ApiWrite* write = (ApiWrite*) api_ptr;
        //Api_set_callback(&(write->super), callback);
            //Set the cmd_count var & increment it
            Api_set_cmd_count(&(write->super), Api_get_inc_cmd_counter(vars));
            ApiWrite_frmtr(write, formatted);
            
            //push transmitted api cmd onto stack
            Api_tx_stack_push(&(vars->head), write, (write->super).cmd_count);
            
            //check for successful transmission (if spi_transmit=1)
            if( !uart_transmit(formatted, 8) ) { 
                //Condition where TX is NOT successful TODO
            }

            break;
        }
        case ACK: {
            ApiAck *ackn = (ApiAck*) api_ptr;
            ApiAck_frmtr(ackn, formatted);
            //check for successful transmission (if spi_transmit=1)
            if( !uart_transmit(formatted, 8) ) { 
                //Condition where TX is NOT successful  TODO
            }

            break;
        }
            
        case NOT: {
            ApiNot *notif = (ApiNot*) api_ptr;
            ApiNot_frmtr(notif, formatted);
            //check for successful transmission (if spi_transmit=1)
            if( !uart_transmit(formatted, 8) ) { 
                //Condition where TX is NOT successful  TODO
            }
            //callback? TODO
            break;
        }
            
        default: {
            return 0;
        }
    }
    return 1;
}

uint8_t Api_set_eqband_type(Channel* chan, uint8_t bandNum, Eq_type_enum type) {
	eqband_set_type(channel_get_eqband(chan, bandNum), type);
	ApiWrite* cmd = malloc(sizeof(ApiWrite));
	Feature_enum feature =  (Feature_enum)bandNum;
	
	ApiWrite_ctor(cmd,  chan->chan_num, chan->io, feature, TYPE, type); 
	Api_set_callback(cmd, callbackFunction);	
	Api_add_cmd_to_cb((void*)cmd);
	return 1;
}

uint8_t Api_set_eqband_bw(Channel* chan, uint8_t bandNum, float q) {
	eqband_set_bw(channel_get_eqband(chan, bandNum), q);
	ApiWrite* cmd = malloc(sizeof(ApiWrite));
	Feature_enum feature =  (Feature_enum)bandNum;
	
	ApiWrite_ctor(cmd, chan->chan_num, chan->io, feature, BW, q); 
	Api_set_callback(cmd, callbackFunction);	
	Api_add_cmd_to_cb((void*)cmd);
	return 1;
}

uint8_t Api_set_eqband_freq(Channel* chan, uint8_t bandNum, float freq) {
	eqband_set_freq(channel_get_eqband(chan, bandNum), freq);
	ApiWrite* cmd = malloc(sizeof(ApiWrite));
	Feature_enum feature =  (Feature_enum)bandNum;
	
	ApiWrite_ctor(cmd, chan->chan_num, chan->io, feature, FREQ, freq); 
	Api_set_callback(cmd, callbackFunction);	
	Api_add_cmd_to_cb((void*)cmd);
	return 1;
}

uint8_t Api_set_eqband_gain(Channel* chan, uint8_t bandNum, float gain) {
	eqband_set_gain(channel_get_eqband(chan, bandNum), gain);
	ApiWrite* cmd = malloc(sizeof(ApiWrite));
	Feature_enum feature =  (Feature_enum)bandNum;
	
	ApiWrite_ctor(cmd, chan->chan_num, chan->io, feature, GAIN, gain); 
	Api_set_callback(cmd, callbackFunction);	
	Api_add_cmd_to_cb((void*)cmd);
	return 1;
}

uint8_t Api_enable_eqband(Channel* chan, uint8_t bandNum) {
	eqband_enable(channel_get_eqband(chan, bandNum));
	ApiWrite* cmd = malloc(sizeof(ApiWrite));
	Feature_enum feature =  (Feature_enum)bandNum;
	
	ApiWrite_ctor(cmd,  chan->chan_num, chan->io, feature, EN, ENABLED); 
	Api_set_callback(cmd, callbackFunction);	
	Api_add_cmd_to_cb((void*)cmd);	
	return 1;
}
uint8_t Api_disable_eqband(Channel* chan, uint8_t bandNum) {
	eqband_disable(channel_get_eqband(chan, bandNum));
	ApiWrite* cmd = malloc(sizeof(ApiWrite));
	Feature_enum feature =  (Feature_enum)bandNum;
	
	ApiWrite_ctor(cmd,  chan->chan_num, chan->io, feature, EN, DISABLED); 
	Api_set_callback(cmd, callbackFunction);
	Api_add_cmd_to_cb((void*)cmd);
	return 1;
}

uint8_t Api_add_cmd_to_cb(void* api_ptr) {
	struct ApiHandlerVars* handler = global_api_handler(0);
	ElemType elem;
    elem.value = api_ptr;

    cbWrite(&(handler->tx_buffer), &elem);
    return 1;
}
