//#include <sstream>
//#include <iostream>
//#include <vector>
//#include <fstream>
//#include "iosfwd"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
//#include <spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
//using std::cout;
//using std::endl;

#define FLASH_CERS 0xC7
#define FLASH_SE 0xD8
//definitions for AT25512 device
#define WRITE_CYCLE_TIME (5000) //AT25512 write cycle time in us
#define WRSR (0x01)             //AT25512 write status register
#define WRITE (0x02)            //AT25512 write data to memory array
#define READ (0x03)             //AT25512 read data from memory array
#define WRDI (0x04)             //AT25512 reset write enable latch
#define RDSR (0x05)             //AT25512 read status register
#define WREN (0x06)             //AT25512 set write enable latch
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 1000;
static void pabort(const char *s)
{
    perror(s);
    abort();
}
typedef unsigned char tByte;

int addresshigh, addressmid, addresslow;
///int fromIndex,toIndex;
tByte getNVM_CRC(tByte *CKBlock, tByte Seed)
{
    tByte val, y, crc, bbit1;
    uint32_t k;
    crc = Seed;
    for (k = 0; k < 256; k++)
    {
        val = CKBlock[k];
        crc = crc ^ ((val << 4));
        for (y = 0; y < 8; y++)
        {
            bbit1 = crc & 0x80;
            if (bbit1)
            {
                crc = crc ^ 74;
                bbit1 = 1;
            }
            else
                bbit1 = 0;
            crc = ((crc << 1) + bbit1);
        }
        crc = crc ^ 82;
        crc = crc ^ (val >> 4);
    }
    return (crc);
}
char readCommand[3];
uint8_t rx[256];
struct spi_ioc_transfer message[2] = {
    0,
};

void init_readSPI_256Bytes_64K() ///tByte mid) ///;///int fd)////tByte mid)
{

    message[0].tx_buf = (unsigned long)readCommand;
    message[0].rx_buf = (unsigned long)NULL;
    message[0].len = sizeof(readCommand);
    message[0].cs_change = 0; //0 working for P9_17
    message[1].tx_buf = (unsigned long)NULL;
    message[1].rx_buf = (unsigned long)rx;
    message[1].len = sizeof(rx);
    message[1].cs_change = 0;
}
int ret;
int fd;
void init_Spi()
{

    bits = 8;
    mode = 0;
    speed = 10000;
    fd = open("/dev/spidev1.0", O_RDWR); //open the spi device
    if (fd < 0)
        pabort("can't open device");

    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode); //set the spi mode
    if (ret == -1)
        pabort("can't set spi mode");
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits); //get the spi bits per word (test)
    if (ret == -1)
        pabort("can't get bits per word");

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed); //get the spi max speed (test)
    if (ret == -1)
        pabort("can't get max speed hz");
}
int main(int argc, char *argv[])

{
    //    int ret = 0;
    //  int fd;

    readCommand[0] = READ;
    readCommand[2] = 0;
    init_readSPI_256Bytes_64K(); ///
    init_Spi();
    //////////////////
    tByte yeed = 80;

    for (addressmid = 0; addressmid < 0x100; addressmid++)
    {
        readCommand[1] = addressmid;
        if (ioctl(fd, SPI_IOC_MESSAGE(2), &message) < 1)
        {
            pabort("can't send spi message");
        }

        yeed = getNVM_CRC(rx, yeed);
        ///usleep(1000);
    }
    close(fd);
    //printf("\nCRC=0x%.2X %d\n", yeed, yeed);
    printf("%d\n", yeed); //	////////////////////////////

    return 0;
}
