#include <iostream>
#include <conio.h>
#include <windows.h>
#include <string>
#include <chrono>
#include <fstream>

#pragma comment(lib, "winmm.lib")

using namespace std;
using namespace std::chrono;

int WIDTH = 40;
const int HEIGHT = 20;
const char PLAYER = 'A';
const char ENEMY1 = 'M';
const char ENEMY2 = 'N';
const char ENEMY3 = 'X';
const char UFO = 'U';
const char BULLET = '|';
const char BULLET_ENEMY1 = 'o'; // گلوله دشمن نوع 1
const char BULLET_ENEMY2 = '+'; // گلوله دشمن نوع 2
const char BULLET_ENEMY3 = '*'; // گلوله دشمن نوع 3
const char SHIELD = '#';
const char EMPTY = ' ';
const int SHIELD_MAX_ROW = 3;
const int SHIELD_MAX_COL = 5;

struct Position
{
    int x, y;
    char type; // To distinguish between enemy types
};

struct Shield
{
    int x, y;                                   // مختصات بالای چپ محافظ
    int health;                                 // تعداد جون محافظ
    char shape[SHIELD_MAX_ROW][SHIELD_MAX_COL]; // شکل محافظ
};

struct PlayerData
{
    string name;
    int score;
    long long timePlayed;

    // مقایسه برای مرتب‌سازی: امتیاز بالاتر اولویت دارد، سپس زمان کمتر
    bool isBigerThan(const PlayerData &other) const
    {
        if (score != other.score)
            return score > other.score;
        return timePlayed < other.timePlayed;
    }
};
void getConsoleSize(int &width, int &height);
void updateMusicBasedOnLevel(int level);
void setConsoleColor(int textColor, int backgroundColor);
void applyRandomTheme();
void resetConsoleColor();
void saveOrUpdatePlayerData(string playerName, int score, long long elapsedTime);
void displayLeaderboard();
void drawShields(string grid[], Shield shields[], int shieldCount);
void checkShieldHit(Shield shields[], int shieldCount, Position bullets[], int &bulletCount, bool &bulletActive, bool checkPlayerBullet);
string createHealthBarString(int health);
void setCursorPosition(int x, int y);
void ShowConsoleCursor(bool showFlag);
void draw(const string grid[], int score, int health, int lives, int level, long long elapsedTime);
void createGrid(string grid[]);
void placeEntity(string grid[], const Position &pos, char symbol);
void moveEnemies(Position enemies[], int enemyCount, bool &direction, int &speed);
void shootBullet(Position bullets[], int &bulletCount, const Position &shooterPos, bool &bulletActive, int maxBullets);
void shootEnemyBullet(Position enemyBullets[], int &enemyBulletCount, const Position enemies[], int enemyCount, int &globalCooldown, int level, int maxEnemyBullets);
void moveBullets(Position bullets[], int &bulletCount, int direction);
void checkCollisions(Position bullets[], int &bulletCount, Position enemies[], int &enemyCount, int &score, bool &bulletActive, Position &ufo, bool &ufoActive);
void checkPlayerHit(Position enemyBullets[], int &enemyBulletCount, const Position &playerPos, int &health, int &lives, bool &gameOver);
void resetLevel(Position enemies[], int &enemyCount, Position bullets[], int &bulletCount, Position enemyBullets[], int &enemyBulletCount, int level);
void moveUFO(Position &ufo, bool &ufoActive, bool &ufoMovingRight);
int showMenu(int &lastX, int &lastY);
int showPauseMenu(int &lastX, int &lastY);
void showHowToPlay();
void showLeaderboard();
void saveGameFull(int width, string playerName, int score, int health, int lives, int level, long long elapsedTime,
                  const Position &playerPos, const Position enemies[], int enemyCount,
                  const Position bullets[], int bulletCount, const Position enemyBullets[], int enemyBulletCount,
                  const Shield shields[], int shieldCount, bool direction, bool bulletActive,
                  bool gameOver, const Position &ufo, int globalCooldown, bool ufoActive,
                  bool ufoMovingRight, int enemyMovementSpeedCounter, int enemyMovementSpeedThreshold);
bool loadGameFull(int &width, string &playerName, int &score, int &health, int &lives, int &level, long long &elapsedTime,
                  Position &playerPos, Position enemies[], int &enemyCount,
                  Position bullets[], int &bulletCount, Position enemyBullets[], int &enemyBulletCount,
                  Shield shields[], int &shieldCount, bool &direction, bool &bulletActive,
                  bool &gameOver, Position &ufo, int &globalCooldown, bool &ufoActive,
                  bool &ufoMovingRight, int &enemyMovementSpeedCounter, int &enemyMovementSpeedThreshold);
void initShields(Shield shields[]);
void initGameParameters(Position &playerPos, int &enemyCount, int &bulletCount, int &enemyBulletCount,
                        int &shieldCount, const int MAX_SHIELDS, Shield shields[], int &level, bool &direction,
                        int &score, bool &bulletActive, bool &gameOver, int &enemySpeed, int &health, int &lives,
                        int &globalCooldown, long long &elapsedTime, Position &ufo, bool &ufoActive, bool &ufoMovingRight,
                        int &enemyMovementSpeedCounter, int &enemyMovementSpeedThreshold);
