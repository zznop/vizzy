#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>
#include <spawn.h>
#include <sys/wait.h>

#define VIZZY_SO_PATH "/tmp/libvizzy.so"
#define PRELOAD_ENV_SZ 256

extern char **environ;
extern uint8_t g_libvizzy[0];
extern int g_libvizzy_size;

static void _print_usage(char *name)
{
    printf("%s <command>\n", name);
}

static bool _drop_vizzy_so(void)
{
    FILE *f = fopen(VIZZY_SO_PATH, "wb");
    if (!f) {
        fprintf(stderr, "Failed to open vizzy shared object\n");
        return false;
    }

    int n = fwrite(g_libvizzy, g_libvizzy_size, 1, f);
    if (n != 1)
        fprintf(stderr, "Failed to write vizzy shared object\n");

    fclose(f);
    return n == 1;
}

static bool _spawn_process(char **argv, pid_t *pid)
{
    char preload_env[PRELOAD_ENV_SZ] = {0};
    int n = snprintf(preload_env, sizeof(preload_env), "LD_PRELOAD=%s", VIZZY_SO_PATH);
    if ((unsigned)n >= sizeof(preload_env)) {
        fprintf(stderr, "LD_PRELOAD environment variable truncated!?\n");
        return false;
    }

    // Count environment variables from our environment
    size_t i = 0;
    while (environ[i] != NULL)
        i++;

    // Create new environment with our variables and the added LD_PRELOAD variable
    char **newenv = malloc((i+2)*sizeof(*environ));
    memcpy(newenv, environ, i*sizeof(*environ));
    newenv[i] = preload_env;
    newenv[i+1] = NULL;

    // Attempt to spawn the command as a child process
    int rc = posix_spawn(pid, argv[0], NULL, NULL, argv, newenv);
    if (rc != 0)
        fprintf(stderr, "Failed to spawn the target process\n");

    free(newenv);
    return rc == 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        _print_usage(argv[0]);
        return 1;
    }

    bool rv = _drop_vizzy_so();
    if (!rv)
        return 1;

    pid_t pid;
    rv = _spawn_process(&argv[1], &pid);
    if (!rv)
        return 1;

    printf("Target process started (pid=%i)\n", pid);
    int status;
    if (waitpid(pid, &status, 0) == -1)
        fprintf(stderr, "Error while waiting on target process\n");

    if (WIFEXITED(status))
        printf("Target process exited with a return code of %i\n", WEXITSTATUS(status));

    if (WIFSIGNALED(status))
        printf("Target process exited via signal %i\n", WTERMSIG(status));
}
