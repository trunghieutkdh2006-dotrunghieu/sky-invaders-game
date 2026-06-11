// ========================== render.cpp ==========================
#include "render.h"
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <cmath>

SDL_Texture *backgroundTexture = nullptr;

// ================================================================
// LOAD BACKGROUND
// ================================================================
bool loadBackground(SDL_Renderer *renderer)
{
    SDL_Surface *surface = IMG_Load("assets/background.png");

    if (!surface)
    {
        SDL_Log("IMG_Load Error: %s", IMG_GetError());
        return false;
    }

    backgroundTexture =
        SDL_CreateTextureFromSurface(renderer, surface);

    SDL_FreeSurface(surface);

    if (!backgroundTexture)
    {
        SDL_Log("CreateTexture Error: %s", SDL_GetError());
        return false;
    }

    return true;
}

// ================================================================
// DRAW SCROLLING BACKGROUND
// ================================================================
void drawBackground(SDL_Renderer *r, int scrollY)
{
    if (!backgroundTexture)
        return;

    SDL_Rect dst1 = {0, scrollY, W, H};
    SDL_Rect dst2 = {0, scrollY - H, W, H};

    SDL_RenderCopy(r, backgroundTexture, NULL, &dst1);
    SDL_RenderCopy(r, backgroundTexture, NULL, &dst2);
}

