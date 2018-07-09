#include <bitset>
#include <string>

#define KEY_LENGTH 32

using namespace std;

class Key {
private:
    bitset<KEY_LENGTH> value;
public:
    Key(string k);
    string String();
};