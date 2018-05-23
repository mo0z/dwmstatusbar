/*
 * dwmstatusbar.c
 * Very simple statusbar for dynamic window manager (dwm)
 * (battery capacity, link status, date-time)
 *
 * Andrey Shashanov (2018)
 *
 * Edit your: IFACE and BATTERY
 *
 * Compile with:
 * gcc -Wall -pedantic -std=c99 -Os -s -lX11 dwmstatusbar.c -o dwmstatusbar
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#define IFACE "wlp3s0"
#define BATTERY "BAT0"
#define PATH_LINK "/sys/class/net/" IFACE "/operstate"
#define PATH_CAPACITY "/sys/class/power_supply/" BATTERY "/capacity"
#define DATETIME_FORMAT "%Y-%m-%d %a %H:%M"
#define BUF_FORMAT BATTERY ":%s%%  " IFACE ":%s  %s"
#define SLEEP_SEC 5
#define SLEEP_1 2  /* SLEEP_SEC * SLEEP_1 + SLEEP_SEC */
#define SLEEP_2 11 /* SLEEP_SEC * SLEEP_2 + SLEEP_SEC */
#define BUF_CAPACITY_SZ 16
#define BUF_LINK_SZ 32
#define BUF_DATETIME_SZ 128
#define BUF_SZ 256

int main(void)
{
    Display *dpy;
    Window w;
    size_t count_1 = SLEEP_1, count_2 = SLEEP_2;
    char capacity[BUF_CAPACITY_SZ], lnk[BUF_LINK_SZ], datetime[BUF_DATETIME_SZ],
        status[BUF_SZ];

    if ((dpy = XOpenDisplay(NULL)) == NULL)
    {
        fprintf(stderr, "Error open DISPLAY\n");
        exit(EXIT_FAILURE);
    }

    w = DefaultRootWindow(dpy);

    while (1)
    {
        FILE *fp;
        time_t timeraw;
        size_t i;

        if (++count_1 > SLEEP_1)
        {
            count_1 = 0;

            fp = fopen(PATH_LINK, "rb");
            fgets(lnk, BUF_LINK_SZ, fp);
            fclose(fp);
            /* trim new line */
            for (i = 0; lnk[i]; i++)
                if (lnk[i] == '\n')
                {
                    lnk[i] = '\0';
                    break;
                }

            /* check internet availability (simply uncomment)
             * can IFACE without 00000000 address */
            /*
            char rstr[] = IFACE "	00000000";
            char rbuf[64];
            fp = fopen("/proc/net/route", "rb");
            while (fgets(rbuf, sizeof(rbuf), fp) != NULL)
                if (strstr(rbuf, rstr) != NULL)
                {
                    strcat(lnk, "*");
                    break;
                }
            fclose(fp);
            */
        }

        if (++count_2 > SLEEP_2)
        {
            count_2 = 0;

            fp = fopen(PATH_CAPACITY, "rb");
            fgets(capacity, BUF_CAPACITY_SZ, fp);
            fclose(fp);
            /* trim new line */
            for (i = 0; capacity[i]; i++)
                if (capacity[i] == '\n')
                {
                    capacity[i] = '\0';
                    break;
                }
        }

        timeraw = time(NULL);
        strftime(datetime, BUF_DATETIME_SZ, DATETIME_FORMAT,
                 localtime(&timeraw));

        sprintf(status, BUF_FORMAT, capacity, lnk, datetime);

        XStoreName(dpy, w, status);
        XFlush(dpy);

        sleep(SLEEP_SEC);
    }

    /* code will never be executed
    XCloseDisplay(dpy);
    exit(EXIT_SUCCESS);
    */
}
