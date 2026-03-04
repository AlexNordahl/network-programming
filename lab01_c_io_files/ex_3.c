#include <stdio.h>
#include <stdbool.h>

bool is_printable_buf(const void* buf, int len)
{
    char* strs = (char*) buf;
    char* begin = strs;
    char* end = strs + len;

    while (begin != end)
    {
        if (*begin < 32 || *begin > 126)
            return false;

        ++begin;
    }
    return true;
}

int main()
{
    printf("%d\n", is_printable_buf("abcdefg", 7));
    printf("%d\n", is_printable_buf("abc\0defg", 8));
}