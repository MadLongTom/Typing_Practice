#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>
#include <time.h>

double sumTimeSpan = 0, sumChar = 0;// record total time span and total char count

int wrongCount[26] = {0};// record wrong count of each char
int enterCount[26] = {0};// record enter count of each char
double delayTime[26] = {0};// record delay time of each char

// build a timer,I can let it start,stop,reset and read its timespan
typedef struct Timer
{
    LARGE_INTEGER start;
    LARGE_INTEGER stop;
    LARGE_INTEGER frequency;
    bool isRunning;
} Timer;
/**
 * @brief start timer
 * 
 * @param timer
 */
void Timer_Start(Timer *timer)
{
    QueryPerformanceFrequency(&timer->frequency);
    QueryPerformanceCounter(&timer->start);
    timer->isRunning = true;
}
/**
 * @brief stop timer
 * 
 * @param timer 
 */
void Timer_Stop(Timer *timer)
{
    QueryPerformanceCounter(&timer->stop);
    timer->isRunning = false;
}
/**
 * @brief get timer timespan
 * 
 * @param timer 
 */
double Timer_GetTimeSpan(Timer *timer)
{
    return (double)(timer->stop.QuadPart - timer->start.QuadPart) / (double)timer->frequency.QuadPart;
}

void Init_Console()
{
    system("chcp 65001");// set console code page to utf-8
    system("cls");
    //disable quick edit mode
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dwMode;
    GetConsoleMode(hInput, &dwMode);
    dwMode &= ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(hInput, dwMode);
    //hide cursor
    CONSOLE_CURSOR_INFO cursor_info = {1, 0};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

void PrintDetailChart()
{
    // format: char,wrong rate,average delay time
    // combine three chars in one row
    wprintf(L"\n\nCHAR\tWRONG RATE\tDELAY TIME\tCHAR\tWRONG RATE\tDELAY TIME\tCHAR\tWRONG RATE\tDELAY TIME\n\n");
    int i = 0, count = 0;
    for (int j = 0; j < 26; j++)
    {
        double wrongRate;
        double avgDelayTime;
        if (enterCount[j] == 0)
        {
            wrongRate = 0;
            avgDelayTime = 0;
        }
        else
        {
            wrongRate = (double)wrongCount[j] / (double)enterCount[j];
            avgDelayTime = delayTime[j] / (double)enterCount[j];
        }
        if (count == 3)
        {
            count = 0;
            wprintf(L"\n\n");
        }
        wprintf(L"%lc\t%lf\t%lf\t", j + L'A', wrongRate, avgDelayTime);
        i++;
        count++;
    }
}

int main()
{
    Init_Console();
    // clear console
    system("cls");
    // wchar_t *words[5] = {L"hello", L"world", L"this", L"is", L"test"};
    //  let words read from file "words.txt" and generate its count
    FILE *fp = fopen("words.txt", "r");
    if (fp == NULL)
    {
        wprintf(L"Can't open file words.txt");
        return 0;
    }
    int count = 0;
    // get count
    while (!feof(fp))
    {
        wchar_t c = fgetwc(fp);
        if (c == L'\n')
            count++;
    }
    wchar_t **words = (wchar_t **)malloc(sizeof(wchar_t *) * count);
    fclose(fp);
    // read words from file
    fp = fopen("words.txt", "r");
    for (int i = 0; i < count; i++)
    {
        words[i] = (wchar_t *)malloc(sizeof(wchar_t) * 100);
        fgetws(words[i], 100, fp);
        words[i][wcslen(words[i]) - 1] = L'\0';
    }
    fclose(fp);
    // randomize words
    srand((unsigned)time(NULL));
    for (int i = 0; i < count; i++)
    {
        int index = rand() % count;
        wchar_t *temp = words[i];
        words[i] = words[index];
        words[index] = temp;
    }
    Timer timer, timer1;
    for (int j = 0; j < count; j++)
    {
        bool isWrong = false;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        wprintf(words[j]);
        // set cursor position to 0,0
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){0, 0});

        for (int i = 0; i < wcslen(words[j]); i++)
        {
            wchar_t c;
            if (!isWrong)
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
            else
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
            // print this char
            wprintf(L"%lc", words[j][i]);
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){i, 0});
            Timer_Start(&timer1);
            c = _getwch();
            Timer_Stop(&timer1);
            if (!timer.isRunning)
                Timer_Start(&timer);
            if (c == words[j][i])
            {
                // use gray color print this char
                if (!isWrong)
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
                else
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
                wprintf(L"%lc", c);
                isWrong = false;
                enterCount[c - L'a'] += 1;
                if (i != 0)
                {
                    delayTime[c - L'a'] += Timer_GetTimeSpan(&timer1);
                }
            }
            else
            {
                // use darkred color print this char
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
                wprintf(L"%lc", words[j][i]);
                // move cursor back
                SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){i, 0});
                i--;
                if (!isWrong)
                    wrongCount[c - L'a'] += 1;
                isWrong = true;
            }
        }
        Timer_Stop(&timer);
        double span = Timer_GetTimeSpan(&timer);
        // calculate speed
        double speed = wcslen(words[j]) / span;
        sumTimeSpan += span;
        sumChar += wcslen(words[j]);
        // print speed
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){0, 1});
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        wprintf(L"\nSpeed:\t\t%.2lf \tchar/sec", speed);
        wprintf(L"\nTimeSpan:\t%.2lf \ts\n", span);
        // print sum span and speed
        wprintf(L"\nSumSpeed:\t%.2lf \tchar/sec", sumChar / sumTimeSpan);
        wprintf(L"\nSumTimeSpan:\t%.2lf \ts", sumTimeSpan);
        // print wrong rate
        // calculate sumWrongCount
        int sumWrongCount = 0;
        for (int j = 0; j < 26; j++)
        {
            sumWrongCount += wrongCount[j];
        }
        wprintf(L"\nWrongRate:\t%.2lf \t%%", (double)sumWrongCount / (double)sumChar * 100);
        // calculate confidence based on delay time, As the average delayTime is lower, confidence should approaches 1
        PrintDetailChart();
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){0, 0});
        // build a string to cover word
        wchar_t *cover = (wchar_t *)malloc(sizeof(wchar_t) * (wcslen(words[j]) + 1));
        for (int i = 0; i < wcslen(words[j]); i++)
        {
            cover[i] = L' ';
        }
        cover[wcslen(words[j])] = L'\0';
        wprintf(cover);
        free(cover);
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){0, 0});
    }
    _getwch();
}