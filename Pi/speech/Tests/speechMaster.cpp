#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#define I2C_SPEECH 0x04

int main()
{
	char rxBuffer[32];	//	receive buffer
	char txBuffer[] = {0x1B, 0x02, 0x23, 0x18, 0x23, 0x16, 0x37,0x3E,0x0E,0x0E,0x2B,0x15,0x00,0x09,0x29,0x0B,0x09,0x0D,0x3F,0x3E,0x2F,0x00,0x0D,0x1E,0x3F,0x3E,0x19,0x2B,0x09,0x1F,0x2A,0x2C,0x0D,0x15,0x3F,0xFF};	//	transmit buffer
	int speechAddress = I2C_SPEECH;  //speech devices address
	int tenBitAddress = 0;
	int opResult = 0;

	// Create a file descriptor for the I2C bus
	int i2cHandle = open("/dev/i2c-1", O_RDWR); 

	// I2C device is not 10-bit
	opResult = ioctl(i2cHandle, I2C_TENBIT, tenBitAddress);
	
	// set address of speech board to I2C
	opResult = ioctl(i2cHandle, I2C_SLAVE, speechAddress);
	
	//Clear out the buffers
	memset(rxBuffer, 0, sizeof(rxBuffer));

	for(unsigned int i = 0; i < sizeof(txBuffer); i++)
	{
		opResult = write(i2cHandle, &txBuffer[i], 1);
		if(opResult != 1) printf("No ACK bit!\n");
		usleep(1000); //sleep for 1 millisecond
	}
	opResult = read(i2cHandle, rxBuffer, 32);
	printf("Sent: %d, received: %d\n", sizeof(txBuffer), rxBuffer[0]);
	
	return 0;
}
