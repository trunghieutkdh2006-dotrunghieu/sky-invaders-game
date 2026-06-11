#include "powerup.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

void spawnPowerUp(vector<PowerUp>& powerUps) {
    PowerUp p = {
        static_cast<float>(rand() % W),
        static_cast<float>(rand() % H),
        rand() % 4,
        300
    };
    powerUps.push_back(p);
}

void spawnExplosion(vector<Particle>& p, float x, float y) {
    for (int i = 0; i < 50; i++) {
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float speed = (float)(rand() % 6 + 2);
        p.push_back({x, y, cos(angle)*speed, sin(angle)*speed, 40});
    }
}

void applyPowerUp(int& playerHP, float& fireRate, int& bulletPower,
                  bool& hasShield, bool& hasLaser, int type)
{
    switch (type) {
        case 0: fireRate    *= 1.5f; break;
        case 1: hasShield    = true; break;
        case 2: bulletPower *= 2;    break;
        case 3: hasLaser     = true; break;
    }
}

void checkPowerUpCollision(float px, float py, vector<PowerUp>& powerUps,
                           int& playerHP, float& fireRate, int& bulletPower,
                           bool& hasShield, bool& hasLaser)
{
    for (size_t i = 0; i < powerUps.size(); ++i) {
        PowerUp& p = powerUps[i];
        if (abs(px - p.x) < 20 && abs(py - p.y) < 20) {
            applyPowerUp(playerHP, fireRate, bulletPower, hasShield, hasLaser, p.type);
            powerUps.erase(powerUps.begin() + i);
            break;
        }
    }
}

void updateHealthDrops(vector<HealthDrop>& healthDrops, float px, float py,
                       int& playerHP, int H)
{
    for (auto& h : healthDrops) {
        if (!h.active) continue;

        h.y  += h.vy;
        h.vy += 0.2f;

        if (h.y >= H - 50) {
            h.y  = H - 50;
            h.vy = 0;
            h.onGround = true;
            h.groundTime++;
        } else {
            h.groundTime = 0;
        }

        float dx   = px - h.x;
        float dy   = py - h.y;
        float dist = sqrt(dx*dx + dy*dy);

        if (dist < 120) {
            h.x += dx * 0.05f;
            h.y += dy * 0.05f;
        }

        if (dist < 25) {
            if (playerHP < 10) {
                playerHP += h.value;
                if (playerHP > 10) playerHP = 10;
            }
            h.active = false;
        }
    }

    healthDrops.erase(
        std::remove_if(healthDrops.begin(), healthDrops.end(), [](HealthDrop& h) {
            if (!h.active) return true;
            if (h.onGround && h.groundTime > 180) return true;
            return false;
        }),
        healthDrops.end()
    );
}
