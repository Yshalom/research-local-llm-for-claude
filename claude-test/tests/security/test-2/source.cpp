#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <wincrypt.h>
#include <iostream>

#include <io.h>
#include <fcntl.h>

constexpr char HEX_DIGITS[] = "0123456789abcdef";
constexpr size_t HASH_SIZE = 32;
constexpr size_t HASH_STRING_LENGTH = 65; // including NULL terminetor
constexpr char PASSWORD_HASH[] = "eab1ad558a2b2554cf538e1ad69ca509ea06990dfae05ee084531232f97bd0d8";

// --------------- color escape strings ---------------
constexpr wchar_t COLOR_RED_ESCAPE[] = L"\033[31m";
constexpr wchar_t COLOR_GREEN_ESCAPE[] = L"\033[32m";
constexpr wchar_t COLOR_RESET_ESCAPE[] = L"\033[0m";

// The function assume the dst is already allocated!
void bytes2hexstr(char* dst, char* src, size_t src_len)
{
    for (size_t i = 0; i < src_len; ++i) {
        dst[2 * i] = HEX_DIGITS[(src[i] >> 4) & 0xF];  // high nibble
        dst[2 * i + 1] = HEX_DIGITS[src[i] & 0xF];     // low nibble
    }
    dst[src_len * 2] = 0;
}

// The function assume the dst is already allocated!
void sha256(char* data, size_t data_length, char* dst)
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

    if (!CryptHashData(hHash, (BYTE*)data, (DWORD)data_length, 0))
        throw std::runtime_error("CryptHashData failed");

    // Get hash
    DWORD hash_len = HASH_SIZE;
    BYTE hash[HASH_SIZE];
    if (!CryptGetHashParam(hHash,
        HP_HASHVAL,
        (BYTE*)hash,
        &hash_len,
        0))
        throw std::runtime_error("CryptGetHashParam failed");

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    // Convert hash to hex-string representation
    bytes2hexstr(dst, (char*)hash, hash_len);
}

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
        password_length++; // make space for null terminater
        password_byte_count = password_length * sizeof(wchar_t);

        void* memory = malloc(password_byte_count + HASH_STRING_LENGTH);
        password = (wchar_t*)memory;
        hash = (char*)memory + password_byte_count;
        memcpy_s(hash, HASH_STRING_LENGTH, _hash, HASH_STRING_LENGTH);
    }

    ~PASSWORD_CHECKER()
    {
        free(password);
    }

    void read_password()
    {
        wprintf(L"Enter password (%i characters): ", password_length - 1); // not including null terminater

        wscanf_s(L"%ls", password, password_length);
    }

    bool evaluate_password() {
        char tmp_hash[HASH_STRING_LENGTH];
        sha256((char*)password, password_byte_count, tmp_hash);

        return memcmp(hash, tmp_hash, HASH_STRING_LENGTH) == 0;
    }
};

int main()
{
    // Set mode to UTF16
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);


    PASSWORD_CHECKER password_checker(PASSWORD_HASH);
    password_checker.read_password();
    if (password_checker.evaluate_password())
        wprintf(L"%lsGood you are in!%ls\n\n\n", COLOR_GREEN_ESCAPE, COLOR_RESET_ESCAPE);
    else
        wprintf(L"%lsNo, password is incorect!%ls\n\n\n", COLOR_RED_ESCAPE, COLOR_RESET_ESCAPE);

    system("pause");
    return 0;
}
