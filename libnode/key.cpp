#include <key.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

Key::Key(std::string k) {
    for(int i = 0; i < k.length(); i += 2) {
        value[i/2] = std::stoi(k.substr(i, i+2), nullptr, 16);
    }
};
Key::Key(char k[KEY_LENGTH]) {
    strcpy(value, k);
};

Key* Key::random_key() {
    char value[KEY_LENGTH];
    srand(time(0));

    for(int i = 0; i < KEY_LENGTH; i++) {
        value[i] = rand() % 256;
    }
    return new Key(value);
};
std::string Key::String() {
    std::string str;
    for(int i = 0; i < KEY_LENGTH; i++) {
        str += std::to_string(value[i]);
    }
    return str;
};
bool Key::operator==(const Key& other) {
    for(int i = 0; i < KEY_LENGTH; i++) {
        if(value[i] != other.value[i]) return false;
    }
    return true;
};
bool Key::operator<(const Key& other) {
    for(int i = 0; i < KEY_LENGTH; i++) {
        if(value[i] < other.value[i]) return true;
    }
    return false;
};
bool Key::operator>(const Key& other) {
    for(int i = 0; i < KEY_LENGTH; i++) {
        if(value[i] > other.value[i]) return true;
    }
    return false;
};
Key* Key::operator^(const Key& other) {
    char val[KEY_LENGTH];

    for(int i = 0; i < KEY_LENGTH; i++) {
        val[i] = value[i] ^ other.value[i];
    }
    return new Key(val);
};
int Key::prefix_length() {
    for(int i = 0; i < KEY_LENGTH; i++) {
        for(int j = 0; j < 8; j++) {
            if ((value[i] >> (7 - j)) && 0x1 != 0) {
                return i * 8 + j;
            }
        }
    }
    return KEY_LENGTH * 8 - 1;
};
std::ostream& operator<<(std::ostream& os, const Key& key) {
    for(int i = 0; i < KEY_LENGTH; i++) {
        os << std::to_string(key.value[i]);
    }
    return os;
};
