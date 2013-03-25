PeerCastIM4Linux
================

PeerCastIMをベースにPeerCast Qtを移植しました。

ui/linuxでmakeしたpeercastバイナリのみストリームの取得が成功していることを確認。

ui/qt4はなんかしらのエラーで動いてない模様。

```shell
$ mplayer <mms address> -dumpstream -dumpfile <output file path>
```

## 動作検証/開発環境
* Kubuntu 12.04 amd64
* Qt Creator 2.4.1 based on Qt 4.8.1

## ライセンス
* GPLv2

## 謝辞
* [PeerCastIM](http://sourceforge.jp/projects/peercast-im/)
* [PeerCast Qt](http://mosax.sakura.ne.jp/yp4g/fswiki.cgi?page=PeerCast+Qt)