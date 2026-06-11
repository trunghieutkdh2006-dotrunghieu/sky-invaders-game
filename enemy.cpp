#include "enemy.h"
#include "bullet.h"
#include "powerup.h"
#include "gamestate.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <SDL2/SDL_mixer.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846 // Định nghĩa số PI nếu chưa có
#endif

extern Mix_Chunk *sfxExplode; // Âm thanh nổ (khai báo từ main.cpp)

// ============================================================
// HÀM TÍNH ĐỘ KHÓ THEO LEVEL
// ============================================================
// Đầu vào: level hiện tại (1, 2, 3, ...)
// Đầu ra: cấu trúc Difficulty chứa các thông số độ khó
Difficulty getDifficulty(int level)
{
    Difficulty d;

    // enemySpeed: tốc độ di chuyển của enemy
    // Công thức: 1.03^level (tăng 3% mỗi level)
    // Ví dụ: level 1 = 1.03, level 10 = 1.34, level 30 = 2.43
    d.enemySpeed = pow(1.03f, level);

    // fireRate: tần suất bắn (số frame giữa các lần bắn)
    // Công thức: max(40, 200 - level*8)
    // Level 1: 192, Level 10: 120, Level 20: 40, Level 25+: 40
    // Càng về sau, fireRate càng nhỏ → bắn càng nhanh
    d.fireRate = std::max(40.0f, 200.0f - level * 8);

    // enemyCount: số lượng enemy (không dùng trực tiếp, dùng ở hàm spawn)
    d.enemyCount = level * 5;

    // dodgePower: khả năng né đạn (càng cao càng né giỏi)
    // Công thức: 1.5 + level*0.15
    // Level 1: 1.65, Level 10: 3.0, Level 20: 4.5
    d.dodgePower = 1.5f + level * 0.15f;

    // bulletSpeed: tốc độ đạn của enemy
    // Công thức: 3.0 + level*0.2
    // Level 1: 3.2, Level 10: 5.0, Level 20: 7.0
    d.bulletSpeed = 3.0f + level * 0.2f;

    return d;
}

// ============================================================
// HÀM TÍNH MÁU ENEMY
// ============================================================
// Công thức đơn giản: 1 + level
// Level 1: 2 máu, Level 5: 6 máu, Level 10: 11 máu
float enemyHP(int level)
{
    return 1 + level;
}

// ============================================================
// GIỚI HẠN ENEMY TRONG MÀN HÌNH
// ============================================================
// Ngăn enemy đi ra ngoài màn hình
static void clampEnemy(Enemy &en, int W, int H)
{
    // Padding (khoảng cách từ mép màn hình)
    float padX = 60.0f; // Lề trái/phải 60px
    float padY = 80.0f; // Lề trên 80px

    float minX = padX;          // Giới hạn trái
    float maxX = W - padX;      // Giới hạn phải
    float minY = padY;          // Giới hạn trên
    float maxY = H - padY - 50; // Giới hạn dưới (trừ thêm 50 để tránh chui xuống đáy)

    // Clamp các giá trị (giữ trong khoảng min-max)
    en.x = std::clamp(en.x, minX, maxX);
    en.y = std::clamp(en.y, minY, maxY);
    en.centerX = std::clamp(en.centerX, minX, maxX);
    en.centerY = std::clamp(en.centerY, minY, maxY);
}

