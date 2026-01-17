#include "config.h"
#include <WiFi.h>
#include <time.h>
#include <FastLED.h>

extern MushroomConfig currentConfig;
extern GrowthPhase currentPhase;

PhaseConfig getActivePhaseConfig() {
  switch (currentPhase) {
    case INCUBATION:
      return currentConfig.incubation;
    case PRIMORDIA_FORMATION:
      return currentConfig.primordiaFormation;
    case FRUITING:
      return currentConfig.fruiting;
    default:
      return currentConfig.fruiting;
  }
}

MushroomConfig getMushroomConfig(MushroomType type) {
  switch (type) {
    case SHIITAKE:
      // Incubation (bag): 24-26°C (75-79°F), 70% RH, DARK - no light needed for colonization
      // Primordia: 12-18°C (53-64°F), 90%+ RH, BLUE/COOL WHITE light triggers pins
      // Fruiting: 7-18°C (45-65°F), 65-85% RH, 8-12h BLUE/COOL WHITE light (6500K)
      return {
        "Shiitake",
        { 25.0, 2.0, 70.0, 5.0, 1013.0, 8.0, 0, 0, CRGB::Black },           // Incubation: DARK
        { 15.0, 2.0, 92.0, 5.0, 1013.0, 8.0, 6, 10, CRGB(100, 150, 255) },  // Primordia: Cool blue-white
        { 13.0, 3.0, 75.0, 10.0, 1013.0, 8.0, 8, 12, CRGB(100, 150, 255) }  // Fruiting: Cool blue-white
      };

    case OYSTER:
      // Incubation (bag): 22-24°C (72-75°F), 70% RH, DARK - no light during colonization
      // Primordia: 10-15°C (50-60°F), 90-95% RH, BLUE/COOL WHITE light crucial for pins
      // Fruiting: 15-21°C (60-70°F), 85-90% RH, 12h BLUE/COOL WHITE (6500K)
      return {
        "Oyster",
        { 24.0, 2.0, 70.0, 5.0, 1013.0, 8.0, 0, 0, CRGB::Black },           // Incubation: DARK
        { 13.0, 2.0, 93.0, 5.0, 1013.0, 8.0, 8, 12, CRGB(100, 150, 255) },  // Primordia: Cool blue-white
        { 18.0, 3.0, 88.0, 5.0, 1013.0, 8.0, 8, 12, CRGB(100, 150, 255) }   // Fruiting: Cool blue-white
      };

    case KING_OYSTER:
      // Incubation (bag): 24-26°C (75-79°F), 90-95% RH, DARK - no light during colonization
      // Primordia: 15°C (59°F), 95-100% RH, BLUE/COOL WHITE light critical
      // Fruiting: 15-18°C (59-65°F), 85-88% RH, 10-16h BLUE/COOL WHITE (needs more light)
      return {
        "King Oyster",
        { 25.0, 2.0, 92.0, 5.0, 1013.0, 8.0, 0, 0, CRGB::Black },           // Incubation: DARK
        { 15.0, 1.0, 97.0, 5.0, 1013.0, 8.0, 8, 12, CRGB(100, 150, 255) },  // Primordia: Cool blue-white
        { 16.5, 1.5, 86.0, 3.0, 1013.0, 8.0, 10, 16, CRGB(100, 150, 255) }  // Fruiting: Cool blue-white
      };

    case SHIMEJI:
      // Incubation (bag): 24-26°C (75-79°F), 70-75% RH, DARK - no light during colonization
      // Primordia: 15-16°C, 80-90% RH, BLUE/COOL WHITE (500-600 lux)
      // Fruiting: 13-18°C (55-65°F), 85-95% RH, 8-12h BLUE/COOL WHITE
      return {
        "Shimeji (Beech)",
        { 25.0, 2.0, 72.0, 5.0, 1013.0, 8.0, 0, 0, CRGB::Black },           // Incubation: DARK
        { 15.5, 1.0, 87.0, 5.0, 1013.0, 8.0, 8, 12, CRGB(100, 150, 255) },  // Primordia: Cool blue-white
        { 15.5, 2.5, 90.0, 5.0, 1013.0, 8.0, 8, 12, CRGB(100, 150, 255) }   // Fruiting: Cool blue-white
      };

    case LIONS_MANE:
      // Incubation (bag): 24-26°C (75-79°F), 90-95% RH, DARK - no light during colonization
      // Primordia: 15-18°C (60-65°F), 85-95% RH, INDIRECT BLUE/COOL WHITE
      // Fruiting: 15-20°C (59-68°F), 85-95% RH, INDIRECT BLUE/COOL WHITE (sensitive to direct)
      return {
        "Lion's Mane",
        { 25.0, 2.0, 92.0, 5.0, 1013.0, 8.0, 0, 0, CRGB::Black },           // Incubation: DARK
        { 16.5, 1.5, 90.0, 5.0, 1013.0, 8.0, 6, 8, CRGB(120, 170, 255) },   // Primordia: Soft blue-white
        { 17.5, 2.5, 88.0, 5.0, 1013.0, 8.0, 8, 12, CRGB(120, 170, 255) }   // Fruiting: Soft blue-white
      };
    
    case MAITAKE:
      // Incubation (bag): 24-26°C (75-79°F), 75-80% RH, DARK (very long: 6-10 weeks)
      // Primordia: 10-16°C (50-60°F), 85-95% RH, BLUE/COOL WHITE (500-1000 lux, 12h)
      // Fruiting: 12-18°C (55-65°F), 85-95% RH, 12h BLUE/COOL WHITE cycle
      return {
        "Maitake (Hen of Woods)",
        { 25.0, 3.0, 75.0, 5.0, 1013.0, 8.0, 0, 0, CRGB::Black },           // Incubation: DARK
        { 13.0, 3.0, 90.0, 5.0, 1013.0, 8.0, 12, 12, CRGB(100, 150, 255) }, // Primordia: Cool blue-white
        { 15.0, 3.0, 88.0, 5.0, 1013.0, 8.0, 12, 12, CRGB(100, 150, 255) }  // Fruiting: Cool blue-white
      };
    default:
      // Fallback to general mushroom cultivation parameters
      return {
        "Generic Mushroom",
        { 22.0, 2.0, 70.0, 5.0, 1013.0, 8.0, 0, 0, CRGB::Black },
        { 15.0, 2.0, 90.0, 5.0, 1013.0, 8.0, 8, 12, CRGB::White },
        { 18.0, 2.0, 88.0, 5.0, 1013.0, 8.0, 8, 12, CRGB::White }
      };
  }
}

