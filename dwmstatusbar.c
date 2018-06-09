/*
 * dwmstatusbar.c
 * Very simple statusbar for dynamic window manager (dwm)
 * (battery capacity, link status, date-time)
 *
 * Andrey Shashanov (2018)
 *
 * Edit your: IFACE and BATTERY
 *
 * Build:
 * gcc -Wall -pedantic -std=c99 -O3 -s -lX11 dwmstatusbar.c -o dwmstatusbar
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
#define BUF_SZ (64 + BUF_CAPACITY_SZ + BUF_LINK_SZ + BUF_DATETIME_SZ)

int main(void)
{
    Display *dpy;
    Window w;
    size_t count_1 = SLEEP_1, count_2 = SLEEP_2;
    char capacity[BUF_CAPACITY_SZ], lnk[BUF_LINK_SZ], datetime[BUF_DATETIME_SZ],
        buf[BUF_SZ];

    if ((dpy = XOpenDisplay(NULL)) == NULL)
    {
        fprintf(stderr, "Error open DISPLAY\n");
        exit(EXIT_FAILURE);
    }

    w = DefaultRootWindow(dpy);

    for (;; sleep(SLEEP_SEC))
    {
        FILE *fp;
        size_t i;
        time_t timeraw;

        if (++count_1 > SLEEP_1)
        {
            count_1 = 0;

            if ((fp = fopen(PATH_LINK, "rb")) != NULL)
            {
                fgets(lnk, BUF_LINK_SZ, fp);
                fclose(fp);
                /* trim new line */
                for (i = 0; lnk[i]; ++i)
                    if (lnk[i] == '\n')
                    {
                        lnk[i] = '\0';
                        break;
                    }

                /* possible WIFI_IFNAME without "	00000000" address
                 * (tab in "	00000000" is required) */
                /*
                char rstr[] = IFACE "	00000000";
                char rbuf[64];
                if ((fp = fopen("/proc/net/route", "rb")) != NULL)
                {
                    while (fgets(rbuf, sizeof(rbuf), fp) != NULL)
                        if (strstr(rbuf, rstr) != NULL && lnk[0] != 'd')
                        {
                            strcat(lnk, "*");
                            break;
                        }
                    fclose(fp);
                }
                */
            }
            else
                lnk[0] = '\0';
        }

        if (++count_2 > SLEEP_2)
        {
            count_2 = 0;

            if ((fp = fopen(PATH_CAPACITY, "rb")) != NULL)
            {
                fgets(capacity, BUF_CAPACITY_SZ, fp);
                fclose(fp);
                /* trim new line */
                for (i = 0; capacity[i]; ++i)
                    if (capacity[i] == '\n')
                    {
                        capacity[i] = '\0';
                        break;
                    }
            }
            else
                capacity[0] = '\0';
        }

        timeraw = time(NULL);
        if (strftime(datetime, BUF_DATETIME_SZ, DATETIME_FORMAT,
                     localtime(&timeraw)) == 0)
            datetime[0] = '\0';

        sprintf(buf, BUF_FORMAT, capacity, lnk, datetime);

        XStoreName(dpy, w, buf);
        XFlush(dpy);
    }

    /* code will never be executed
    XCloseDisplay(dpy);
    exit(EXIT_SUCCESS);
    */
}