int main()
{
    string palyerName;
    bool loadGame = false;

    int x, y;

    // نمایش منوی اولیه
    while (true)
    {
        int menuChoice = showMenu(x, y);

        if (menuChoice == 1)
        {
            setCursorPosition(x, y++);
            cout << "Enter your name: ";
            cin >> palyerName;
            setCursorPosition(x, y++);
            cout << "Enter your wanted screen to play(1- 40*20, 2- 50*20): ";
            int widthChoice;
            cin >> widthChoice;
            switch (widthChoice)
            {
            case 1:
                WIDTH = 40;
                break;
            case 2:
                WIDTH = 50;
                break;
            default:
                break;
            }
            setCursorPosition(x, y++);
            cout << "Press any key to start...";
            _getch();
            system("cls");
            break; // شروع بازی جدید
        }
        else if (menuChoice == 2)
        {
            loadGame = true;
            break;
        }
        else if (menuChoice == 3)
        {
            showHowToPlay(); // نمایش قوانین بازی
        }
        else if (menuChoice == 4)
        {
            showLeaderboard(); // نمایش جدول امتیازات
        }
        else if (menuChoice == 5)
        {
            return 0; // خروج از بازی
        }
    }
    applyRandomTheme();//تم رندوم گزاشتن
    system("cls");
    ShowConsoleCursor(false);
    srand(static_cast<unsigned>(time(0)));

    // آرایه‌ها و شمارنده‌ها
    string grid[HEIGHT];
    createGrid(grid);// مقداردهی هر خط با کاراکتر خالی

    Position playerPos;

    const int MAX_ENEMIES = 50;
    Position enemies[MAX_ENEMIES];
    int enemyCount;

    const int MAX_BULLETS = 100;
    Position bullets[MAX_BULLETS];
    int bulletCount;

    Position enemyBullets[MAX_BULLETS];
    int enemyBulletCount;

    const int MAX_SHIELDS = 3;
    Shield shields[MAX_SHIELDS];

    int shieldCount;
    int level;
    bool direction;
    int score;
    bool bulletActive;
    bool gameOver;
    int enemySpeed;
    int health;
    int lives;
    int globalCooldown;
    long long elapsedTime;//مدت زمان سپری شده از زمانی مشخص

    Position ufo;
    bool ufoActive;
    bool ufoMovingRight;
    int enemyMovementSpeedCounter;
    int enemyMovementSpeedThreshold;

    initGameParameters(playerPos, enemyCount, bulletCount, enemyBulletCount, shieldCount, MAX_SHIELDS,
                       shields, level, direction, score, bulletActive, gameOver, enemySpeed, health, lives,
                       globalCooldown, elapsedTime, ufo, ufoActive, ufoMovingRight, enemyMovementSpeedCounter, enemyMovementSpeedThreshold); // تنظیمات اولیه بازی
    resetLevel(enemies, enemyCount, bullets, bulletCount, enemyBullets, enemyBulletCount, level);

    updateMusicBasedOnLevel(1);

    // ثبت زمان شروع بازی
    steady_clock::time_point gameStartTime = steady_clock::now();

    if (loadGame)
    {
        if (loadGameFull(WIDTH, palyerName, score, health, lives, level, elapsedTime, playerPos, enemies, enemyCount, bullets, bulletCount, enemyBullets, enemyBulletCount, shields, shieldCount, direction, bulletActive, gameOver, ufo, globalCooldown, ufoActive, ufoMovingRight, enemyMovementSpeedCounter, enemyMovementSpeedThreshold))
        {
            gameStartTime -= seconds(elapsedTime);// . این کار باعث می‌شود بازیکن بتواند بازی را از همان نقطه زمانی که ذخیره کرده است ادامه دهد.
        }
    }

    while (!gameOver)
    {
        createGrid(grid);

        // رسم موجودیت‌ها
        placeEntity(grid, playerPos, PLAYER);
        for (int i = 0; i < enemyCount; ++i)
        {
            placeEntity(grid, enemies[i], enemies[i].type);
        }
        for (int i = 0; i < bulletCount; ++i)
        {
            placeEntity(grid, bullets[i], BULLET);
        }
        for (int i = 0; i < enemyBulletCount; ++i)
        {
            placeEntity(grid, enemyBullets[i], enemyBullets[i].type);
        }
        if (ufoActive)
        {
            placeEntity(grid, ufo, UFO);
        }

        drawShields(grid, shields, shieldCount);

        // محاسبه زمان سپری‌شده از شروع بازی
        steady_clock::time_point now = steady_clock::now();
        elapsedTime = duration_cast<seconds>(now - gameStartTime).count();

        draw(grid, score, health, lives, level, elapsedTime);

        // مدیریت ورودی بازیکن
        if (_kbhit())
        {
            char input = _getch();
            if (input == 'a' && playerPos.x > 0)
                playerPos.x--;
            if (input == 'd' && playerPos.x < WIDTH - 1)
                playerPos.x++;
            if (input == ' ')
                shootBullet(bullets, bulletCount, playerPos, bulletActive, MAX_BULLETS);
            if (input == 'p' || input == 'P')
            {

                // نمایش منوی درون‌بازی هنگام Pause
                int pauseChoice = showPauseMenu(x, y);

                if (pauseChoice == 1)
                {
                    system("cls");
                    initGameParameters(playerPos, enemyCount, bulletCount, enemyBulletCount, shieldCount, MAX_SHIELDS,
                                       shields, level, direction, score, bulletActive, gameOver, enemySpeed, health, lives,
                                       globalCooldown, elapsedTime, ufo, ufoActive, ufoMovingRight, enemyMovementSpeedCounter, enemyMovementSpeedThreshold);
                    gameStartTime = steady_clock::now();
                    resetLevel(enemies, enemyCount, bullets, bulletCount, enemyBullets, enemyBulletCount, level);
                }
                else if (pauseChoice == 2)
                {
                    system("cls");
                    // ادامه بازی (هیچ کاری انجام نمی‌دهد، فقط منو بسته می‌شود)
                    continue;
                }
                else if (pauseChoice == 3)
                {
                    system("cls");
                    saveGameFull(WIDTH, palyerName, score, health, lives, level, elapsedTime, playerPos, enemies, enemyCount, bullets, bulletCount, enemyBullets, enemyBulletCount, shields, shieldCount, direction, bulletActive, gameOver, ufo, globalCooldown, ufoActive, ufoMovingRight, enemyMovementSpeedCounter, enemyMovementSpeedThreshold);
                    // خروج از بازی
                    setCursorPosition(x, 10);
                    return 0;
                }
            }
        }

        // حرکت گلوله‌ها
        moveBullets(bullets, bulletCount, -1);          // حرکت گلوله‌های بازیکن به بالا
        moveBullets(enemyBullets, enemyBulletCount, 1); // حرکت گلوله‌های دشمن به پایین

        // حرکت دشمن‌ها و بشقاب پرنده با سرعت متفاوت
        if (enemyMovementSpeedCounter % enemyMovementSpeedThreshold == 0)
        {
            moveEnemies(enemies, enemyCount, direction, enemySpeed); // حرکت دشمن‌ها
            moveUFO(ufo, ufoActive, ufoMovingRight);                 // حرکت بشقاب پرنده
        }
        enemyMovementSpeedCounter++;

        // شلیک گلوله‌های دشمن
        shootEnemyBullet(enemyBullets, enemyBulletCount, enemies, enemyCount, globalCooldown, level, MAX_BULLETS);

        // بررسی برخوردها
        checkCollisions(bullets, bulletCount, enemies, enemyCount, score, bulletActive, ufo, ufoActive);//برخورد با ufo
        checkShieldHit(shields, shieldCount, bullets, bulletCount, bulletActive, true);// گلوله بازیکن به شیلد
        checkShieldHit(shields, shieldCount, enemyBullets, enemyBulletCount, bulletActive, false);// گلوله دشمن به شیلد
        checkPlayerHit(enemyBullets, enemyBulletCount, playerPos, health, lives, gameOver);//برخورد گلوله‌های دشمن با بازیکن

        // بررسی پایان بازی
        for (int i = 0; i < enemyCount; ++i)
        {
            if (enemies[i].y >= HEIGHT - 1)
            { // اگر دشمن به پایین صفحه برسد
                draw(grid, score, health, lives, level, elapsedTime);
                gameOver = true;
                break;
            }
        }

        // رفتن به مرحله بعد
        if (enemyCount == 0)
        { // اگر همه دشمن‌ها از بین بروند
            level++;
            resetLevel(enemies, enemyCount, bullets, bulletCount, enemyBullets, enemyBulletCount, level);
            updateMusicBasedOnLevel(level);
        }

        // کاهش کول‌داون شلیک دشمنان
        if (globalCooldown > 0)
            globalCooldown--;

        Sleep(enemySpeed); // تنظیم سرعت بازی
    }

    // نمایش امتیاز نهایی
    cout << endl;
    setCursorPosition(x, 25);
    cout << "Game Over!" << endl;
    saveOrUpdatePlayerData(palyerName, score, elapsedTime);
    PlaySound(0, 0, 0); // توقف آهنگ

    return 0;
}

