#include <stdio.h>

/*
 * 台灣電費與瓦斯費估算器 (2024 年料金）
 * 註解詳細，處理不同用戶類別與季節
 */

// 定義用戶類與季節
typedef enum { RESIDENTIAL, COMMERCIAL, INDUSTRIAL } UserType;
typedef enum { SUMMER, NON_SUMMER } Season;
typedef enum { NATURAL_GAS, LPG } GasType;

// 電價級距
typedef struct {
    int min;
    int max; //若max = -1，表示無上限
    double rate;
    char description[50];
} ElectricityRate;

// 天然氣級距
typedef struct {
    int min;
    int max;
    double rate;
    char description[50];
} GasRate;

// 計算電費
double calculateElectricity(double kWh, UserType user, Season season, int *steps, double breakdown[10]) {
    ElectricityRate residential_summer[] = {
        {0, 120, 1.63, "120度以下"},
        {121, 330, 2.38, "121-330度"},
        {331, 500, 3.52, "331-500度"},
        {501, 700, 4.80, "501-700度"},
        {701, 1000, 5.66, "701-1000度"},
        {1001, -1, 6.41, "1001度以上"}
    };
    ElectricityRate residential_non_summer[] = {
        {0, 120, 1.63, "120度以下"},
        {121, 330, 2.10, "121-330度"},
        {331, 500, 2.89, "331-500度"},
        {501, 700, 3.94, "501-700度"},
        {701, 1000, 4.60, "701-1000度"},
        {1001, -1, 5.03, "1001度以上"}
    };
    ElectricityRate commercial_summer[] = {
        {0, 330, 2.53, "330度以下"},
        {331, 700, 3.55, "331-700度"},
        {701, 1500, 4.25, "701-1500度"},
        {1501, -1, 4.73, "1501度以上"}
    };
    ElectricityRate commercial_non_summer[] = {
        {0, 330, 2.12, "330度以下"},
        {331, 700, 2.91, "331-700度"},
        {701, 1500, 3.44, "701-1500度"},
        {1501, -1, 3.81, "1501度以上"}
    };
    ElectricityRate industrial_summer[] = { {0, -1, 2.85, "工業用"} };
    ElectricityRate industrial_non_summer[] = { {0, -1, 2.28, "工業用"} };
  
    ElectricityRate *rates;
    int rate_count = 0;
    double baseFee = 0.0;

    if (user == RESIDENTIAL) {
        if (season == SUMMER) {
            rates = residential_summer;
            rate_count = sizeof(residential_summer) / sizeof(residential_summer[0]);
        } else {
            rates = residential_non_summer;
            rate_count = sizeof(residential_non_summer) / sizeof(residential_non_summer[0]);
        }
    } else if (user == COMMERCIAL) {
        baseFee = 79.7;
        if (season == SUMMER) {
            rates = commercial_summer;
            rate_count = sizeof(commercial_summer) / sizeof(commercial_summer[0]);
        } else {
            rates = commercial_non_summer;
            rate_count = sizeof(commercial_non_summer) / sizeof(commercial_non_summer[0]);
        }
    } else { // Industrial
        baseFee = 223.7;
        if (season == SUMMER) {
            rates = industrial_summer;
            rate_count = sizeof(industrial_summer) / sizeof(industrial_summer[0]);
        } else {
            rates = industrial_non_summer;
            rate_count = sizeof(industrial_non_summer) / sizeof(industrial_non_summer[0]);
        }
    }
  
    double total = baseFee;
    int i;
    *steps = 0;

    if (baseFee > 0) {
        breakdown[(*steps)++] = baseFee;
    }
  
    int remaining = (int)kWh;

    for (i = 0; i < rate_count && remaining > 0; i++) {
        int usage = 0;

        if (rates[i].max == -1) { // no upper limit
            usage = remaining;
        } else if (remaining > (rates[i].max - rates[i].min + 1)) {
            usage = rates[i].max - rates[i].min + 1;
        } else {
            usage = remaining;
        }
        total += usage * rates[i].rate;
        breakdown[(*steps)++] = usage * rates[i].rate;
        remaining -= usage;
    }
  
    // 5% Tax
    double tax = total * 0.05;
    breakdown[(*steps)++] = tax;
    total += tax;

    return total;
}

// 計算瓦斯費
double calculateGas(double m3, GasType gType, UserType user, int *steps, double breakdown[10]) {
    GasRate natural_residential[] = {
        {0, 50, 9.52, "50度以下"},
        {51, 100, 10.75, "51-100度"},
        {101, 200, 11.42, "101-200度"},
        {201, 500, 12.66, "201-500度"},
        {501, -1, 13.25, "501度以上"}
    };
    GasRate natural_commercial[] = {
        {0, 100, 11.18, "100度以下"},
        {101, 500, 11.85, "101-500度"},
        {501, -1, 12.52, "501度以上"}
    };
    GasRate lpg_rates[] = {
        {0, -1, 32.5, "桶裝瓦斯"}
    };
  
    GasRate *rates;
    int rate_count = 0;
    double baseFee = 0.0;

    if (gType == NATURAL_GAS) {
        if (user == RESIDENTIAL) {
            rates = natural_residential;
            rate_count = sizeof(natural_residential) / sizeof(natural_residential[0]);
            baseFee = 93;
        } else { // commercial
            rates = natural_commercial;
            rate_count = sizeof(natural_commercial) / sizeof(natural_commercial[0]);
            baseFee = 150;
        }
    } else { // LPG
        rates = lpg_rates;
        rate_count = sizeof(lpg_rates) / sizeof(lpg_rates[0]);
        baseFee = 0;
    }
  
    double total = baseFee;
    int i;
    *steps = 0;

    if (baseFee > 0) {
        breakdown[(*steps)++] = baseFee;
    }
  
    int remaining = (int)m3;

    for (i = 0; i < rate_count && remaining > 0; i++) {
        int usage = 0;

        if (rates[i].max == -1) { // no upper limit
            usage = remaining;
        } else if (remaining > (rates[i].max - rates[i].min + 1)) {
            usage = rates[i].max - rates[i].min + 1;
        } else {
            usage = remaining;
        }
        total += usage * rates[i].rate;
        breakdown[(*steps)++] = usage * rates[i].rate;
        remaining -= usage;
    }
  
    if (gType == NATURAL_GAS) { // 5% tax for natural
        double tax = total * 0.05;
        breakdown[(*steps)++] = tax;
        total += tax;
    }
  
    return total;
}
