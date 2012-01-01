//
//  ApiHandler.c
//  SDtestC
//
//  Created by Matt Webb on 12/23/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>

#include "ApiHandler.h"
#include "APICommand.h"

/*////////////////////////////////////////////////////////////////////////
////////////                HANDLER GETTERS                 //////////////
////////////////////////////////////////////////////////////////////////*/

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


/*////////////////////////////////////////////////////////////////////////
////////////                HANDLER SETTERS                 //////////////
////////////////////////////////////////////////////////////////////////*/

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
}

void Api_register_notif_callback(struct ApiHandlerVars* vars, void (*notif_callback)(ApiNot*)) {
    vars->notif_callback = notif_callback;
}

/*////////////////////////////////////////////////////////////////////////
////////////            HANDLER STACK MANAGEMENT            //////////////
////////////////////////////////////////////////////////////////////////*/

//push a new value to the front of the 
void Api_tx_stack_push(struct ApiCmdNode** head, 
                                     void* api_ptr, uint8_t cmd_count) {
    struct ApiCmdNode* newNode = malloc(sizeof(struct ApiCmdNode));
    newNode->cmd_count = cmd_count;
    newNode->api_ptr = api_ptr;
    newNode->next = *head;
    newNode->previous = NULL;
    *head = newNode;
    struct ApiCmdNode* oldHead = (struct ApiCmdNode*)&head;
    oldHead->previous = newNode;
    //set previous for old head! 
}

//
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

//Remove an api cmd from the stack
//Returns 1 for success, 0 for deletion failure
uint8_t Api_tx_stack_delete(struct ApiCmdNode** head, uint8_t cmd_count) {
    //locate the node that needs to be deleted
    struct ApiCmdNode* removalNode =  Api_tx_stack_locate(head, cmd_count);
    
    //check if removalNode was even found
    if (removalNode != NULL) {
        //change previous & next pointers
        struct ApiCmdNode* previousNode = (struct ApiCmdNode*) removalNode->previous;
        struct ApiCmdNode* nextNode = (struct ApiCmdNode*) removalNode->next;
        
        previousNode->next = nextNode;
        nextNode->previous = previousNode;
        
        //free node that it being removed from memory
        free(removalNode);
        return 1;
    } else {
        return 0;
    }
    

}


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

void* Api_tx_stack_locate_api_ptr(struct ApiCmdNode** head, uint8_t cmd_count) {
    struct ApiCmdNode* node = Api_tx_stack_locate(head, cmd_count);
    return node->api_ptr;
}


