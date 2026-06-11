#include "bullet.h"
#include <cmath>
#include <algorithm>

// ============================================================
// ĐẠN CƠ BẢN (BULLET)
// ============================================================

// Cập nhật vị trí và xóa đạn hết thời gian sống
void updateBullets(vector<Bullet> &bullets)
{
    // Di chuyển từng viên đạn theo vận tốc
    for (auto &b : bullets)
    {
        b.x += b.vx;
        b.y += b.vy;
    }

    // Xóa đạn có life <= 0 (hết thời gian sống)
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
                       [](Bullet &b)
                       { return b.life <= 0; }),
        bullets.end());
}

// ============================================================
// LASER THẲNG (DÀNH CHO ENEMY THƯỜNG)
// ============================================================

// Bắn tia laser thẳng từ enemy về phía player
void shootLaser(vector<Laser> &lasers, Enemy &en, float px, float py, int level)
{
    // Tính hướng từ enemy đến player
    float dx = px - en.x;
    float dy = py - en.y;
    float len = sqrt(dx * dx + dy * dy);
    if (len == 0)
        len = 1;

    // Tạo laser mới: vị trí enemy, hướng đã chuẩn hóa, sống 25 frame, cấp độ
    lasers.push_back({en.x, en.y, dx / len, dy / len, 25, level});
}

// Vẽ tất cả laser thẳng lên màn hình
void renderLasers(SDL_Renderer *r, vector<Laser> &lasers)
{
    for (auto &l : lasers)
    {
        // Điểm cuối của laser (kéo dài 800px)
        float endX = l.x + l.dx * 800;
        float endY = l.y + l.dy * 800;

        // Độ dày laser tăng theo cấp độ
        int thickness = 2 + l.level;

        // Vẽ các lớp glow (hiệu ứng phát sáng)
        for (int i = thickness; i > 0; i--)
        {
            SDL_SetRenderDrawColor(r, 255, 0, 0, 40); // Màu đỏ mờ
            SDL_RenderDrawLine(r, (int)l.x, (int)l.y, (int)endX, (int)endY);
        }

        // Vẽ lõi laser màu đỏ tươi
        SDL_SetRenderDrawColor(r, 255, 80, 80, 255);
        SDL_RenderDrawLine(r, (int)l.x, (int)l.y, (int)endX, (int)endY);
    }
}

// Cập nhật laser: giảm thời gian sống, kiểm tra va chạm với player
void updateLasers(vector<Laser> &lasers, float px, float py,
                  int &playerHP, int &damageCooldown, GameState &state)
{
    for (auto &l : lasers)
    {
        l.life--; // Giảm thời gian sống

        // Dùng phép chiếu vector để tìm điểm gần nhất trên laser đến player
        float px2 = px - l.x;
        float py2 = py - l.y;
        float proj = px2 * l.dx + py2 * l.dy; // Độ dài hình chiếu

        // Nếu điểm chiếu nằm trong đoạn laser
        if (proj > 0 && proj < 1000.0f)
        {
            // Tìm điểm gần nhất trên laser
            float cx = l.x + l.dx * proj;
            float cy = l.y + l.dy * proj;
            float dist = sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));

            // Nếu player cách laser < 25px và không trong thời gian miễn nhiễm
            if (dist < 25 && damageCooldown == 0)
            {
                playerHP--;          // Giảm máu player
                damageCooldown = 10; // Bật miễn nhiễm 10 frame
                if (playerHP <= 0)
                    state = GAMEOVER; // Kết thúc game nếu hết máu
            }
        }
    }

    // Xóa laser hết thời gian sống
    lasers.erase(
        std::remove_if(lasers.begin(), lasers.end(),
                       [](Laser &l)
                       { return l.life <= 0; }),
        lasers.end());
}

// ============================================================
// LASER VÒNG CUNG (DÀNH CHO BOSS LEVEL 3+)
// ============================================================

