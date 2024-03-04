# Raspberry Pi 環境構築方法
- [OSはraspbianをインストールする](https://qiita.com/tksnkym/items/31a237e27cbc51790cdd)
- [初期設定をする](https://qiita.com/HeRo/items/c1c30d7267faeb304538)
- [python をデフォルト3にする](https://www.ingenious.jp/articles/howto/raspberry-pi-howto/python-3-change/)
- OSの最新化および、以下のパッケージをインストール・設定する
  - `sudo apt-get update`
  - `sudo apt-get upgrade`
  - `sudo apt-get install cmake libssl-dev git`
  - `sudo apt-get install python3-pip libglib2.0-dev`
  - `sudo apt-get install python3-dev bluetooth libbluetooth-dev libcap2-bin`
  - `sudo setcap 'cap_net_raw,cap_net_admin+eip' "$(readlink -f "$(which python3)")"`
- 以下のPythonライブラリをインストールする(必ずsudoをつけること)
  - `sudo pip3 install --upgrade pip`
  - `sudo pip3 install pybluez bluepy awsiotsdk`
- AWS CLIのインストール
  - `sudo apt install awscli`
  - `sudo pip3 install awscli --upgrade`
- [AWS CLIを設定する](https://docs.aws.amazon.com/ja_jp/cli/latest/userguide/cli-configure-quickstart.html)
```bash
$ aws configure
AWS Access Key ID [None]: アクセスキー
AWS Secret Access Key [None]: シークレットアクセスキー
Default region name [None]: ap-northeast-1
Default output format [None]: json
```
# プログラム
全てのプログラムはpython3.5以降での実行を前提としている
## gateway.py
以下の情報を集約しAWS IoT Coreに送る本番用プログラム。Raspberry PiがCentralとして動く  
- Arduinoからの動作判定結果
- Beacon端末からのRSSI

AWS IoT Coreには `device/Arduinoのアドレス/data(例: device/85a5ca785ca4/data)` の Topicを送信する  
内部で [publish.py](#publishpy) と [ibscanner.py](#ibscannerpy) をモジュールとして使用している
### 事前準備
- sudo apt-get install python3-pip libglib2.0-dev
### ライブラリ
- bluepy
- awsiotsdk
- beacontools
### 使い方
```bash
sudo python gateway.py --cert certs/certificate.pem.crt --key certs/private.pem.key --root certs/root.pem --ep endpoint.iot.ap-northeast-1.amazonaws.com --uuid 00000000-e132-1001-b000-001c4de2af03
```
### 引数
- cert: AWS IoT デバイス証明書のファイルパス
- key: AWS IoT プライベートキーのファイルパス
- root: AWS IoT ルート CA 証明書のファイルパス
- ep: AWS IoT エンドポイント
- uuid: スキャンするiBeaconのUUID、デフォルトは00000000-e132-1001-b000-001c4de2af03
### 注意
- 証明書などを誤ってコミットしないこと  
テストする際は証明書などはgit管理下以外、もしくは.gitignoreされているcertsディレクトリ以下に保管した上でテストすること
<br>

## dummy_gateway.py
ダミーの動作判定結果を内部で生成し、Beacon端末からのRSSIと結合してAWS IoT Coreに送る検証用プログラム。Arduino無しで動く  
以下のダミーデータを生成し、JSONデータに格納する
- address: `01:23:45:67:89:AB`
- action: `none` もしくは `touch` もしくは `wipe` の中からランダム  
AWS IoT Coreには常に `device/0123456789AB/data` の Topicを送信する  
内部で [publish.py](#publishpy) と [ibscanner.py](#ibscannerpy) をモジュールとして使用している
### 事前準備
- sudo apt-get install python3-pip libglib2.0-dev
### ライブラリ
- awsiotsdk
- beacontools
### 使い方
```bash
sudo python dummy_gateway.py --interval 5 --cert certs/certificate.pem.crt --key certs/private.pem.key --root certs/root.pem --topic test/testing --ep endpoint.iot.ap-northeast-1.amazonaws.com --uuid 00000000-e132-1001-b000-001c4de2af03
```
### 引数
- interval: AWS IoT への送信間隔(秒)、省略時は5秒
- cert: AWS IoT デバイス証明書のファイルパス
- key: AWS IoT プライベートキーのファイルパス
- root: AWS IoT ルート CA 証明書のファイルパス
- ep: AWS IoT エンドポイント
- uuid: スキャンするiBeaconのUUID、デフォルトは00000000-123c-1001-B000-001c4D05C3A8
### 注意
- 証明書などを誤ってコミットしないこと  
テストする際は証明書などはgit管理下以外、もしくは.gitignoreされているcertsディレクトリ以下に保管した上でテストすること
<br>

## publish.py
AWS IoTにデータを送るプログラム。上記 [gateway.py](#gatewaypy) からモジュールとして呼び出されることを想定している  
単独での実行時はテストが可能
### ライブラリ
- awsiotsdk
### 使い方
```bash
python publish.py --cert certs/certificate.pem.crt --key certs/private.pem.key --root certs/root.pem --data sample.json  --topic test/testing --ep endpoint.iot.ap-northeast-1.amazonaws.com
```
### 引数
- cert: AWS IoT デバイス証明書のファイルパス
- key: AWS IoT プライベートキーのファイルパス
- root: AWS IoT ルート CA 証明書のファイルパス
- data: データ(JSON形式)のファイルパス
- topic: MQTTのトピック名
- ep: AWS IoT エンドポイント
### 注意
- 証明書などを誤ってコミットしないこと  
テストする際は証明書などはgit管理下以外、もしくは.gitignoreされているcertsディレクトリ以下に保管した上でテストすること
<br>

## ibscanner.py
iBeaconを検出するプログラム。上記[gateway.py](#gatewaypy)からモジュールとして呼び出されることを想定している  
単独での実行時はテストが可能
### 事前準備
- sudo apt-get install python3-dev bluetooth libbluetooth-dev libcap2-bin
- sudo setcap 'cap_net_raw,cap_net_admin+eip' "$(readlink -f "$(which python3)")"
### ライブラリ
- pybluez
### 使い方
```bash
sudo python ibscanner.py --uuid 00000000-e132-1001-b000-001c4de2af03 --interval 1
```
### 引数
- uuid: スキャンするiBeaconのUUID、デフォルトは00000000-e132-1001-b000-001c4de2af03
- interval: スキャンの間隔(秒)、省略時は1秒
<br>

## env_gateway.py
環境センサー情報の収集用プログラム