void setCursorPosition(int x, int y)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hConsole, position);
}

void getConsoleSize(int &width, int &height)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;  // عرض کنسول
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1; // ارتفاع کنسول
    }
}

void bringToMiddle()
{
    for (int j = 0; j < 35; j++)
    {
        cout << " ";
    }
}

void printLogo()
{
    bringToMiddle();
    cout << "\033[38;5;60m███████╗██████╗  █████╗  ██████╗███████╗        ██╗███╗   ██╗██╗   ██╗ █████╗ ██████╗ ███████╗██████╗ ███████╗" << endl;
    bringToMiddle();
    cout << "██╔════╝██╔══██╗██╔══██╗██╔════╝██╔════╝        ██║████╗  ██║██║   ██║██╔══██╗██╔══██╗██╔════╝██╔══██╗██╔════╝" << endl;
    bringToMiddle();
    cout << "███████╗██████╔╝███████║██║     █████╗          ██║██╔██╗ ██║██║   ██║███████║██║  ██║█████╗  ██████╔╝███████╗" << endl;
    bringToMiddle();
    cout << "╚════██║██╔═══╝ ██╔══██║██║     ██╔══╝          ██║██║╚██╗██║╚██╗ ██╔╝██╔══██║██║  ██║██╔══╝  ██╔══██╗╚════██║" << endl;
    bringToMiddle();
    cout << "███████║██║     ██║  ██║╚██████╗███████╗        ██║██║ ╚████║ ╚████╔╝ ██║  ██║██████╔╝███████╗██║  ██║███████║" << endl;
    bringToMiddle();
    cout << "╚══════╝╚═╝     ╚═╝  ╚═╝ ╚═════╝╚══════╝        ╚═╝╚═╝  ╚═══╝  ╚═══╝  ╚═╝  ╚═╝╚═════╝ ╚══════╝╚═╝  ╚═╝╚══════╝\033[0m" << endl;
}

