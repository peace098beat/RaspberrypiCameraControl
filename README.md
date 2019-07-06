
# Linux Camera Control

## goal

 - Cameraで撮影した画を、フレームバッファで表示する.
 - Cameraで撮影した画を、websocket or webrtc で送信する

## references
[Raspberry PiとLinux的方法によるデジタルカメラの作成 - Qiita](https://qiita.com/iwatake2222/items/d6645880c5bb91ce8a85)

[ラズパイでフレームバッファ(/dev/fb0)を使用して、直接ディスプレイ画像を入出力する - Qiita](https://qiita.com/iwatake2222/items/0a7a2fefec9d93cdf6db)

[v4l2-framebuffer/video_capture.c at master · lincolnhard/v4l2-framebuffer](https://github.com/lincolnhard/v4l2-framebuffer/blob/master/video_capture.c)

## 設計

モジュール

1. ioctlを使ってcameraの画データをメモリ空間のバッファに格納 (startCapture, stopCapture, copyBuffer)
2. ユーザアプリケーションより、cameraの画データをユーザのバッファにコピー (main)
3. ユーザアプリケーションより、frameBufferへユーザのバッファーからコピー (drawFrameBuffer)

アプリケーション

1. 指定フレーム数だけ動作
2. ioctrlでcameraのcaptureをstart
3. ioctrlから画データをフレームバッファにコピー
4. 繰り返し
5. ioctrlでcameraのcaptureをstop

要件

1. カメラの画像サイズと、描画の画像サイズを合わせる必要がある.


## よく使う関連コマンド

カメラのinfomation

```bash
sudo modprobe bcm2835-v4l2
ls -al /dev/video*
v4l2-ctl -d /dev/video0 --info
# デバイスの情報を表示
v4l2-ctl -d /dev/video0 --all
# 対応解像度の表示
v4l2-ctl -d /dev/video0 --list-formats-ext
# コントロール一覧の表示
v4l2-ctl -d /dev/video0 --list-ctrls-menu
# コントロールの変更
v4l2-ctl -d /dev/video0 -c <ctrl>=<val>

```

フレームバッファの確認

```bash
# フレームバッファファイルの確認
$ ls /dev/fb*
/dev/fb0

# $ フレームバッファのinfo確認
$ fbset
$ fbset -fb /dev/fb0

mode "1184x624"
    geometry 1184 624 1184 624 32
    timings 0 0 0 0 0 0 0
    rgba 8/16,8/8,8/0,8/24
endmode
```


FFmepgを使ったストリーム表示

```bash
ffmpeg -i /dev/video0 -pix_fmt bgra -f fbdev /dev/fb0
```


## cameraの形式とframebufferの形式を表示

Raspi Camera

- 対応解像度: 32の倍数
- format: BGR4(32bit BGRA)
- format: BGR3(24bit BGR)
- format: RGB3(24bit RGB)

## V4L2のフォーマット

```c
/* RGB formats */
#define V4L2_PIX_FMT_RGB332  v4l2_fourcc('R', 'G', 'B', '1') /*  8  RGB-3-3-2     */
#define V4L2_PIX_FMT_RGB444  v4l2_fourcc('R', '4', '4', '4') /* 16  xxxxrrrr ggggbbbb */
#define V4L2_PIX_FMT_ARGB444 v4l2_fourcc('A', 'R', '1', '2') /* 16  aaaarrrr ggggbbbb */
#define V4L2_PIX_FMT_XRGB444 v4l2_fourcc('X', 'R', '1', '2') /* 16  xxxxrrrr ggggbbbb */
#define V4L2_PIX_FMT_RGBA444 v4l2_fourcc('R', 'A', '1', '2') /* 16  rrrrgggg bbbbaaaa */
#define V4L2_PIX_FMT_RGBX444 v4l2_fourcc('R', 'X', '1', '2') /* 16  rrrrgggg bbbbxxxx */
#define V4L2_PIX_FMT_ABGR444 v4l2_fourcc('A', 'B', '1', '2') /* 16  aaaabbbb ggggrrrr */
#define V4L2_PIX_FMT_XBGR444 v4l2_fourcc('X', 'B', '1', '2') /* 16  xxxxbbbb ggggrrrr */
#define V4L2_PIX_FMT_BGRA444 v4l2_fourcc('B', 'A', '1', '2') /* 16  bbbbgggg rrrraaaa */
#define V4L2_PIX_FMT_BGRX444 v4l2_fourcc('B', 'X', '1', '2') /* 16  bbbbgggg rrrrxxxx */
#define V4L2_PIX_FMT_RGB555  v4l2_fourcc('R', 'G', 'B', 'O') /* 16  RGB-5-5-5     */
#define V4L2_PIX_FMT_ARGB555 v4l2_fourcc('A', 'R', '1', '5') /* 16  ARGB-1-5-5-5  */
#define V4L2_PIX_FMT_XRGB555 v4l2_fourcc('X', 'R', '1', '5') /* 16  XRGB-1-5-5-5  */
#define V4L2_PIX_FMT_RGBA555 v4l2_fourcc('R', 'A', '1', '5') /* 16  RGBA-5-5-5-1  */
#define V4L2_PIX_FMT_RGBX555 v4l2_fourcc('R', 'X', '1', '5') /* 16  RGBX-5-5-5-1  */
#define V4L2_PIX_FMT_ABGR555 v4l2_fourcc('A', 'B', '1', '5') /* 16  ABGR-1-5-5-5  */
#define V4L2_PIX_FMT_XBGR555 v4l2_fourcc('X', 'B', '1', '5') /* 16  XBGR-1-5-5-5  */
#define V4L2_PIX_FMT_BGRA555 v4l2_fourcc('B', 'A', '1', '5') /* 16  BGRA-5-5-5-1  */
#define V4L2_PIX_FMT_BGRX555 v4l2_fourcc('B', 'X', '1', '5') /* 16  BGRX-5-5-5-1  */
#define V4L2_PIX_FMT_RGB565  v4l2_fourcc('R', 'G', 'B', 'P') /* 16  RGB-5-6-5     */
#define V4L2_PIX_FMT_RGB555X v4l2_fourcc('R', 'G', 'B', 'Q') /* 16  RGB-5-5-5 BE  */
#define V4L2_PIX_FMT_ARGB555X v4l2_fourcc_be('A', 'R', '1', '5') /* 16  ARGB-5-5-5 BE */
#define V4L2_PIX_FMT_XRGB555X v4l2_fourcc_be('X', 'R', '1', '5') /* 16  XRGB-5-5-5 BE */
#define V4L2_PIX_FMT_RGB565X v4l2_fourcc('R', 'G', 'B', 'R') /* 16  RGB-5-6-5 BE  */
#define V4L2_PIX_FMT_BGR666  v4l2_fourcc('B', 'G', 'R', 'H') /* 18  BGR-6-6-6     */
#define V4L2_PIX_FMT_BGR24   v4l2_fourcc('B', 'G', 'R', '3') /* 24  BGR-8-8-8     */
#define V4L2_PIX_FMT_RGB24   v4l2_fourcc('R', 'G', 'B', '3') /* 24  RGB-8-8-8     */
#define V4L2_PIX_FMT_BGR32   v4l2_fourcc('B', 'G', 'R', '4') /* 32  BGR-8-8-8-8   */
#define V4L2_PIX_FMT_ABGR32  v4l2_fourcc('A', 'R', '2', '4') /* 32  BGRA-8-8-8-8  */
#define V4L2_PIX_FMT_XBGR32  v4l2_fourcc('X', 'R', '2', '4') /* 32  BGRX-8-8-8-8  */
#define V4L2_PIX_FMT_BGRA32  v4l2_fourcc('R', 'A', '2', '4') /* 32  ABGR-8-8-8-8  */
#define V4L2_PIX_FMT_BGRX32  v4l2_fourcc('R', 'X', '2', '4') /* 32  XBGR-8-8-8-8  */
#define V4L2_PIX_FMT_RGB32   v4l2_fourcc('R', 'G', 'B', '4') /* 32  RGB-8-8-8-8   */
#define V4L2_PIX_FMT_RGBA32  v4l2_fourcc('A', 'B', '2', '4') /* 32  RGBA-8-8-8-8  */
#define V4L2_PIX_FMT_RGBX32  v4l2_fourcc('X', 'B', '2', '4') /* 32  RGBX-8-8-8-8  */
#define V4L2_PIX_FMT_ARGB32  v4l2_fourcc('B', 'A', '2', '4') /* 32  ARGB-8-8-8-8  */
#define V4L2_PIX_FMT_XRGB32  v4l2_fourcc('B', 'X', '2', '4') /* 32  XRGB-8-8-8-8  */
```

## 現課題

 - カメラバッファのサイズが、想定よりも小さい

Camera Resolusion:      1184x640 = 757760
Camera Format:          3031040 [byte]
Camera CopyBuffer Size: 1136640-byte(284160-px)
FrameBuffer:            1184x624 (32)

(想定) 3,031,040[byte]: Camera の解像度(1184x640)とフォーマット(V4L2_PIX_FMT_XBGR32:4byte)から、バッファのサイズは3031040[byte]となりそう
(実際) 1,136,640[byte] ※ 2.6倍程度異なる

## はまりどころ

 - (注意) Raspberry PI はリトルエンディアン

 - Camera GBRA32
 - Camera (mem) ARBG32
 - Copied Buffer (mem) ARBG32
 - FrameBuffer ARBG32
 - FrameBuffer (mem) GBRA32

フォーマットとメモリ格納時の並びに注意.
openしてuint8_t*で調べているときには、リトルエンディアンで並んでいる。
バッファをコピー(メモリコピー)をした場合は、リトルエンディアンの並びでコピーされる。


## Author

@tomoyuki_nohara