// ================================================================
// DRAW NORMAL TEXT
// ================================================================
void drawText(SDL_Renderer *r, string text, int x, int y)
{
    SDL_Color c = {0, 255, 200};

    SDL_Surface *s =
        TTF_RenderText_Blended(font, text.c_str(), c);

    if (!s)
        return;

    SDL_Texture *t =
        SDL_CreateTextureFromSurface(r, s);

    SDL_Rect rect = {x, y, s->w, s->h};

    SDL_RenderCopy(r, t, NULL, &rect);

    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

// ================================================================
// NEON TEXT
// ================================================================
void drawTextNeon(SDL_Renderer *r,
                  string text,
                  int x,
                  int y)
{

    SDL_Color core = {0, 255, 200};
    SDL_Color glow = {0, 120, 255};

    SDL_Surface *base =
        TTF_RenderText_Blended(font, text.c_str(), core);

    if (!base)
        return;

    int w = base->w;
    int h = base->h;

    for (int i = 3; i >= 1; i--)
    {

        SDL_Surface *s =
            TTF_RenderText_Blended(font, text.c_str(), glow);

        SDL_Texture *t =
            SDL_CreateTextureFromSurface(r, s);

        SDL_FreeSurface(s);

        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(t, 60);

        SDL_Rect rect = {
            x - i,
            y - i,
            w + i * 2,
            h + i * 2};

        SDL_RenderCopy(r, t, NULL, &rect);

        SDL_DestroyTexture(t);
    }

    SDL_Texture *t =
        SDL_CreateTextureFromSurface(r, base);

    SDL_FreeSurface(base);

    SDL_Rect rect = {x, y, w, h};

    SDL_RenderCopy(r, t, NULL, &rect);

    SDL_DestroyTexture(t);
}

// ================================================================
// CENTER TEXT
// ================================================================
void drawTextCenter(SDL_Renderer *r,
                    string text,
                    int y)
{

    SDL_Color c = {0, 255, 200};

    SDL_Surface *s =
        TTF_RenderText_Blended(font, text.c_str(), c);

    if (!s)
        return;

    SDL_Texture *t =
        SDL_CreateTextureFromSurface(r, s);

    int x = (W - s->w) / 2;

    SDL_Rect rect = {
        x,
        y,
        s->w,
        s->h};

    SDL_RenderCopy(r, t, NULL, &rect);

    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

// ================================================================
// BUTTON
// ================================================================
void drawButton(SDL_Renderer *r, Button &b)
{

    if (b.hover)
        SDL_SetRenderDrawColor(r, 0, 200, 255, 255);
    else
        SDL_SetRenderDrawColor(r, 0, 100, 180, 255);

    SDL_RenderFillRect(r, &b.rect);

    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderDrawRect(r, &b.rect);

    SDL_Color c = {0, 255, 200};

    SDL_Surface *s =
        TTF_RenderText_Blended(font, b.text.c_str(), c);

    if (!s)
        return;

    SDL_Texture *t =
        SDL_CreateTextureFromSurface(r, s);

    SDL_Rect dst;

    dst.x = b.rect.x + (b.rect.w - s->w) / 2;
    dst.y = b.rect.y + (b.rect.h - s->h) / 2;
    dst.w = s->w;
    dst.h = s->h;

    SDL_RenderCopy(r, t, NULL, &dst);

    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

// ================================================================
// UPDATE BUTTON
// ================================================================
void updateButton(Button &b)
{

    int mx, my;

    SDL_GetMouseState(&mx, &my);

    b.hover =
        (mx >= b.rect.x &&
         mx <= b.rect.x + b.rect.w &&
         my >= b.rect.y &&
         my <= b.rect.y + b.rect.h);
}

// ================================================================
// PLAYER HP BAR
// ================================================================
void drawHealthBar(SDL_Renderer *r,
                   int hp,
                   int maxHp)
{

    int barW = 200;
    int barH = 18;

    float ratio = (float)hp / (float)maxHp;

    if (ratio < 0)
        ratio = 0;
    if (ratio > 1)
        ratio = 1;

    int x = 10;
    int y = 70;

    SDL_Rect bg = {x, y, barW, barH};

    SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
    SDL_RenderFillRect(r, &bg);

    SDL_Rect fill = {
        x,
        y,
        (int)(barW * ratio),
        barH};

    if (ratio > 0.6f)
        SDL_SetRenderDrawColor(r, 0, 255, 0, 255);

    else if (ratio > 0.3f)
        SDL_SetRenderDrawColor(r, 255, 200, 0, 255);

    else
        SDL_SetRenderDrawColor(r, 255, 0, 0, 255);

    SDL_RenderFillRect(r, &fill);

    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderDrawRect(r, &bg);
}

// ================================================================
// ENEMY HP BAR
// ================================================================
void drawEnemyHP(SDL_Renderer *r, Enemy &en)
{

    float ratio = en.hpDisplay / en.maxHP;

    if (ratio < 0)
        ratio = 0;
    if (ratio > 1)
        ratio = 1;

    int barW = 40;
    int barH = 6;

    int x = (int)en.x;
    int y = (int)en.y - 12;

    SDL_Rect bg = {
        x - barW / 2,
        y,
        barW,
        barH};

    SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
    SDL_RenderFillRect(r, &bg);

    Uint8 red, green;

    if (ratio > 0.6f)
    {
        red = (Uint8)(255 * (1 - ratio));
        green = 255;
    }
    else if (ratio > 0.3f)
    {
        red = 255;
        green = (Uint8)(255 * ratio);
    }
    else
    {
        red = 255;
        green = 0;
    }

    SDL_Rect fill = {
        x - barW / 2,
        y,
        (int)(barW * ratio),
        barH};

    SDL_SetRenderDrawColor(r, red, green, 0, 255);
    SDL_RenderFillRect(r, &fill);

    for (int i = 1; i <= 2; i++)
    {

        SDL_SetRenderDrawColor(r, red, green, 0, 40);

        SDL_Rect glow = {
            bg.x - i,
            bg.y - i,
            bg.w + i * 2,
            bg.h + i * 2};

        SDL_RenderDrawRect(r, &glow);
    }

    SDL_SetRenderDrawColor(r, 255, 255, 255, 200);
    SDL_RenderDrawRect(r, &bg);
}

// ================================================================
// DESTROY BACKGROUND
// ================================================================
void destroyBackground()
{
    if (backgroundTexture)
    {
        SDL_DestroyTexture(backgroundTexture);
        backgroundTexture = nullptr;
    }
}