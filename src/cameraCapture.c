/*


 :Setup:
    sudo modprobe bcm2835-v4l2
    ls -al /dev/video*
 */

#include "cameraCapture.h"

#include <stdio.h>

int fd;
void* v4l2Buffer[v4l2BufferNum];
uint32_t v4l2BufferSize[v4l2BufferNum];

/**
 * キャプチャ開始
 * 
 * ※ デバイスがない場合はフリーズしてしまう。エラー処理を追加する.
 */
void startCapture(int width, int height, int pix_format)
{
    // デバイスファイルをオープン
    fd = open("/dev/video0", O_RDWR);
    if (fd < 0) {
        printf("fd = %d, errno=%d: %s\n", fd, errno, strerror(errno));
        printf("Please enter command: $sudo modprobe bcm2835-v4l2 \n");
        return;
    }

    // フォーマットの指定: 320x240のJPEG形式でキャプチャしてください
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width   = width;
    fmt.fmt.pix.height  = height;
    fmt.fmt.pix.pixelformat = pix_format;
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
        printf("fopen error %s\n", strerror(errno ));
        return -1;
    }
    fwrite(data, sizeof(uint8_t), size, fp);
    fclose(fp);
    return 0;
}