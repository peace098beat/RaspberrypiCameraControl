/*
 CPUのエンディアンを調べる

 参考
 [バイナリデータを扱うときはエンディアンに注意 - Corgi Lab. ～備忘録のための技術ブログ～](https://corgi-lab.com/programming/c-lang/warn-endian/)

 2019 T.Nohara
 */

#include <stdio.h>
#include <stdint.h>


int main(void) {
    uint32_t num = 0xABCDEF01;
    uint8_t* p = (uint8_t*)(&num);

    uint32_t* p32 = (uint32_t*)(&num);
    printf("0x%x \n", p32[0]); // abcdef01 // おそらくprintfがエンディアン変換している. 
    
    // メモリに直接アクセスする
    for(int i = 0; i < sizeof(uint32_t); i++) {
        printf("0x%x ", p[i]); // 0x1 0xef 0xcd 0xab // リトルエンディアン
    }
 
    return 0;
}

void test_uint32(){
    long num = 0xABCDEF01;
    char* p = (char*)(&num);
    int i;
 
    printf("num = ");
 
    for(i = 0; i < sizeof(long); i++) {
        printf("0x%x ", p[i]);
    }
 
    printf("\n");
}