static bool timeIsSynced = false;

void setupTime() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️  WiFi not connected - skipping time sync");
    return;
  }
  
  Serial.println("Syncing time with NTP...");
  
  // Try multiple NTP servers (GMT+1 for Sweden/Europe)
  const char* ntpServers[] = {
    "pool.ntp.org",
    "time.cloudflare.com",
    "se.pool.ntp.org"
  };
  
  for (int i = 0; i < 3 && !timeIsSynced; i++) {
    configTime(3600, 0, ntpServers[i]);
    
    struct tm timeinfo;
    for (int attempt = 0; attempt < 10; attempt++) {
      if (getLocalTime(&timeinfo) && timeinfo.tm_year + 1900 >= 2020) {
        timeIsSynced = true;
        Serial.printf("✅ Time synced: %02d:%02d:%02d\n", 
                     timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        return;
      }
      delay(500);
    }
  }
  
  Serial.println("❌ Time sync failed - lights may not work");
}

bool isTimeSynced() {
  return timeIsSynced;
}

void setManualTime(int year, int month, int day, int hour, int minute, int second) {
  struct tm timeinfo;
  timeinfo.tm_year = year - 1900;
  timeinfo.tm_mon = month - 1;
  timeinfo.tm_mday = day;
  timeinfo.tm_hour = hour;
  timeinfo.tm_min = minute;
  timeinfo.tm_sec = second;
  timeinfo.tm_isdst = 0;
  
  time_t t = mktime(&timeinfo);
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
  
  timeIsSynced = true;
  Serial.printf("⚙️ Time set manually: %02d:%02d:%02d\n", hour, minute, second);
}