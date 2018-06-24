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

#include <locale.h>
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
#define SLEEP_SEC 10
#define SLEEP_1 2 /* SLEEP_SEC * SLEEP_1 */
#define SLEEP_2 6 /* SLEEP_SEC * SLEEP_2 */
#define BUF_CAPACITY_SZ 16
#define BUF_LINK_SZ 32
#define BUF_DATETIME_SZ 128
#define BUF_SZ (64 + BUF_CAPACITY_SZ + BUF_LINK_SZ + BUF_DATETIME_SZ)

static char *getprogname_of_argv(char *argv_zero_tzs);

int main(int argc __attribute__((unused)), char *argv[])
{
    Display *dpy;
    Window w;
    size_t count_1 = 1, count_2 = 1;
    char capacity[BUF_CAPACITY_SZ], lnk[BUF_LINK_SZ], datetime[BUF_DATETIME_SZ],
        buf[BUF_SZ];

    /* setlocale(LC_TIME, ""); */

    if ((dpy = XOpenDisplay(NULL)) == NULL)
    {
        fprintf(stderr, "%s: Error open DISPLAY\n",
                getprogname_of_argv(argv[0]));
        return EXIT_FAILURE;
    }

    w = DefaultRootWindow(dpy);

    for (;; sleep(SLEEP_SEC))
    {
        time_t tm;

        if (!--count_1)
        {
            FILE *fp = fopen(PATH_LINK, "rb");
            if (fp != NULL)
            {
                size_t i;

                fgets(lnk, BUF_LINK_SZ, fp);
                fclose(fp);
                /* trim first newline and everything after */
                for (i = 0; lnk[i]; ++i)
                    if (lnk[i] == '\n')
                    {
                        lnk[i] = '\0';
                        break;
                    }

                /* check internet availability
                   possible WIFI_IFNAME without "	00000000" address
                   (tab in "	00000000" is required) */
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

            count_1 = SLEEP_1;
        }

        if (!--count_2)
        {
            FILE *fp = fopen(PATH_CAPACITY, "rb");
            if (fp != NULL)
            {
                size_t i;

                fgets(capacity, BUF_CAPACITY_SZ, fp);
                fclose(fp);
                /* trim first newline and everything after */
                for (i = 0; capacity[i]; ++i)
                    if (capacity[i] == '\n')
                    {
                        capacity[i] = '\0';
                        break;
                    }
            }
            else
                capacity[0] = '\0';

            count_2 = SLEEP_2;
        }

        tm = time(NULL);
        strftime(datetime, BUF_DATETIME_SZ, DATETIME_FORMAT, localtime(&tm));

        sprintf(buf, BUF_FORMAT, capacity, lnk, datetime);

        XStoreName(dpy, w, buf);
        XFlush(dpy);
    }

    /* code will never be executed
    XCloseDisplay(dpy);
    */
}

/* get current process name without path */
char *getprogname_of_argv(char *argv_zero_tzs)
{
    char *p;
    if (argv_zero_tzs == NULL)
        return NULL;
    for (p = argv_zero_tzs; *argv_zero_tzs; ++argv_zero_tzs)
        if (*argv_zero_tzs == '/')
            p = argv_zero_tzs + 1;
    return p;
}
