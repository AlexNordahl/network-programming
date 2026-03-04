#include <stdio.h>

void drukuj_alt(int* arr, int size)
{
    int* begin = arr;
    int* end = arr + size;
    while (begin != end)
    {
        if (*begin > 10 && *begin < 100)
        {
            printf("Number: %d\n", *begin);
        }

        ++begin;
    }
}

int main()
{
    int size = 50;
    int arr[size];

    for (int i = 0; i < size; ++i)
    {
        int num;
        printf("Enter next number: ");
        scanf("%d", &num);

        if (num == 0)
        {
            size = i;
            break;
        }

        arr[i] = num;
    }

    drukuj_alt(arr, size);
}