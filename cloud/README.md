# 各プログラムの説明
AWS Sagemaker上での実施を前提とする

## boto3_utilities.py
boto3を使用する汎用モジュール群

## touch_model_collect_data.ipynb
EMG/IMUデータを整形し、学習用と本番用データに分割するプログラム

## touch_model_tuning.ipynb
整形されたEMG/IMUデータのハイパーパラメータをチューニングするプログラム

## touch_model_conv_tflite.ipynb
整形されたEMG/IMUデータを学習し、Tensorflow Lite for Microcontroller に変換するプログラム

## env_model.ipynb
環境センサデータを解析するプログラム

## co2_model.ipynb
CO2データを解析するプログラム