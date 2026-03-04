#include <stdio.h>

void drukuj(int arr[], int size)
{
    for (int i = 0; i < size; ++i)
    {
        if (arr[i] > 10 && arr[i] < 100)
        {
            printf("Number #%d: %d\n", i, arr[i]);
        }
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

    drukuj(arr, size);
}