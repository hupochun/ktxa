#ifndef UTIL_H
#define UTIL_H


#include <QString>
#include <QBuffer>
#include <QImage>
#include <QRect>
#include <QMap>
#include "Windows.h"
#include <iostream>
#include <qjsonobject.h>
#include <QString>
#include <QRandomGenerator>


enum window_priority {
    WINDOW_PRIORITY_CLASS,
    WINDOW_PRIORITY_TITLE,
    WINDOW_PRIORITY_EXE,
};

enum window_search_mode {
    INCLUDE_MINIMIZED,
    EXCLUDE_MINIMIZED
};

const int tolerance = 2;

// 比较两个box对象的坐标顺序，先按y排序，再按x排序，考虑tolerance
bool compareBoxes(const QJsonObject &a, const QJsonObject &b);

QString generateRandomString(int length);

// 函数用于将图像数据转换为Base64编码的字符串
QString imageToBase64(const QImage &image);

QString getSelectedRectJPGImageBase64Data(QRect geo);

bool isRectValid(QRect &rect);

QString getOcrResult(QString respData);

QString getllmAnswer(QString respData);

// 模拟 Ctrl+V 粘贴操作
void SimulatePaste();

void SimulateMouseClick(int x, int y);


bool check_window_valid(HWND window, enum window_search_mode mode);

HWND next_window(HWND window, enum window_search_mode mode);

HWND first_window(enum window_search_mode mode);

int window_rating(HWND window,
        enum window_priority priority,
        QString className,
        QString title,
        QString exe);

HWND find_window(enum window_search_mode mode,
        enum window_priority priority,
        QString className,
        QString title,
        QString exe);

QRect getWindowRect(HWND hwnd);

QString get_window_exe(HWND window);
QString get_window_title(HWND hwnd);
QString get_window_class(HWND hwnd);

QMap<HWND, QString> getWindowList(window_search_mode mode);


BOOL isWndGetFocus(HWND hwnd);

// 回调函数原型
BOOL CALLBACK IsDescendantEnumProc(HWND hwnd, LPARAM lParam);

BOOL IsDescendant(HWND childWnd, HWND parentWnd);

bool isValidTime(const QString &timeStr);

#endif // UTIL_H
