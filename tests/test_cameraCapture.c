/*
 
 ioctlを使ってカメラデバイスから画をキャプチャする.
 キャプチャ後のデータのサイズが想定と合っているかをテストする

*/

#include <assert.h>

#include "../src/cameraCapture.h"

int main()
{

    int width = 320;
    int height = 320;
    int pix_format = V4L2_PIX_FMT_BGR32;
    int ColorDepth = 4;

    // 1. キャプチャ開始: .
    startCapture(width, height, pix_format);

    // 2. バッファを作る. W x H x 3byte
    uint8_t *buff;
    uint32_t size;
    buff = (uint8_t *)malloc( ColorDepth * width * height *sizeof(uint8_t));

    // 3. コピー
    copyBuffer(buff, &size);

    // 4. 画像保存
    saveFileBinary("test_cameraCapture.raw", buff, size);

    // 5. キャプチャ停止
    stopCapture();

    // 6. Clean UP
    free(buff);

    // T1. テスト
    if (size == ( width * height * ColorDepth ) ){
        // printf("All Green!");
        return 0;
    }else{
        printf("[Debug] Size:%d = %d 4byte \n", size, width*height*ColorDepth);
        assert(size == ( width * height * ColorDepth ) );
        return 1;
    }


    return 0;
}
