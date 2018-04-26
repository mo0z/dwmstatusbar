/*
 * dwmstatusbar.c
 * Very simple statusbar (battery, link, date-time) for dynamic window manager (dwm)
 *
 * Andrey Shashanov (2018)
 *
 * (edit your: IFACE and BATTERY)
 *
 * Compile with:
 * gcc -Wall -pedantic -std=c99 -Os -s -lX11 dwmstatusbar.c -o dwmstatusbar
*/

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
#define SLEEP_1 2  /* SLEEP_SEC * SLEEP_1 + SLEEP_SEC */
#define SLEEP_2 11 /* SLEEP_SEC * SLEEP_2 + SLEEP_SEC */
#define BUF_DATETIME_SZ 128
#define BUF_LINK_SZ 64
#define BUF_CAPACITY_SZ 32
#define BUF_SZ 256

int main(void)
{
    Display *dpy;
    time_t timeraw;
    FILE *fp;
    int count_1 = SLEEP_1, count_2 = SLEEP_2;
    char timefrm[BUF_DATETIME_SZ], lnk[BUF_LINK_SZ],
        capacity[BUF_CAPACITY_SZ], status[BUF_SZ];

    if ((dpy = XOpenDisplay(NULL)) == NULL)
    {
        fprintf(stderr, "Error open DISPLAY\n");
        exit(EXIT_FAILURE);
    }

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
            /* trim new line */
            for (int i = 0; i < BUF_LINK_SZ; i++)
            {
                if (lnk[i] == '\n')
                {
                    lnk[i] = '\0';
                    break;
                }
                else if (lnk[i] == '\0')
                    break;
            }
        }

        if (count_2 > SLEEP_2)
        {
            count_2 = 0;

            fp = fopen(PATH_CAPACITY, "r");
            fgets(capacity, BUF_CAPACITY_SZ, fp);
            fclose(fp);
            /* trim new line */
            for (int i = 0; i < BUF_CAPACITY_SZ; i++)
            {
                if (capacity[i] == '\n')
                {
                    capacity[i] = '\0';
                    break;
                }
                else if (capacity[i] == '\0')
                    break;
            }
        }

        snprintf(status, BUF_SZ, "%s:%s%%  %s:%s  %s",
                 BATTERY, capacity, IFACE, lnk, timefrm);

        XStoreName(dpy, DefaultRootWindow(dpy), status);
        XSync(dpy, False);
    }

    /* code will never be executed
    XCloseDisplay(dpy);
    exit(EXIT_SUCCESS);
    */
}