// ============================================================
// SPAWN ENEMY THEO LEVEL
// ============================================================
void spawn(vector<Enemy> &enemies, int lv, int W, int H)
{
    // Xóa enemy cũ
    enemies.clear();

    // Số enemy = level x 5 (bảng nhân 5)
    // Level 1: 5, Level 2: 10, Level 3: 15, Level 10: 50
    int enemyCount = lv * 5;

    // Giới hạn tối đa 50 enemy (tránh quá tải)
    if (enemyCount > 50)
        enemyCount = 50;

    // Tạo từng enemy
    for (int i = 0; i < enemyCount; i++)
    {
        Enemy en{}; // Khởi tạo tất cả giá trị về 0

        // ----- VỊ TRÍ SPAWN -----
        // Spawn trong vùng an toàn, tránh sát mép
        // x: 100 đến W-100, y: 50 đến H-200
        en.x = rand() % (W - 200) + 100;
        en.y = rand() % (H - 250) + 50;

        // Tâm dùng cho di chuyển orbit
        en.centerX = en.x;
        en.centerY = en.y;

        // Bán kính quỹ đạo (dùng cho MOVE_ORBIT)
        en.radius = 80.0f;

        // Góc khởi tạo ngẫu nhiên (0-360 độ)
        en.angle = (rand() % 360) * M_PI / 180.0f;

        // Tốc độ di chuyển từ hàm getDifficulty
        en.speed = getDifficulty(lv).enemySpeed;

        // Vận tốc ngẫu nhiên cho các kiểu di chuyển (dash, bounce, random)
        en.vx = ((rand() % 200) - 100) / 100.0f; // -1.0 đến 1.0
        en.vy = ((rand() % 200) - 100) / 100.0f;

        // Trạng thái dash
        en.timer = 0;       // Bộ đếm cho dash, teleport
        en.state = PREPARE; // Bắt đầu ở trạng thái chuẩn bị

        // ----- MÁU -----
        en.maxHP = enemyHP(lv) * 5; // Máu tối đa
        en.hp = en.maxHP;           // Máu hiện tại
        en.hpDisplay = en.hp;       // Máu hiển thị (hiệu ứng mượt)

        // Trạng thái chết
        en.dead = false;   // Chưa chết
        en.dying = false;  // Chưa trong trạng thái chết
        en.deathFrame = 0; // Số frame đã chết

        // Bắn đạn
        en.shootTimer = 0;    // Bộ đếm thời gian bắn
        en.teleportTimer = 0; // Bộ đếm teleport
        en.boss = false;      // Không phải boss (mặc định)
        en.shootCooldown = 0; // Cooldown giữa các lần bắn

        // ----- PHÂN LOẠI ENEMY -----
        // Random 0-99 để quyết định loại enemy
        int typeRand = rand() % 100;

        // BOSS: xuất hiện từ level 3, là enemy cuối cùng
        if (lv >= 3 && i == enemyCount - 1)
        {
            en.type = 6;                 // Loại boss
            en.boss = true;              // Đánh dấu là boss
            en.maxHP = enemyHP(lv) * 20; // Máu gấp 20 lần
            en.hp = en.maxHP;
            en.speed = 1.0f;          // Tốc độ chậm
            en.moveType = MOVE_ORBIT; // Di chuyển theo quỹ đạo
        }
        // ORBIT (40%): di chuyển theo đường tròn
        else if (typeRand < 40)
        {
            en.type = 0;
            en.moveType = MOVE_ORBIT;
        }
        // ZIGZAG (25%): di chuyển ngoằn ngoèo
        else if (typeRand < 65)
        {
            en.type = 1;
            en.moveType = MOVE_ZIGZAG;
        }
        // SWARM (15%): di chuyển theo bầy đàn
        else if (typeRand < 80)
        {
            en.type = 2;
            en.moveType = MOVE_SWARM;
        }
        // TELEPORT (10%): dịch chuyển tức thời
        else if (typeRand < 90)
        {
            en.type = 4;
            en.moveType = MOVE_TELEPORT;
        }
        // PURSUE (10%): đuổi theo player
        else
        {
            en.type = 5;
            en.moveType = MOVE_PURSUE;
        }

        // Thêm vào danh sách
        enemies.push_back(en);
    }
}

// ============================================================
// DI CHUYỂN THEO QUỸ ĐẠO TRÒN (ORBIT)
// ============================================================
// Enemy xoay quanh một tâm
void moveOrbit(Enemy &en, int W, int H)
{
    // Tăng góc lên 0.03 radian (~1.7 độ) mỗi frame
    en.angle -= 0.03f;

    // Giới hạn tâm trong màn hình
    float pad = 100.0f;
    en.centerX = std::clamp(en.centerX, pad, (float)W - pad);
    en.centerY = std::clamp(en.centerY, pad, (float)H - pad - 50);

    // Tính vị trí mới: tâm + (cos góc * bán kính, sin góc * bán kính)
    en.x = en.centerX + cos(en.angle) * en.radius;
    en.y = en.centerY + sin(en.angle) * en.radius;

    // Đảm bảo không ra ngoài màn hình
    clampEnemy(en, W, H);
}

