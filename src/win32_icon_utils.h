#ifndef LEDIT_WIN32_ICON_UTILS
#define LEDIT_WIN32_ICON_UTILS

#include <string>
#include <vector>
#include <windows.h>
#define IDI_ICON1 101

inline std::string winStrToStr(LPWSTR lpwstr) {
    std::wstring ws(lpwstr);
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8String(bufferSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &utf8String[0], bufferSize, nullptr, nullptr);
    if (!utf8String.empty() && utf8String.back() == '\0') {
        utf8String.pop_back();
    }
    return utf8String;
}
struct IconData {
    int width, height;
    std::vector<int8_t> pixels;

    IconData(int w, int h) : width(w), height(h), pixels(w * h * 4) {} // Assuming 4 bytes per pixel (RGBA)
};
inline IconData ExtractIconData(HICON hIcon) {
    ICONINFO iconInfo;
    BITMAP bm;
    HDC hdcScreen, hdcMem;
    HBITMAP hbmOld;
    GetIconInfo(hIcon, &iconInfo);
    hdcScreen = GetDC(NULL);
    hdcMem = CreateCompatibleDC(hdcScreen);
    hbmOld = (HBITMAP)SelectObject(hdcMem, iconInfo.hbmColor);
    GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bm);
    IconData iconData(bm.bmWidth, bm.bmHeight);
    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = -bm.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32; 
    bi.biCompression = BI_RGB;
    GetDIBits(hdcMem, iconInfo.hbmColor, 0, bm.bmHeight, iconData.pixels.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
    for(size_t i = 0; i < iconData.width * iconData.height * 4;i+=4){
        std::swap(iconData.pixels[i], iconData.pixels[i+2]);
    }
    return iconData;
}
#endif