uint16_t Api_tx_stack_length(struct ApiCmdNode* head) {
    uint16_t count = 0;
    struct ApiCmdNode* current = head;
    while( current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}



/*////////////////////////////////////////////////////////////////////////
////////////               HANDLER RECEIVERS                //////////////
////////////////////////////////////////////////////////////////////////*/


uint8_t Api_rx_all(char* chars, struct ApiHandlerVars* vars) {
    char rx_chars[7];

    rx_chars[0] = chars[0];
    rx_chars[1] = chars[1];
    rx_chars[2] = chars[2];
    rx_chars[3] = chars[3];
    rx_chars[4] = chars[4];
    rx_chars[5] = chars[5];
    rx_chars[6] = chars[6];
    
    Type_enum type = Api_decode_type(rx_chars[0]);
    
    switch (type) {
        case READ: {
            ApiRead read;
            ApiRead_rector(&read, rx_chars);
            float return_value = 10000.01;  //(float) dsp_get_value(1, INPUT, EQB1, FREQ);
            
            ApiAck ack;
            ApiAck_ctor(&ack, read.super.cmd_count , return_value);
            Api_tx_all(&ack, vars, NULL);
            break;   
        }
            
        case WRITE: {
            ApiWrite write;
            ApiWrite_rector(&write, rx_chars);
            
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
            void* api_ptr = Api_tx_stack_locate_api_ptr(&(vars->head), ack.super.cmd_count);
            Type_enum* api_cmd_type = (Type_enum*) api_ptr;
            
            if ( *api_cmd_type == READ ) {
                ApiRead* read_match = (ApiRead*) api_ptr;
                void (*callback)(void*, float) = Api_get_callback(&(read_match->super));
                (*callback)(read_match, ack.value);
                //TODO where do I free it?
                //free(read_match);
                
            } else if ( *api_cmd_type == WRITE ) {
                ApiWrite* write_match = (ApiWrite*) api_ptr;
                void (*callback)(void*, float) = Api_get_callback(&(write_match->super));
                if ( (write_match->value) == (ack.value) ) {
                    //call the callback!  Set float param to 1 for success
                    (*callback)(write_match, 1);
                    //free it because we're done with it.  TODO: is this the right place to free?
                    free(write_match);
                } else {
                    //oh dear.  Command didn't write correctly to slave
                    //So, transmit again.  TODO:  Can I recursively call this OK?
                    //TODO:  does anything else need to be free'd here?
                    Api_tx_all(api_ptr, vars, callback);
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


/*////////////////////////////////////////////////////////////////////////
////////////             HANDLER TRANSMITTERS               //////////////
////////////////////////////////////////////////////////////////////////*/

uint8_t Api_tx_all(void* api_ptr, struct ApiHandlerVars* vars, void(*callback)(void*, float)) {
    
    //First, need to identify the type of api command being passed in
    Type_enum* api_type = (Type_enum*) api_ptr;
    
    //create a variable to place the formatted api_cmd type in
    char formatted[7];
    
    switch (*api_type) {
        case READ: {
            //recover the ApiRead object from a pointer
            ApiRead* read = (ApiRead*) api_ptr;
            //setup the function to be called when response is received
            Api_set_callback(&(read->super), callback);
            //Set the cmd_count var & increment it
            Api_set_cmd_count(&(read->super), Api_get_inc_cmd_counter(vars));
            //convert ApiRead object to char array for transmission
            ApiRead_frmtr(read, formatted);
            
            //push transmitted api cmd onto stack
            Api_tx_stack_push(&(vars->head), read, (read->super).cmd_count);
            
            //check for successful transmission (if spi_transmit=1)
            if( !(spi_transmit(formatted, vars)) ) {  
                //Condition where TX is NOT successful  TODO
            }
            
            break;        
        }
            
        case WRITE: {
            ApiWrite* write = (ApiWrite*) api_ptr;
            Api_set_callback(&(write->super), callback);
            //Set the cmd_count var & increment it
            Api_set_cmd_count(&(write->super), Api_get_inc_cmd_counter(vars));
            ApiWrite_frmtr(write, formatted);
            
            //push transmitted api cmd onto stack
            Api_tx_stack_push(&(vars->head), write, (write->super).cmd_count);
            
            //check for successful transmission (if spi_transmit=1)
            if( !(spi_transmit(formatted, vars)) ) { 
                //Condition where TX is NOT successful TODO
            }

            break;
        }
        case ACK: {
            ApiAck *ackn = (ApiAck*) api_ptr;
            ApiAck_frmtr(ackn, formatted);
            //check for successful transmission (if spi_transmit=1)
            if( !(spi_transmit(formatted, vars)) ) { 
                //Condition where TX is NOT successful  TODO
            }

            break;
        }
            
        case NOT: {
            ApiNot *notif = (ApiNot*) api_ptr;
            ApiNot_frmtr(notif, formatted);
            //check for successful transmission (if spi_transmit=1)
            if( !(spi_transmit(formatted, vars)) ) { 
                //Condition where TX is NOT successful  TODO
            }
            //callback? TODO
            break;
        }
            
        default: {
            return 0;
            break;
        }
    }
    return 1;
}

uint8_t spi_transmit(char* formatted, struct ApiHandlerVars* vars) {
    //FOR TESTING PURPOSES ONLY
    Api_rx_all(formatted, vars);
    
    return 1;
}