// ============================================================
// DI CHUYỂN NGOẰN NGOÈO (ZIGZAG)
// ============================================================
// Di chuyển sang phải, lắc lư lên xuống theo hình sin
void moveZigzag(Enemy &en, int W, int H)
{
    // Di chuyển sang phải theo tốc độ
    en.x += en.speed;
    // Lắc lư theo sin(vị trí x) * 4
    en.y += sin(en.x * 0.05f) * 4.0f;
    clampEnemy(en, W, H);
}

// ============================================================
// DI CHUYỂN THEO HÌNH SIN (SINE)
// ============================================================
void moveSine(Enemy &en, int W, int H)
{
    // Di chuyển sang phải
    en.x += en.speed;
    // Lên xuống theo sin(thời gian)
    en.y = en.centerY + sin(SDL_GetTicks() * 0.01f) * 80;
    clampEnemy(en, W, H);
}

// ============================================================
// DI CHUYỂN THEO ĐƯỜNG XOẮN ỐC (SPIRAL)
// ============================================================
// Xoay quanh player và thu nhỏ bán kính dần
void moveSpiral(Enemy &en, float px, float py, int W, int H)
{
    // Tăng góc 0.05 rad mỗi frame
    en.angle -= 0.05f;
    // Giảm bán kính 0.2px mỗi frame
    en.radius -= 0.2f;
    // Nếu bán kính quá nhỏ, đặt lại
    if (en.radius < 20)
        en.radius = 80;

    // Vị trí mới: xoay quanh player
    en.x = px + cos(en.angle) * en.radius;
    en.y = py + sin(en.angle) * en.radius;
    clampEnemy(en, W, H);
}

// ============================================================
// LAO VỀ PHÍA PLAYER (DASH)
// ============================================================
// Gồm 3 pha: Chuẩn bị -> Lao -> Hồi phục
void moveDash(Enemy &en, float px, float py, int W, int H)
{
    en.timer++; // Tăng bộ đếm mỗi frame

    // PHA 1: CHUẨN BỊ (PREPARE)
    // Enemy đứng yên trong 60 frame
    if (en.state == PREPARE)
    {
        // Sau 60 frame, tính hướng về player
        if (en.timer > 60)
        {
            // Vector từ enemy đến player
            float dx = px - en.x;
            float dy = py - en.y;
            float len = sqrt(dx * dx + dy * dy);
            if (len < 0.001f)
                len = 1;

            // Đặt vận tốc = hướng * 12 (lao rất nhanh)
            en.vx = (dx / len) * 12.0f;
            en.vy = (dy / len) * 12.0f;

            // Chuyển sang pha lao
            en.state = DASH;
            en.timer = 0;
        }
    }
    // PHA 2: LAO (DASH)
    else if (en.state == DASH)
    {
        // Di chuyển theo vận tốc đã tính
        en.x += en.vx;
        en.y += en.vy;
        clampEnemy(en, W, H);

        // Sau 20 frame, chuyển sang pha hồi phục
        if (en.timer > 20)
        {
            en.state = RECOVER;
            en.timer = 0;
        }
    }
    // PHA 3: HỒI PHỤC (RECOVER)
    else if (en.state == RECOVER)
    {
        // Sau 40 frame, quay lại pha chuẩn bị
        if (en.timer > 40)
        {
            en.state = PREPARE;
            en.timer = 0;
        }
    }
}

// ============================================================
// DI CHUYỂN NGẪU NHIÊN (RANDOM)
// ============================================================
void moveRandom(Enemy &en)
{
    en.timer--; // Giảm timer

    // Khi timer <= 0, đổi hướng ngẫu nhiên
    if (en.timer <= 0)
    {
        // Vận tốc ngẫu nhiên từ -1.0 đến 1.0
        en.vx = (rand() % 200 - 100) / 100.0f;
        en.vy = (rand() % 200 - 100) / 100.0f;
        en.timer = 60; // Reset timer
    }

    // Di chuyển
    en.x += en.vx * 2;
    en.y += en.vy * 2;
}

