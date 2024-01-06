
// For now only baudrates that are multiples of 16 are supported due to internal timer settings
#define SIO_BAUD_115200 115200
#define SIO_BAUD_9600   9600

// Initializes serial communication with the given baudrate. At this point only 8 data bits and 1 stop bit is supported
void sio_init(int baudrate);

// Prints a string of characters to the serial
void sio_print(const char* str);