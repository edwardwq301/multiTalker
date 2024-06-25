#ifndef FileLogger_H
#define FileLogger_H

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

class FileLogger {
private:
    std::ofstream file;
    static std::unique_ptr<FileLogger> instance;

    FileLogger(std::string fileName = "log.txt") {
        file.open(fileName, std::ios::out | std::ios::app);
    }

public:
    static std::unique_ptr<FileLogger> GetInstance() {
        std::unique_ptr<FileLogger> instance(new FileLogger);
        return instance;
    }

    bool Write(const std::string & info) {
        if (file.is_open()) {
            file << info << '\n';
        }
        else
            return false;

        return true;
    }

    void Flush() {
        file.flush();
    }

    ~FileLogger() {
        if (file.is_open())
            file.close();
    }
};

#endif