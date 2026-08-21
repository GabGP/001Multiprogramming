#include "stdlib.h"

// ============================================================================
// Standard Library Functions
// ============================================================================

// Function to convert string to integer
int atoi(const char *s) {
    int num = 0;
    int sign = 1;
    int i = 0;

    // Handle optional sign
    if (s[i] == '-') {
        sign = -1;
        i++;
    }

    for (; s[i] >= '0' && s[i] <= '9'; i++) {
        num = num * 10 + (s[i] - '0');
    }

    return sign * num;
}

// Function to convert integer to string
void itoa(int num, char *buffer) {
    int i = 0;
    int is_negative = 0;
    unsigned int u_num;

    if (num == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = 1;
        u_num = (unsigned int)(-(unsigned int)num);
    } else {
        u_num = (unsigned int)num;
    }

    while (u_num > 0 && i < 14) { // Reserve space for sign and null terminator
        buffer[i++] = '0' + (u_num % 10);
        u_num /= 10;
    }

    if (is_negative) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';

    // Reverse the string
    int start = 0, end = i - 1;
    char temp;
    while (start < end) {
        temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }
}

// Function to convert integer to hex
void itox(int num, char *buffer)
{
    unsigned int u_num = (unsigned int)num;
    const char hex_digits[] = "0123456789abcdef";
    char temp[9];
    int i = 0;

    if (u_num == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while (i < 8)
    {
        temp[i++] = hex_digits[u_num % 16];
        u_num /= 16;
    }

    // Reverse the string
    for (int j = 0; j < i; j++)
    {
        buffer[j] = temp[i - j - 1];
    }
    buffer[i] = '\0';
}
