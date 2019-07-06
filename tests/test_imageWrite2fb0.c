/*
 
    test_frameBufferWriter.c

    フレームバッファへの書き込みテスト

    2019 T.Nohara
*/
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "../src/frameBufferWriter.h"

int main()
{

    int width;
    int height;
    int colorDepth;

    // 1. 
    int fd;
    fd = open("fb0.raw", O_RDONLY);

    uint32_t size=10000000;
    uint8_t * buff;
    buff = (uint8_t * )malloc(size * sizeof(uint8_t));

    int buff_size = read(fd, buff, size);
    printf("s:%d \n", buff_size);
    // T1. 32bit only

    for(int i=0; i<buff_size; i++) {
        buff[i] = 0xFF;
    }

    drawFrameBuffer("/dev/fb0", buff, buff_size);


    // ※ Clean UP
    free(buff);
    close(fd);

    // T1. テスト
    // ※ UIテストなので目視で確認

    return 0;
}
