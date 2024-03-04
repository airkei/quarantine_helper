* main  
筋電+IMUによる手の動きを分類するプログラム
* env  
環境センサー取得用プログラム

以下、mainについて記載する。また基本操作はググれば出てくる。
# Arduino IDE セットアップ
* ファームウェアを更新したり、開発するためにはArduino IDEのインストールが必要
[こちら](https://www.indoorcorgielec.com/resources/arduinoide%E8%A8%AD%E5%AE%9A/arduino-ide%E3%81%AE%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB%E3%81%A8%E8%A8%AD%E5%AE%9A/)の方法に沿って、Arduino IDEをインストールする。寄付(donate)はおまかせ。  
* [こちら](https://www.arduino.cc/en/Guide/NANO33BLE)に沿って、BoardをArduino Nano 33 BLEに変更する。

## ライブラリ
[こちら](http://make.bcde.jp/arduino/arduino%E3%83%A9%E3%82%A4%E3%83%95%E3%82%99%E3%83%A9%E3%83%AA%E3%81%AE%E8%BF%BD%E5%8A%A0/)の方法に沿って、IDE上から以下のライブラリを追加する。Installed と表示されている場合はインストール済みのため、追加は不要。
* NRF52_MBED_TimerInterrupt
* Arduino_TensorFlowLite
* ArduinoBLE
* Arduino_LSM9DS1
* arduinoFFT
* Arduino_APDS9960

# ボードセットアップ
Arduino nano BLE 33 SENSE のピン配は[こちら](https://content.arduino.cc/assets/Pinout-NANOble_latest.pdf?_gl=1*hhy82y*_ga*NzcxNDEyMzExLjE2MzIyOTM1MTM.*_ga_SELSHHP7SG*MTYzNDcwNzIzMi42LjEuMTYzNDcwNzIzMi4w)なり、EMGセンサーのデータシートは[こちら](https://cdn.sparkfun.com/datasheets/Sensors/Biometric/MyowareUserManualAT-04-001.pdf)となる。  
以下の通り、ArduinoとEMGセンサーを接続すること
![image](https://user-images.githubusercontent.com/16249131/138039272-1f085203-09a0-4e7c-bc67-5a414f346690.png)


|       | Arduino | EMGセンサー |
| :---: | :-----: | :---------: |
|  赤   |  +3V3   |      +      |
|  黒   |   GND   |      -      |
|  緑   |   A0    |     SIG     |

## EMGセンサー
接触が悪い場合は、ジャンパーワイヤーとの接点を養生テープで固定したり半田付けすること。
* 緑色LED: 電源供給時に点灯
* 赤色LED: SIG(筋電位)検出時に点灯

# ファームアップ方法
PCとArduinoをUSBケーブルで接続した状態でArduino IDEを立ち上げ、Uploadボタンを押す。

# 開発方法
PCとArduinoを接続した状態でArduino IDEを立ち上げ、File -> Open -> main.ino を選択すると全てのソースコードが展開される。  
Tools -> Serial Monitor や Serial Plotter でデバッグ文やプロット図がリアルタイムに確認できる(Baud Rateは115200)。その他はググる。

