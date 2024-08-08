#include "util.h"
#include <QApplication>
#include <qpixmap.h>
#include <QScreen>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <windows.h> // for SendInput
#include <psapi.h>
#include <QDebug>
#include <QImageWriter>
#include <qdatetime.h>
#include <QFile>

QString generateRandomString(int length)
{
    // 可能的字符集
    const QString possibleChars = "0123456789"
                                  "abcdefghijklmnopqrstuvwxyz"
                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    QRandomGenerator *generator = QRandomGenerator::global();

    QString randomString;
    for (int i = 0; i < length; ++i) {
        int index = generator->bounded(possibleChars.size());
        QChar nextChar = possibleChars[index];
        randomString.append(nextChar);
    }

    return randomString;
}

// 函数用于将图像数据转换为Base64编码的字符串
QString imageToBase64(const QImage &image)
{
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);

    // 创建一个QImageWriter实例，并设置输出设备和格式
    QImageWriter writer(&buffer, "PNG");
    // 设置JPEG的质量
    writer.setQuality(100); // 这里的数值可以根据需要调整

    if (!writer.write(image)) {
        // 处理写入失败的情况
        qDebug() << "Failed to save image to buffer";
    }

    buffer.close();

    //image.save(&buffer, "JPG"); // 保存为JPEG格式
    image.save("D:/test_img.png");

    //return imageData.toBase64().replace('\n', ""); // 转换为Base64并去除换行符
    return imageData.toBase64();
}


QString getSelectedRectJPGImageBase64Data(QRect geo){
    if(geo.width() == 0 ||
            geo.height() == 0){
        return "";
    }

    QScreen *screen = QApplication::screenAt(geo.topLeft());
    if(!screen){

        return "";
    }

    // 捕获屏幕的特定区域
    QPixmap screenshot = screen->grabWindow(0, geo.x(), geo.y(), geo.width(), geo.height());

    // 将QPixmap转换为QImage
    QImage image = screenshot.toImage();

    // 将截图转换为JPEG格式的Base64编码
    return imageToBase64(image);
}

bool isRectValid(QRect &rect){
    return rect.width() > 0 && rect.height() > 0;
}

// 比较两个box对象的坐标顺序，先按y排序，再按x排序，考虑tolerance
bool compareBoxes(const QJsonObject &a, const QJsonObject &b) {
    int ay1 = a["text_region"].toArray()[0].toArray()[1].toInt();
    int ax1 = a["text_region"].toArray()[0].toArray()[0].toInt();
    int by1 = b["text_region"].toArray()[0].toArray()[1].toInt();
    int bx1 = b["text_region"].toArray()[0].toArray()[0].toInt();

    // 先按y坐标排序，考虑tolerance
    if (std::abs(ay1 - by1) <= tolerance) {
        // y坐标相近或相同，再按x坐标排序
        return ax1 < bx1;
    }
    // 直接按y坐标排序
    return ay1 < by1;
}

QString getOcrResult(QString jsonResponse){
    QString resultText;
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonResponse.toUtf8(), &jsonError);

    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误: " << jsonError.errorString();
        return "";
    }

    QJsonArray results = jsonDoc.object()["result"].toArray();

    // 转换QJsonArray为QJsonObject的list
    QList<QJsonObject> boxes;
    for (const auto &value : results) {
        boxes.append(value.toObject());
    }

    // 使用std::sort进行排序
    std::sort(boxes.begin(), boxes.end(), compareBoxes);

    // 连接文本
    int prevY = -100;
    for (const auto &obj : boxes) {
        int curY = obj["text_region"].toArray()[0].toArray()[1].toInt();

        if (!resultText.isEmpty()){

            if(prevY != -100 && !std::abs(prevY - curY) > tolerance){
                resultText += ",";
            }
        }

        resultText += obj["text"].toString();

        prevY = curY;
    }

    return resultText;
}

// http://192.168.100.20/app/7f617cb2-d96c-4ca2-80b3-af0867254204/develop
// ChatCompletionResponse
QString getllmAnswer(QString jsonResponse){
    QString resultText;
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonResponse.toUtf8(), &jsonError);

    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误: " << jsonError.errorString();
        return "";
    }

    return jsonDoc.object()["answer"].toString();
}

