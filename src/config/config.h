#ifndef CONFIG_H
#define CONFIG_H

// 騒音検出
#define MAX_NOISE_EVENTS  10  // 騒音イベント記録の最大数

// 基準値2: XX dB以上の瞬間的な騒音がYY分間にZZ回以上観測された場合通知
#define INSTANT_NOISE_THRESHOLD_DB (80) //XX
#define OBSERVATION_DURATION_SECOND (20*1000) //YY(ms)
#define TIME_IGNORE_NOISE (2000) //ms
#define NOISE_EVENT_COUNT_THRESHOLD (5) //ZZ

//#define RECORD_1s_LEN 1000*1000/TIME_IRQ
// 10000を超えるデータはDRAMが足りない。PSRAMを使う必要がある。
#define RECORD_BEFORE_LEN (5000)
#define RECORD_AFTER_LEN (5000)
#define RECORD_MAX_LEN (RECORD_BEFORE_LEN + RECORD_AFTER_LEN)

#define TIME_IRQ (100) //us
// デシベル計算用の積分時間　
#define TIME_INTEGRAL_DETECT (2*1000) //us
// RECORD_MAX_LENを超えない値にする
#define INTEGRAL_SAMPLES_DETECT (TIME_INTEGRAL_DETECT / TIME_IRQ)

// グラフ描画
#define TIME_WAVE (30*1000) //us
#define TIME_INTEGRAL (300*1000) //us
#define INTEGRAL_SAMPLES (TIME_INTEGRAL / TIME_WAVE)

// デバイス管理
#define TIME_UPDATE_MAX_VALUE (1000)
#define TIMEOUT_WIFI_CONNECT (30000)
#define JST (3600L * 9)

// サーバー関連
#define BASE_URL "https://6ansren87i.execute-api.ap-northeast-1.amazonaws.com/Prod/"

// ファイル関連
#define APARTMENT_NAME "APARTMENT_A"
#define ROOM_NAME "ROOM_A"

// 表示関連
#define FONT_SIZE 2

#endif