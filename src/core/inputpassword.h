#include <iostream>
#include <string>

#ifdef Q_OS_WIN
#include <conio.h>
#else
#include <unistd.h>
#endif



static void inputPassword(std::string &str, int size)
{

#ifndef Q_OS_WIN
    str = getpass("");
#else


    char c;
    int count = 0;
    char *password = new char[size];
    memset(password, 0, size);

    while ((c = getch()) != '\r') {

        if (c == 8) {
            if (count == 0) {
                continue;
            }
            putchar('\b');
            putchar(' ');
            putchar('\b');
            password[count] = 0;
            count--;
        }

        if (count == size - 1) {
            break;
        }

        if(c < 32 || c > 126){
            continue;
        }

        putchar('*');
        password[count] = c;
        count++;

    }

    password[count] = '\0';
    str = password;
    delete[] password;
    std::cout << std::endl;

#endif

}
