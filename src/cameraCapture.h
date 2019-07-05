#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#define v4l2BufferNum 2

void startCapture(int width, int height, int pix_format);
void copyBuffer(uint8_t *dstBuffer, uint32_t *size) ;
void stopCapture();
int saveFileBinary(const char *filename, uint8_t *data, int size);

