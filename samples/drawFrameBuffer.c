/*
 Frame Buffer への書き込みテスト

 参考:
  - ラズパイでフレームバッファ(/dev/fb0)を使用して、直接ディスプレイ画像を入出力する - Qiita
    - https://qiita.com/iwatake2222/items/0a7a2fefec9d93cdf6db

 描画テスト:
  - drawColor():   600枚/23s -> 26fps
  - drawColor2():  600枚/15s -> 40fps

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
#include <time.h>

void drawColor(int width, int height, uint32_t color);
void drawColor2(int width, int height, uint32_t color);
void getFrameBufferSizeIoctl(int* width, int* height, int* colorWidth);

int main() {

    printf("main run ..\n");

    int width;
    int height;
    int colorWidth;
    
    getFrameBufferSizeIoctl(&width, &height, &colorWidth);
    printf("(Width, Height)=(%d, %d), ColorBitDepth:%d \n", width, height, colorWidth);

    clock_t start, end;
    start = clock();

    int N = 100;
    for (int i=0; i<N; i++) {
        drawColor2(width, height, (uint32_t)0-(uint32_t)i*100);
    }

    end = clock();
    double elapsed = (double)(end-start)/CLOCKS_PER_SEC;
    printf("FPS: %f,  -- %d/%fs \n", (double)N / elapsed , N, elapsed);

    return 0;
}


void drawColor(int width, int height, uint32_t color) {
    uint32_t*  canvas;
    canvas = (uint32_t *)malloc(width * height * sizeof(uint32_t));

    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            canvas[x + y*width] = color;
        }
    }
    int fd;
    fd = open("/dev/fb0", O_WRONLY);
    write(fd, canvas, width * height * 4);
    close(fd);

    free(canvas);
}

/*
   drawColor2()
   ※ 32bit color = AARRGGBB
   ※ test: 600枚/15s -> 40fps
   mmapを使ってframeBuffer空間へのメモリを保持.
   msyncの実行により値を反映している.
 */
void drawColor2(int width, int height, uint32_t color)
{
    int fd;
    fd = open("/dev/fb0", O_RDWR);
    uint32_t *frameBufferAddres = (uint32_t *)mmap(NULL, width * height * 4, PROT_WRITE, MAP_SHARED, fd, 0);
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            frameBufferAddres[x + y * width] = color; 
        }
    }
    // 実際に値を書き込む
    msync(frameBufferAddres, width * height * 4, 0);

    // clean up
    munmap(frameBufferAddres, width * height * 4);
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