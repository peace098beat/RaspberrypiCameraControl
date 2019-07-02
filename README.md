
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
v4l2-ctl -d /dev/video0 --all
v4l2-ctl -d /dev/video0 --list-formats-ext
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


## (メモ) FFmepgの利用

```bash
ffmpeg -i /dev/video0 -pix_fmt bgra -f fbdev /dev/fb0
```


## cameraの形式とframebufferの形式を表示

## Author

@tomoyuki_nohara