// ============================================================
// DI CHUYỂN THEO BẦY ĐÀN (SWARM)
// ============================================================
// Enemy tránh xa nhau để không chồng lên
void moveSwarm(Enemy &en, vector<Enemy> &enemies)
{
    float sepX = 0, sepY = 0;

    // Duyệt tất cả enemy khác
    for (auto &other : enemies)
    {
        if (&other == &en)
            continue; // Bỏ qua chính nó

        // Tính khoảng cách
        float dx = en.x - other.x;
        float dy = en.y - other.y;
        float dist = sqrt(dx * dx + dy * dy);

        // Nếu quá gần (< 40px) thì đẩy ra xa
        if (dist < 40 && dist > 0)
        {
            sepX += dx / dist; // Lực đẩy theo phương X
            sepY += dy / dist; // Lực đẩy theo phương Y
        }
    }

    // Áp dụng lực đẩy
    en.x += sepX * 2;
    en.y += sepY * 2;
}

// ============================================================
// CHẠY TRỐN KHỎI PLAYER (FLEE)
// ============================================================
void moveFlee(Enemy &en, float px, float py)
{
    // Vector từ enemy đến player (hướng ngược lại để chạy trốn)
    float dx = en.x - px;
    float dy = en.y - py;
    float len = sqrt(dx * dx + dy * dy);
    if (len < 0.001f)
        return;

    // Di chuyển ra xa player
    en.x += (dx / len) * 3;
    en.y += (dy / len) * 3;
}

// ============================================================
// ĐUỔI THEO PLAYER (PURSUE) - CÓ DỰ ĐOÁN
// ============================================================
// Dự đoán vị trí player trong tương lai (30 frame sau)
void movePursue(Enemy &en, float px, float py, float pvx, float pvy)
{
    float predict = 30; // Dự đoán 30 frame sau

    // Vị trí player trong tương lai
    float futureX = px + pvx * predict;
    float futureY = py + pvy * predict;

    // Hướng từ enemy đến vị trí tương lai
    float dx = futureX - en.x;
    float dy = futureY - en.y;
    float len = sqrt(dx * dx + dy * dy);
    if (len < 0.001f)
        return;

    // Di chuyển về phía vị trí dự đoán
    en.x += (dx / len) * 4;
    en.y += (dy / len) * 4;
}

// ============================================================
// DI CHUYỂN THEO SÓNG (WAVE)
// ============================================================
void moveWave(Enemy &en)
{
    // Lắc lư theo cos(thời gian)
    en.x += cos(SDL_GetTicks() * 0.01f) * 3;
    // Đi xuống dần
    en.y += en.speed;
}

// ============================================================
// DỊCH CHUYỂN TỨC THỜI (TELEPORT)
// ============================================================
void moveTeleport(Enemy &en, int W, int H)
{
    en.timer++;

    // Mỗi 120 frame (2 giây), teleport đến vị trí mới
    if (en.timer > 120)
    {
        int padX = 60;
        int padY = 80;
        // Vị trí ngẫu nhiên trong màn hình
        en.x = rand() % (W - padX * 2) + padX;
        en.y = rand() % (H - padY * 2 - 100) + padY;
        en.timer = 0;
    }
    clampEnemy(en, W, H);
}

// ============================================================
// NẢY KHI CHẠM TƯỜNG (BOUNCE)
// ============================================================
void moveBounce(Enemy &en, int W, int H)
{
    en.x += en.vx;
    en.y += en.vy;

    // Nếu chạm tường trái/phải, đảo ngược vận tốc X
    if (en.x < 0 || en.x > W)
        en.vx *= -1;

    // Nếu chạm tường trên/dưới, đảo ngược vận tốc Y
    if (en.y < 0 || en.y > H)
        en.vy *= -1;
}

// ============================================================
// XOAY QUANH PLAYER (CIRCLE PLAYER)
// ============================================================
void moveCirclePlayer(Enemy &en, float px, float py)
{
    // Tăng góc 0.04 rad mỗi frame
    en.angle -= 0.04f;
    // Xoay quanh player với bán kính 150px
    en.x = px + cos(en.angle) * 150;
    en.y = py + sin(en.angle) * 150;
}

