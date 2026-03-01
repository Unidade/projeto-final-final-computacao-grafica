#pragma once

void setupIndoorLightOnce();
void setupSunLightOnce();
void setSunDirectionEachFrame();

void setupFlashlightOnce();
void setFlashlightEachFrame(float camX, float camY, float camZ, float dirX, float dirY, float dirZ, bool enabled);
