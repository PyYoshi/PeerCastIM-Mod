PeerCastIM-Mod
================

PeerCastIMをベースにPeerCast Qtを移植しました.

ただしQt GUIを利用できるわけではなくCLIのみサポートします.

## ビルド

### Linux

```bash
cd ui/linux
make
ln -s ../html ./html
```

### OSX

```bash
cd ui/osx
make
ln -s ../html ./html
```

### Windows

VS2012で

```
PeerCast.sln
```

を開いてSimple_vpをビルド

PeerCast.exeと同じフォルダにhtmlフォルダをおく

## 実行

```bash
./peercast
```

## 設定

WebUIかpeercast.iniを利用してください.

## 再生

mplayerとvlcにて再生確認済み.

再生まで結構時間かかります.

FLVやMKVはサポートしていません. セグフォります.

## 動作検証/開発環境
* Kubuntu 12.04 amd64
* Qt Creator 2.4.1 based on Qt 4.8.1
* OSX 10.9

## ライセンス
* GPLv2

## 謝辞
* [PeerCastIM](http://sourceforge.jp/projects/peercast-im/)
* [PeerCast Qt](http://mosax.sakura.ne.jp/yp4g/fswiki.cgi?page=PeerCast+Qt)

## TODO

* FLVへの対応
* MKVへの対応
* Qt5への対応
