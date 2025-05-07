#include <stdio.h>
#include <unistd.h>

#include <EMA.h>

#define printl(MSG) do { printf(MSG "\n"); } while (0)

#define HANDLE_ERROR(ERR) do { \
    if (ERR) { \
        printf("Error: %d\n", ERR); \
        return 1; \
    } \
} while (0)


int main() {
    printl("Initiallizing EMA...");
    int err = EMA_init(NULL);
    HANDLE_ERROR(err);

    printl("Finalizing EMA...");
    err = EMA_finalize();
    HANDLE_ERROR(err);

    return 0;
}
