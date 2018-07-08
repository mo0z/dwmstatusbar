/*
Very simple statusbar for dwm (dynamic window manager):
battery capacity, link status, date-time.
Andrey Shashanov (2018)

Edit your: IFNAME and BATTERY
gcc -O3 -s -lX11 -o dwmstatusbar dwmstatusbar.c
*/

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#define IFNAME "wlp3s0"
#define BATTERY "BAT0"

#define PATH_LINK "/sys/class/net/" IFNAME "/operstate"
#define PATH_CAPACITY "/sys/class/power_supply/" BATTERY "/capacity"
#define DATETIME_FORMAT "%Y-%m-%d %a %H:%M"
#define BUF_FORMAT BATTERY ":%s%%  " IFNAME ":%s  %s"
#define SLEEP_SEC 10U
#define SLEEP_1 2U /* SLEEP_SEC * SLEEP_1 */
#define SLEEP_2 6U /* SLEEP_SEC * SLEEP_2 */

static char *program_name(char *s);

int main(int argc __attribute__((unused)), char *argv[])
{
    Display *dpy;
    Window w;
    size_t count_1 = 1, count_2 = 1;
    char capacity[32], lnk[32], datetime[128],
        buf[64 + sizeof capacity + sizeof lnk + sizeof datetime];

    /* setlocale(LC_TIME, ""); */

    if ((dpy = XOpenDisplay(NULL)) == NULL)
    {
        fprintf(stderr, "%s: Error open DISPLAY\n", program_name(argv[0]));
        return EXIT_FAILURE;
    }

    w = DefaultRootWindow(dpy);

    for (;; sleep(SLEEP_SEC))
    {
        time_t timer;

        if (!(--count_1))
        {
            FILE *fp = fopen(PATH_LINK, "rb");
            if (fp != NULL)
            {
                char filebuf[sizeof lnk];

                setvbuf(fp, filebuf, _IOFBF, sizeof filebuf);

                if (fgets(lnk, sizeof lnk, fp) != NULL)
                {
                    /* tuncate newline */
                    size_t i;
                    for (i = 0; lnk[i]; ++i)
                        ;
                    --i;
                    if (lnk[i] == '\n')
                        lnk[i] = '\0';
                }
                else
                    lnk[0] = '\0';

                fclose(fp);

                /* check internet availability
                   possible IFNAME without "	00000000" address
                   (tab in "	00000000" is required) */
                /*
                if ((fp = fopen("/proc/net/route", "rb")) != NULL)
                {
                    char rbuf[256], rstr[] = IFNAME "	00000000";
                    while (fgets(rbuf, sizeof rbuf, fp) != NULL)
                        if (strstr(rbuf, rstr) != NULL && !strcmp(lnk, "up"))
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

        if (!(--count_2))
        {
            FILE *fp = fopen(PATH_CAPACITY, "rb");
            if (fp != NULL)
            {
                char filebuf[sizeof capacity];

                setvbuf(fp, filebuf, _IOFBF, sizeof filebuf);

                if (fgets(capacity, sizeof capacity, fp) != NULL)
                {
                    /* tuncate newline */
                    size_t i;
                    for (i = 0; capacity[i]; ++i)
                        ;
                    --i;
                    if (capacity[i] == '\n')
                        capacity[i] = '\0';
                }
                else
                    capacity[0] = '\0';

                fclose(fp);
            }
            else
                capacity[0] = '\0';

            count_2 = SLEEP_2;
        }

        timer = time(NULL);
        strftime(datetime, sizeof datetime, DATETIME_FORMAT, localtime(&timer));

        sprintf(buf, BUF_FORMAT, capacity, lnk, datetime);

        XStoreName(dpy, w, buf);
        XFlush(dpy);
    }

    /* code will never be executed
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
    */
}

/* get short program name */
char *program_name(char *s)
{
    char *p;
    if (s == NULL)
        return NULL;
    for (p = s; *s; ++s)
        if (*s == '/')
            p = s + 1U;
    return p;
}
