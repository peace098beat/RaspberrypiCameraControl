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
#include <linux/fb.h>
#include <linux/omapfb.h>

#define v4l2BufferNum 2
#define CAMERA_WIDTH 32*37 // 1184
#define CAMERA_HEIGHT 32*20 // 642

/* camera control */
int fd;
void *v4l2Buffer[v4l2BufferNum];
uint32_t v4l2BufferSize[v4l2BufferNum];

void startCapture();
void copyBuffer(uint8_t *dstBuffer, uint32_t *size) ;
void stopCapture();
int saveFileBinary(const char *filename, uint8_t *data, int size);

/* frame buffer control */
void drawFrameBufferBGRA32(uint8_t* buffer, uint32_t size, int width, int height);
void getFrameBufferSizeIoctl(int* width, int* height, int* colorWidth);


int main()
{
    startCapture();

    uint8_t *buff;
    uint32_t size;
    buff = (uint8_t *)malloc( 4* CAMERA_WIDTH * CAMERA_HEIGHT *sizeof(uint8_t)); // 14241bytes // 14363 // 13510

    copyBuffer(buff, &size);
    copyBuffer(buff, &size);
    copyBuffer(buff, &size);

    int width;
    int height;
    int colorWidth;
    getFrameBufferSizeIoctl(&width, &height, &colorWidth);

    printf("Camera Resolusion:      %dx%d = %d\n", CAMERA_WIDTH, CAMERA_HEIGHT, CAMERA_WIDTH*CAMERA_HEIGHT);
    printf("Camera Format:          %d [byte]\n", CAMERA_WIDTH * CAMERA_HEIGHT * 4);
    printf("Camera CopyBuffer Size: %d-byte(%d-px) \n", size, size/4);
    printf("FrameBuffer:            %dx%d (%d)\n", width, height, colorWidth);

    drawFrameBufferBGRA32(buff, size, width, height);
    
    stopCapture();

    free(buff);

    return 0;
}

#include <stdint.h>
// unit8 = 1byte = 0xFF
uint8_t reverseByte(uint8_t value)
{
    uint8_t lil = (value | 0x0F);
    uint8_t big = (value | 0xF0);

    uint8_t concat = (lil << 1) | (big >> 1);

    return concat;
}

uint32_t reverse(uint32_t value)
{
    union {
        uint32_t value;
        uint8_t u[4];
    } from, to;
    from.value = value;
    to.u[0] = from.u[3];
    to.u[1] = from.u[2];
    to.u[2] = from.u[1];
    to.u[3] = from.u[0];

    // to.u[0] = reverseByte(from.u[3]);
    // to.u[1] = reverseByte(from.u[2]);
    // to.u[2] = reverseByte(from.u[1]);
    // to.u[3] = reverseByte(from.u[0]);
    return to.value;
}



// FrameBufferはARGB形式
// CameraはBGRA形式
void drawFrameBufferBGRA32(uint8_t* buffer, uint32_t size, int width, int height) {
    int fd;
    fd = open("/dev/fb0", O_RDWR);
    uint32_t *frameBufferAddres = (uint32_t *)mmap(NULL, width * height * 4, PROT_WRITE, MAP_SHARED, fd, 0);

    // bufferをキャストして、そのまま、反転(AGBR)させてコピーしてみる
    // uint32_t * buffer32 = (uint32_t *)buffer;
    // for( int i=0; i<size; i++) {
    //     frameBufferAddres[i] = reverse(buffer32[i]);
    // }

    // for (int y=0; y<height; y++) {
    //     for (int x=0; x<width; x++) {

    //         int index = x + y *width;

    //         frameBufferAddres[index] = 0x00FF0000;

    //         // camera image
    //         uint8_t B = buffer[index/4+0];
    //         uint8_t G = buffer[index/4+1];
    //         uint8_t R = buffer[index/4+2];
    //         uint8_t A = buffer[index/4+3];

    //         // FrameBuffer: AARRBBGG
    //         frameBufferAddres[index] = 0xFF000000;
    //         frameBufferAddres[index] |= (uint32_t)(B) <<  0; // B
    //         frameBufferAddres[index] |= (uint32_t)(G) <<  8; // G
    //         frameBufferAddres[index] |= (uint32_t)(R) <<  16; // R

    //         // frameBufferAddres[x + y*width] = 0x00FF0000;
    //         // frameBufferAddres[x + y*width] = (uint32_t)(0x00) <<  0; // B
    //         // frameBufferAddres[x + y*width] = (uint32_t)(0x00) <<  0; // B
    //         // frameBufferAddres[x + y*width] = (uint32_t)(0x00) <<  8; // G
    //         // frameBufferAddres[x + y*width] = (uint32_t)(0x00) << 16; // R
    //         // frameBufferAddres[x + y*width] = (uint32_t)(0x00) << 24; // A
    //     }
    // }

    // for (int i=0; i<size; i=i+4){
    //     int frame_index = i/4;
    //         frameBufferAddres[frame_index] = (uint32_t)(0xFF); // init
    //         frameBufferAddres[frame_index] |= (uint32_t)(buffer[i+3]) << 24; // A
    //         frameBufferAddres[frame_index] |= (uint32_t)(buffer[i+2]) << 16; // R
    //         frameBufferAddres[frame_index] |= (uint32_t)(buffer[i+1]) <<  8; // G
    //         frameBufferAddres[frame_index] |= (uint32_t)(buffer[i+0]) <<  0; // B
    // }


    // フレームバッファにバッファの内容をそのままコピーしてみる.
    // memcpy(frameBufferAddres, buffer, size);

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

    // デバイスファイルをオープン
    fd = open("/dev/video0", O_RDWR);

    // フォーマットの指定: 320x240のJPEG形式でキャプチャしてください
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width   = CAMERA_WIDTH; //320;
    fmt.fmt.pix.height  = CAMERA_HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_ABGR32; //V4L2_PIX_FMT_JPEG;
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
        printf("-- v4l2BufferSize[%d]=%d \n", i, buf.length);
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