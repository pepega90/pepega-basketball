#include <raylib.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

const int WIDTH = 850;
const int HEIGHT = 480;
const int FPS = 60;

struct Ball
{
    Texture2D img;
    Vector2 pos;
    Vector2 vel;
    Vector2 grav;

    Ball(float x, float y, Texture2D img)
    {
        this->img = img;
        this->pos = Vector2(x, y);
        this->vel = Vector2(0, 0);
    }
    ~Ball() = default;

    void Update(float &dt, bool &lempar)
    {
        if (lempar)
        {
            grav.y += 8 * dt;
            lempar = false;
        }
        vel.x += grav.x;
        vel.y += grav.y;
        pos.x += vel.x;
        pos.y += vel.y;
    }
    void Draw()
    {
        DrawTextureV(img, pos, WHITE);
        // DrawCircleLines(pos.x + img.width / 2, pos.y + img.width / 2, 15.0, GREEN);
    }
};

int main()
{
    InitWindow(WIDTH, HEIGHT, "Pepega Basketball");
    InitAudioDevice();
    SetTargetFPS(FPS);

    Vector2 mousePos;

    // load sfx
    Sound applauseSfx = LoadSound("texture/crowd.ogg");

    // load font
    Font f = LoadFont("texture/fonts/coolvetica rg.ttf");

    // load assets
    Texture2D bg = LoadTexture("texture/bg_flip.png");
    bg.width = 850;
    bg.height = 480;
    Texture2D ball_img = LoadTexture("texture/ball.png");
    Texture2D ranjang_img = LoadTexture("texture/ranjang.png");
    ranjang_img.width *= 0.5;
    ranjang_img.height *= 0.5;

    // ranjang
    Vector2 ranjang_pos = Vector2(695, 175);
    Rectangle ranjang_rect;

    // ball
    std::vector<Ball *> balls;
    Ball *b1 = new Ball(200, 350, ball_img);
    balls.push_back(b1);
    bool hold = false;
    bool lempar = false;
    float force = 10.0;
    float angle = 0;
    bool collide = false;
    int score = 0;
    int countdown = 30;
    int startCount = 5;
    float lastCount = GetTime();
    float lastStart = GetTime();
    bool inAir = false;
    bool startGame = false;
    bool playGame = false;
    bool gameOver = false;

    // TODO: MENAMBAHKAN UI

    while (!WindowShouldClose())
    {
        ClearBackground(Color(0, 0, 0));
        DrawTexture(bg, 0, 0, WHITE);
        BeginDrawing();

        mousePos = GetMousePosition();
        float dt = GetFrameTime();

        if (!startGame)
        {
            DrawTextEx(f, "Pepega Basketball", Vector2(WIDTH / 2. - 140., HEIGHT / 4. + 50.), 40., 0.0, WHITE);
            DrawTextEx(f, "Tekan \"SPACE\" untuk bermain", Vector2(WIDTH / 2. - 140., HEIGHT / 2. + 20.), 25., 0.0, WHITE);
            DrawTextEx(f, "created by aji mustofa @pepega90", Vector2(20., HEIGHT - 35.), 25., 0.0, WHITE);
        }

        if (IsKeyPressed(KEY_SPACE))
        {
            startGame = true;
        }

        // throw ball
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !hold && !inAir && playGame && !gameOver)
        {
            hold = true;
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && !lempar && !inAir && playGame && !gameOver)
        {
            Vector2 mb = (mousePos - balls.front()->pos);
            float angle = std::atan2(mb.y, mb.x);

            float fx = -force * std::cos(angle);
            float fy = -force * std::sin(angle);

            balls.front()->vel.x = fx;
            balls.front()->vel.y = fy;

            hold = false;
            lempar = true;
        }

        // update ball
        for (auto &b : balls)
        {
            b->Update(dt, lempar);
        }

        // hapus bola jika melebihi batas lebar/tinggi layar
        auto removedBall = std::remove_if(balls.begin(), balls.end(), [&](Ball *b)
                                          { return b->pos.y > HEIGHT || b->pos.x > WIDTH; });

        if (removedBall != balls.end())
        {
            balls.erase(removedBall);
        }

        // jika bola kurang dari 1, push lagi
        if (balls.size() < 1)
        {
            Ball *b = new Ball(GetRandomValue(191, 656), 350, ball_img);
            balls.push_back(b);
            collide = false;
            inAir = false;
        }

        // draw trajectory
        Vector2 a = mousePos - balls.front()->pos;
        angle = std::atan2(a.y, a.x);
        float velo_angle = -angle;
        for (int i = 0; i < 100; i++)
        {
            float x = -force * i * std::cos(velo_angle) * 0.99;
            float y = -force * i * std::sin(velo_angle) * 0.99 - (8 * dt * i * i / 2.);
            if (hold)
            {
                DrawCircle(x + balls.front()->pos.x + 20., balls.front()->pos.y - y + 10., 2., YELLOW);
            }
        }

        // check jika bola sedang ada di udara
        if (balls.front()->pos.y + balls.front()->img.width / 2 < 364 && !inAir)
        {
            inAir = true;
        }

        // check collision bola dengan ranjang
        if (CheckCollisionCircleRec(Vector2(balls.front()->pos.x + balls.front()->img.width / 2, balls.front()->pos.y + balls.front()->img.width / 2), 15.0, ranjang_rect) && !collide)
        {
            if (balls.front()->pos.y + balls.front()->img.width / 2 < ranjang_rect.y)
            {
                collide = true;
                PlaySound(applauseSfx);
                score += 1;
            }
        }

        // draw ball
        for (auto &b : balls)
        {
            b->Draw();
        }

        // update countdown
        if (GetTime() - lastCount > 1. && countdown > 0 && playGame)
        {
            countdown -= 1;
            lastCount = GetTime();
        }

        // update startCount countdown
        if (GetTime() - lastStart > 1. && startCount > 0 && startGame)
        {
            startCount -= 1;
            lastStart = GetTime();
        }

        // jika countdown = 0 maka game over
        if (countdown == 0)
        {
            gameOver = true;
        }

        // jika startoucnt 0 maka mulai permainan
        if (startCount == 0)
        {
            playGame = true;
        }

        if (startCount > 0 && startGame)
        {
            // draw countdown game
            std::string startCountText = std::to_string(startCount);
            DrawTextEx(f, startCountText.c_str(), Vector2(WIDTH / 2., HEIGHT / 2.), 25., 0.0, WHITE);
        }

        if (playGame)
        {
            // draw countdown game
            std::string countText = std::to_string(countdown);
            DrawTextEx(f, countText.c_str(), Vector2(WIDTH / 2., 30.), 25., 0.0, BLACK);

            // draw score
            std::string scoreText = "Score: " + std::to_string(score);
            DrawTextEx(f, scoreText.c_str(), Vector2(740., 30.), 25., 0.0, BLACK);
        }

        if (gameOver)
        {
            playGame = false;
            std::string lastScoreText = "Score Kamu: " + std::to_string(score);
            DrawTextEx(f, lastScoreText.c_str(), Vector2(WIDTH / 2 - 60., HEIGHT / 2.), 25., 0.0, WHITE);

            DrawTextEx(f, "Tekan\"R\" untuk restart", Vector2(WIDTH / 2 - 90., HEIGHT / 2. + 50), 25., 0.0, WHITE);
        }

        if (gameOver && IsKeyPressed(KEY_R))
        {
            gameOver = false;
            startCount = 5;
            countdown = 30;
            score = 0;
            balls.clear();
        }

        // draw ranjang
        DrawTextureV(ranjang_img, ranjang_pos, WHITE);
        ranjang_rect.x = ranjang_pos.x;
        ranjang_rect.y = ranjang_pos.y;
        ranjang_rect.width = ranjang_img.width;
        ranjang_rect.height = ranjang_img.height;
        // DrawRectangleLinesEx(ranjang_rect, 3.0, RED);

        EndDrawing();
    }
    UnloadFont(f);
    UnloadTexture(ranjang_img);
    UnloadTexture(bg);
    UnloadTexture(ball_img);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}