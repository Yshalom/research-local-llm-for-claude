The program uses a mechanism that tries to prevent the user from entering more characters than intended.
When reading the input, at most N characters are written to the buffer, the buffer size is 2*N bytes because the data is stored in UTF‑16 (2 bytes per character).

**Key Points**  
1. `password_length` and `password_byte_count` are both `unsigned int`.  
2. `password_length` is read with `wscanf_s` to safely limit the number of characters entered and avoid a buffer‑overflow when the user supplies an excessively long string.  
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

*Memory layout*  
| Member               | Size (bytes) |
|----------------------|--------------|
| `password`           | N            |
| `hash`               | 65           |

* We can bypass the constrain that `password_byte_count = password_length * sizeof(wchar_t)` by causing an integer overflow. Moreover, we can do it in a way that the `hash` string can be overwritten with writing to `password` at the `wscanf_s` function. The overflow can be triggered at the `password_length * sizeof(wchar_t)` when `password_length` is too big.
* Entering `password_length = 2147483649` causes the allocation size to wrap around to a much smaller value, while password_byte_count still reflects the large length, which allow the user to overwrite memory with the `wscanf_s` function.
* An attacker can supply a password consisting of two visible characters followed by a crafted UTF‑16 sequence that, when interpreted, yields the expected hash value.

**test-2.cpp [Lines 101-106]**
```cpp
    bool evaluate_password() {
        char tmp_hash[HASH_STRING_LENGTH];
        sha256((char*)password, password_byte_count, tmp_hash);

        return memcmp(hash, tmp_hash, HASH_STRING_LENGTH) == 0;
    }
```
## Demo:
```
Enter how long the password is: 2147483649
Enter password (-2147483647 characters): aa搷攸ㄶㄱ攷愹愱㉢〸昸㤲㔵㘴㘶昱㈹戱敢攷〲っ㔱挸攷ㄸ㕢ㅤㅣ㘳㐵㑢㜵
```
