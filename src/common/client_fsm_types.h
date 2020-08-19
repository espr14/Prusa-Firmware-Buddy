#pragma once

#include <stdint.h>

#ifdef __cplusplus
//C++ checks enum classes

//Client finite state machines
enum class ClientFSM : uint8_t {
    Serial_printing,
    Load_unload,
    G162,
    Printing,
    FirstLayer,
    _none, //cannot be created, must have the same index as _count
    _count = _none
};

enum class LoadUnloadMode : uint8_t {
    Change,
    Load,
    Unload,
    Purge
};

//open dialog has parameter
//because I need to set caption of change filament dialog (load / unload / change)
//use extra state of statemachine to set caption would be cleaner, but I can miss events
//only last sent event is guaranteed to pass its data
using fsm_create_t = void (*)(ClientFSM, uint8_t);                                               //create finite state machine
using fsm_destroy_t = void (*)(ClientFSM);                                                       //destroy finite state machine
using fsm_change_t = void (*)(ClientFSM, uint8_t phase, uint8_t progress_tot, uint8_t progress); //change fsm state or progress

#else  // !__cplusplus
//C
typedef void (*fsm_create_t)(uint8_t, uint8_t);                                               //create finite state machine
typedef void (*fsm_destroy_t)(uint8_t);                                                       //destroy finite state machine
typedef void (*fsm_change_t)(uint8_t, uint8_t phase, uint8_t progress_tot, uint8_t progress); //change fsm state or progress
#endif //__cplusplus
