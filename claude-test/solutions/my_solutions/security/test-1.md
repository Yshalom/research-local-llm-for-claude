A simple buffer overflow can be triggered by entering a password longer than 20 characters.

**test-1.cpp [Lines 67-77]**  
```cpp
void read_password()
{
    printf("Enter password (max 20 characters): ");
    scanf("%s", password); // <--- Here

    char tmp_hash[65];
    sha256(password, tmp_hash);

    if (strcmp(hash, tmp_hash) == 0)
        evaluated_password = true;
}
```

**test-1.cpp [Lines 16-20]**  
```cpp
class PASSWORD_CHECKER {
private:
    char password[21]{ 0 };
    char hash[65]{ 0 };
    bool evaluated_password = false;
```

*Memory layout*  
| Member               | Size (bytes) |
|----------------------|--------------|
| `password`           | 21           |
| `hash`               | 65           |
| `evaluated_password` | 1            |

If the user supplies 87 characters, the input overruns the password buffer, continues into the hash array, and finally overwrites the single byte that holds `evaluated_password`, allowing authentication bypass.

## Demo
```
Enter password (max 20 characters): 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111
```
