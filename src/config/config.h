#ifndef CONFIG_H
#define CONFIG_H

// 騒音検出
#define NOISE_CONSTANT_VALUE 2500
#define TIME_IRQ (100) //us
//#define RECORD_1s_LEN 1000*1000/TIME_IRQ
// 10000を超えるデータはDRAMが足りない。PSRAMを使う必要がある。
#define RECORD_BEFORE_LEN 5000
#define RECORD_AFTER_LEN 5000
#define RECORD_MAX_LEN (RECORD_BEFORE_LEN + RECORD_AFTER_LEN)

// デバイス管理
#define TIME_UPDATE_MAX_VALUE 1000
#define TIMEOUT_WIFI_CONNECT 30000
#define JST (3600L * 9)

// 表示関連
#define FONT_SIZE 2

#endif