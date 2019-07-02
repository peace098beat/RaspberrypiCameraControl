/*
 Frame Buffer の情報を取得する

 参考:
  - ラズパイでフレームバッファ(/dev/fb0)を使用して、直接ディスプレイ画像を入出力する - Qiita
    - https://qiita.com/iwatake2222/items/0a7a2fefec9d93cdf6db

 */
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


void getFrameBufferSize(int* width, int* height, int* colorWidth);
void getFrameBufferSizeIoctl(int* width, int* height, int* colorWidth);


int main() {
    printf("main run ..\n");
    int width;
    int height;
    int colorWidth;
    getFrameBufferSize(&width, &height, &colorWidth);
    printf("(Width, Height)=(%d, %d), ColorBitDepth:%d \n", width, height, colorWidth);

    getFrameBufferSizeIoctl(&width, &height, &colorWidth);
    printf("(Width, Height)=(%d, %d), ColorBitDepth:%d \n", width, height, colorWidth);

    return 0;
}


void getFrameBufferSize(int* width, int* height, int* colorWidth) {

    int fd;
    int n;
    char str[64];

    // Get Bits/px
    fd = open("/sys/class/graphics/fb0/bits_per_pixel", O_RDONLY);
    n = read(fd, str, sizeof(str));
    str[n] = '\0';
    *colorWidth = atoi(str);
    close(fd);

    // Get Frame Buffer Size
    fd = open("/sys/class/graphics/fb0/virtual_size", O_RDONLY);
    n = read(fd, str, sizeof(str));
    str[n] = '\0';

    // format
    *width = atoi(strtok(str, ","));
    *height = atoi(strtok(NULL, ","));
    close(fd);

}

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

void drawColor(int width, int height, int color) {
    uint32_t*  canvas
    canvas = (uint32_t *)malloc(width * height * sizeof(uint32_t));

    for (int y=0; y<height; y++) {
        for (int x=0; x<width, x++) {
            canvas[x + y*width] = color;
        }
    }


    int fd;
    fd = open("/dev/fb0", O_WRONLY);
    write(fd, canvs, width * height * 4);
    close(fd);

    free(canvas);
}