#ifndef CONFIG_H
#define CONFIG_H

#include "mushroom_types.h"

extern GrowthPhase currentPhase;

// Function to get config based on type
MushroomConfig getMushroomConfig(MushroomType type);

// Function to setup time synchronization
void setupTime();
bool isTimeSynced();
void setManualTime(int year, int month, int day, int hour, int minute, int second);
PhaseConfig getActivePhaseConfig();


#endif
