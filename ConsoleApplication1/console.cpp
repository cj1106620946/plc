#include "console.h"
#include <windows.h>
#include <iostream>
#include <sstream>
void Console::printGBK(const std::string& text)
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD w;
    WriteConsoleA(h, text.c_str(), (DWORD)text.size(), &w, NULL);
}
std::string GBKtoUTF8(const std::string& gbk)
{
    // GBK → UTF-16
    int wlen = MultiByteToWideChar(936, 0, gbk.c_str(), -1, NULL, 0);
    std::wstring wbuf;
    wbuf.resize(wlen);
    MultiByteToWideChar(936, 0, gbk.c_str(), -1, &wbuf[0], wlen);

    // UTF-16 → UTF-8
    int u8len = WideCharToMultiByte(CP_UTF8, 0, wbuf.c_str(), -1, NULL, 0, NULL, NULL);
    std::string utf8;
    utf8.resize(u8len);
    WideCharToMultiByte(CP_UTF8, 0, wbuf.c_str(), -1, &utf8[0], u8len, NULL, NULL);

    return utf8;
}
void Console::printUTF8(const std::string& text)
{
    int wlen = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
    if (wlen <= 0) return;

    std::wstring wbuf;
    wbuf.resize(wlen);

    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wbuf[0], wlen);

    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD w;
    WriteConsoleW(h, wbuf.c_str(), (DWORD)wbuf.size(), &w, NULL);
}

// ==========================================================
// 构造函数
// ==========================================================
Console::Console() {}

// ==========================================================
// 主循环
// ==========================================================
void Console::run()
{
    while (true)
    {
        showMainHeader();
        mainMenu();
    }
}

// ==========================================================
// 主界面头部
// ==========================================================
void Console::showMainHeader()
{
    printGBK("\n===================================\n");
    printGBK("          PLC + AI 控制台\n");
    printGBK("===================================\n");

    printGBK("PLC 连接状态：");
    printGBK(plc.isConnected() ? "已连接\n" : "未连接\n");

    printGBK("AI Key 状态：");
    printGBK(hasAIKey ? "已设置\n" : "未设置\n");

    printGBK("-----------------------------------\n");
    printGBK("主菜单：\n");
    printGBK("  1. 输入 IP 连接 PLC\n");
    printGBK("  2. 输入密钥连接 AI\n");
    printGBK("  3. 手动控制 PLC（读/写）\n");
    printGBK("  4. AI 对话模式\n");
    printGBK("  5. AI 自动控制 PLC\n");
    printGBK("  0. 退出\n");
    printGBK("===================================\n");
}

// ==========================================================
// 主菜单跳转
// ==========================================================
void Console::mainMenu()
{
    printGBK("\n> ");

    std::string choice;
    std::getline(std::cin, choice);

    if (choice == "0")
    {
        printGBK("程序已退出。\n");
        exit(0);
    }
    else if (choice == "1")
        menuPLCConnect();
    else if (choice == "2")
        menuAIKey();
    else if (choice == "3")
        menuPLCManual();
    else if (choice == "4")
        menuAIDialog();
    else if (choice == "5")
        menuAIControlPLC();
    else
        printGBK("无效输入，请输入 0~5。\n");
}

bool Console::checkBreak(const std::string& cmd)
{
    return cmd == "break0";
}

// ==========================================================
// 1. PLC 连接
// ==========================================================
void Console::menuPLCConnect()
{
    printGBK("\n--- PLC 连接 ---\n");
    printGBK("输入 IP 地址连接 PLC，例如：192.168.10.1\n");
    printGBK("输入 break0 返回主菜单\n");
    printGBK("IP> ");

    std::string ip;
    std::getline(std::cin, ip);

    if (checkBreak(ip))
        return;

    printGBK("尝试连接 PLC...\n");

    if (plc.connectPLC(ip, 0, 1))
        printGBK("PLC 连接成功！\n");
    else
        printGBK("PLC 连接失败，请检查 IP 或 PLCSIM。\n");
}

// ==========================================================
// 2. 设置 AI Key
// ==========================================================
void Console::menuAIKey()
{
    printGBK("\n--- 设置 AI Key ---\n");
    printGBK("输入您的 DeepSeek API Key：\n");
    printGBK("输入 break0 返回主菜单\n");
    printGBK("KEY> ");

    std::string key;
    std::getline(std::cin, key);

    if (checkBreak(key))
        return;

    ai.setAPIKey(key);
    hasAIKey = true;

    printGBK("AI Key 已设置。\n");
}

// ==========================================================
// 3. PLC 手动读写
// ==========================================================
void Console::menuPLCManual()
{
    if (!plc.isConnected())
    {
        printGBK("错误：PLC 尚未连接。\n");
        return;
    }

    printGBK("\n--- 手动控制 PLC ---\n");
    printGBK("读：read I0.0\n");
    printGBK("写：write Q0.0 1\n");
    printGBK("输入 break0 返回主菜单\n");

    while (true)
    {
        printGBK("plc> ");

        std::string cmd;
        std::getline(std::cin, cmd);

        if (checkBreak(cmd))
            return;

        std::stringstream ss(cmd);
        std::string op;
        ss >> op;

        if (op == "read")
        {
            std::string addr;
            ss >> addr;

            int val = 0;
            if (plc.readAddress(addr, val))
            {
                printGBK(addr + " = ");
                std::cout << val << "\n";
            }
            else printGBK("读取失败\n");
        }
        else if (op == "write")
        {
            std::string addr;
            int v = 0;
            ss >> addr >> v;

            if (plc.writeAddress(addr, v))
                printGBK("写入成功\n");
            else
                printGBK("写入失败\n");
        }
        else
        {
            printGBK("未知指令，请使用 read / write\n");
        }
    }
}

