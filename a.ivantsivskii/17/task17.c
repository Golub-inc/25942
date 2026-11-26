#define _XOPEN_SOURCE 600

#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

#define CTRL_D 4
#define CTRL_G 7
#define CTRL_W 23

#ifdef _POSIX_VDISABLE
static int key_ok(cc_t c) {
    return c != _POSIX_VDISABLE;
}
#else
static int key_ok(cc_t c) {
    (void)c;
    return 1;
}
#endif

static void wr(const char *s, size_t n) {
    ssize_t w;
    size_t off;

    off = 0;

    while (off < n) {
        w = write(STDOUT_FILENO, s + off, n - off);
        if (w <= 0) {
            break;
        }
        off += (size_t)w;
    }
}

static void outc(unsigned char c) {
    wr((const char *)&c, 1);
}

static void beep(void) {
    outc(CTRL_G);
}

static void erase_one(void) {
    wr("\b \b", 3);
}

int main(void) {
    struct termios orig;
    struct termios raw;
    cc_t erasec;
    cc_t killc;
    unsigned char c;
    ssize_t n;
    char line[128];
    size_t len;
    size_t i;
    size_t ws;
    size_t wlen;
    int esc;

    if (tcgetattr(STDIN_FILENO, &orig) == -1) {
        return 1;
    }

    erasec = orig.c_cc[VERASE];
    killc = orig.c_cc[VKILL];

    raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL | INLCR | IGNCR | ISTRIP);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) {
        return 1;
    }

    len = 0;
    esc = 0;

    while ((n = read(STDIN_FILENO, &c, 1)) == 1) {
        if (c == 27) {
            esc = 1;
            beep();
            continue;
        }

        if (esc == 1) {
            if (c == '[' || c == 'O') {
                esc = 2;
                continue;
            }
            esc = 0;
        }

        if (esc == 2) {
            if (c >= '@' && c <= '~') {
                esc = 0;
            }
            continue;
        }
        if (c == CTRL_D) {
            if (len == 0) {
                break;
            }

            beep();
            continue;
        }

        if (key_ok(erasec) && c == erasec) {
            if (len == 0) {
                beep();
            } else {
                len--;
                erase_one();
            }

            continue;
        }

        if (key_ok(killc) && c == killc) {
            if (len == 0) {
                beep();
            } else {
                while (len > 0) {
                    erase_one();
                    len--;
                }
            }

            continue;
        }

        if (c == CTRL_W) {
            if (len == 0) {
                beep();
            } else {
                while (len > 0 && line[len - 1] == ' ') {
                    len--;
                    erase_one();
                }

                while (len > 0 && line[len - 1] != ' ') {
                    len--;
                    erase_one();
                }
            }

            continue;
        }

        if (c >= ' ' && c <= '~') {
            if (len < 40) {
                line[len++] = (char)c;
                outc(c);
            } else if (c == ' ') {
                beep();
            } else {
                ws = len;

                while (ws > 0 && line[ws - 1] != ' ') {
                    ws--;
                }

                wlen = len - ws;

                if (wlen + 1 > 40) {
                    beep();
                    continue;
                }

                for (i = 0; i < wlen; i++) {
                    erase_one();
                }

                outc('\n');

                if (wlen > 0) {
                    wr(line + ws, wlen);
                }

                outc(c);

                if (wlen > 0) {
                    memmove(line, line + ws, wlen);
                }

                line[wlen] = (char)c;
                len = wlen + 1;
            }

            continue;
        }

        beep();
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &orig);

    return 0;
}