// Bắn laser vòng cung từ boss (3-6 tia, tỏa hình quạt)
void shootArcLaser(vector<ArcLaser> &arcLasers, Enemy &en, float px, float py, int level)
{
    // Số lượng tia: 3 + level/3, tối đa 6 tia
    int numArcs = 3 + (level / 3);
    if (numArcs > 6)
        numArcs = 6;

    // Hướng chính về phía player
    float baseAngle = atan2(py - en.y, px - en.x);
    float spreadAngle = 60.0f * M_PI / 180.0f; // Góc trải rộng 60 độ

    for (int i = 0; i < numArcs; i++)
    {
        // Tính tỷ lệ từ 0 đến 1 để phân bố đều các tia
        float t = (numArcs == 1) ? 0.5f : (float)i / (numArcs - 1);
        float angle = baseAngle - spreadAngle / 2 + t * spreadAngle;

        // Tạo tia laser vòng cung
        ArcLaser arc;
        arc.x = en.x;
        arc.y = en.y;
        arc.startAngle = angle;
        // Độ cong: random ±30 độ
        arc.endAngle = angle + (float)(rand() % 60 - 30) * M_PI / 180.0f;
        arc.radius = 100 + rand() % 100; // Bán kính cong random
        arc.life = 60;                   // Sống 60 frame (1 giây)
        arc.level = level;
        arc.direction = (rand() % 2 == 0) ? 1 : -1;

        arcLasers.push_back(arc);
    }
}

// Cập nhật laser vòng cung
void updateArcLasers(vector<ArcLaser> &arcLasers, float px, float py,
                     int &playerHP, int &damageCooldown, GameState &state)
{
    for (auto &arc : arcLasers)
    {
        arc.life--; // Giảm thời gian sống

        // Duyệt các điểm trên đường cong để kiểm tra va chạm
        // Lấy 20 điểm trên đường cong (từ t=0 đến t=1)
        for (float t = 0; t <= 1.0f; t += 0.05f)
        {
            // Tính góc và vị trí tại điểm t
            float angle = arc.startAngle + (arc.endAngle - arc.startAngle) * t;
            float x = arc.x + cos(angle) * arc.radius * t;
            float y = arc.y + sin(angle) * arc.radius * t;

            // Kiểm tra khoảng cách đến player
            float dx = px - x;
            float dy = py - y;
            float dist = sqrt(dx * dx + dy * dy);

            // Nếu trúng player
            if (dist < 25 && damageCooldown == 0)
            {
                playerHP--;
                damageCooldown = 10;
                if (playerHP <= 0)
                    state = GAMEOVER;
                break;
            }
        }
    }

    // Xóa laser hết thời gian sống
    arcLasers.erase(
        std::remove_if(arcLasers.begin(), arcLasers.end(),
                       [](ArcLaser &a)
                       { return a.life <= 0; }),
        arcLasers.end());
}

// Vẽ laser vòng cung
void renderArcLasers(SDL_Renderer *r, vector<ArcLaser> &arcLasers)
{
    for (auto &arc : arcLasers)
    {
        // Vẽ đường cong bằng 20 đoạn thẳng nhỏ
        int segments = 20;

        // Điểm bắt đầu
        float prevX = arc.x + cos(arc.startAngle) * arc.radius * 0.1f;
        float prevY = arc.y + sin(arc.startAngle) * arc.radius * 0.1f;

        // Vẽ các đoạn thẳng nối tiếp nhau tạo thành đường cong
        for (int i = 1; i <= segments; i++)
        {
            float t = (float)i / segments;
            float angle = arc.startAngle + (arc.endAngle - arc.startAngle) * t;
            float x = arc.x + cos(angle) * arc.radius * t;
            float y = arc.y + sin(angle) * arc.radius * t;

            // Màu sắc thay đổi theo thời gian sống (cam đỏ)
            SDL_SetRenderDrawColor(r, 255, 100 + arc.life, 0, 200);
            SDL_RenderDrawLine(r, (int)prevX, (int)prevY, (int)x, (int)y);

            prevX = x;
            prevY = y;
        }

        // Vẽ hiệu ứng glow (3 lớp mờ xung quanh)
        for (int glow = 1; glow <= 3; glow++)
        {
            prevX = arc.x + cos(arc.startAngle) * arc.radius * 0.1f;
            prevY = arc.y + sin(arc.startAngle) * arc.radius * 0.1f;
            SDL_SetRenderDrawColor(r, 255, 50, 0, 40); // Màu đỏ cam mờ

            for (int i = 1; i <= segments; i++)
            {
                float t = (float)i / segments;
                float angle = arc.startAngle + (arc.endAngle - arc.startAngle) * t;
                float x = arc.x + cos(angle) * arc.radius * t;
                float y = arc.y + sin(angle) * arc.radius * t;
                SDL_RenderDrawLine(r, (int)prevX, (int)prevY, (int)x, (int)y);
                prevX = x;
                prevY = y;
            }
        }
    }
}