#ifndef _LANG_H
#define _LANG_H

// Раскомментируйте только одну строку для выбора языка
#define LANG_UA
// #define LANG_RU
// #define LANG_EN

// ======================= УКРАЇНСЬКА =======================
#if defined(LANG_UA)
    #define STR_VERSION       "Програма Помычник v 0.3"
    #define STR_QUANTITY      "Кылькысть"
    #define STR_FULL          "Повний"
    #define STR_NOT_FOUND     "Датчикыв 1-Wire не знайдено!"
    #define STR_DHT_1WR       "DHT на лыныъ 1-Wire."
    #define STR_DHT_AM2301    "DHT на лыныъ AM2301"
    #define STR_HIH5030       "Датчик вологосты HIH-5030."
    #define STR_CONNECTED     "ПЫДКЛЮЧЕН."
    #define STR_NOT_CONNECTED "НЕ ПЫДКЛЮЧЕН!"
// ========================= РУССКИЙ =========================
#elif defined(LANG_RU)

#endif // _LANG_H
