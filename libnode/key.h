#include <bitset>
#include <string>

#define KEY_LENGTH 2

class Key {
public:
    static Key* random_key();
    Key(std::string k);
    Key(char k[KEY_LENGTH]);
    std::string String();
    char value[KEY_LENGTH];
    bool operator==(const Key& other);
    bool operator<(const Key& other);
    bool operator>(const Key& other);
    Key* operator^(const Key& other);
    int prefix_length();
    friend std::ostream& operator<<(std::ostream& os, const Key& key);
};