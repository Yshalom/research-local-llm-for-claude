# What you found?
The code is NOT secure. It contains a buffer overflow vulnerability in the `read_password` method of the `PASSWORD_CHECKER` class.

# How it could be badly used?
An attacker can provide an input string longer than 20 characters (e.g., by entering 100 characters) into the `scanf("%s", password)` call. Since the `password` buffer is only 21 bytes long, this will cause a buffer overflow on the stack. An attacker could potentially use this to overwrite adjacent memory, including return addresses or other local variables, leading to arbitrary code execution or a crash (denial of service).

# Which line(s) of the original source are problematic?
Line 70: `scanf("%s", password);`

# Recommendation for a fix
Replace the unsafe `scanf("%s", password)` with a length-limited function like `fgets` or use a width specifier in `scanf`, such as `scanf("%20s", password)`. Additionally, consider using safer alternatives to `printf` and `scanf` from C++ standard libraries (e.g., `std::cin` or `std::getline`) which provide better bounds checking and type safety.