void updateMusicBasedOnLevel(int level)
{
    if (level == 1)
    {
        PlaySound(TEXT("level1.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    }
    else if (level == 2)
    {
        PlaySound(TEXT("level2.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    }
    else if (level >= 3)
    {
        PlaySound(TEXT("level3.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    }
}

// تابع برای تنظیم رنگ کنسول
void setConsoleColor(int textColor, int backgroundColor)// دریافت کدرنگ پس زمینه و متن
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    int colorAttribute = (backgroundColor << 4) | textColor; // ترکیب رنگ متن و پس‌زمینه
    SetConsoleTextAttribute(consoleHandle, colorAttribute);
}

// تابع برای انتخاب رنگ تصادفی
void applyRandomTheme()
{
    srand(time(0));                    // مقداردهی اولیه برای تولید اعداد تصادفی
    int textColor = rand() % 16;       // تولید رنگ تصادفی برای متن (0 تا 15)
    int backgroundColor = rand() % 16; // تولید رنگ تصادفی برای پس‌زمینه (0 تا 15)

    // اطمینان از اینکه متن و پس‌زمینه یکسان نباشند
    while (textColor == backgroundColor)
    {
        backgroundColor = rand() % 16;
    }

    setConsoleColor(textColor, backgroundColor);
}

// تابع برای بازنشانی رنگ کنسول به پیش‌فرض
void resetConsoleColor()
{
    setConsoleColor(7, 0); // رنگ متن سفید، پس‌زمینه سیاه
}

void saveOrUpdatePlayerData(string playerName, int score, long long elapsedTime)
{
    const int MAX_PLAYERS = 100; // حداکثر تعداد بازیکنان
    PlayerData players[MAX_PLAYERS];
    int playerCount = 0;

    // باز کردن فایل و خواندن داده‌ها در آرایه
    ifstream inputFile("game_data.txt");
    if (inputFile.is_open())
    {
        while (inputFile >> players[playerCount].name >> players[playerCount].score >> players[playerCount].timePlayed)
        {
            playerCount++;
            if (playerCount >= MAX_PLAYERS)
                break; // جلوگیری از پر شدن آرایه
        }
        inputFile.close();
    }

    // بررسی وجود بازیکن در لیست و به‌روزرسانی در صورت لزوم
    bool found = false;
    for (int i = 0; i < playerCount; ++i)
    {
        if (players[i].name == playerName)
        {
            found = true;
            if (score > players[i].score || (score == players[i].score && elapsedTime < players[i].timePlayed))
            {
                // به‌روزرسانی رکورد بازیکن
                players[i].score = score;
                players[i].timePlayed = elapsedTime;
            }
            break;
        }
    }

    // اگر بازیکن جدید باشد، اضافه شود
    if (!found && playerCount < MAX_PLAYERS)
    {
        players[playerCount].name = playerName;
        players[playerCount].score = score;
        players[playerCount].timePlayed = elapsedTime;
        playerCount++;
    }

    // مرتب‌سازی بازیکنان بر اساس امتیاز و زمان
    for (int i = 0; i < playerCount - 1; ++i)
    {
        for (int j = i + 1; j < playerCount; ++j)
        {
            if (!players[i].isBigerThan(players[j]))
            {
                PlayerData temp = players[i];
                players[i] = players[j];
                players[j] = temp;
            }
        }
    }

    // باز کردن فایل برای بازنویسی داده‌های مرتب‌شده
    ofstream outputFile("game_data.txt");
    if (outputFile.is_open())
    {
        for (int i = 0; i < playerCount; ++i)
        {
            outputFile << players[i].name << " " << players[i].score << " " << players[i].timePlayed << endl;
        }
        outputFile.close();
    }
    else
    {
        cerr << "Error: Unable to open file for writing!" << endl;
    }
}

void displayLeaderboard()
{
    const int MAX_PLAYERS = 100; // حداکثر تعداد بازیکنان
    PlayerData players[MAX_PLAYERS];
    int playerCount = 0;

    // باز کردن فایل برای خواندن
    ifstream inputFile("game_data.txt");
    if (inputFile.is_open())
    {
        int consoleWidth, height;
        getConsoleSize(consoleWidth, height);

        string menu = "============= Leaderboard =============";

        int x = (consoleWidth - menu.length()) / 2;
        int y = 0;

        setCursorPosition(x, y++);
        cout << menu;
        setCursorPosition(x, y++);
        cout << "Rank\tName\tScore\tTime (s)";

        // خواندن فایل خط به خط
        while (inputFile >> players[playerCount].name >> players[playerCount].score >> players[playerCount].timePlayed)
        {
            playerCount++;
            if (playerCount >= MAX_PLAYERS)
                break; // جلوگیری از پر شدن آرایه
        }
        inputFile.close();

        // نمایش اطلاعات در کنسول
        for (int i = 0; i < playerCount; ++i)
        {
            setCursorPosition(x, y++);
            cout << (i + 1) << "\t" << players[i].name << "\t" << players[i].score << "\t" << players[i].timePlayed;
        }
    }
    else
    {
        cerr << "Error: Unable to open file!";
    }
}

void drawShields(string grid[], Shield shields[], int shieldCount)
{
    for (int i = 0; i < shieldCount; ++i)
    {
        Shield &shield = shields[i];
        if (shield.health <= 0)
            continue; // اگر محافظ از بین رفته باشد، رسم نشود

        for (size_t row = 0; row < SHIELD_MAX_ROW; ++row)
        {
            for (size_t col = 0; col < SHIELD_MAX_COL; ++col)
            {
                if (shield.shape[row][col] != ' ')
                { // فقط کاراکترهای غیر خالی رسم شوند
                    grid[shield.y + row][shield.x + col] = shield.shape[row][col];//کپی کردن شکل در فضای خالی ایجاد شده
                }
            }
        }
    }
}

void checkShieldHit(Shield shields[], int shieldCount, Position bullets[], int &bulletCount, bool &bulletActive, bool checkPlayerBullet)
{
    for (int i = 0; i < bulletCount; ++i)
    { // بررسی تمام گلوله‌ها
        for (int j = 0; j < shieldCount; ++j)
        {                                // بررسی تمام محافظ‌ها
            Shield &shield = shields[j]; // دسترسی به محافظ
            if (shield.health > 0 &&
                bullets[i].y >= shield.y && bullets[i].y < shield.y + SHIELD_MAX_ROW && // بررسی برخورد با ارتفاع محافظ
                bullets[i].x >= shield.x && bullets[i].x < shield.x + SHIELD_MAX_COL)
            { // بررسی برخورد با عرض محافظ

                // محاسبه مختصات برخورد داخل شکل محافظ
                int relY = bullets[i].y - shield.y;
                int relX = bullets[i].x - shield.x;

                if (shield.shape[relY][relX] == '#')
                {                                             // اگر بخشی از محافظ برخورد کرده باشد
                    shield.shape[relY][relX] = ' ';           // حذف بخش برخوردشده
                    shield.health--;                          // کاهش جون محافظ
                    shield.shape[1][2] = shield.health + '0'; // به‌روزرسانی نمایش تعداد جون در مرکز

                    if (checkPlayerBullet)//گر گلوله بازیکن باشد وضعیت گلوله غیرفعال می شود
                    {
                        bulletActive = false;
                    }

                    // حذف گلوله از آرایه
                    bullets[i] = bullets[--bulletCount];
                    --i;   // بررسی مجدد موقعیت فعلی
                    break; // ادامه بررسی برای گلوله‌های دیگر
                }
            }
        }
    }
}

string createHealthBarString(int health)
{
    int barWidth = 20;                      // Width of the health bar
    int filled = (health * barWidth) / 100; // Calculate filled portion

    string healthBar = "Health: [";
    for (int i = 0; i < barWidth; ++i)
    {
        if (i < filled)
            healthBar.append("#");
        else
            healthBar.append(" ");
    }
    healthBar.append("]");
    return healthBar;
}

void ShowConsoleCursor(bool showFlag)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);
}

void draw(const string grid[], int score, int health, int lives, int level, long long elapsedTime)
{
    ShowConsoleCursor(false);

    int consoleWidth, height;
    getConsoleSize(consoleWidth, height);

    int y = (height - (HEIGHT + 7)) / 2;
    int x = (consoleWidth - WIDTH) / 2;
    setCursorPosition(x, y++);
    // رسم حاشیه بالایی
    cout << "+" << string(WIDTH, '-') << "+" << endl;

    // رسم محتوای صفحه (گرید)
    for (int i = 0; i < HEIGHT; ++i)
    {
        setCursorPosition(x, y++);
        cout << "|" << grid[i] << "|" << endl;
    }

    setCursorPosition(x, y++);
    // رسم حاشیه پایینی
    cout << "+" << string(WIDTH, '-') << "+" << endl;

    // نمایش اطلاعات بازی
    setCursorPosition(x, y++);
    string text = "| Score: ";
    text.append(to_string(score)).append("    Lives: ").append(to_string(lives)).append("    Level: ").append(to_string(level));
    text.append(string(WIDTH - text.length(), ' ')).append(" |");
    cout << text;

    setCursorPosition(x, y++);
    text = "| "; // نمایش نوار سلامت
    text.append(createHealthBarString(health));
    text.append(string(WIDTH - text.length(), ' ').append(" |"));
    cout << text;

    setCursorPosition(x, y++);
    text = "| ";
    text.append(string(17, ' ')).append(to_string(health)).append(" ");
    text.append(string(WIDTH - text.length(), ' ').append(" |"));
    cout << text;

    setCursorPosition(x, y++);
    text = "| Time Elapsed: ";
    text.append(to_string(elapsedTime));
    text.append(" seconds");
    text.append(string(WIDTH - text.length(), ' ').append(" |"));
    cout << text;

    setCursorPosition(x, y++);
    cout << "+" << string(WIDTH, '-') << "+" << endl;
}

void createGrid(string grid[])
{
    for (int i = 0; i < HEIGHT; ++i)
    {
        grid[i] = string(WIDTH, EMPTY); // مقداردهی هر خط با کاراکتر خالی
    }
}

void placeEntity(string grid[], const Position &pos, char symbol)
{
    if (pos.x >= 0 && pos.x < WIDTH && pos.y >= 0 && pos.y < HEIGHT)
    {
        grid[pos.y][pos.x] = symbol;
    }
}

void moveEnemies(Position enemies[], int enemyCount, bool &direction, int &speed)
{
    // حرکت افقی دشمنان
    for (int i = 0; i < enemyCount; ++i)
    {
        enemies[i].x += direction ? 1 : -1;
    }

    // بررسی تغییر جهت
    bool changeDirection = false;
    for (int i = 0; i < enemyCount; ++i)
    {
        if (enemies[i].x <= 0 || enemies[i].x >= WIDTH - 1)
        {
            changeDirection = true;
            break;
        }
    }

    // اگر تغییر جهت لازم است
    if (changeDirection)
    {
        direction = !direction;
        for (int i = 0; i < enemyCount; ++i)
        {
            enemies[i].y++; // حرکت عمودی دشمنان
        }
        speed = max(speed - 10, 10); // افزایش سرعت حرکت
    }
}

void shootBullet(Position bullets[], int &bulletCount, const Position &shooterPos, bool &bulletActive, int maxBullets)
{
    if (!bulletActive && bulletCount < maxBullets)
    {
        bullets[bulletCount++] = {shooterPos.x, shooterPos.y - 1, ' '}; // اضافه کردن گلوله به آرایه
        bulletActive = true;
    }
}

void shootEnemyBullet(Position enemyBullets[], int &enemyBulletCount, const Position enemies[], int enemyCount, int &globalCooldown, int level, int maxEnemyBullets)
{
    if (enemyCount > 0 && globalCooldown == 0)
    {
        int maxShots = 3 + level; // افزایش تعداد شلیک‌ها با افزایش سطح
        int shotsFired = 0;

        for (int i = 0; i < enemyCount; ++i)
        {
            if (shotsFired >= maxShots || enemyBulletCount >= maxEnemyBullets)
                break;

            int shootChance = 0;
            char bulletChar; // کاراکتر پیش‌فرض گلوله

            // تعیین شانس شلیک و نوع گلوله بر اساس نوع دشمن
            switch (enemies[i].type)
            {
            case ENEMY1:
                shootChance = 5; // شانس کمتر
                bulletChar = BULLET_ENEMY1;
                break;
            case ENEMY2:
                shootChance = 7; // شانس متوسط
                bulletChar = BULLET_ENEMY2;
                break;
            case ENEMY3:
                shootChance = 3; // شانس بیشتر
                bulletChar = BULLET_ENEMY3;
                break;
            }

            // شلیک گلوله اگر شانس شلیک برقرار باشد
            if (rand() % 100 < shootChance)
            {
                enemyBullets[enemyBulletCount++] = {enemies[i].x, enemies[i].y + 1, bulletChar};
                shotsFired++;
            }
        }
        globalCooldown = 10; // بازنشانی کول‌داون
    }
}

void moveBullets(Position bullets[], int &bulletCount, int direction)
{
    for (int i = 0; i < bulletCount; ++i)
    {
        bullets[i].y += direction;
    }
}

void checkCollisions(Position bullets[], int &bulletCount, Position enemies[], int &enemyCount, int &score, bool &bulletActive, Position &ufo, bool &ufoActive)
{
    for (int i = 0; i < bulletCount; ++i)
    {
        // بررسی برخورد گلوله بازیکن با دشمن‌ها
        for (int j = 0; j < enemyCount; ++j)
        {
            if (bullets[i].x == enemies[j].x && bullets[i].y == enemies[j].y)
            {
                switch (enemies[j].type)
                {
                case ENEMY1:
                    score += 10;
                    break;
                case ENEMY2:
                    score += 20;
                    break;
                case ENEMY3:
                    score += 40;
                    break;
                }
                enemies[j] = enemies[--enemyCount];  // حذف دشمن
                bullets[i] = bullets[--bulletCount]; // حذف گلوله بازیکن
                bulletActive = false;
                --i; // بررسی مجدد اندیس فعلی
                break;
            }
        }

        // بررسی برخورد گلوله بازیکن با بشقاب پرنده
        if (ufoActive && bullets[i].x == ufo.x && bullets[i].y == ufo.y)
        {
            score += 100;                        // امتیاز اضافی
            ufoActive = false;                   // بشقاب پرنده از بین می‌رود
            bullets[i] = bullets[--bulletCount]; // حذف گلوله بازیکن
            bulletActive = false;
            --i; // بررسی مجدد اندیس فعلی
            continue;
        }

        // اگر گلوله از صفحه خارج شد
        if (i >= 0 && bullets[i].y < 0)
        {
            bullets[i] = bullets[--bulletCount]; // حذف گلوله
            bulletActive = false;
            --i; // بررسی مجدد اندیس فعلی
        }
    }
}

void checkPlayerHit(Position enemyBullets[], int &enemyBulletCount, const Position &playerPos, int &health, int &lives, bool &gameOver)
{
    for (int i = 0; i < enemyBulletCount; ++i)
    {
        if (enemyBullets[i].x == playerPos.x && enemyBullets[i].y == playerPos.y)
        {
            int damage = 0;

            // تعیین میزان آسیب بر اساس نوع گلوله
            switch (enemyBullets[i].type)
            {
            case BULLET_ENEMY1:
                damage = 10;
                break;
            case BULLET_ENEMY2:
                damage = 5;
                break;
            case BULLET_ENEMY3:
                damage = 20;
                break;
            default:
                damage = 0;
            }

            health -= damage; // کاهش سلامت بازیکن بر اساس آسیب

            if (health <= 0)
            {
                lives--;
                if (lives <= 0)
                {
                    gameOver = true; // بازیکن تمام جان‌ها را از دست می‌دهد
                }
                else
                {
                    health = 100; // بازنشانی سلامت برای زندگی بعدی
                }
            }

            // حذف گلوله دشمن
            enemyBullets[i] = enemyBullets[--enemyBulletCount];
            --i; // بررسی مجدد اندیس فعلی
            continue;
        }

        // حذف گلوله‌های دشمن که از صفحه خارج شده‌اند
        if (enemyBullets[i].y >= HEIGHT)
        {
            enemyBullets[i] = enemyBullets[--enemyBulletCount];
            --i; // بررسی مجدد اندیس فعلی
        }
    }
}

void resetLevel(Position enemies[], int &enemyCount, Position bullets[], int &bulletCount, Position enemyBullets[], int &enemyBulletCount, int level)
{
    bulletCount = 0;      // پاکسازی گلوله‌های بازیکن
    enemyBulletCount = 0; // پاکسازی گلوله‌های دشمن
    enemyCount = 0;       // پاکسازی دشمن‌ها

    // اضافه کردن دشمن‌های ردیف بالا (نوع 3)
    for (int x = 5; x < WIDTH - 10; x += 4)
    {
        enemies[enemyCount++] = {x, 1, ENEMY3};
    }

    // اضافه کردن دشمن‌های ردیف میانی (نوع 2)
    for (int x = 5; x < WIDTH - 10; x += 4)
    {
        enemies[enemyCount++] = {x, 2, ENEMY2};
    }

    // اضافه کردن دشمن‌های دو ردیف پایین (نوع 1)
    for (int y = 3; y <= 4; ++y)
    {
        for (int x = 5; x < WIDTH - 10; x += 4)
        {
            enemies[enemyCount++] = {x, y, ENEMY1};
        }
    }
}

void moveUFO(Position &ufo, bool &ufoActive, bool &ufoMovingRight)
{
    if (!ufoActive && rand() % 100 < 2)
    { // 5% احتمال فعال شدن
        ufoActive = true;
        ufoMovingRight = rand() % 100 <= 49;
        ufo.x = ufoMovingRight ? 0 : WIDTH - 1; // شروع از چپ یا راست
        ufo.y = 0;
    }

    if (ufoActive)
    {
        ufo.x += ufoMovingRight ? 1 : -1;

        // اگر از صفحه خارج شد، غیر فعال شود
        if (ufo.x < 0 || ufo.x >= WIDTH)
        {
            ufoActive = false;
        }
    }
}

// تابع نمایش منو
int showMenu(int &lastX, int &lastY)
{
    int choice = 0;
    lastX = 0;
    lastY = 0;

    while (true)
    {
        system("cls"); // پاک کردن صفحه
        printLogo();

        // تنظیم اندازه کنسول
        int consoleWidth, consoleHeight;
        getConsoleSize(consoleWidth, consoleHeight);

        // مرکز کردن منو
        string menuTitle = "====== Main Menu ======";
        lastX = (consoleWidth - menuTitle.length()) / 2;
        lastY = 8; // موقعیت Y شروع منو

        // نمایش عنوان منو
        setCursorPosition(lastX, lastY++);
        cout << "\033[38;5;14m" << menuTitle << "\033[0m"; // رنگ آبی روشن برای عنوان
        setCursorPosition(lastX, lastY++);
        cout << " ";

        // نمایش گزینه‌ها با کادر
        int optionX = lastX - 2; // مکان X برای گزینه‌ها
        setConsoleColor(11, 0); // زرد روشن برای کادر
        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;11m╔════════════════════════════╗\033[0m"; // بالا
        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;11m║\033[0m" << "   1 - Start New Game       " << "\033[38;5;11m║\033[0m";
        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;11m╚════════════════════════════╝\033[0m"; // پایین

        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;10m╔════════════════════════════╗\033[0m"; // سبز روشن برای کادر
        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;10m║\033[0m" << "   2 - Load Game            " << "\033[38;5;10m║\033[0m";
        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;10m╚════════════════════════════╝\033[0m";

        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;13m╔════════════════════════════╗\033[0m"; // صورتی روشن
        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;13m║\033[0m" << "   3 - How to Play          " << "\033[38;5;13m║\033[0m";
        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;13m╚════════════════════════════╝\033[0m";

        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;12m╔════════════════════════════╗\033[0m"; // آبی روشن
        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;12m║\033[0m" << "   4 - Leaderboard          " << "\033[38;5;12m║\033[0m";
        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;12m╚════════════════════════════╝\033[0m";

        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;9m╔════════════════════════════╗\033[0m"; // قرمز روشن
        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;9m║\033[0m" << "   5 - Exit                 " << "\033[38;5;9m║\033[0m";
        setCursorPosition(optionX, lastY++);
        cout << "\033[38;5;9m╚════════════════════════════╝\033[0m";

        // خط پایین منو
        setCursorPosition(lastX, lastY++);
        cout << "\033[38;5;14m==========================\033[0m";

        // دریافت ورودی از کاربر
        setCursorPosition(lastX, lastY++);
        cout << "Enter your choice: ";
        cin >> choice;

        // بررسی ورودی معتبر
        if (choice >= 1 && choice <= 5)
        {
            break; // خروج از حلقه
        }
        else
        {
            setCursorPosition(lastX, lastY++);
            cout << "\033[38;5;196mInvalid choice! Please try again.\033[0m";
            _getch(); // توقف و انتظار برای فشردن کلید
        }
    }

    return choice; // بازگشت گزینه انتخاب‌شده
}

// تابع نمایش منوی درون‌بازی
int showPauseMenu(int &lastX, int &lastY)
{
    int choice = 0;
    lastX = 0;
    lastY = 0;

    while (true)
    {
        system("cls");

        int consoleWidth, height;
        getConsoleSize(consoleWidth, height);

        string menu = "====== Pause Menu ======";

        lastX = (consoleWidth - menu.length()) / 2;
        lastY = 10;

        setCursorPosition(lastX, lastY++);
        cout << menu;

        setCursorPosition(lastX, lastY++);
        cout << "1- Restart Game";
        setCursorPosition(lastX, lastY++);
        cout << "2- Resume Game";
        setCursorPosition(lastX, lastY++);
        cout << "3- Exit and Save Game";
        setCursorPosition(lastX, lastY++);
        cout << "========================";
        setCursorPosition(lastX, lastY++);
        cout << "Enter your choice: ";
        cin >> choice;

        if (choice >= 1 && choice <= 3)
        {
            break; // انتخاب معتبر
        }
        else
        {
            setCursorPosition(lastX, lastY++);
            cout << "Invalid choice! Please select between 1 and 3.";
            _getch();
        }
    }

    return choice;
}

void showHowToPlay()
{
    system("cls");

    int consoleWidth, height;
    getConsoleSize(consoleWidth, height);

    string menu = "====== How to Play ======";

    int x = (consoleWidth - menu.length()) / 2;
    int y = 10;
    setCursorPosition(x, y++);
    cout << menu;
    setCursorPosition(x, y++);
    cout << "1. Use 'A' to move left.";
    setCursorPosition(x, y++);
    cout << "2. Use 'D' to move right.";
    setCursorPosition(x, y++);
    cout << "3. Press 'Space' to shoot.";
    setCursorPosition(x, y++);
    cout << "4. Press 'P' to pause the game and show the menu.";
    setCursorPosition(x, y++);
    cout << "5. Defend yourself and destroy the enemies.";
    setCursorPosition(x, y++);
    cout << "6. Avoid enemy bullets and keep your health above 0.";
    setCursorPosition(x, y++);
    cout << "==========================";
    _getch();
}

void showLeaderboard()
{
    system("cls");
    displayLeaderboard();
    _getch();
}

void saveGameFull(int width, string playerName, int score, int health, int lives, int level, long long elapsedTime,
                  const Position &playerPos, const Position enemies[], int enemyCount,
                  const Position bullets[], int bulletCount, const Position enemyBullets[], int enemyBulletCount,
                  const Shield shields[], int shieldCount, bool direction, bool bulletActive,
                  bool gameOver, const Position &ufo, int globalCooldown, bool ufoActive,
                  bool ufoMovingRight, int enemyMovementSpeedCounter, int enemyMovementSpeedThreshold)
{
    ofstream saveFile("savefile.txt");
    if (saveFile.is_open())
    {
        // ذخیره متغیرهای اصلی
        saveFile << width << endl;
        saveFile << playerName << endl;
        saveFile << score << endl;
        saveFile << health << endl;
        saveFile << lives << endl;
        saveFile << level << endl;
        saveFile << elapsedTime << endl;
        saveFile << playerPos.x << " " << playerPos.y << endl;

        // ذخیره دشمنان
        saveFile << enemyCount << endl;
        for (int i = 0; i < enemyCount; ++i)
        {
            saveFile << enemies[i].x << " " << enemies[i].y << " " << enemies[i].type << endl;
        }

        // ذخیره گلوله‌های بازیکن
        saveFile << bulletCount << endl;
        for (int i = 0; i < bulletCount; ++i)
        {
            saveFile << bullets[i].x << " " << bullets[i].y << " " << bullets[i].type << endl;
        }

        // ذخیره گلوله‌های دشمن
        saveFile << enemyBulletCount << endl;
        for (int i = 0; i < enemyBulletCount; ++i)
        {
            saveFile << enemyBullets[i].x << " " << enemyBullets[i].y << " " << enemyBullets[i].type << endl;
        }

        // ذخیره شیلدها
        saveFile << shieldCount << endl; // تعداد شیلدها
        for (int i = 0; i < shieldCount; ++i)
        {
            saveFile << shields[i].x << " " << shields[i].y << " " << shields[i].health << endl; // مختصات و سلامت شیلد

            // ذخیره شکل شیلد، خط به خط
            for (int row = 0; row < SHIELD_MAX_ROW; ++row)
            {
                for (int col = 0; col < SHIELD_MAX_COL; ++col)
                {
                    saveFile << shields[i].shape[row][col]; // ذخیره هر کاراکتر
                }
                saveFile << endl; // پایان هر ردیف
            }
        }

        // ذخیره متغیرهای دیگر
        saveFile << direction << endl;
        saveFile << bulletActive << endl;
        saveFile << gameOver << endl;
        saveFile << ufo.x << " " << ufo.y << " " << ufo.type << endl;
        saveFile << globalCooldown << endl;
        saveFile << ufoActive << endl;
        saveFile << ufoMovingRight << endl;
        saveFile << enemyMovementSpeedCounter << endl;
        saveFile << enemyMovementSpeedThreshold << endl;

        saveFile.close();
    }
    else
    {
        cerr << "Error: Unable to save the game!" << endl;
    }
}

bool loadGameFull(int &width, string &playerName, int &score, int &health, int &lives, int &level, long long &elapsedTime,
                  Position &playerPos, Position enemies[], int &enemyCount,
                  Position bullets[], int &bulletCount, Position enemyBullets[], int &enemyBulletCount,
                  Shield shields[], int &shieldCount, bool &direction, bool &bulletActive,
                  bool &gameOver, Position &ufo, int &globalCooldown, bool &ufoActive,
                  bool &ufoMovingRight, int &enemyMovementSpeedCounter, int &enemyMovementSpeedThreshold)
{
    ifstream loadFile("savefile.txt");
    if (loadFile.is_open())
    {
        // خواندن متغیرهای اصلی
        loadFile >> width;
        loadFile >> playerName;
        loadFile >> score;
        loadFile >> health;
        loadFile >> lives;
        loadFile >> level;
        loadFile >> elapsedTime;
        loadFile >> playerPos.x >> playerPos.y;

        // خواندن دشمنان
        loadFile >> enemyCount;
        for (int i = 0; i < enemyCount; ++i)
        {
            loadFile >> enemies[i].x >> enemies[i].y >> enemies[i].type;
        }

        // خواندن گلوله‌های بازیکن
        loadFile >> bulletCount;
        for (int i = 0; i < bulletCount; ++i)
        {
            loadFile >> bullets[i].x >> bullets[i].y >> bullets[i].type;
        }

        // خواندن گلوله‌های دشمن
        loadFile >> enemyBulletCount;
        for (int i = 0; i < enemyBulletCount; ++i)
        {
            loadFile >> enemyBullets[i].x >> enemyBullets[i].y >> enemyBullets[i].type;
        }

        // خواندن شیلدها
        loadFile >> shieldCount; // تعداد شیلدها
        for (int i = 0; i < shieldCount; ++i)
        {
            loadFile >> shields[i].x >> shields[i].y >> shields[i].health; // خواندن مختصات و سلامت شیلد
            loadFile.ignore();                                             // حذف کاراکتر اضافی '\n' بعد از خواندن سلامت

            // بازیابی شکل شیلد
            for (int row = 0; row < SHIELD_MAX_ROW; ++row)
            {
                string line;
                getline(loadFile, line); // خواندن هر خط
                for (int col = 0; col < SHIELD_MAX_COL; ++col)
                {
                    shields[i].shape[row][col] = line[col];
                }
            }
        }

        // خواندن متغیرهای دیگر
        loadFile >> direction;
        loadFile >> bulletActive;
        loadFile >> gameOver;
        loadFile >> ufo.x >> ufo.y >> ufo.type;
        loadFile >> globalCooldown;
        loadFile >> ufoActive;
        loadFile >> ufoMovingRight;
        loadFile >> enemyMovementSpeedCounter;
        loadFile >> enemyMovementSpeedThreshold;

        loadFile.close();
        return true;
    }
    else
    {
        cerr << "Error: No saved game found!" << endl;
        return false;
    }
}

void initShields(Shield shields[])
{
    Shield temp[] = {
        {10, HEIGHT - 5, 9, {{'#', '#', '#', '#', '#'}, {'#', ' ', '9', ' ', '#'}, {'#', ' ', ' ', ' ', '#'}}},
        {20, HEIGHT - 5, 9, {{'#', '#', '#', '#', '#'}, {'#', ' ', '9', ' ', '#'}, {'#', ' ', ' ', ' ', '#'}}},
        {30, HEIGHT - 5, 9, {{'#', '#', '#', '#', '#'}, {'#', ' ', '9', ' ', '#'}, {'#', ' ', ' ', ' ', '#'}}}};
    for (int i = 0; i < 3; ++i)
    {
        shields[i] = temp[i];
    }
}

void initGameParameters(Position &playerPos, int &enemyCount, int &bulletCount, int &enemyBulletCount,
                        int &shieldCount, const int MAX_SHIELDS, Shield shields[], int &level, bool &direction,
                        int &score, bool &bulletActive, bool &gameOver, int &enemySpeed, int &health, int &lives,
                        int &globalCooldown, long long &elapsedTime, Position &ufo, bool &ufoActive, bool &ufoMovingRight,
                        int &enemyMovementSpeedCounter, int &enemyMovementSpeedThreshold)
{

    // تنظیمات اولیه بازی
    playerPos = {WIDTH / 2, HEIGHT - 1, ' '};
    enemyCount = 0;
    bulletCount = 0;
    enemyBulletCount = 0;
    initShields(shields);
    shieldCount = MAX_SHIELDS;
    level = 1;        // مرحله اولیه بازی
    direction = true; // جهت حرکت دشمنان
    score = 0;
    bulletActive = false;
    gameOver = false;
    enemySpeed = 100;   // سرعت اولیه دشمنان
    health = 100;       // سلامت بازیکن
    lives = 2;          // تعداد جان‌ها
    globalCooldown = 0; // کول‌داون جهانی شلیک دشمنان
    elapsedTime = 0;
    ufo = {-1, 0, UFO}; // بشقاب پرنده ابتدا غیرفعال است
    ufoActive = false;
    ufoMovingRight = rand() % 100 <= 49; // جهت حرکت بشقاب پرنده
    enemyMovementSpeedCounter = 0;       // شمارنده برای مدیریت سرعت دشمنان
    enemyMovementSpeedThreshold = 3;     // هر چند حلقه یکبار دشمنان حرکت کنند
}

