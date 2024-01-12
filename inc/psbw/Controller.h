#pragma once

#include <stdint.h>
typedef uint8_t ControllerPort;

#define CONTROLLER_PORT_1 0
#define CONTROLLER_PORT_2 1


/**
 * \class ControllerButton
 * \brief Use this struct to select a controller button
 */
enum ControllerButton {
    Select =            0x1,            // Bit 0
    L3 =                0x1 << 1,       // Bit 1
    R3 =                0x1 << 2,       // Bit 2
    Start =             0x1 << 3,       // Bit 3
    Up =                0x1 << 4,       // Bit 4
    Right =             0x1 << 5,       // Bit 5
    Down =              0x1 << 6,       // Bit 6
    Left =              0x1 << 7,       // Bit 7
    L2 =                0x1 << 8,       // Bit 8
    R2 =                0x1 << 9,       // Bit 9
    L1 =                0x1 << 10,      // Bit 10
    R1 =                0x1 << 11,      // Bit 11
    Triangle =          0x1 << 12,      // Bit 12
    Circle =            0x1 << 13,      // Bit 13
    X =                 0x1 << 14,      // Bit 14
    Square =            0x1 << 15       // Bit 15
};

// Initialize SIO0 for controllers. DO NOT RUN FROM GAME CODE. Managed by engine.
void ctrl_init();


/**
 * \class Controller
 * \brief This class allows you to access input data from a Controller of your choice
 *
 *
 */
class Controller {
    public:
        ControllerPort port;
        
        // Initialize controller with selected port
        Controller(ControllerPort controllerPort);

/**
 * \brief Returns 1 if the selected button is pressed. 0 if otherwise
 */        
int8_t GetButton(ControllerButton button);

};