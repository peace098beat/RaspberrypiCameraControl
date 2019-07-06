/*
 Frame Buffer の情報を取得する

 参考:
  - ラズパイでフレームバッファ(/dev/fb0)を使用して、直接ディスプレイ画像を入出力する - Qiita
    - https://qiita.com/iwatake2222/items/0a7a2fefec9d93cdf6db

 */


#include "frameBufferWriter.h"


void getFrameBufferSizeIoctl(int* width, int* height, int* colorWidth)
{
    struct fb_var_screeninfo var;
    int fd;
    fd = open("/dev/fb0", O_RDWR);
    ioctl(fd, FBIOGET_VSCREENINFO, &var);
    *colorWidth = var.bits_per_pixel;
    *width      = var.xres_virtual;
    *height     = var.yres_virtual;
    close(fd);
}

void drawFrameBuffer(char* device_path, uint8_t* buff, int size) {

    int fd;
    fd = open("/dev/fb0", O_WRONLY);

    // write(fd, canvs, width * height * 4);
    write(fd, buff, size);
    close(fd);
}

void drawFrameBuffer32(char* device_path, uint32_t* buff, int size) {

    int fd;
    fd = open("/dev/fb0", O_WRONLY);

    // write(fd, canvs, width * height * 4);
    write(fd, (uint8_t*)buff, size);
    close(fd);
}