// ========================== main.cpp ==========================
// GAME INVADERS - Chương trình chính
// Tác giả: [Your Name]
// Mô tả: Game bắn súng góc nhìn từ trên xuống, điều khiển tàu chiến bắn quái vật

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

#include "types.h"
#include "render.h"
#include "enemy.h"
#include "bullet.h"
#include "powerup.h"
#include "audio.h"
#include "gamestate.h"

using namespace std;

// ============================================================
// BIẾN TOÀN CỤC (GLOBAL VARIABLES)
// ============================================================

int W, H;                             // Chiều rộng và chiều cao màn hình
TTF_Font *font;                       // Font chữ để vẽ text
Mix_Chunk *sfxExplode = nullptr;      // Âm thanh nổ khi enemy chết
Mix_Music *backgroundMusic = nullptr; // Nhạc nền

float restartZoom = 1.0f;        // Hiệu ứng zoom khi restart
float lowHpFlash = 0.0f;         // Hiệu ứng nhấp nháy khi máu thấp
float formationTime = 0.0f;      // Thời gian formation (hiệu ứng)
int damageCooldown = 0;          // Cooldown nhận sát thương (miễn nhiễm)
int playerShootCooldown = 0;     // Cooldown bắn đạn của player
bool restarting = false;         // Cờ hiệu ứng restart
GameMode currentMode = HARDCORE; // Chế độ chơi hiện tại

// ================= HIỆU ỨNG SAO BAY =================
struct Star
{
    float x, y;  // Vị trí
    float speed; // Tốc độ rơi
    int size;    // Kích thước
};

vector<Star> starsFar;  // Sao ở xa (nhỏ, chậm)
vector<Star> starsNear; // Sao ở gần (to, nhanh)