// 模拟 Ctrl+V 粘贴操作
void SimulatePaste()
{
    // 创建输入事件数组
    INPUT inputs[4] = {};

    // 按下 Ctrl 键
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    // 按下 V 键
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'V';

    // 松开 V 键
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'V';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // 松开 Ctrl 键
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    // 发送输入事件
    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

void SimulateMouseClick(int x, int y)
{
    // Move the cursor to the specified screen coordinates
    SetCursorPos(x, y);

    // Create the mouse down input event
    INPUT mouseDown = { 0 };
    mouseDown.type = INPUT_MOUSE;
    mouseDown.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    // Send the mouse down event
    SendInput(1, &mouseDown, sizeof(INPUT));

    // Create the mouse up input event
    INPUT mouseUp = { 0 };
    mouseUp.type = INPUT_MOUSE;
    mouseUp.mi.dwFlags = MOUSEEVENTF_LEFTUP;

    // Send the mouse up event
    SendInput(1, &mouseUp, sizeof(INPUT));
}

bool check_window_valid(HWND window, enum window_search_mode mode)
{
    DWORD styles, ex_styles;
    RECT  rect;

    if (/*!IsWindowVisible(window) ||*/
        (mode == EXCLUDE_MINIMIZED && IsIconic(window)))
        return false;

    GetClientRect(window, &rect);
    styles    = (DWORD)GetWindowLongPtr(window, GWL_STYLE);
    ex_styles = (DWORD)GetWindowLongPtr(window, GWL_EXSTYLE);

    if (ex_styles & WS_EX_TOOLWINDOW)
        return false;
    if (styles & WS_CHILD)
        return false;
    if (mode == EXCLUDE_MINIMIZED && (rect.bottom == 0 || rect.right == 0))
        return false;

    return true;
}

HWND next_window(HWND window, enum window_search_mode mode)
{
    while (true) {
        window = GetNextWindow(window, GW_HWNDNEXT);
        if (!window || check_window_valid(window, mode))
            break;
    }

    return window;
}

HWND first_window(enum window_search_mode mode)
{
    HWND window = GetWindow(GetDesktopWindow(), GW_CHILD);
    if (!check_window_valid(window, mode))
        window = next_window(window, mode);
    return window;
}

int window_rating(HWND window,
        enum window_priority priority,
        QString className,
        QString title,
        QString exe)
{
    int         class_val = 1;
    int         title_val = 1;
    int         exe_val   = 0;
    int         total     = 0;

    QString windowExe = get_window_exe(window);
    if(windowExe.isEmpty()){
        return 0;
    }

    QString windowTitle = get_window_title(window);

    QString windowClass = get_window_class(window);

    if (priority == WINDOW_PRIORITY_CLASS)
        class_val += 3;
    else if (priority == WINDOW_PRIORITY_TITLE)
        title_val += 3;
    else
        exe_val += 3;

    if (windowClass == className)
        total += class_val;
    if (windowTitle == title)
        total += title_val;
    if (windowExe == exe)
        total += exe_val;

    return total;
}

HWND find_window(enum window_search_mode mode,
        enum window_priority priority,
        QString className,
        QString title,
        QString exe)
{
    HWND window      = first_window(mode);
    HWND best_window = NULL;
    int  best_rating = 0;

    while (window) {
        int rating = window_rating(window, priority, className, title, exe);
        if (rating > best_rating) {
            best_rating = rating;
            best_window = window;
        }

        window = next_window(window, mode);
    }

    return best_window;
}

#define LOWER_HALFBYTE(x) ((x) & 0xF)
#define UPPER_HALFBYTE(x) (((x) >> 4) & 0xF)

static void deobfuscate_str(char *str, uint64_t val)
{
    uint8_t *dec_val = (uint8_t*)&val;
    int i = 0;

    while (*str != 0) {
        int pos = i / 2;
        bool bottom = (i % 2) == 0;
        uint8_t *ch = (uint8_t*)str;
        uint8_t xor_value = bottom ?
            LOWER_HALFBYTE(dec_val[pos]) :
            UPPER_HALFBYTE(dec_val[pos]);

        *ch ^= xor_value;

        if (++i == sizeof(uint64_t) * 2)
            i = 0;

        str++;
    }
}

void *get_obfuscated_func(HMODULE module, const char *str, uint64_t val)
{
    char new_name[128];
    strcpy(new_name, str);
    deobfuscate_str(new_name, val);
    return reinterpret_cast<void*>(GetProcAddress(module, new_name));
}

static HMODULE kernel32(void)
{
    static HMODULE kernel32_handle = NULL;
    if (!kernel32_handle)
        kernel32_handle = GetModuleHandleA("kernel32");
    return kernel32_handle;
}

HANDLE open_process(DWORD desired_access, bool inherit_handle,
        DWORD process_id)
{
    static HANDLE (WINAPI *open_process_proc)(DWORD, BOOL, DWORD) = NULL;
    if (!open_process_proc)
        open_process_proc = (HANDLE (WINAPI *)(DWORD, BOOL, DWORD))get_obfuscated_func(kernel32(),
                "B}caZyah`~q", 0x2D5BEBAF6DDULL);

    return open_process_proc(desired_access, inherit_handle, process_id);
}

QRect getWindowRect(HWND hwnd){
    RECT  rect;

    GetWindowRect(hwnd, &rect);

    QRect rt(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

    return rt;
}

QString get_window_exe(HWND window){
    wchar_t        name[MAX_PATH];
    HANDLE      process = NULL;
    DWORD       id;

    GetWindowThreadProcessId(window, &id);
    if (id == GetCurrentProcessId())
        return "";

    process = open_process(PROCESS_QUERY_LIMITED_INFORMATION, false, id);
    if (!process){
        CloseHandle(process);
        return "";
    }

    if (!GetProcessImageFileNameW(process, name, MAX_PATH)){
        CloseHandle(process);
        return "";
    }


    QString exeName = QString::fromWCharArray(name);

    int lastSlashIndex = exeName.lastIndexOf('\\'); // 查找最后一个'\'
    if (lastSlashIndex != -1) { // 确保找到了'\'
        exeName = exeName.mid(lastSlashIndex + 1); // 获取'\''之后的内容
    }

    return exeName;
}

QString get_window_title(HWND hwnd){
    QString title = "";

    wchar_t *temp;
    int len;

    len = GetWindowTextLengthW(hwnd);
    if (!len)
        return "";

    temp = (wchar_t*)malloc(sizeof(wchar_t) * (len+1));
    if (GetWindowTextW(hwnd, temp, len+1))
        title = QString::fromWCharArray(temp);
    free(temp);

    return title;
}

QString get_window_class(HWND hwnd){
    wchar_t temp[256];

    temp[0] = 0;
    if (GetClassNameW(hwnd, temp, sizeof(temp) / sizeof(wchar_t)))
        return QString::fromWCharArray(temp);

    return "";
}

QMap<HWND, QString> getWindowList(window_search_mode mode){
    QMap<HWND, QString> hwndMap;

    HWND window = first_window(mode);

    while (window) {
        QString winExe = get_window_exe(window);
        QString winTitle = get_window_title(window);
        QString winClass = get_window_class(window);

        QString str = QString("[%1]:%2").arg(winExe).arg(winTitle);

        hwndMap[window] = str;

        window = next_window(window, mode);
    }

    return hwndMap;

}

BOOL isWndGetFocus(HWND hwnd){
    HWND hwndForeground = GetForegroundWindow();
    if(hwnd == hwndForeground){
        return true;
    }

    if(IsDescendant(hwndForeground, hwnd)){
        return true;
    }

    QString exeName = get_window_exe(hwndForeground);
    QString titleName = get_window_title(hwndForeground);

    qDebug() << "isWndGetFocus, exeName:" << exeName << ", titleName:" << titleName;

    return false;
}

// 回调函数原型
BOOL CALLBACK IsDescendantEnumProc(HWND hwnd, LPARAM lParam)
{
    HWND childWnd = (HWND)lParam;
    if (IsChild(hwnd, childWnd))
    {
        // 找到了后代窗口，提前结束枚举
        SetLastError((DWORD)1);
        return FALSE;
    }
    return TRUE;
}

BOOL IsDescendant(HWND childWnd, HWND parentWnd)
{
    // 首先检查是否是直接子窗口
    if (IsChild(parentWnd, childWnd))
        return TRUE;

    // 枚举父窗口的所有子窗口，传递childWnd
    EnumChildWindows(parentWnd, IsDescendantEnumProc, (LPARAM)childWnd);

    // 如果在枚举过程中设置了错误码1，说明找到了后代窗口
    return (GetLastError() == 1);
}

bool isValidTime(const QString &timeStr)
{
    QDateTime dateTime;
    bool isValid = dateTime.fromString(timeStr, "hh:mm:ss").isValid();
    return isValid;
}



