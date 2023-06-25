#ifndef HISTORY_CXX
#define HISTORY_CXX

#include <cstring>
#include <iostream>
#include <vector>

namespace History{
    [[maybe_unused]] static const char* HISTORY_FILE_NAME = "history.bin";
    [[maybe_unused]] static const char* OFFSETS_FILE_NAME = "offsets.bin";

    struct InputHistory{
        
        InputHistory() = default;
        InputHistory(std::string time, std::string user, std::string input, std::string output):
            time(time), user(user), input(input), output(output){}

        size_t serialized_size() const;
        std::ostream& serialize(std::ostream& os) const;
        std::istream& deserialize(std::istream& is);

        std::string time;
        std::string user;
        std::string input;
        std::string output;
    };

    class HistoryManager{
        public:
        HistoryManager() = default;
        void push_back(InputHistory history);
        InputHistory get_last();
        //std::vector<InputHistory> get_all();
        private:
    };
}

#endif