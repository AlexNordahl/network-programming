#include <stdio.h>
#include <stdbool.h>

bool is_printable_str(const char* str)
{
    char* ptr = str;
    while (*ptr != '\0')
    {
        if (*ptr < 32 || *ptr > 126)
            return false;

        ++ptr;
    }

    return true;
}

int main()
{
    printf("%d\n", is_printable_str("abcd"));
    printf("%d\n", is_printable_str("ab\128cd"));
}