// ==========================================================
// 4. AI 对话
// ==========================================================
void Console::menuAIDialog()
{
    if (!hasAIKey)
    {
        printGBK("错误：AI Key 未设置。\n");
        return;
    }

    printGBK("\n--- AI 对话模式 ---\n");
    printGBK("输入 break0 返回主菜单\n");

    while (true)
    {
        printGBK("you> ");

        std::string msg;
        std::getline(std::cin, msg);

        if (checkBreak(msg))
            return;

        std::string utf8msg = GBKtoUTF8(msg);
        std::string reply = ai.ask(utf8msg);
        printGBK("ai> ");
        printUTF8(reply);
        printGBK("\n");
    }
}

// ==========================================================
// 5. AI 自动控制 PLC（文本 W: + 控制 C: 格式）
// ==========================================================
void Console::menuAIControlPLC()
{
    if (!hasAIKey)
    {
        printGBK("错误：AI Key 未设置。\n");
        return;
    }
    if (!plc.isConnected())
    {
        printGBK("错误：PLC 未连接。\n");
        return;
    }

    printGBK("\n--- AI 控制 PLC ---\n");
    printGBK("AI 输出格式示例：\n");
    printGBK("  W: 这是返回给用户的文本\n");
    printGBK("  C: write Q0.0 1\n");
    printGBK("支持指令：read I0.0 / write Q0.0 1\n");
    printGBK("输入 break0 返回主菜单\n\n");

    while (true)
    {
        printGBK("ai-control> ");
        std::string userText;
        std::getline(std::cin, userText);
        if (checkBreak(userText)) return;

        // =====================================================
        // 1) 生成 prompt
        // =====================================================
        std::string utf8User = GBKtoUTF8(userText);
        std::string aiText = ai.ask(utf8User,
            u8"你是 fuduji-PLC 上位机助手，请按照以下格式输出：\n"
            u8"W: <用户可读的中文文本，不含 JSON、代码块、特殊字符>\n"
            u8"C: <PLC 指令：read I0.0 或 write Q0.0 1，没有则写 none>\n"
        );
        // =====================================================
        // 2) 清洗 AI 输出（删除 BOM、隐藏字符、首尾空格）
        // =====================================================
        auto clean = [](std::string s) {
            // 删除 UTF-8 BOM
            if (s.size() >= 3 &&
                (unsigned char)s[0] == 0xEF &&
                (unsigned char)s[1] == 0xBB &&
                (unsigned char)s[2] == 0xBF)
            {
                s = s.substr(3);
            }
            // 去掉 \r
            s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
            // 去掉首尾空格
            while (!s.empty() && isspace((unsigned char)s.front())) s.erase(s.begin());
            while (!s.empty() && isspace((unsigned char)s.back()))  s.pop_back();
            return s;
        };
        aiText = clean(aiText);

        printGBK("\nAI 输出：\n");
        printUTF8(aiText);
        printGBK("\n");

        // =====================================================
        // 3) 解析 W: 与 C:
        // =====================================================
        std::string lineW, lineC;
        {
            std::istringstream ss(aiText);
            std::string line;

            while (std::getline(ss, line))
            {
                line = clean(line);  // 再清洗一遍
                if (line.rfind("W:", 0) == 0)
                    lineW = clean(line.substr(2));
                else if (line.rfind("C:", 0) == 0)
                    lineC = clean(line.substr(2));
            }
        }
        // 显示文本部分
        if (!lineW.empty())
        {
            printGBK("W: ");
            printUTF8(lineW);
            printGBK("\n");
        }

        // =====================================================
        // 4) 执行控制指令
        // =====================================================
        if (lineC.empty() || lineC == "none" || lineC == " none")
        {
            printGBK("无控制指令。\n\n");
            continue;
        }

        printGBK("执行控制：");
        printGBK(lineC + "\n");

        // 拆分指令
        std::stringstream cs(lineC);
        std::string op, addr;
        int value = 0;

        cs >> op;
        if (op == "read")
        {
            cs >> addr;

            int val = 0;
            if (plc.readAddress(addr, val))
            {
                printGBK("[PLC] ");
                printGBK(addr + " = ");
                std::cout << val << "\n\n";
            }
            else
            {
                printGBK("[PLC] 读取失败\n\n");
            }
        }
        else if (op == "write")
        {
            cs >> addr >> value;
            if (plc.writeAddress(addr, value))
            {
                printGBK("[PLC] 写入成功：");
                printGBK(addr + " = " + std::to_string(value) + "\n\n");
            }
            else
            {
                printGBK("[PLC] 写入失败\n\n");
            }
        }
        else
        {
            printGBK("[错误] 无法识别控制命令\n\n");
        }
    }
}
