/*
** dwmstatusbars.c
** Very simple statusbar (battery, link, date-time) for dynamic window manager (dwm)
** VERSION = 1.0
** Andrey Shashanov <shashanov@outlook.com>
**
** Compile with:
** gcc -Wall -pedantic -std=c99 -O3 -lX11 dwmstatusbars.c -o dwmstatusbars
*/

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#define IFACE "wlp3s0"
#define BATTERY "BAT0"
#define PATH_LINK "/sys/class/net/" IFACE "/operstate"
#define PATH_CAPACITY "/sys/class/power_supply/" BATTERY "/capacity"
#define DATETIME_FORMAT "%Y-%m-%d %a %H:%M"
#define SLEEP_SEC 5
#define SLEEP_1 2  // SLEEP * SLEEP_1 + SLEEP
#define SLEEP_2 11 // SLEEP * SLEEP_2 + SLEEP
#define BUF_DATETIME_SZ 128
#define BUF_LINK_SZ 64
#define BUF_CAPACITY_SZ 32
#define BUF_SZ 256

int trimnewln(int size_buf, char *buf);

int trimnewln(int size_buf, char *buf)
{
    int i;
    for (i = 0; i < size_buf; i++)
    {
        if (buf[i] == '\n')
        {
            buf[i] = '\0';
            break;
        }
        else if (buf[i] == '\0')
            break;
    }
    return i;
}

int main(void)
{
    Display *dpy;
    time_t timeraw;
    FILE *fp;
    int count_1 = SLEEP_1, count_2 = SLEEP_2;
    char timefrm[BUF_DATETIME_SZ], lnk[BUF_LINK_SZ],
        capacity[BUF_CAPACITY_SZ], status[BUF_SZ];

    dpy = XOpenDisplay(NULL);

    for (;; sleep(SLEEP_SEC))
    {
        count_1++;
        count_2++;

        timeraw = time(NULL);
        strftime(timefrm, BUF_DATETIME_SZ, DATETIME_FORMAT,
                 localtime(&timeraw));

        if (count_1 > SLEEP_1)
        {
            count_1 = 0;

            fp = fopen(PATH_LINK, "r");
            fgets(lnk, BUF_LINK_SZ, fp);
            fclose(fp);
            trimnewln(BUF_LINK_SZ, lnk);
        }

        if (count_2 > SLEEP_2)
        {
            count_2 = 0;

            fp = fopen(PATH_CAPACITY, "r");
            fgets(capacity, BUF_CAPACITY_SZ, fp);
            fclose(fp);
            trimnewln(BUF_CAPACITY_SZ, capacity);
        }

        snprintf(status, BUF_SZ, "%s:%s%%  %s:%s  %s",
                 BATTERY, capacity, IFACE, lnk, timefrm);

        XStoreName(dpy, DefaultRootWindow(dpy), status);
        XSync(dpy, False);
    }

    XCloseDisplay(dpy);
    exit(EXIT_SUCCESS);
}
