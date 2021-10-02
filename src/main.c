#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>
#include <spawn.h>
#include <sys/wait.h>
#include <limits.h>
#include <unistd.h>
#include "log.h"
#include "hooks.h"

#define VIZZY_SO_PATH "/tmp/libvizzy.so"
#define PRELOAD_ENV_SZ 256

extern char **environ;
extern uint8_t g_libvizzy[0];
extern int g_libvizzy_size;
extern const char *program_invocation_short_name;

static void _print_banner(void)
{
    printf(
        "\n _  _  ____  ____  ____  _  _\n"
        "( \\/ )(_  _)(_   )(_   )( \\/ )\n"
        " \\  /  _)(_  / /_  / /_  \\  /\n"
        "  \\/  (____)(____)(____) (__)\n\n"
    );
}

static void _print_usage(void)
{
    printf(
        "%s <log> <command>\n\n"

        "Required:\n"
        "  log        Path to output trace log\n"
        "  command    Command to execute (specify the full path to the executable)\n\n"

        "Example:\n"
        "  vizzy /tmp/heaptrace.csv /bin/find . -name vizzy\n", program_invocation_short_name
    );
}

static bool _drop_vizzy_so(char *filepath)
{
    info("Dropping libvizzy to disk at %s", VIZZY_SO_PATH);

    char tag[] = FILENAME_TAG;
    char *ptr = memmem(g_libvizzy, g_libvizzy_size, tag, FILENAME_TAG_SIZE);
    if (!ptr) {
        err("Failed to fixup libvizzy log file path\n");
        return false;
    }
    strncpy(ptr+FILENAME_TAG_SIZE, filepath, PATH_MAX);

    FILE *f = fopen(VIZZY_SO_PATH, "wb");
    if (!f) {
        err("Failed to open vizzy shared object");
        return false;
    }

    int n = fwrite(g_libvizzy, g_libvizzy_size, 1, f);
    if (n != 1)
        err("Failed to write vizzy shared object");

    fclose(f);
    return n == 1;
}

static bool _spawn_process(char **argv, pid_t *pid)
{
    char preload_env[PRELOAD_ENV_SZ] = {0};
    int n = snprintf(preload_env, sizeof(preload_env), "LD_PRELOAD=%s", VIZZY_SO_PATH);
    if ((unsigned)n >= sizeof(preload_env)) {
        err("LD_PRELOAD environment variable truncated!?");
        return false;
    }

    int rc = setenv("LD_PRELOAD", VIZZY_SO_PATH, 1);
    if (rc) {
        err("Failed to set LD_PRELOAD");
        return false;
    }

    rc = posix_spawn(pid, argv[0], NULL, NULL, argv, environ);
    if (rc != 0)
        err("Failed to spawn the target process");

    return rc == 0;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        _print_usage();
        return 1;
    }

    _print_banner();

    unlink(argv[1]);
    bool rv = _drop_vizzy_so(argv[1]);
    if (!rv)
        return 1;

    pid_t pid;
    rv = _spawn_process(&argv[2], &pid);
    if (!rv)
        return 1;

    info("Child process started (pid=%i)", pid);
    int status;
    if (waitpid(pid, &status, 0) == -1)
        err("Error while waiting on target process\n");

    if (WIFEXITED(status))
        info("Child process exited with a return code of %i", WEXITSTATUS(status));

    if (WIFSIGNALED(status))
        info("Child process exited via signal %i", WTERMSIG(status));
}