// ============================================================
// DI CHUYỂN NHƯ RẮN (SNAKE)
// ============================================================
void moveSnake(Enemy &en)
{
    // Di chuyển theo hình sin
    en.x += cos(en.angle) * 4;
    en.y += sin(en.angle * 3) * 5;
    en.angle -= 0.05f;
}

// ============================================================
// BOSS THAY ĐỔI CÁCH DI CHUYỂN THEO % MÁU
// ============================================================
void moveBoss(Enemy &en, float px, float py, int W, int H)
{
    float hpRatio = en.hp / en.maxHP; // Tỷ lệ máu còn lại

    // Máu > 70%: di chuyển orbit
    if (hpRatio > 0.7f)
        moveOrbit(en, W, H);
    // Máu 40% - 70%: dash về phía player
    else if (hpRatio > 0.4f)
        moveDash(en, px, py, W, H);
    // Máu < 40%: teleport liên tục
    else
        moveTeleport(en, W, H);
}

// ============================================================
// ĐIỀU KHIỂN DI CHUYỂN (SWITCH CASE)
// ============================================================
void updateMovement(Enemy &en, vector<Enemy> &enemies, float px, float py,
                    float pvx, float pvy, int W, int H)
{
    // Gọi hàm di chuyển phù hợp với moveType
    switch (en.moveType)
    {
    case MOVE_ORBIT:
        moveOrbit(en, W, H);
        break;
    case MOVE_ZIGZAG:
        moveZigzag(en, W, H);
        break;
    case MOVE_SINE:
        moveSine(en, W, H);
        break;
    case MOVE_SPIRAL:
        moveSpiral(en, px, py, W, H);
        break;
    case MOVE_DASH:
        moveDash(en, px, py, W, H);
        break;
    case MOVE_RANDOM:
        moveRandom(en);
        break;
    case MOVE_SWARM:
        moveSwarm(en, enemies);
        break;
    case MOVE_FLEE:
        moveFlee(en, px, py);
        break;
    case MOVE_PURSUE:
        movePursue(en, px, py, pvx, pvy);
        break;
    case MOVE_WAVE:
        moveWave(en);
        break;
    case MOVE_TELEPORT:
        moveTeleport(en, W, H);
        break;
    case MOVE_BOUNCE:
        moveBounce(en, W, H);
        break;
    case MOVE_CIRCLE_PLAYER:
        moveCirclePlayer(en, px, py);
        break;
    case MOVE_SNAKE:
        moveSnake(en);
        break;
    }
}

