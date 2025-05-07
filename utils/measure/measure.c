#include <stdio.h>
#include <stdlib.h>

#include <EMA.h>

#define printl(MSG) do { printf(MSG "\n"); } while (0)

#define HANDLE_ERROR(ERR) do { \
    if (ERR) { \
        printf("Error: %d\n", ERR); \
        return 1; \
    } \
} while (0)


int main(int argc, char** argv) {
    if (argc != 2) {
        printl(
            "USAGE: ema_measure <CMD>\n\n"
            "CMD - execution command as a string (e.g. 'sleep 5')"
        );
        return EXIT_FAILURE;
    }

    char* cmd = argv[1];
    printf("CMD: %s\n", cmd);

    printl("Initiallizing EMA...");
    int err = EMA_init(NULL);
    HANDLE_ERROR(err);

    EMA_REGION_DECLARE(region);
    EMA_REGION_DEFINE(&region, "region");

    system(cmd);

    EMA_REGION_BEGIN(region);
    EMA_REGION_END(region);

    printl("Finalizing EMA...");
    err = EMA_finalize();
    HANDLE_ERROR(err);

    return 0;
}
