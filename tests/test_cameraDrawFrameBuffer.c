/*
 

    フレームバッファへの書き込みテスト

    2019 T.Nohara
*/

#include <assert.h>
#include <arpa/inet.h> 
#include <time.h>

#include "../src/frameBufferWriter.h"
#include "../src/cameraCapture.h"

int main() {
    clock_t start,end;
    start = clock();

    // cameraサイズ
    int width = 1184; //1184;
    int height = 624;
    int pix_format = V4L2_PIX_FMT_BGR32;
    int ColorDepth = 4;

    // 1. キャプチャ開始: .
    startCapture(width, height, pix_format);
    // 2. バッファを作る. W x H x 3byte
    uint8_t *buff;
    uint32_t size;
    buff = (uint8_t *)malloc( ColorDepth * width * height *sizeof(uint8_t));

    int old_fps=0;
    
    for(int i=0; i<10000; i++) {
        start = clock();

        // 3. コピー
        copyBuffer(buff, &size);

        // GrayScale
        // uint8_t c1, c2, c3, c4;
        // for(int j=loop_start_index; j<size/ColorDepth; j=j+skip) {
        //     c1 = buff[4*j + 0];
        //     c2 = buff[4*j + 1];
        //     c3 = buff[4*j + 2];
        //     c4 = buff[4*j + 3];
        //     uint8_t c = (c1 + c2 + c3 + c4) / 4;
        //     buff[4*j + 0] = c;
        //     buff[4*j + 1] = c;
        //     buff[4*j + 2] = c;
        //     buff[4*j + 3] = c;
        // }

        // フレームバッファに表示
        drawFrameBuffer("/dev/fb0", (uint8_t*)buff, size);

        // FPSを表示
        end = clock();
        double duration_s = (double)(end-start)/CLOCKS_PER_SEC;
        int fps = (int)(1/duration_s);
        // fps = (fps + old_fps) / 2;
        // old_fps = fps;

        if( i % 10 == 0){
            printf("FPS %d  - [%d]\n", fps, i );
        }
    }


    // 5. キャプチャ停止
    stopCapture();

    // --- 
    // > Raspberry PIの場合の挙動
    // 1. CameraSetting GBRA-32
    // 2. Camera Memory ARBG-32 (little endian)
    // uint8_t* で渡すとそのままフレームバッファにコピーされて
    // 3. FrameBuffer Mem ARBG-32;
    // おそらくこれをuint32_tとして取り出すと. エンディアン変換されて.
    // ※. uint32_t GBRA-32で取り出される.

    // たまたまリトルエンディアンなので、GBRA->ARBGとしてメモリ配置されて正常に動いている.

    free(buff);

    return 0;
}