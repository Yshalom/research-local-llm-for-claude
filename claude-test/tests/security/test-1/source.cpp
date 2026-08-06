#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <wincrypt.h>
#include <iostream>

constexpr char HEX_DIGITS[] = "0123456789abcdef";
constexpr char PASSWORD_HASH[] = "f82a7d02e8f0a728b7c3e958c278745cb224d3d7b2e3b84c0ecafc5511fdbdb7";


// --------------- color escape strings ---------------
constexpr char COLOR_RED_ESCAPE[] = "\033[31m";
constexpr char COLOR_GREEN_ESCAPE[] = "\033[32m";
constexpr char COLOR_RESET_ESCAPE[] = "\033[0m";

class PASSWORD_CHECKER {
private:
    char password[21]{ 0 };
    char hash[65]{ 0 };
    bool evaluated_password = false;

    void sha256(char* data, char* res)
    {
        HCRYPTPROV hProv = 0;
        if (!CryptAcquireContext(&hProv,
            nullptr,          // NULL: use default provider
            nullptr,          // NULL: default provider name
            PROV_RSA_AES,     // provider type (AES supports SHA‑256)
            CRYPT_VERIFYCONTEXT))   // no key container needed
            throw std::runtime_error("CryptAcquireContext failed");

        HCRYPTHASH hHash = 0;
        if (!CryptCreateHash(hProv,
            CALG_SHA_256,     // algorithm
            0,                // key handle (none)
            0,                // flags
            &hHash))
            throw std::runtime_error("CryptCreateHash failed");

        if (!CryptHashData(hHash, (BYTE*)data, (DWORD)strlen(data), 0))
            throw std::runtime_error("CryptHashData failed");

        BYTE hash[32];
        DWORD hash_len = 32;
        if (!CryptGetHashParam(hHash,
            HP_HASHVAL,
            hash,
            &hash_len,
            0))
            throw std::runtime_error("CryptGetHashParam failed");

        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);

        for (size_t i = 0; i < hash_len; ++i) {
            res[2 * i] = HEX_DIGITS[(hash[i] >> 4) & 0xF];   // high nibble
            res[2 * i + 1] = HEX_DIGITS[hash[i] & 0xF];          // low nibble
        }
        res[hash_len * 2] = '\0';      // terminate the string
    }

public:
    PASSWORD_CHECKER(const char* hash) {
        memcpy(this->hash, hash, 65);
    }

    void read_password()
    {
        printf("Enter password (max 20 characters): ");
        scanf("%s", password);

        char tmp_hash[65];
        sha256(password, tmp_hash);

        if (strcmp(hash, tmp_hash) == 0)
            evaluated_password = true;
    }

    bool evaluate_password() {
        return evaluated_password;
    }
};

int main()
{
    PASSWORD_CHECKER password_checker(PASSWORD_HASH);
    password_checker.read_password();
    if (password_checker.evaluate_password())
        printf("%sGood you are in!%s\n\n\n", COLOR_GREEN_ESCAPE, COLOR_RESET_ESCAPE);
    else
        printf("%sNo, password is incorect!%s\n\n\n", COLOR_RED_ESCAPE, COLOR_RESET_ESCAPE);

    system("pause");
    return 0;
}

