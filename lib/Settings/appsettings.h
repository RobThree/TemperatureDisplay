#ifndef APPSETTINGS_H
#define APPSETTINGS_H

struct AppSettings {
    unsigned long updateInterval;
    bool showFahrenheit;
    float tempOffset;
    float humidityOffset;
    char deviceName[32];
};

#endif // APPSETTINGS_H