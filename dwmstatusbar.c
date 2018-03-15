/*
** dwmstatusbar.c
** Simple statusbar (battery, wifi, date-time) for dynamic window manager (dwm)
** VERSION = 1.0
** Andrey Shashanov <shashanov@outlook.com>
**
** Compile with:
** gcc -Wall -pedantic -std=c99 -O3 -liw -lX11 dwmstatusbar.c -o dwmstatusbar
*/

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <iwlib.h>

#define WIFI_IFNAME "wlp3s0"
#define BAT_NOW "/sys/class/power_supply/BAT0/energy_now"
#define BAT_FULL "/sys/class/power_supply/BAT0/energy_full"
#define BAT_V_NOW "/sys/class/power_supply/BAT0/voltage_now"
#define BAT_STAT "/sys/class/power_supply/BAT0/status"
#define DATETIME_FORMAT "%Y-%m-%d %a %H:%M"
#define DATETIME_BUF_SZ 128
#define CHARGING_BUF_SZ 64
#define BAT_BUF_SZ (32 + CHARGING_BUF_SZ)
#define WIFI_BUF_SZ 32
#define BUF_SZ 320
#define LOWBATTERY 20

#define SLEEP_SEC 5
#define SLEEP_1 2  // SLEEP_SEC * SLEEP_1 + SLEEP_SEC
#define SLEEP_2 11 // SLEEP_SEC * SLEEP_2 + SLEEP_SEC

int trimnewln(int size_buf, char *buf);
int getbattery(int size_buf, char *buf);
int getwifi(void);
int getint(void);
void getdatetime(char *buf);

static Display *dpy;

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

int getbattery(int size_buf, char *buf)
{
    FILE *fd;
    int energy_now, energy_full, voltage_now;

    fd = fopen(BAT_NOW, "r");
    fscanf(fd, "%d", &energy_now);
    fclose(fd);

    fd = fopen(BAT_FULL, "r");
    fscanf(fd, "%d", &energy_full);
    fclose(fd);

    fd = fopen(BAT_V_NOW, "r");
    fscanf(fd, "%d", &voltage_now);
    fclose(fd);

    fd = fopen(BAT_STAT, "r");
    fgets(buf, size_buf, fd);
    fclose(fd);

    trimnewln(size_buf, buf);

    return (int)(((float)energy_now * 1000 / (float)voltage_now) * 100 /
                 ((float)energy_full * 1000 / (float)voltage_now));
}

int getwifi(void)
{
    struct wireless_info winfo[sizeof(struct wireless_config)];
    int socket;

    memset((unsigned char *)winfo, 0, sizeof(struct wireless_config));

    socket = iw_sockets_open();

    if (iw_get_basic_config(socket, WIFI_IFNAME, &(winfo->b)))
    {
        iw_sockets_close(socket);
        return -1;
    }

    iw_get_stats(socket, WIFI_IFNAME, &(winfo->stats), &winfo->range,
                 winfo->has_range);

    iw_get_range_info(socket, WIFI_IFNAME, &(winfo->range));

    iw_sockets_close(socket);

    return (winfo->stats.qual.qual * 100) / winfo->range.max_qual.qual;
}

// Check internet availability (maybe there's a better way?)
int getint(void)
{
    FILE *sp;
    unsigned int i = 0;
    if ((sp = popen("/sbin/route -n | grep -c '^0\\.0\\.0\\.0'", "r")) != NULL)
    {
        fscanf(sp, "%u", &i);
        pclose(sp);
        if (i > 0)
            return 1;
    }
    return 0;
}
/*
int getint(void)
{
    struct addrinfo *info;
    if (getaddrinfo("8.8.8.8", "80", NULL, &info) == 0)
    {
        freeaddrinfo(info);
        return 1;
    }
    return 0;
}
*/

void getdatetime(char *buf)
{
    time_t timeraw;

    timeraw = time(NULL);
    strftime(buf, DATETIME_BUF_SZ, DATETIME_FORMAT, localtime(&timeraw));
}

int main(void)
{
    int bat0, wifi, count_1 = SLEEP_1, count_2 = SLEEP_2;
    char datetime_st[DATETIME_BUF_SZ],
        bat0_st[BAT_BUF_SZ],
        charging[CHARGING_BUF_SZ],
        wifi_st[WIFI_BUF_SZ],
        status[BUF_SZ];

    dpy = XOpenDisplay(NULL);

    for (;; sleep(SLEEP_SEC))
    {
        count_1++;
        count_2++;

        getdatetime(datetime_st);

        if (count_1 > SLEEP_1)
        {
            count_1 = 0;

            wifi = getwifi();

            if (wifi < 0)
                wifi_st[0] = 0;
            else if (wifi > 0 && getint() > 0)
                snprintf(wifi_st, WIFI_BUF_SZ, " WiFi:%i%%* ", wifi);
            else
                snprintf(wifi_st, WIFI_BUF_SZ, " WiFi:%i%% ", wifi);
        }

        if (count_2 > SLEEP_2)
        {
            count_2 = 0;

            bat0 = getbattery(CHARGING_BUF_SZ, charging);

            if (bat0 > LOWBATTERY)
                snprintf(bat0_st, BAT_BUF_SZ, "Bat:%i%% %s", bat0, charging);
            else
                snprintf(bat0_st, BAT_BUF_SZ, "Low Bat:%i%% %s", bat0, charging);
        }

        snprintf(status, BUF_SZ, "%s %s %s", bat0_st, wifi_st, datetime_st);

        XStoreName(dpy, DefaultRootWindow(dpy), status);
        XSync(dpy, False);
    }

    XCloseDisplay(dpy);
    exit(EXIT_SUCCESS);
}
