static int MAX_DATA = 512;  // TODO: what is our max data packet size?
static char YASHD_PORT[] = "3820";

void clear_string(char* str, int strlen)
{
    for (int i = 0; i < strlen; i++)
    {
        str[i] = 0;
    }
}