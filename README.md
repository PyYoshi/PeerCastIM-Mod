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

### Qt4

Qt4のbinディレクトリのパスを通し

```bash
cd ui/qt4
qmake
make
ln -s ../html ./html
```

### Qt5 ⚠未完成

Qt5のbinディレクトリのパスを通し

```bash
cd ui/qt5
qmake
make
ln -s ../html ./html
```

## 実行

```bash
./peercast
```

## 設定

WebUIかpeercast.iniを利用してください.

## 再生

mplayerとvlcにて再生確認済み.

再生まで結構時間かかります.

## 動作検証/開発環境

* LinuxMint 17
* OSX 10.9
* Windows 7
* Qt Creator 3.3.0 based on Qt 4.8.6

## ライセンス
* GPLv2

## 謝辞
* [PeerCastIM](http://sourceforge.jp/projects/peercast-im/)
* [PeerCast Qt](http://mosax.sakura.ne.jp/yp4g/fswiki.cgi?page=PeerCast+Qt)
