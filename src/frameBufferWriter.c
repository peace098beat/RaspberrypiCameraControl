/*
 Frame Buffer の情報を取得する

 参考:
  - ラズパイでフレームバッファ(/dev/fb0)を使用して、直接ディスプレイ画像を入出力する - Qiita
    - https://qiita.com/iwatake2222/items/0a7a2fefec9d93cdf6db

 :memo: フレームバッファの情報の確認
    $ fbset
    mode "1184x624"
        geometry 1184 624 1184 624 32
        timings 0 0 0 0 0 0 0
        rgba 8/16,8/8,8/0,8/24
    endmode

    // 読み方
    rgba 8/16, 8/8, 8/0, 8/24
     ==> r:8/16 - 8 bit per pix / 16bit shift
     ==> g:8/8  - 8 bit per pix / 8 bit shift
     ==> b:8/0  - 8 bit per pix / 0 bit shift
     ==> a:8/24 - 8 bit per pix / 24bit shift
     = ARGB - 8-8-8-8 フォーマットということ.

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

    write(fd, buff, size);
    close(fd);
}


void drawColor(int width, int height, uint32_t color) {

    uint32_t*  canvas;
    canvas = (uint32_t *)malloc(width * height * sizeof(uint32_t));

    for (int i=0; i<width*height; i++ ){
        canvas[i] = color;
    }

    drawFrameBuffer("/dev/fb0", (uint8_t*)canvas, width*height*4);

    free(canvas);
}
