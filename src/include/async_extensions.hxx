#ifndef ASYNC_EXTENSIONS_HEADER
#define ASYNC_EXTENSIONS_HEADER

#include <string>
#include <future>
#include <stdio.h>
#include <array>

inline std::future<std::string> run_async_cmd(std::string const& cmd){
    auto cmd_lambda = [cmd]{
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) 
            return std::string("");
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr){
            result += buffer.data();
        }
        return result;
    };
    auto handle = std::async(std::launch::async, [cmd_lambda]{return cmd_lambda();});
    return handle;
}

#define RUN_ASYNC(func) std::async(std::launch::async, func)

#endif
