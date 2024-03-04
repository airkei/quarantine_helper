# プログラム
全てのプログラムはpython3.5以降での実行を前提としている
## data_upload.py
Arduinoからシリアル経由でEMG/IMUのデータを受け取り、プロットするプログラム  
入力されたラベルを付与した上で、CSV形式でAWS S3にアップロードする
### ライブラリ
- boto3
- matplotlib.pyplot
- pyserial

### 使い方
```bash
sudo python data_upload.py label (samples)
```
### 引数
- label: ラベル名
- sample: サンプリング数、省略時は256 
### 注意
- 事前に `aws configure` を[設定](https://docs.aws.amazon.com/ja_jp/cli/latest/userguide/cli-configure-quickstart.html)しておくこと
```bash
$ aws configure
AWS Access Key ID [None]: アクセスキー
AWS Secret Access Key [None]: シークレットアクセスキー
Default region name [None]: ap-northeast-1
Default output format [None]: json
```
- /tmp/data.csv に一時ファイルを保存するため、書き込み権限を渡しておくこと
- プログラム内でシリアルデバイス名を直書しているため、デバイス名が異なる場合はプログラム内の `SERIAL_DEVICE_NAME` を所望のデバイス名に書き換えること

## serial_disp.py
Arduinoからのシリアル出力を表示するだけのプログラム  
### ライブラリ
- pyserial

### 使い方
```bash
sudo python serial_disp.py
```

## co2_upload.py
Arduinoからシリアル経由でCO2/温湿度/気圧/ドアの状態のデータを受け取るプログラム
CSV形式でAWS S3にアップロードする
### ライブラリ
- boto3
- pyserial

### 使い方
```bash
sudo python co2_upload.py (conditioner) (ventilation) (fan)
```
### 引数
- conditioner: エアコンの状態(デフォルト:ON)
- ventilation: 機械換気の状態(デフォルト:ON)
- fan: 扇風機の状態(デフォルト:ON)
### 注意
- 事前に `aws configure` を[設定](https://docs.aws.amazon.com/ja_jp/cli/latest/userguide/cli-configure-quickstart.html)しておくこと
```bash
$ aws configure
AWS Access Key ID [None]: アクセスキー
AWS Secret Access Key [None]: シークレットアクセスキー
Default region name [None]: ap-northeast-1
Default output format [None]: json
```
- /tmp/data_air.csv に一時ファイルを保存するため、書き込み権限を渡しておくこと
- プログラム内でシリアルデバイス名を直書しているため、デバイス名が異なる場合はプログラム内の `SERIAL_DEVICE1_NAME`(ドアの状態側) および `SERIAL_DEVICE2_NAME`(CO2センサー側) を所望のデバイス名に書き換えること
### セットアップ方法
![image](https://user-images.githubusercontent.com/16249131/150780985-4369ccde-c1aa-433b-951a-26120c40e3b9.png)
1. Arduinoをブレッドボードにはめこむ
2. ArduinoとパソコンをUSBケーブルで接続し、Arduino IDEから[co2](https://github.com/airkei/quarantine_helper/tree/main/arduino)のスケッチを開いてUploadする(Arduinoにプログラムを書き込む)
3. Raspberry Piを起動し、以下のPythonパッケージをインストールする  
`pip3 install boto3 serial`
4. PCに刺さっているUSBケーブルをRaspberry Piに刺し替えることで、Raspberry Pi と Arduino をUSBケーブルで接続する
5. ブレッドボードから出ている5Vのジャンパー線をRaspberry Piの5V(2もしくは4)に刺し、GNDのジャンパー線をRaspberry PiのGND(6)に刺す。ピン配置は[コチラ](https://ymt-lab.com/post/2021/raspberrypi-pin-assugnment/)
7. Raspberry Pi から Arduinoの[シリアルポート名を確認](https://algorithm.joho.info/programming/python/raspberrypi3-arduino-dmesg-port/)する(ttyACM0 など)
8. co2_upload.py プログラム内の `SERIAL_DEVICE2_NAME` を上記のシリアルポート名に書き換える(`SERIAL_DEVICE2_NAME = "/dev/ttyACM0"` など)

### 使い方
セットアップが完了したら、Raspberry Pi上でco2_upload.pyプログラムを実行すると、Arduinoからのデータが標準出力に表示される。
データを取得し終えたら、「q」を押すことでデータがAWS(S3)にアップロードされてプログラムが終了する。  
**「q」を押さないとデータはAWSにアップロードされないため注意!!**
