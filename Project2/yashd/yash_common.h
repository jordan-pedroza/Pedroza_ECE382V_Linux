#define MAX_DATA 1024
static char YASHD_PORT[] = "3820";

void clear_string(char* str, int strlen)
{
    for (int i = 0; i < strlen; i++)
    {
        str[i] = 0;
    }
}
