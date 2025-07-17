# DetectSound

M5Stackベースの多機能音響解析システム。ノイズ検出、FFT分析、波形描画、データアノテーション、ニューラルネットワーク推論機能を統合したデバイスです。

## 概要

このプロジェクトは、M5Stack Fireを使用して音響データの収集、解析、分類を行う包括的なソリューションです。NNabla C Runtimeを活用したニューラルネットワーク推論により、リアルタイムでの音響パターン認識が可能です。

## 主な機能

### 1. ノイズ検出 (Noise Detector)
- リアルタイムでの騒音レベル監視
- 設定可能な閾値（デフォルト70dB）による自動検出
- AWS連携による通知機能

### 2. フーリエ変換解析 (FFT)
- 最大2048サンプルのFFT解析
- 周波数スペクトラム表示
- 10kHzサンプリング周波数対応

### 3. 波形描画 (Waveform Drawer)
- リアルタイム音声波形の可視化
- 高解像度音声データキャプチャ

### 4. データアノテーション (Annotation)
- 音響データの手動ラベリング機能
- SDカード保存対応

### 5. ニューラルネットワーク推論 (Neural Network)
- NNabla C Runtime搭載
- リアルタイム音響分類
- 事前学習済みモデル実行

## ハードウェア要件

- M5Stack Fire
- SDカード（データ保存用）
- 内蔵マイクまたは外部マイク

## ソフトウェア依存関係

- PlatformIO Framework
- M5Stack Library (v0.4.6)
- ArduinoFFT Library (v2.0.4)
- NNabla C Runtime

## セットアップ

1. **リポジトリのクローン**
   ```bash
   git clone https://github.com/sota1111/DetectSound.git
   cd DetectSound
   git submodule update --init --recursive
   ```
5. **設定ファイル**
   - `src/config/secret.h`にWiFi設定やAWS認証情報を記載
   ```
   #define WIFI_SSID "hogehoge"
   #define WIFI_PASSWORD "hogehoge"
   ```

2. **PlatformIOプロジェクトのビルド**
   ```bash
   pio run
   ```
2. **PlatformIOプロジェクトのビルド**
   ```bash
   pio run -t clean
   ```

4. **M5Stack Fireへのアップロード**
   ```bash
   pio run -t upload
   ```



## 使用方法

1. M5Stackの電源を入れると、メニュー画面が表示されます
2. ボタンA（左）: メニュー下移動
3. ボタンB（中央）: 選択
4. ボタンC（右）: メニュー上移動

各モードの詳細:
- **Noise Detect**: 自動騒音監視開始
- **FFT**: ボタンBでFFT解析実行
- **Wave Drawer**: リアルタイム波形表示
- **Annotation**: データ収集とラベリング
- **Neural Network**: AI推論実行

## アーキテクチャ

```
src/
├── core/                    # コア機能
│   ├── NoiseDetector.*     # 騒音検出エンジン
│   ├── FourierTransform.*  # FFT解析
│   ├── WaveformDrawer.*    # 波形描画
│   ├── AnnotationData.*    # データアノテーション
│   ├── NeuralNetwork.*     # NN推論エンジン
│   └── Validation/         # NNabla検証コード
├── DeviceHandler/          # デバイス制御
│   ├── SDCardHandler.*     # SDカード操作
│   ├── SpeakerHandler.*    # スピーカー制御
│   └── WiFiHandler.*       # WiFi通信
├── config/                 # 設定管理
└── main.cpp               # メインアプリケーション
```

## 技術仕様

- **サンプリング周波数**: 10kHz
- **FFTサンプル数**: 最大2048
- **ノイズ検出閾値**: 70dB（設定可能）
- **データバッファ**: 10,000サンプル（前後5,000ずつ）
- **積分時間**: 200ms（デシベル計算用）

## ライセンス

このプロジェクトに含まれるNNabla C Runtimeは、Sony Network Communications Inc.によるオープンソースライブラリです。詳細は`lib/nnabla-c-runtime/LICENSE`を参照してください。
