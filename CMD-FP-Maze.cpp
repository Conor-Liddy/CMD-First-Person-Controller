/*
   * @Author: Conor Liddy
   * 
   * First person game that runs in CMD
   * 
   * C++
*/

#include <iostream>
#include <Windows.h>
#include <chrono>
#include <vector>
#include <algorithm>

using namespace std;

// Set up screen dimentions for console
int nScreenWidth = 120;
int nScreenHeight = 40;


// Map constants
int nMapHeight = 18;
int nMapWidth = 18;

// Player variables
float fPlayerX = nMapWidth / 2.0f;
float fPlayerY = nMapHeight / 2.0f;
float fPlayerA = 3.3f;      // Angle
float fFOV = 3.14159 / 4.0;   // Field of view
float fDepth = 16.0f;   // Field of view

float fTurnSpeed = 1.5f;
float fMoveSpeed = 5.0f;


int main()
{
    // Create screen buffer
    wchar_t* screen = new wchar_t[nScreenWidth*nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // Map setup
    wstring map;

    map += L"##################"; // 1
    map += L"#.......#........#"; // 2
    map += L"#...#...#........#"; // 3
    map += L"#.......#...######"; // 4
    map += L"#.......#...#....#"; // 5
    map += L"#...#.......#....#"; // 6
    map += L"#...#...#...#....#"; // 7
    map += L"#...#.......#....#"; // 8
    map += L"#...#.......#....#"; // 9
    map += L"#...#########....#"; // 10
    map += L"#.......#........#"; // 11
    map += L"#.......#........#"; // 12
    map += L"#..#....#...######"; // 13
    map += L"#..#.............#"; // 14
    map += L"#..#.............#"; // 15
    map += L"#..#.............#"; // 16
    map += L"#..#.............#"; // 17
    map += L"##################"; // 18

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    // Game loop
    while (1) {
        // Timer
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();


        // Player Controls
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)     // Turn left
            fPlayerA -= (fTurnSpeed) * fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)     // Turn right
            fPlayerA += (fTurnSpeed) * fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {   // Move forward
            fPlayerX += sinf(fPlayerA) * fMoveSpeed * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * fMoveSpeed * fElapsedTime;

            // Colission detection
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX -= sinf(fPlayerA) * fMoveSpeed * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * fMoveSpeed * fElapsedTime;
            }
        }


        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {   // Move backward
            fPlayerX -= sinf(fPlayerA) * fMoveSpeed * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * fMoveSpeed * fElapsedTime;

            // Colission detection
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX += sinf(fPlayerA) * fMoveSpeed * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * fMoveSpeed * fElapsedTime;
            }
        }


        for (int x = 0; x < nScreenWidth; x++) {
            // For each column, cast ray into world space
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
            float fDistanceToWall = 0;
            bool bHitWall = false;
            bool bBoundry = false;

            float fEyeX = sinf(fRayAngle);  // Unit vector for ray in player space
            float fEyeY = cosf(fRayAngle);

            while (!bHitWall && fDistanceToWall < fDepth) {
                fDistanceToWall += 0.1f;

                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                // Test if out of bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                }
                else {
                    if (map[nTestY * nMapWidth + nTestX] == '#') {
                        bHitWall = true;

                        // Highlight corners

                        vector<pair<float, float>> p;   // Distance, Dot

                        for (int tx = 0; tx < 2; tx++) {
                            for (int ty = 0; ty < 2; ty++) {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }
                        }

                        // Sort pairs close to far
                        sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });  //long lambda function  

                        float fBound = 0.005;
                        if (acos(p.at(0).second) < fBound) bBoundry = true;
                        if (acos(p.at(1).second) < fBound) bBoundry = true;
                        //if (acos(p.at(2).second) < fBound) bBoundry = true;
                    }
                }

            }

            // Calculate distance to ceiling and floor
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            // Set up distance based shading
            short nShade = ' ';

            if (fDistanceToWall <= fDepth / 4.0f)       nShade = 0x2588;    // Close
            else if (fDistanceToWall < fDepth / 3.0f)   nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)   nShade = 0x2592;
            else if (fDistanceToWall < fDepth)          nShade = 0x2591;
            else                                        nShade = ' ';       // Far

            if (bBoundry) nShade = '|';

            for (int y = 0; y < nScreenHeight; y++) {
                if (y < nCeiling) {
                    screen[y * nScreenWidth + x] = ' ';
                }
                else if (y > nCeiling && y <= nFloor) {
                    screen[y * nScreenWidth + x] = nShade;
                }
                else {
                    // Shade floor based on distance
                    short nShade2 = ' ';

                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    ///*
                    if (b < 0.25f)       nShade2 = '#';      // Close
                    else if (b < 0.5f)   nShade2 = 'x';
                    else if (b < 0.75f)  nShade2 = '.';
                    else if (b < 0.9f)   nShade2 = '-';
                    else                 nShade2 = ' ';      // Far

                    screen[y * nScreenWidth + x] = nShade2;
                    //*/

                    //screen[y * nScreenWidth + x] = ' ';
                }
            }

        }

        // Display stats
        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

        // Display Map
        for (int nx = 0; nx < nMapWidth; nx++)
            for (int ny = 0; ny < nMapWidth; ny++)
            {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
            }

        screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';

        // Write to screen
        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
    }

    return 0;
}

