/*
    app.c

*/

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

/* camera control */
int fd;
void *v4l2Buffer[v4l2BufferNum];
uint32_t v4l2BufferSize[v4l2BufferNum];

void startCapture();
void copyBuffer(uint8_t *dstBuffer, uint32_t *size) ;
void stopCapture();
int saveFileBinary(const char *filename, uint8_t *data, int size);

/* frame buffer control */
void drawColor(int width, int height, uint32_t color);
void getFrameBufferSizeIoctl(int* width, int* height, int* colorWidth);


int main()
{
    printf("main");

    startCapture();

    // copyBuffer(buff, &size);

    stopCapture();

    return 0;
}



/*
   drawColor2()
   ※ 32bit color = AARRGGBB
   ※ test: 600枚/15s -> 40fps
   mmapを使ってframeBuffer空間へのメモリを保持.
   msyncの実行により値を反映している.
 */
void drawColor(int width, int height, uint32_t color)
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

/*
 Camera Control

 */
void startCapture(int width, int height)
{

    printf("start");


    // デバイスファイルをオープン
    fd = open("/dev/video0", O_RDWR);

    printf("open");

    // フォーマットの指定: 320x240のJPEG形式でキャプチャしてください
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width   = width; //320;
    fmt.fmt.pix.height  = 240;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_ARGB32; //V4L2_PIX_FMT_JPEG;
    ioctl(fd, VIDIOC_S_FMT, &fmt);

    // 2. バッファリクエスト: バッファを2面メモリ空間に準備してください
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = v4l2BufferNum;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    ioctl(fd, VIDIOC_REQBUFS, &req);

    // 3. メモリ空間に確保されたバッファを、ユーザプログラムからアクセスできるようにmmapする
    struct v4l2_buffer buf;
    for (uint32_t i=0; i < v4l2BufferNum; i++) {

        // 確保したバッファ情報を教えて下さい
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        ioctl(fd, VIDIOC_QUERYBUF, &buf);

        // 3.2 取得したバッファ情報を基にmmapして、後でアクセスできるようにアドレスを保持しておく
        // buf -> v4l2Buffer[i]
        v4l2Buffer[i] = mmap(NULL, buf.length, PROT_READ, MAP_SHARED, fd, buf.m.offset);
        v4l2BufferSize[i] = buf.length;
    }

    // 4. バッファのエンキュー
    for (uint32_t i=0; i<v4l2BufferNum; i++) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        ioctl(fd, VIDIOC_QBUF, &buf);
    }

    // 5. ストリーミング開始。キャプチャを開始してください。
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMON, &type);


    printf("started");


}


void copyBuffer(uint8_t *dstBuffer, uint32_t *size) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    // 6. バッファに画データが書き込まれるまで待つ
    while (select(fd+1, &fds, NULL, NULL, NULL) < 0){
        printf(".");
    }

    if (FD_ISSET(fd, &fds)) {
        // 7. バッファのデキュー. 最も古くキャプチャされたバッファをデキューして、そのインデックス番号を教えてください
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        ioctl(fd, VIDIOC_DQBUF, &buf); // deque

        // 8. デキューされたバッファのインデックス(buf.index)と書き込まれたサイズ(buf.byteuserd)が返ってくる
        *size = buf.bytesused;

        // 9. ユーザプログラムで使用するために、指定のバッファにコピーする (v4l2Buffer[i] -> dstBuffer)
        memcpy(dstBuffer, v4l2Buffer[buf.index], buf.bytesused);

        // 10. 先ほどデキューした、バッファを再度エンキューする.
        // カメラデバイスはこのバッファに対して再びキャプチャした画を書き込む
        ioctl(fd, VIDIOC_QBUF, &buf);

    }

}


void stopCapture() {

    // 11. ストリーミング終了
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMOFF, &type);

    // 12. リソース解放
    for (uint32_t i=0; i<v4l2BufferNum; i++) {
        munmap(v4l2Buffer[i], v4l2BufferSize[i]);
    }

    // 13. デバイスディスクリプタを閉じる
    close(fd);
}

int saveFileBinary(const char* filename, uint8_t* data, int size) {
    FILE *fp;
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return -1;
    }
    fwrite(data, sizeof(uint8_t), size, fp);
    fclose(fp);
    return 0;
}