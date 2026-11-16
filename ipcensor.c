/* Made by ouroboros a.k.a WhereIsRooot */
#include <pty.h>
#include <unistd.h>
#include <poll.h>
#include <termios.h>
#include <string.h>

int count_num(const char *s) {
    int n = 0;
    while (s[n] >= '0' && s[n] <= '9') {
        n++;
    }
    return n;
}

int match_ips(const char *s) {
    const char *p = s;

    for (int i = 0; i < 4; i++) {
        int c = count_num(p);
        if (c < 1 || c > 3)
            return 0;
        p += c;

        if (i < 3) {
            if (*p != '.')
                return 0;
            p++;
        }
    }
    return p - s;
}

int censor(const char *in, int n, char *out) {
    int o = 0;
    int i = 0;
    while (i < n) {
        int prev_ok = (i == 0) ||
            !((in[i-1] >= '0' && in[i-1] <= '9') || in[i-1] == '.');

        int len = prev_ok ? match_ips(in + i) : 0;

        if (len > 0) {
            memcpy(out + o, "*.*.*.*", 15);
            o += 15;
            i += len;
        } else {
            out[o++] = in[i++];
        }
    }
    return o;
}

int main(void) {
    int master;
    pid_t pid = forkpty(&master, NULL, NULL, NULL);
    if (pid == 0) {
        execlp("bash", "bash", NULL);
        _exit(1);
    }

    struct termios orig, raw;
    tcgetattr(STDIN_FILENO, &orig);
    raw = orig;
    cfmakeraw(&raw);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    struct pollfd fds[2] = {
        { STDIN_FILENO, POLLIN, 0 },
        { master,       POLLIN, 0 },
    };
    char buf[4096];
    char out[4096 * 3];

    for (;;) {
        if (poll(fds, 2, -1) < 0) break;

        if (fds[0].revents & POLLIN) {
            ssize_t n = read(STDIN_FILENO, buf, sizeof buf);
            if (n <= 0) break;
            write(master, buf, n);
        }
        if (fds[1].revents & (POLLIN | POLLHUP)) {
            ssize_t n = read(master, buf, sizeof buf - 1);
            if (n <= 0) break;
            buf[n] = '\0';
            int m = censor(buf, n, out);
            write(STDOUT_FILENO, out, m);
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &orig);
    return 0;
}