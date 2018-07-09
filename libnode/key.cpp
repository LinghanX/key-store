#include <key.h>

Key::Key(string k) {
    value = bitset<KEY_LENGTH>(k);
};
string Key::String() {
    return value.to_string();
}