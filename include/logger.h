#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <windows.h>
#include <filesystem>

namespace {
    /// <summary>
    /// ログレベル
    /// </summary>
    enum class LogLevel {
        LOG_INFO,
        LOG_DEBUG,
        LOG_WARN,
        LOG_ERROR,
        LOG_ASSERT
    };

    /// <summary>
    /// ログレベルを文字列に変換
    /// </summary>
    /// <param name="level"></param>
    /// <returns></returns>
    constexpr std::string_view LogLevelToString(LogLevel level)
    {
        switch (level) {
        case LogLevel::LOG_INFO:    return "INFO";
        case LogLevel::LOG_DEBUG:   return "DEBUG";
        case LogLevel::LOG_WARN:    return "WARN";
        case LogLevel::LOG_ERROR:   return "ERROR";
        case LogLevel::LOG_ASSERT:  return "ASSERT";
        default:                    return "UNKNOWN";
        }
    }

    /// <summary>
    /// ログファイルの出力
    /// </summary>
    /// <param name="logMessage">メッセージ</param>
    inline void CustomLog(LogLevel level, const std::string& logMessage)
    {
        // 現在時刻を取得
        std::time_t now = std::time(nullptr);
        std::tm localTime = *std::localtime(&now); // ローカルタイムに変換

        // 時間をフォーマット
        char dateBuffer[20]; // "YYYY-MM-DD" 用
        std::strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", &localTime);

        // 時刻をフォーマット
        char timeBuffer[10]; // "HH:MM:SS" 用
        std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &localTime);

        // 文字列
        std::string message = "[" + std::string(dateBuffer) + " " + std::string(timeBuffer) + "] " +
            "[" + LogLevelToString(level).data() + "] " +
            logMessage;

        // 標準出力
        if (level < LogLevel::LOG_ERROR)
        {
            std::cout << message << std::endl;
        }
        else
        {
            std::cerr << message << std::endl;
        }

        // ディレクトリ "log/" を作成（存在しない場合のみ）
        const std::string logDir = "log";
        if (!std::filesystem::exists(logDir)) {
            std::filesystem::create_directory(logDir);
        }

        // ファイルにログを記録
        std::ofstream logFile("log/" + std::string(dateBuffer) + ".log", std::ios::app);
        if (logFile)
        {
            logFile << message << std::endl;
        }
        else
        {
            std::cerr << "Failed to open log file." << std::endl;
        }
    }

    /// <summary>
    /// カスタムアサート関数
    /// </summary>
    /// <param name="condition">状態</param>
    /// <param name="message">メッセージ</param>
    /// <param name="file">ファイル名</param>
    /// <param name="line">行数</param>
    inline void CustomAssert(bool condition, const std::string& message, const char* file, int line)
    {
        if (!condition)
        {
            std::string errorMessage = "Assertion failed: " + message +
                "\nFile: " + file + ", Line: " + std::to_string(line);
            std::cerr << errorMessage << std::endl;
            CustomLog(LogLevel::LOG_ASSERT, errorMessage);
            MessageBoxA(NULL, errorMessage.c_str(), "Assertion Failed", MB_ICONERROR | MB_OK);
            std::exit(EXIT_FAILURE);
        }
    }
}

namespace Logger
{
#ifdef DEBUG
    /// <summary>
    /// ログの削除
    /// </summary>
    inline void ClearLog()
    {
        // 現在時刻を取得
        std::time_t now = std::time(nullptr);
        std::tm localTime = *std::localtime(&now); // ローカルタイムに変換

        // 時間をフォーマット
        char dateBuffer[20]; // "YYYY-MM-DD" 用
        std::strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", &localTime);

        std::ofstream logFile("log/" + std::string(dateBuffer) + ".log", std::ios::trunc);
    }
#endif

    /// <summary>
    /// [Log]Info
    /// </summary>
    /// <param name="logMessage">メッセージ</param>
    inline void Info(const std::string& logMessage)
    {
        CustomLog(LogLevel::LOG_INFO, logMessage);
    }

    /// <summary>
    /// [Log]Debug
    /// </summary>
    /// <param name="logMessage">メッセージ</param>
    inline void Debug(const std::string& logMessage)
    {
        CustomLog(LogLevel::LOG_DEBUG, logMessage);
    }

    /// <summary>
    /// [Log]Warning
    /// </summary>
    /// <param name="logMessage">メッセージ</param>
    inline void Warning(const std::string& logMessage)
    {
        CustomLog(LogLevel::LOG_WARN, logMessage);
    }

    /// <summary>
    /// [Log]Error
    /// </summary>
    /// <param name="logMessage">メッセージ</param>
    inline void Error(const std::string& logMessage)
    {
        CustomLog(LogLevel::LOG_ERROR, logMessage);
    }
}

#ifdef NDEBUG
// リリース時に無効化
#define CustomAssert(condition, message) ((void)0)
#else
// デバッグ時に有効化
#define ASSERT(condition, message) \
        CustomAssert((condition), (message), __FILE__, __LINE__)
#endif