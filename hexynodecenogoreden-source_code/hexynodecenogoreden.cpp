#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

HDC hdc;

int rnd(int minv, int maxv) {
    if (minv > maxv) return minv;
    if (minv == maxv) return minv;   // защита от деления на ноль в %
    return minv + rand() % (maxv - minv + 1);
}

// Случайный символ всего юникода.
// Пишет 1 или 2 wchar_t в buf, возвращает длину (1 или 2).
int randUnicode(wchar_t* buf) {
    // Равные шансы у каждого блока, CJK — один блок, а не половина юникода
    static const struct { int lo, hi; } blocks[] = {
        { 0x0021,  0x024F },  // латиница+расширения
        { 0x0370,  0x052F },  // греческий и кириллица
        { 0x0530,  0x07FF },  // армянский, иврит, арабский
        { 0x0900,  0x0DFF },  // индийские письменности
        { 0x10A0,  0x10FF },  // грузинский
        { 0x1400,  0x167F },  // канадское слоговое
        { 0x1680,  0x169F },  // огам
        { 0x16A0,  0x16FF },  // руны
        { 0x1700,  0x18AF },  // тагальский, тибетский и прочие
        { 0x1E00,  0x1FFF },  // латиница-расширения, греческий-расширения
        { 0x2000,  0x23FF },  // пунктуация и техсимволы
        { 0x2500,  0x27BF },  // псевдографика, стрелки, дингбаты
        { 0x2C60,  0x2FFF },  // редкие латинские, коптский
        { 0x3000,  0x33FF },  // кана и CJK-символы-«мелочь»
        { 0x4E00,  0x4FFF },  // немного CJK — только кусок, не весь
        { 0xA000,  0xA4CF },  // йи
        { 0xA720,  0xAFFF },  // модификаторы
        { 0xFB00,  0xFEFF },  // формы букв, вертикальный текст
        { 0x10000, 0x101FF }, // линейное письмо Б
        { 0x10300, 0x103FF }, // староперсидский
        { 0x103A0, 0x103FF }, // клинопись (старая)
        { 0x10400, 0x104FF }, // дезерет
        { 0x10800, 0x10FFF }, // кипрское, финикийское
        { 0x11000, 0x11FFF }, // брагми и прочие индийские
        { 0x12000, 0x123FF }, // клинопись
        { 0x13000, 0x1342F }, // египетские иероглифы
        { 0x14400, 0x1467F }, // анатолийские иероглифы
        { 0x16800, 0x16A3F }, // пальмира
        { 0x1B000, 0x1B0FF }, // катакана фонетическая
        { 0x1D400, 0x1D7FF }, // математические буквы
        { 0x1F300, 0x1FAFF }, // эмодзи
        { 0x2F800, 0x2FA1F }, // CJK-совместимость (немного)
    };
    int n = sizeof(blocks) / sizeof(blocks[0]);
    int r = rnd(0, n - 1);
    int c = rnd(blocks[r].lo, blocks[r].hi);

    if (c >= 0xD800 && c <= 0xDFFF) c = 0x263A;

    if (c < 0x10000) {
        buf[0] = (wchar_t)c;
        return 1;
    } else {
        c -= 0x10000;
        buf[0] = (wchar_t)(0xD800 + (c >> 10));
        buf[1] = (wchar_t)(0xDC00 + (c & 0x3FF));
        return 2;
    }
}

void myBlt(int x, int y, int w, int h, int sx, int sy, DWORD rop) {
    BitBlt(hdc, x, y, w, h, hdc, sx, sy, rop);
}

void drawText(int x, int y) {
    const wchar_t* phrase = L"hexynodecenogoreden.exe";

    const wchar_t* fonts[] = { L"Consolas", L"Arial", L"Times New Roman",
                               L"Courier New", L"Impact", L"Comic Sans MS",
                               L"Verdana", L"Georgia" };
    int fontCount = sizeof(fonts) / sizeof(fonts[0]);

    wchar_t buf[128];
    int len;
    bool isName;

    if (rnd(0, 4) == 0) {
        len = lstrlenW(phrase);
        lstrcpyW(buf, phrase);
        isName = true;
    } else {
        len = 0;
        while (len < rnd(4, 24)) {
            len += randUnicode(buf + len);
        }
        buf[len] = 0;
        isName = false;
    }

    int size, weight, italic, underline, strikeout;
    const wchar_t* fontName;

    if (isName) {
        // Имя программы: всегда одинаковые шрифт и стиль
        size      = 30;
        weight    = 700;
        italic    = 0;
        underline = 0;
        strikeout = 0;
        fontName  = L"Consolas";
    } else {
        // Случайный юникод: рандом как раньше
        size      = rnd(30, 30);
        weight    = rnd(100, 900) / 100 * 100;
        italic    = rnd(0, 1);
        underline = rnd(0, 1);
        strikeout = rnd(0, 1);
        fontName  = fonts[rnd(0, fontCount - 1)];
    }

    HFONT font = CreateFontW(size, 0, 0, 0, weight,
                             italic, underline, strikeout,
                             DEFAULT_CHARSET, 0, 0, 0, 0,
                             fontName);
    HFONT old = (HFONT)SelectObject(hdc, font);

    SetTextColor(hdc, RGB(rnd(-255, 255), rnd(-255, 255), rnd(-255, 255)));
    SetBkColor(hdc,  RGB(rnd(-255, 255), rnd(-255, 255), rnd(-255, 255)));
    SetBkMode(hdc, rnd(0, 1) ? OPAQUE : TRANSPARENT);

    TextOutW(hdc, x, y, buf, len);

    SelectObject(hdc, old);
    DeleteObject(font);
}

