/*
 
    test_frameBufferWriter.c

    フレームバッファへの書き込みテスト

    2019 T.Nohara
*/

#include <assert.h>
#include <arpa/inet.h> 

#include "../src/frameBufferWriter.h"

int main()
{

    int width;
    int height;
    int colorDepth;

    // 1. frameBufferのサイズを取得
    getFrameBufferSizeIoctl(&width, &height, &colorDepth);
    printf("colorDepth: %d \n", colorDepth);
    printf("width: %d \n", width);
    printf("height: %d \n", height);

    printf("[] Memsize: %d \n", colorDepth/8 * width * height);

    // T1. 32bit only
    assert(colorDepth == 32);

    // 2. 書き込む内容が書かれた画像バッファを作る. W x H x 3byte : ARBG32
    // uint8_t *buff;
    // int size = 4 * width * height;
    // buff = (uint8_t *)malloc( size * sizeof(uint8_t) ); // 動的?
    // assert( size = 2955264);

    uint32_t* buff32;
    buff32 = (uint32_t *)malloc( width * height * sizeof(uint32_t) );

    for (int i=0; i<(width * height); i++ ){
        // buff32[i] = htonl(0xFF00000);
        // buff32[i] = 0xFF000000; // A
        // buff32[i] = 0x00FF0000; // R
        // buff32[i] = 0x0000FF00; // G
        buff32[i] = 0x000000FF; // B
    }

    drawFrameBuffer("/dev/fb0", (uint8_t*)buff32, 4*width*height);

    // drawColorFrameBuffer("/dev/fb0", 0x0000FF00, width, height); // G
    // drawColor( width,  height, 0x0000FF00) ; // G

    // drawFrameBuffer32("/dev/fb0", buff32, buff32_length);

    // printf("buffer size [byte] :%d \n", buff32_length*4);
    // assert(2955264 == buff32_length*4);


    // 3. 書き込み
    // drawFrameBuffer(char* device_path, uint8_t* buff, int size) 
    // drawFrameBuffer("/dev/fb0", buff, size);

    // ※ Clean UP
    // free(buff);
    free(buff32);


    return 0;
}
