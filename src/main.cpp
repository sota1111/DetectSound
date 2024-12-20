#include <M5Stack.h>
#include <WiFi.h>
#include "Free_Fonts.h"

#define MIC_Unit 36
#define MAX_LEN 320
#define X_OFFSET 0
#define Y_OFFSET 100
#define X_SCALE 1
#define NOISE_CONSTANT_VALUE 2500 // グラフに赤線で表示する定数値
#define JST (3600L * 9)

const char* ssid = "aterm-90fa24-a";
const char* password = "0fbd815a5b8b9";

volatile int16_t val_buf[MAX_LEN] = {0}; // 描画用バッファ
volatile int16_t write_index = 0;       // 書き込みインデックス
volatile int16_t data_count = 0;        // 有効データの個数
volatile int micValue = 0;              // 最新のAD値
volatile int maxMicValue = 0;           // 1秒間の最大AD値
volatile unsigned long lastUpdateTime = 0;

// タイマー割り込みで呼び出される関数
void IRAM_ATTR onTimer() {
  micValue = analogRead(MIC_Unit);

  // 最大値を更新
  if (micValue > maxMicValue) {
    maxMicValue = micValue;
  }

  // データをバッファに書き込み
  val_buf[write_index] = map((int16_t)(micValue * X_SCALE), 1800, 4095, 0, 100);

  // 次の書き込み位置を計算
  write_index = (write_index + 1) % MAX_LEN;

  // データ数を更新（MAX_LENを超えないように）
  if (data_count < MAX_LEN) {
    data_count++;
  }

  // 1秒ごとに最大値をリセット
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= 1000) {
    lastUpdateTime = currentTime;
    maxMicValue = 0;
  }
}

// 描画関数
static void draw_waveform() {
  if (data_count <= 1) return; // 有効なデータが不足している場合はスキップ

  int read_index = write_index; // 現在の書き込み位置を起点に読み出し開始
  for (int i = 0; i < data_count - 1; i++) {
    int next_index = (read_index + 1) % MAX_LEN;

    // データを描画
    M5.Lcd.drawLine(i + X_OFFSET, val_buf[read_index] + Y_OFFSET, 
                    i + 1 + X_OFFSET, val_buf[next_index] + Y_OFFSET, TFT_GREEN);

    read_index = next_index; // 次のデータへ
  }

  // 定数値を赤線で描画
  int constantY = map(NOISE_CONSTANT_VALUE, 1800, 4095, 0, 100) + Y_OFFSET;
  M5.Lcd.drawLine(X_OFFSET, constantY, X_OFFSET + MAX_LEN, constantY, TFT_RED);
}

hw_timer_t *timer = NULL;

void setup() {
  struct tm tm;

  M5.begin();
  // wifi接続
  WiFi.begin(ssid, password);
  M5.Lcd.print('wifi接続中');
  while (WiFi.status() != WL_CONNECTED){
      delay(500);
      M5.Lcd.print('.');
  }
  configTime(JST,0,"ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");

  if(getLocalTime(&tm)){
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(60,80);
    M5.Lcd.printf("%d/%2d/%2d",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    M5.Lcd.setCursor(80,140);
    M5.Lcd.printf("%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
  }
  delay(5000);

  // LCDの初期化
  M5.Lcd.setFreeFont(FSS12);
  M5.Lcd.setTextDatum(TC_DATUM);

  // タイマーの設定
  timer = timerBegin(0, 80, true);         // タイマー0を設定 (80分周 -> 1µs)
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000, true);      // 1msごとに割り込み
  timerAlarmEnable(timer);                 // タイマーを有効化
}

void loop() {
  M5.Lcd.fillScreen(TFT_BLACK); // 描画領域をクリア
  
  // 最大AD値を描画
  M5.Lcd.setTextDatum(TC_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextSize(1); // AD値の文字サイズを初期化
  M5.Lcd.drawString("Max AD Value: " + String(maxMicValue), 160, 10, GFXFF);
  int max_dB = (maxMicValue-1920)*104/(4095-1920)+30;
  M5.Lcd.drawString("dB: " + String(max_dB), 160, 30, GFXFF);

  // ノイズ検出時の表示
  if (maxMicValue > NOISE_CONSTANT_VALUE) {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
      M5.Lcd.drawString("NOISE", 160, 360, GFXFF);
  }

  draw_waveform();              // 描画処理
  delay(20);                    // 少し遅延を入れる（スムーズな描画のため）
}
