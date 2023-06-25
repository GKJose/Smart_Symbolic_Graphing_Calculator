#include <history.hxx>
#include <cstdint>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <iostream>

bool fileExists(const std::string& fileName) {
    std::ifstream file(fileName);
    return file.good();
}

size_t History::InputHistory::serialized_size() const {
    return time.size() + 
        user.size() + 
        input.size() +
        output.size() + 4; // 1 for each null terminator.
}

std::ostream& History::InputHistory::serialize(std::ostream& os) const {
    // +1 because of null character
    os.write(time.c_str(), time.size()+1); 
    os.write(user.c_str(), user.size()+1);
    os.write(input.c_str(), input.size()+1);
    os.write(output.c_str(), output.size()+1);
    return os;
}

std::istream& History::InputHistory::deserialize(std::istream& is) {
    std::getline(is, time, '\0');
    std::getline(is, user, '\0');
    std::getline(is, input, '\0');
    std::getline(is, output, '\0');
    return is;
}

void History::HistoryManager::push_back(InputHistory history) {
    std::fstream offsets(History::OFFSETS_FILE_NAME, std::fstream::out | std::ios::in | std::ios::out | std::ios::binary );
    if (!offsets) {
        offsets.close();
        std::ofstream f(History::OFFSETS_FILE_NAME, std::ios::binary );
        f.close();
        offsets.open(History::OFFSETS_FILE_NAME,  std::fstream::out | std::ios::in | std::ios::out | std::ios::binary );
    }
    offsets.seekg(0, offsets.end);
    size_t offsets_len = offsets.tellg();
    // if length of offsets file is too small to be correctly initialized, initialize it.
    if (offsets_len < sizeof(uint64_t)) {
        uint64_t init = 0;
        offsets.seekp(0, std::ios_base::beg);
        offsets.write((char*)(void*)(&init), sizeof(uint64_t));
        offsets.clear();
    } 

    // increment the length of offsets.
    offsets.seekg(0, std::ios_base::beg);
    uint64_t len;
    offsets.read((char*)(void*)(&len), sizeof(uint64_t));
    offsets.clear();
    len += 1;
    offsets.seekp(0, std::ios_base::beg);
    offsets.write((char*)(void*)(&len), sizeof(uint64_t));

    // append uint32 to end of offsets
    uint32_t last_value = 0, new_value = 0;
    if (offsets_len > sizeof(uint64_t)) { // more than just offsets len
        offsets.seekg(-sizeof(uint32_t), std::ios::end);
        offsets.read((char*)(void*)(&last_value), sizeof(uint32_t));
        new_value = last_value + (uint32_t)(history.serialized_size());
    }
    
    offsets.seekp(0, std::ios::end); // go to the end and append new offset
    offsets.write((char*)(void*)(&new_value), sizeof(uint32_t));
    offsets.close();

    // done with offsets.

    std::ofstream hist(History::HISTORY_FILE_NAME, std::ios::app | std::ios::binary);
    history.serialize(hist);
    hist.close();
}

History::InputHistory History::HistoryManager::get_last(){
    uint32_t offset;
    History::InputHistory result;

    std::ifstream offsets(History::OFFSETS_FILE_NAME, std::ios::binary | std::ios::in);
    offsets.seekg(-sizeof(uint32_t), std::ios::end);
    offsets.read((char*)(void*)(&offset), sizeof(uint32_t));
    offsets.close();

    std::ifstream hist(History::HISTORY_FILE_NAME, std::ios::binary | std::ios::in);
    hist.seekg((size_t)offset, std::ios::beg);
    result.deserialize(hist);
    return result;
}