void programms() {
    while (true) {
        system("start calc");
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        system("start notepad");
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));;
        system("start explorer");
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        system("start mspaint");
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        ShellExecuteA(NULL, "open", "https://www.google.com/search?q=как+скачать+антивирус&sca_esv=0bb81005916a86f4&sxsrf=AKL5yDi2IwR8UqehGZYJENRPpEafSwl-KQ%3A1794653209919&ei=GTz4asfLN6WOxc8Po4vJmAY&biw=1920&bih=936&uact=5&oq=как+скачать+антивирус&gs_lp=Egxnd3Mtd2l6LXNlcnAiKNC60LDQuiDRgdC60LDRh9Cw0YLRjCDQsNC90YLQuNCy0LjRgNGD0YEyBRAAGIAEMggQABiABBi0BzIFEAAYgAQyBRAAGIAEMggQABiABBi0BzIIEAAYgAQYtAcyCBAAGIAEGLQHMggQABiABBi0BzIIEAAYgAQYtAcyCBAAGIAEGLQHSPctUABY2iNwAXgBkAEAmAHWA6ABuhWqAQg4LjEyLjQtMbgBA8gBAPgBAZgCFqACoheoAhTCAhQQABjlBhiKAxjnBhjqAhi0AtgBAcICEBAAGAMYjwEY6gIYtALYAQLCAgsQABiABBixAxiDAcICDhAAGIAEGIoFGLEDGIMBwgIOEC4YgAQYigUYsQMYgwHCAgUQLhiABMICERAAGIAEGIoFGI0GGLEDGIMBwgIIEAAYgAQYsQPCAg0QABiABBiKBRhDGLEDwgIKEAAYgAQYigUYQ8ICCxAAGIAEGIoFGLEDwgINEAAYgAQYigUYQxi0B8ICCxAAGIAEGLEDGLQHmAMP8QWOHQ3RvpAd1boGBAgBGAe6BgYIAhABGAqSBwgxLjIwLjQtMaAHt7YBsgcIMC4yMC40LTG4B5MXwgcGMi0xNi42yAemAYAIAQ&sclient=gws-wiz-serp", 0, 0, SW_SHOWNORMAL);
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    }
}
int main() {
    FreeConsole();
    srand((unsigned)time(nullptr));
    hdc = GetDC(0);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    POINT p;
    int stage = 1;
    int frame = 0;
    int totalStages = 2;
    DWORD lastSwitch = GetTickCount();
    std::thread programms_thread(programms);
    programms_thread.detach();
    while (true) {
        GetCursorPos(&p);
        SetCursorPos(p.x + (rnd(-3,3)), p.y + (rnd(-3,3)));

        if (GetTickCount() - lastSwitch >= rnd(10000, 20000)) {  // смена стадии раз в 30 секунд
            stage++;
            lastSwitch = GetTickCount();
        }
        if (stage > totalStages) {
            break;
        }

        if (stage == 1) {
            myBlt(0, 1, w, h, rnd(0, 0), rnd(0, 0), DSTINVERT);
            myBlt(0, 1, w, h, rnd(-20, 20), rnd(-20, 20), SRCCOPY);
            myBlt(0, 0, rnd(-w, w), rnd(-h, h), rnd(-1, 1), rnd(-1, 1), SRCCOPY);
            SetCursorPos(rnd(-w, w), rnd(-h, h));

            frame++;
            if (frame % 1 == 0) {
                drawText(rnd(0, w - 400), rnd(0, h - 100));
            }
        } else if (stage == 2) {
            int rx = rnd(-w, w);   // куда
            int ry = rnd(-h, h);
            int rw = rnd(-w, w);   // размер куска
            int rh = rnd(-h, h);
            int dx = rnd(-1, 1);   // сдвиг: -1, 0 или +1 по X
            int dy = rnd(-1, 1);   // сдвиг: -1, 0 или +1 по Y

            myBlt(rx, ry, rw, rh, rx + dx, ry + dy, SRCCOPY);
            myBlt(rnd(-w, w), rnd(-h,h), rnd(-w, w), rnd(-h,h), rnd(-w, w), rnd(-h,h), DSTINVERT);
            myBlt(0, rnd(-h,h), w, rnd(-h,h), 0, rnd(0, h), SRCCOPY);
        }
    }
}
