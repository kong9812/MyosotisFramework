#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <windows.h>
#include <filesystem>

namespace {
    /// <summary>
    /// ���O���x��
    /// </summary>
    enum class LogLevel {
        LOG_INFO,
        LOG_DEBUG,
        LOG_WARN,
        LOG_ERROR,
        LOG_ASSERT
    };

    /// <summary>
    /// ���O���x���𕶎���ɕϊ�
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
    /// ���O�t�@�C���̏o��
    /// </summary>
    /// <param name="logMessage">���b�Z�[�W</param>
    inline void CustomLog(LogLevel level, const std::string& logMessage)
    {
        // ���ݎ������擾
        std::time_t now = std::time(nullptr);
        std::tm localTime = *std::localtime(&now); // ���[�J���^�C���ɕϊ�

        // ���Ԃ��t�H�[�}�b�g
        char dateBuffer[20]; // "YYYY-MM-DD" �p
        std::strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", &localTime);

        // �������t�H�[�}�b�g
        char timeBuffer[10]; // "HH:MM:SS" �p
        std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &localTime);

        // ������
        std::string message = "[" + std::string(dateBuffer) + " " + std::string(timeBuffer) + "] " +
            "[" + LogLevelToString(level).data() + "] " +
            logMessage;

        // �W���o��
        if (level < LogLevel::LOG_ERROR)
        {
            std::cout << message << std::endl;
        }
        else
        {
            std::cerr << message << std::endl;
        }

        // �f�B���N�g�� "log/" ���쐬�i���݂��Ȃ��ꍇ�̂݁j
        const std::string logDir = "log";
        if (!std::filesystem::exists(logDir)) {
            std::filesystem::create_directory(logDir);
        }

        // �t�@�C���Ƀ��O���L�^
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
    /// �J�X�^���A�T�[�g�֐�
    /// </summary>
    /// <param name="condition">���</param>
    /// <param name="message">���b�Z�[�W</param>
    /// <param name="file">�t�@�C����</param>
    /// <param name="line">�s��</param>
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
    /// ���O�̍폜
    /// </summary>
    inline void ClearLog()
    {
        // ���ݎ������擾
        std::time_t now = std::time(nullptr);
        std::tm localTime = *std::localtime(&now); // ���[�J���^�C���ɕϊ�

        // ���Ԃ��t�H�[�}�b�g
        char dateBuffer[20]; // "YYYY-MM-DD" �p
        std::strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", &localTime);

        std::ofstream logFile("log/" + std::string(dateBuffer) + ".log", std::ios::trunc);
    }
#endif

    /// <summary>
    /// [Log]Info
    /// </summary>
    /// <param name="logMessage">���b�Z�[�W</param>
    inline void Info(const std::string& logMessage)
    {
        CustomLog(LogLevel::LOG_INFO, logMessage);
    }

    /// <summary>
    /// [Log]Debug
    /// </summary>
    /// <param name="logMessage">���b�Z�[�W</param>
    inline void Debug(const std::string& logMessage)
    {
        CustomLog(LogLevel::LOG_DEBUG, logMessage);
    }

    /// <summary>
    /// [Log]Warning
    /// </summary>
    /// <param name="logMessage">���b�Z�[�W</param>
    inline void Warning(const std::string& logMessage)
    {
        CustomLog(LogLevel::LOG_WARN, logMessage);
    }

    /// <summary>
    /// [Log]Error
    /// </summary>
    /// <param name="logMessage">���b�Z�[�W</param>
    inline void Error(const std::string& logMessage)
    {
        CustomLog(LogLevel::LOG_ERROR, logMessage);
    }
}

#ifdef NDEBUG
// �����[�X���ɖ�����
#define CustomAssert(condition, message) ((void)0)
#else
// �f�o�b�O���ɗL����
#define ASSERT(condition, message) \
        CustomAssert((condition), (message), __FILE__, __LINE__)
#endif