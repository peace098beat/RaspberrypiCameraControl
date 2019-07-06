#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/omapfb.h>


void getFrameBufferSizeIoctl(int* width, int* height, int* colorWidth);

void drawFrameBuffer(char* device_path, uint8_t* buff, int size);
void drawColor(int width, int height, uint32_t color) ;
