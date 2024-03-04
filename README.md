# 本リポジトリの目的
「市販のEMGとIMUを使用し、エッジ端末で手の動きを分類した上でAWS IoTに結果を送信する」プログラム
|学習| 推論|
|---|---|
|![training](https://user-images.githubusercontent.com/16249131/152665007-90aeec22-7407-4464-bea7-9912f687525b.svg)|![inference](https://user-images.githubusercontent.com/16249131/152664901-3534ad8c-bf4d-46e6-8025-66089154f0f3.svg)|

# はじめに
開発用PCやRaspberry piに以下のコマンドで、本リポジトリをダウンロードしておく  
`git clone https://github.com/airkei/quarantine_helper.git`


# 学習手順
1. arduino nano ble 33 sense(以下、arduino)上で、`__COLLECT_RAW_DATA__`を`true`にした`arduino/main` プログラムを実行し、データをPCに送信する
2. PC上で`pc/data_upload.py`プログラムを実行し、データをS3にアップロードする
3. S3に保存されたデータに対して、`cloud`ディレクトリの以下を実行する
   1. `touch_model_collect_data.ipynb`を実行し、データを整形する
   2. `touch_model_tuning.ipynb`を実行し、ハイパーパラメータのチューニングをかける
   3. `touch_model_conv_tflite.ipynb`を実行し、TF Lite用の学習データを作成する

# 使用(推論)手順
1. 学習で作成したモデルを`arduino/main/model.c`にコピーし、arduino上で`arduino/main` プログラムを実行する
2. Raspberry Pi 4上で、`rasp/gateway.py` プログラムを実行する



# ディレクトリ構成
詳細は各ディレクトリ内のREADME.mdに記載
* arduino  
arduinoのプログラム
* cloud  
AWSのプログラム
* rasp  
raspberry piのプログラム
* pc  
arduinoとシリアル接続したpcのプログラム