// ============================================================
// CẬP NHẬT ENEMY MỖI FRAME (QUAN TRỌNG NHẤT)
// ============================================================
void updateEnemies(
    vector<Enemy> &enemies,
    vector<Bullet> &bullets,
    vector<Particle> &particles,
    vector<FloatingText> &texts,
    vector<HealthDrop> &healthDrops,
    vector<PowerUp> &powerUps,
    vector<Laser> &lasers,
    vector<ArcLaser> &arcLasers,
    float px, float py,
    int level, int bulletPower,
    float formationTime,
    int &score, int &hitStopFrames,
    int W, int H)
{
    // Duyệt từng enemy
    for (auto &en : enemies)
    {
        // ----- XỬ LÝ ENEMY ĐANG CHẾT (HIỆU ỨNG) -----
        if (en.dying)
        {
            en.deathFrame++;        // Tăng số frame đã chết
            if (en.deathFrame > 10) // Sau 10 frame thì xóa hẳn
                en.dead = true;
            continue; // Bỏ qua các xử lý khác
        }

        // ----- ENEMY LOẠI HEALER (TYPE 2) -----
        // Healer có thể hồi máu cho đồng đội xung quanh
        if (en.type == 2)
        {
            for (auto &other : enemies)
            {
                if (&other == &en)
                    continue; // Không hồi máu cho chính mình

                // Tính khoảng cách đến enemy khác
                float dx = other.x - en.x;
                float dy = other.y - en.y;
                float dist = sqrt(dx * dx + dy * dy);

                // Nếu trong phạm vi 140px
                if (dist < 140 && other.hp > 0)
                {
                    // Mỗi 15 frame hồi 0.2 máu
                    if (SDL_GetTicks() % 15 == 0)
                    {
                        other.hp += 0.2f;
                        if (other.hp > other.maxHP)
                            other.hp = other.maxHP; // Không vượt quá max
                    }
                }
            }
        }

        // ----- GÓC XOAY -----
        // Boss xoay nhanh hơn (0.05 rad/frame), thường 0.03 rad/frame
        en.angle -= en.boss ? 0.05f : 0.03f;

        // ----- TRÁNH XA PLAYER (KHÔNG CHO ENEMY ĐẾN QUÁ GẦN) -----
        // Tính vector từ enemy đến player
        float dx = en.x - px;
        float dy = en.y - py;
        float len = sqrt(dx * dx + dy * dy);
        if (len == 0)
            len = 1;

        // Hướng tránh xa (ngược chiều với player)
        float avoidX = dx / len;
        float avoidY = dy / len;

        // ----- TỐC ĐỘ DI CHUYỂN -----
        float speed = en.boss ? 1.5f : 1.0f;

        // Enemy type 3: càng yếu càng nhanh (tăng tốc khi máu thấp)
        if (en.type == 3)
        {
            float hpRatio = en.hp / 10.0f;
            speed += (1.0f - hpRatio) * 3.0f; // Máu càng thấp, speed càng cao
        }

        float finalSpeed = speed;
        if (en.type == 2)
            finalSpeed *= 0.6f; // Healer chậm hơn
        if (en.type == 3)
            finalSpeed *= 1.8f; // Type 3 nhanh hơn

        // Áp dụng di chuyển tránh xa
        en.x += avoidX * finalSpeed;
        en.y += avoidY * finalSpeed;

        // Gọi hàm di chuyển theo loại enemy
        updateMovement(en, enemies, px, py, 0.0f, 0.0f, W, H);
        clampEnemy(en, W, H);

        // ----- AI NÉ ĐẠN CỦA PLAYER -----
        float dodgeX = 0, dodgeY = 0;

        // Duyệt tất cả đạn
        for (auto &b : bullets)
        {
            // Chỉ xét đạn của player (không phải enemy)
            if (!b.enemy)
            {
                // Tính vector từ đạn đến enemy
                float bdx = en.x - b.x;
                float bdy = en.y - b.y;
                float dist = sqrt(bdx * bdx + bdy * bdy);

                // Nếu đạn ở gần (trong 120px)
                if (dist < 120 && dist > 0)
                {
                    // Lực né: càng gần càng mạnh
                    dodgeX += bdx / dist;
                    dodgeY += bdy / dist;
                }
            }
        }

        // Áp dụng lực né đạn
        float lenD = sqrt(dodgeX * dodgeX + dodgeY * dodgeY);
        if (lenD > 0)
        {
            en.x += (dodgeX / lenD) * 2.2f;
            en.y += (dodgeY / lenD) * 2.2f;
        }
        clampEnemy(en, W, H);

        // ----- BẮN ĐẠN -----

        // Nếu là BOSS: bắn laser vòng cung
        if (en.boss)
        {
            en.shootTimer++;                       // Tăng bộ đếm
            int delay = std::max(60, 120 - level); // Delay càng về sau càng nhỏ
            if (en.shootTimer > delay)
            {
                shootArcLaser(arcLasers, en, px, py, level); // Bắn laser vòng cung
                en.shootTimer = 0;
            }
        }
        // Nếu là ENEMY THƯỜNG: bắn đạn tròn
        else
        {
            // Công thức tần suất bắn: giảm dần theo level
            // Level 1: 218, Level 10: 200, Level 20: 180, Level 30: 160
            int rate = 220 - (level * 2);
            if (rate < 60)
                rate = 60; // Tối thiểu 60 frame

            // Kiểm tra cooldown của enemy này
            if (en.shootCooldown <= 0)
            {
                // Random theo rate (rate càng nhỏ, xác suất bắn càng cao)
                if (rand() % rate == 0)
                {
                    // Tính hướng về player
                    float bdx = px - en.x;
                    float bdy = py - en.y;
                    float blen = sqrt(bdx * bdx + bdy * bdy);
                    if (blen == 0)
                        blen = 1;

                    // Tốc độ đạn tăng dần theo level
                    float bulletSpeed = 2.5f + (level * 0.03f);
                    if (bulletSpeed > 4.5f)
                        bulletSpeed = 4.5f;

                    // Tạo đạn mới
                    bullets.push_back({en.x, en.y,
                                       (bdx / blen) * bulletSpeed,
                                       (bdy / blen) * bulletSpeed,
                                       true, // enemy = true (đạn của enemy)
                                       1});

                    // Cooldown 15 frame trước khi bắn tiếp
                    en.shootCooldown = 15;
                }
            }
            else
            {
                en.shootCooldown--; // Giảm cooldown mỗi frame
            }
        }

        // ----- ĐẠN PLAYER BẮN TRÚNG ENEMY -----
        for (auto &b : bullets)
        {
            // Chỉ xét đạn của player
            if (!b.enemy)
            {
                // Tính khoảng cách từ đạn đến enemy
                float bdx = en.x - b.x;
                float bdy = en.y - b.y;
                float dist = sqrt(bdx * bdx + bdy * bdy);

                // Nếu khoảng cách < 30px (trúng đạn)
                if (dist < 30 && !en.dead)
                {
                    // Tính kích thước enemy
                    int size = en.boss ? (120 + level * 5) : (40 + level * 3);
                    float cx = en.x + size / 2; // Tâm X
                    float cy = en.y + size / 2; // Tâm Y
                    float dx2 = cx - b.x;
                    float dy2 = cy - b.y;
                    float headDist = sqrt(dx2 * dx2 + dy2 * dy2);

                    // HEADSHOT: bắn trúng đầu (trong 1/4 kích thước)
                    if (headDist < size * 0.25f)
                    {
                        float halfHP = en.maxHP / 2.0f;
                        en.hp -= halfHP; // Sát thương gấp đôi

                        // Chữ headshot thay đổi theo level
                        string critText;
                        switch (level)
                        {
                        case 1:
                            critText = "HEADSHOT!";
                            break;
                        case 2:
                            critText = "CRITICAL!";
                            break;
                        case 3:
                            critText = "PERFECT!";
                            break;
                        case 4:
                            critText = "SMASH!";
                            break;
                        case 5:
                            critText = "GODLIKE!";
                            break;
                        default:
                            // Level 6+: random 5 loại chữ
                            string words[] = {"HEADSHOT!", "CRITICAL!", "PERFECT!", "SMASH!", "GODLIKE!"};
                            critText = words[rand() % 5];
                            break;
                        }
                        texts.push_back({en.x, en.y, critText, 40}); // Chữ bay lên
                    }
                    else
                    {
                        en.hp -= 1 * bulletPower; // Sát thương thường
                    }

                    // ----- ENEMY CHẾT -----
                    if (en.hp <= 0 && !en.dying)
                    {
                        en.dying = true;   // Đánh dấu đang chết
                        en.deathFrame = 0; // Reset frame chết
                        hitStopFrames = 8; // Dừng màn hình 8 frame

                        // Rơi vật phẩm hồi máu (trái tim)
                        healthDrops.push_back({en.x, en.y, 1.2f, 1, true, false, 300, 0});

                        // 20% cơ hội rơi powerup
                        if (rand() % 5 == 0)
                            spawnPowerUp(powerUps);

                        // Cộng điểm (boss không cộng - đã set = 0)
                        score += en.boss ? 0 : 10;

                        // Tạo hiệu ứng nổ
                        spawnExplosion(particles, en.x, en.y);

                        // Phát âm thanh nổ
                        if (sfxExplode)
                            Mix_PlayChannel(-1, sfxExplode, 0);
                    }

                    // Xóa viên đạn (đặt tọa độ ra ngoài)
                    b.x = -9999;
                    break; // Thoát khỏi vòng lặp đạn
                }
            }
        }
    }

    // ----- XÓA ENEMY ĐÃ CHẾT (dead = true) -----
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
                       [](Enemy &e)
                       { return e.dead; }),
        enemies.end());
}