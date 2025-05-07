#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <EMA.h>

#define printl(MSG) do { printf(MSG "\n"); } while (0)

#define HANDLE_ERROR(ERR) do { \
    if (ERR) { \
        printf("Error: %d\n", ERR); \
        return EXIT_FAILURE; \
    } \
} while (0)

void get_iso_time(char* buffer, size_t buff_size) {
    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, buff_size, "%Y-%m-%dT%H:%M:%S", timeinfo);
    snprintf(
        buffer + strlen(buffer),
        buff_size - strlen(buffer),
        "%c%02d:%02d",
        (timeinfo->tm_gmtoff >= 0 ? '+' : '-'),
        abs(timeinfo->tm_gmtoff) / 3600,
        (abs(timeinfo->tm_gmtoff) % 3600) / 60
    );
}

void append_quoted(char *dest, const char *src) {
    strcat(dest, "\"");

    for (; *src; ++src) {
        if (*src == '\"') {
            strcat(dest, "\"\\\"\"");
        } else {
            strncat(dest, src, 1);
        }
    }
    strcat(dest, "\" ");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printl("USAGE: ema_measure CMD [args...]");
        return EXIT_FAILURE;
    }

    char cmd[4096];
    cmd[0] = '\0';
    for (int i = 1; i < argc; i++) {
        append_quoted(cmd, argv[i]);
    }
    printf("CMD: %s\n", cmd);

    printl("Initiallizing EMA...");
    int err = EMA_init(NULL);
    HANDLE_ERROR(err);


    char ts_start[64];
    char ts_end[64];

    get_iso_time(ts_start, 64);

    EMA_REGION_DECLARE(region);
    EMA_REGION_DEFINE(&region, "region");

    EMA_REGION_BEGIN(region);

    system(cmd);

    EMA_REGION_END(region);
    
    get_iso_time(ts_end, 64);

    printl("Finalizing EMA...");
    err = EMA_finalize();
    HANDLE_ERROR(err);

    pid_t pid = getpid();

    char filename[64];
    snprintf(filename, 64, "timestamps.EMA.%u", pid);

    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        perror("Failed to open timestamps file!");
        return EXIT_FAILURE;
    }

    fprintf(f, "ts_start,ts_end\n");
    fprintf(f, "%s,%s\n", ts_start, ts_end);
    fclose(f);

    return EXIT_SUCCESS;
}
