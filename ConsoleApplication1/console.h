#pragma once
#include <string>
#include "plcclient.h"
#include "deepseek.h"
class Console
{
public:
    Console();
    void run();
    // === UTF8 / GBK Êä³ö´¦Àí ===
    void printGBK(const std::string& text);
    void printUTF8(const std::string& text);
private:
    void showMainHeader();
    void mainMenu();
    bool checkBreak(const std::string& cmd);
    void menuPLCConnect();
    void menuAIKey();
    void menuPLCManual();
    void menuAIDialog();
    void menuAIControlPLC();
private:
    PLCClient plc;
    DeepSeekAI ai;
    bool hasAIKey = false;
};