// ============================================================
// HÀM MAIN - CHƯƠNG TRÌNH CHÍNH
// ============================================================
int main()
{
    // ============================================================
    // KHỞI TẠO SDL VÀ CÁC THƯ VIỆN
    // ============================================================

    // Khởi tạo SDL (video và audio)
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG);                            // Khởi tạo SDL_image (đọc file PNG)
    TTF_Init();                                        // Khởi tạo SDL_ttf (font chữ)
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048); // Khởi tạo âm thanh

    // Tạo cửa sổ game (fullscreen)
    SDL_Window *win = SDL_CreateWindow(
        "🔥 INVADERS 🔥",               // Tiêu đề
        SDL_WINDOWPOS_CENTERED,         // Vị trí X (giữa màn hình)
        SDL_WINDOWPOS_CENTERED,         // Vị trí Y (giữa màn hình)
        0, 0,                           // Kích thước (0 = tự động)
        SDL_WINDOW_FULLSCREEN_DESKTOP); // Chế độ fullscreen desktop

    // Tạo renderer (dùng để vẽ)
    SDL_Renderer *r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    // Lấy kích thước màn hình
    SDL_GetRendererOutputSize(r, &W, &H);

    // Khởi tạo random seed
    srand(SDL_GetTicks());

    // ============================================================
    // TẠO HIỆU ỨNG SAO
    // ============================================================

    // Sao xa (180 ngôi sao nhỏ, rơi chậm)
    for (int i = 0; i < 180; i++)
    {
        Star s;
        s.x = rand() % W;
        s.y = rand() % H;
        s.speed = 0.5f + (rand() % 100) / 100.0f; // 0.5 - 1.5
        s.size = 1;
        starsFar.push_back(s);
    }

    // Sao gần (120 ngôi sao to, rơi nhanh)
    for (int i = 0; i < 120; i++)
    {
        Star s;
        s.x = rand() % W;
        s.y = rand() % H;
        s.speed = 2.0f + (rand() % 200) / 100.0f; // 2.0 - 4.0
        s.size = 2 + rand() % 2;                  // 2 hoặc 3 pixel
        starsNear.push_back(s);
    }

    // ============================================================
    // TẢI FONT
    // ============================================================
    font = TTF_OpenFont("/System/Library/Fonts/Supplemental/Arial.ttf", 24);
    if (!font)
        return -1; // Thoát nếu không có font

    // ============================================================
    // TẢI TEXTURE (HÌNH ẢNH)
    // ============================================================
    SDL_Texture *playerTex = IMG_LoadTexture(r, "assets/player.png"); // Tàu player
    SDL_Texture *enemyTex1 = IMG_LoadTexture(r, "assets/enemy1.png"); // Enemy thường
    SDL_Texture *enemyTex2 = IMG_LoadTexture(r, "assets/enemy2.png"); // Boss
    SDL_Texture *heartTex = IMG_LoadTexture(r, "assets/heart.png");   // Trái tim hồi máu

    // ============================================================
    // TẢI ÂM THANH
    // ============================================================
    sfxExplode = Mix_LoadWAV("assets/shoot.wav");    // Âm thanh nổ
    backgroundMusic = Mix_LoadMUS("assets/bgm.mp3"); // Nhạc nền

    // Kiểm tra load nhạc
    if (!backgroundMusic)
    {
        printf("Lỗi load nhạc: %s\n", Mix_GetError());
    }
    else
    {
        printf("Đã load nhạc thành công!\n");
        Mix_VolumeMusic(MIX_MAX_VOLUME); // Đặt âm lượng max
    }

    // Kiểm tra texture (nếu thiếu thì thoát)
    if (!playerTex || !enemyTex1 || !enemyTex2)
        return -1;

    // ============================================================
    // KHỞI TẠO THÔNG SỐ GAME
    // ============================================================
    float px = 400;           // Vị trí X của player
    float py = 520;           // Vị trí Y của player
    float playerAngle = 0.0f; // Góc xoay của player (khi di chuyển ngang)

    int level = 1;     // Cấp độ hiện tại
    int score = 0;     // Điểm số
    int playerHP = 10; // Máu player (tối đa 10)

    float fireRate = 1.0f;  // Tốc độ bắn (chưa dùng)
    int bulletPower = 1;    // Sức mạnh đạn (mặc định = 1)
    bool hasShield = false; // Có khiên bảo vệ?
    bool hasLaser = false;  // Có laser? (chưa dùng)

    int dashCooldown = 0;   // Cooldown dash (giây)
    int dashTime = 0;       // Thời gian dash còn lại
    bool isDashing = false; // Đang dash?

    // ============================================================
    // CÁC VECTOR CONTAINER (CHỨA CÁC ĐỐI TƯỢNG TRONG GAME)
    // ============================================================
    vector<Bullet> bullets;         // Đạn
    vector<Enemy> enemies;          // Enemy
    vector<Particle> particles;     // Hiệu ứng hạt (nổ)
    vector<FloatingText> texts;     // Chữ bay lên (HEADSHOT, LEVEL UP...)
    vector<PowerUp> powerUps;       // Vật phẩm sức mạnh
    vector<HealthDrop> healthDrops; // Trái tim hồi máu
    vector<Laser> lasers;           // Laser thẳng
    vector<ArcLaser> arcLasers;     // Laser vòng cung (boss)

    // ============================================================
    // TẠO CÁC NÚT BẤM (BUTTON)
    // ============================================================
    Button btnStart = {{0, 250, 200, 50}, "START", false};
    Button btnGuide = {{0, 320, 200, 50}, "GUIDE", false};
    Button btnExit = {{0, 390, 200, 50}, "EXIT", false};
    Button btnResume = {{0, 250, 200, 50}, "RESUME", false};
    Button btnMute = {{0, 320, 200, 50}, "MUTE: ON", false};
    Button btnBack = {{0, 390, 200, 50}, "BACK MENU", false};

    // Hàm căn giữa nút bấm
    auto centerBtn = [&](Button &b)
    { b.rect.x = (W - b.rect.w) / 2; };
    centerBtn(btnStart);
    centerBtn(btnGuide);
    centerBtn(btnExit);
    centerBtn(btnResume);
    centerBtn(btnMute);
    centerBtn(btnBack);

    // Spawn enemy cho level 1
    spawn(enemies, level, W, H);

    // ============================================================
    // VÒNG LẶP CHÍNH CỦA GAME (GAME LOOP)
    // ============================================================
    bool run = true;
    while (run)
    {
        SDL_Event e;

        // ============================================================
        // XỬ LÝ SỰ KIỆN (EVENT HANDLING)
        // ============================================================
        while (SDL_PollEvent(&e))
        {
            // Thoát game khi đóng cửa sổ
            if (e.type == SDL_QUIT)
                run = false;

            // ===== XỬ LÝ CHUỘT =====
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                // MENU: START, GUIDE, EXIT
                if (state == MENU)
                {
                    if (btnStart.hover)
                        startTransition(PLAYING);
                    if (btnGuide.hover)
                        startTransition(INSTRUCTION);
                    if (btnExit.hover)
                        run = false;
                }
                // PAUSED: RESUME, MUTE, BACK
                if (state == PAUSED)
                {
                    if (btnResume.hover)
                        state = PLAYING;
                    if (btnMute.hover)
                    {
                        toggleMute();
                        btnMute.text = muted ? "MUTE: OFF" : "MUTE: ON";
                    }
                    if (btnBack.hover)
                        state = MENU;
                }
            }

            // ===== XỬ LÝ BÀN PHÍM =====
            if (e.type == SDL_KEYDOWN)
            {
                SDL_Keycode key = e.key.keysym.sym;

                // ESC: Pause / Resume / Back
                if (key == SDLK_ESCAPE)
                {
                    if (state == PLAYING)
                        state = PAUSED;
                    else if (state == PAUSED)
                        state = PLAYING;
                    else if (state == INSTRUCTION)
                        startTransition(MENU);
                    else if (state == GAMEOVER)
                        startTransition(MENU);
                }

                // SHIFT: Dash (tăng tốc)
                if (key == SDLK_LSHIFT && state == PLAYING && dashCooldown <= 0)
                {
                    isDashing = true;
                    dashTime = 10;
                    dashCooldown = 60;
                }

                // R: Restart khi game over
                if (state == GAMEOVER && key == SDLK_r)
                {
                    playerHP = 10;
                    score = 0;
                    level = 1;
                    px = 400;
                    py = 520;
                    bullets.clear();
                    enemies.clear();
                    particles.clear();
                    texts.clear();
                    powerUps.clear();
                    healthDrops.clear();
                    lasers.clear();
                    arcLasers.clear();
                    spawn(enemies, level, W, H);
                    state = PLAYING;
                }

                // SPACE: Bắn đạn
                if (state == PLAYING && key == SDLK_SPACE)
                {
                    bullets.push_back({px + 20, py, 0, -10, false, 60});
                }
            }
        }

        // ============================================================
        // CẬP NHẬT HOVER CHO NÚT BẤM
        // ============================================================
        if (state == MENU || state == PAUSED)
        {
            updateButton(btnStart);
            updateButton(btnGuide);
            updateButton(btnExit);
            updateButton(btnResume);
            updateButton(btnMute);
            updateButton(btnBack);
        }

        // ============================================================
        // CẬP NHẬT GAME (UPDATE)
        // ============================================================
        formationTime += 0.02f;
        updateFade(); // Cập nhật hiệu ứng chuyển màn

        // ===== GAME UPDATE CHỈ KHI ĐANG CHƠI VÀ KHÔNG HITSTOP =====
        if (state == PLAYING && hitStopFrames <= 0)
        {
            // Phát nhạc nền nếu chưa phát
            playBGMIfStopped();

            // ----- DI CHUYỂN PLAYER -----
            float vx = 0, vy = 0, targetAngle = 0;
            const Uint8 *k = SDL_GetKeyboardState(NULL);

            if (k[SDL_SCANCODE_LEFT] || k[SDL_SCANCODE_A])
            {
                vx -= 6;
                targetAngle = -25;
            }
            if (k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D])
            {
                vx += 6;
                targetAngle = 25;
            }
            if (k[SDL_SCANCODE_UP] || k[SDL_SCANCODE_W])
                vy -= 6;
            if (k[SDL_SCANCODE_DOWN] || k[SDL_SCANCODE_S])
                vy += 6;

            // Dash: tăng tốc gấp 2.5 lần
            if (isDashing && dashTime > 0)
            {
                vx *= 2.5f;
                vy *= 2.5f;
                dashTime--;
                if (dashTime <= 0)
                    isDashing = false;
            }

            // Cập nhật vị trí
            px += vx;
            py += vy;
            playerAngle += (targetAngle - playerAngle) * 0.12f;
            px = max(0.0f, min(px, (float)(W - 60)));
            py = max(0.0f, min(py, (float)(H - 60)));

            if (dashCooldown > 0)
                dashCooldown--;
            if (damageCooldown > 0)
                damageCooldown--;

            // ----- HIỆU ỨNG MÁU THẤP -----
            if (playerHP <= 1)
            {
                lowHpFlash += 0.15f;
                if (lowHpFlash > 6.28f)
                    lowHpFlash = 0.0f;
            }

            // ----- CẬP NHẬT CÁC HỆ THỐNG -----
            checkPowerUpCollision(px, py, powerUps, playerHP, fireRate, bulletPower, hasShield, hasLaser);
            updateBullets(bullets);
            updateEnemies(enemies, bullets, particles, texts, healthDrops, powerUps, lasers, arcLasers,
                          px, py, level, bulletPower, formationTime, score, hitStopFrames, W, H);
            updateHealthDrops(healthDrops, px, py, playerHP, H);
            updateLasers(lasers, px, py, playerHP, damageCooldown, state);
            updateArcLasers(arcLasers, px, py, playerHP, damageCooldown, state);

            // ----- ĐẠN ENEMY BẮN TRÚNG PLAYER -----
            for (auto &b : bullets)
            {
                if (b.enemy)
                {
                    float dx = b.x - px, dy = b.y - py;
                    if (dx * dx + dy * dy < 625 && damageCooldown == 0)
                    {
                        if (hasShield)
                        {
                            hasShield = false;
                            damageCooldown = 10;
                        }
                        else
                        {
                            playerHP--;
                            damageCooldown = 15;
                        }
                        b.life = 0;
                        if (playerHP <= 0)
                            state = GAMEOVER;
                    }
                }
            }

            // ----- CẬP NHẬT PARTICLE -----
            for (auto &p : particles)
            {
                p.x += p.vx;
                p.y += p.vy;
                p.life--;
            }
            particles.erase(remove_if(particles.begin(), particles.end(),
                                      [](Particle &p)
                                      { return p.life <= 0; }),
                            particles.end());

            // ----- CẬP NHẬT FLOATING TEXT -----
            for (auto &t : texts)
            {
                t.y -= 1.5f;
                t.life--;
            }
            texts.erase(remove_if(texts.begin(), texts.end(),
                                  [](FloatingText &t)
                                  { return t.life <= 0; }),
                        texts.end());

            // ----- LEVEL UP (KHI HẾT ENEMY) -----
            if (enemies.empty())
            {
                level++;
                playerHP = 10;
                int bonusScore = level * 100;
                score += bonusScore;
                texts.push_back({(float)(W / 2 - 100), (float)(H / 2), "LEVEL " + to_string(level), 60});
                texts.push_back({(float)(W / 2 - 80), (float)(H / 2 + 50), "+" + to_string(bonusScore) + " SCORE", 60});
                spawn(enemies, level, W, H);
            }
        }

        // Giảm hitstop mỗi frame
        if (hitStopFrames > 0)
            hitStopFrames--;

        // ============================================================
        // RENDER (VẼ MÀN HÌNH)
        // ============================================================

        // ----- NỀN GRADIENT XANH ĐẬM -----
        SDL_SetRenderDrawColor(r, 2, 2, 12, 255);
        SDL_RenderClear(r);
        for (int y = 0; y < H; y++)
        {
            int blue = 15 + y * 60 / H;
            SDL_SetRenderDrawColor(r, 5, 5, blue, 255);
            SDL_RenderDrawLine(r, 0, y, W, y);
        }

        // ----- SAO XA (LỚP 1) -----
        for (auto &s : starsFar)
        {
            s.y += s.speed;
            if (s.y > H)
            {
                s.y = 0;
                s.x = rand() % W;
            }
            Uint8 glow = 120 + rand() % 80;
            SDL_SetRenderDrawColor(r, glow, glow, 255, 255);
            SDL_Rect rect = {(int)s.x, (int)s.y, s.size, s.size};
            SDL_RenderFillRect(r, &rect);
        }

        // ----- SAO GẦN (LỚP 2) -----
        for (auto &s : starsNear)
        {
            s.y += s.speed;
            if (s.y > H)
            {
                s.y = 0;
                s.x = rand() % W;
            }
            Uint8 glow = 200 + rand() % 55;
            SDL_SetRenderDrawColor(r, glow, glow, 255, 255);
            SDL_Rect rect = {(int)s.x, (int)s.y, s.size, s.size};
            SDL_RenderFillRect(r, &rect);
            // Hiệu ứng glow
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 100, 120, 255, 40);
            SDL_Rect glowRect = {(int)s.x - 2, (int)s.y - 2, s.size + 4, s.size + 4};
            SDL_RenderFillRect(r, &glowRect);
        }

        // ----- HIỆU ỨNG MÁU THẤP (NHẤP NHÁY ĐỎ) -----
        if (playerHP <= 1 && state == PLAYING)
        {
            float alpha = (sin(lowHpFlash) + 1.0f) * 0.5f;
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 255, 0, 0, (Uint8)(alpha * 120));
            SDL_Rect full = {0, 0, W, H};
            SDL_RenderFillRect(r, &full);
        }

        // ----- VẼ THEO TRẠNG THÁI GAME -----

        // MENU
        if (state == MENU)
        {
            drawTextCenter(r, "GAME INVADERS", 140);
            drawButton(r, btnStart);
            drawButton(r, btnGuide);
            drawButton(r, btnExit);
        }
        // PLAYING
        else if (state == PLAYING)
        {
            // Player
            SDL_Rect dst = {(int)px, (int)py, 60, 60};
            SDL_RenderCopyEx(r, playerTex, NULL, &dst, playerAngle, NULL, SDL_FLIP_NONE);

            // Đạn
            for (auto &b : bullets)
            {
                SDL_SetRenderDrawColor(r, b.enemy ? 255 : 0, b.enemy ? 0 : 255, 255, 255);
                SDL_Rect rect = {(int)b.x, (int)b.y, 6, 12};
                SDL_RenderFillRect(r, &rect);
            }

            // Laser
            renderLasers(r, lasers);
            renderArcLasers(r, arcLasers);

            // Trái tim hồi máu
            for (auto &h : healthDrops)
            {
                float alpha = 255;
                if (h.onGround && h.groundTime > 120)
                    alpha = 255 * (0.5f + 0.5f * sin(SDL_GetTicks() * 0.2f));
                SDL_SetTextureAlphaMod(heartTex, (Uint8)alpha);
                float scale = 1.0f + sin(SDL_GetTicks() * 0.01f) * 0.1f;
                int size = (int)(25 * scale);
                SDL_Rect rect = {(int)(h.x - size / 2), (int)(h.y - size / 2), size, size};
                SDL_RenderCopy(r, heartTex, NULL, &rect);
            }

            // Enemy
            for (auto &en : enemies)
            {
                en.hpDisplay += (en.hp - en.hpDisplay) * 0.15f;
                if (en.boss)
                {
                    int size = 100;
                    SDL_Rect rect = {(int)(en.x - size / 2), (int)(en.y - size / 2), size, size};
                    SDL_RenderCopy(r, enemyTex2, NULL, &rect);
                }
                else
                {
                    SDL_Rect rect = {(int)en.x, (int)en.y, 50, 50};
                    SDL_RenderCopy(r, enemyTex1, NULL, &rect);
                }
                drawEnemyHP(r, en);
            }

            // Hiệu ứng nổ
            for (auto &p : particles)
            {
                SDL_SetRenderDrawColor(r, 255, 120, 0, p.life * 6);
                SDL_Rect rect = {(int)p.x, (int)p.y, 4, 4};
                SDL_RenderFillRect(r, &rect);
            }

            // Chữ bay lên
            for (auto &t : texts)
                drawTextNeon(r, t.text, (int)t.x, (int)t.y);

            // UI: thanh máu, điểm, level
            drawHealthBar(r, playerHP, 10);
            drawTextNeon(r, "Score: " + to_string(score), 10, 10);
            drawTextNeon(r, "Level: " + to_string(level), 10, 40);
            if (hasShield)
                drawTextNeon(r, "SHIELD", 10, 100);
        }
        // PAUSED
        else if (state == PAUSED)
        {
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
            SDL_Rect full = {0, 0, W, H};
            SDL_RenderFillRect(r, &full);
            drawTextCenter(r, "PAUSED", 120);
            drawButton(r, btnResume);
            drawButton(r, btnMute);
            drawButton(r, btnBack);
        }
        // INSTRUCTION
        else if (state == INSTRUCTION)
        {
            drawTextCenter(r, "HOW TO PLAY", 100);
            drawTextCenter(r, "MOVE : WASD", 180);
            drawTextCenter(r, "SPACE : SHOOT", 220);
            drawTextCenter(r, "SHIFT : DASH", 260);
            drawTextCenter(r, "ESC : PAUSE", 300);
        }
        // GAME OVER
        else if (state == GAMEOVER)
        {
            drawTextCenter(r, "GAME OVER", 180);
            drawTextCenter(r, "SCORE : " + to_string(score), 240);
            drawTextCenter(r, "PRESS R TO RESTART", 320);
        }

        // ----- HIỆU ỨNG FADE (CHUYỂN MÀN) -----
        if (fade > 0.0f)
        {
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 0, 0, 0, (Uint8)(fade * 255));
            SDL_Rect full = {0, 0, W, H};
            SDL_RenderFillRect(r, &full);
        }

        // Cập nhật màn hình
        SDL_RenderPresent(r);
        SDL_Delay(16); // Giới hạn 60 FPS (~16.6ms/frame)
    }

    // ============================================================
    // DỌN DẸP (CLEANUP)
    // ============================================================
    TTF_CloseFont(font);
    SDL_DestroyTexture(playerTex);
    SDL_DestroyTexture(enemyTex1);
    SDL_DestroyTexture(enemyTex2);
    SDL_DestroyTexture(heartTex);
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(win);
    Mix_FreeMusic(backgroundMusic);
    Mix_FreeChunk(sfxExplode);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}