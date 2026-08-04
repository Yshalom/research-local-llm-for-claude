This code contains a mechanism to prevent unsupervised character reading.
Whe reading the input only max of N characters will be written to the buffer (the buffer size is 2*N), the characters are on UTF-16 (2 bytes per character).

**Key Points**  
1. `password_length` and `password_byte_count` are both `unsigned int`.  
2. `password_length` works with `wscanf_s` to avoid overflow when the user enters too many characters.  
3. `password_byte_count` determines the buffer size: `password_byte_count = password_length * sizeof(wchar_t)`.

**test-2.cpp [Lines 69-87]**  
```cpp
class PASSWORD_CHECKER {
private:
    unsigned int password_length, password_byte_count;
    wchar_t* password;
    char* hash;

public:
    PASSWORD_CHECKER(const char* _hash)
    {
        wprintf(L"Enter how long the password is: ");
        wscanf_s(L"%u", &password_length);
        password_length++; // make space for null terminator
        password_byte_count = password_length * sizeof(wchar_t);
        
        void* memory = malloc(password_byte_count + HASH_STRING_LENGTH);
        password = (wchar_t*)memory;
        hash = (char*)memory + password_byte_count;
        memcpy_s(hash, HASH_STRING_LENGTH, _hash, HASH_STRING_LENGTH);
    }
```

We can breach the `password_byte_count = password_length * sizeof(wchar_t)` constrain with integer overflow. We can do it in a way that the `hash` string can be overwritten with writing to `password` at the wscanf_s function.  
* Entering password_length = 2147483649, will cause a smaller buffer allocation, while `password_byte_count` is how many bytes the password really contains, and `password_length` is a big number, which allow the user to overwrite memory with the wscanf_s function.
* From here, a solution is a password of 2 character followed by characters that can be decoded as the hash of the password, which make the password seems like valid.

`test-2.cpp [Lines 101-106]`
```cpp
    bool evaluate_password() {
        char tmp_hash[HASH_STRING_LENGTH];
        sha256((char*)password, password_byte_count, tmp_hash);

        return memcmp(hash, tmp_hash, HASH_STRING_LENGTH) == 0;
    }
```
## Conclusion:
Such input would be (it's not the only one):
```
Enter how long the password is: 2147483649
Enter password (-2147483647 characters): aa搷攸ㄶㄱ攷愹愱㉢〸昸㤲㔵㘴㘶昱㈹戱敢攷〲っ㔱挸攷ㄸ㕢ㅤㅣ㘳㐵㑢㜵
```
