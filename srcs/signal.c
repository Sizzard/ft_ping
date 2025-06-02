#include "../ft_ping.h"

void sigint_handler_func(int signo) {
    if (signo == SIGINT) {
        g_sigint = signo;
        printf("\nCtrl-C pressed quitting ...\n");
    }
}

void signal_handler() {
    struct sigaction act;
    
    bzero(&act, sizeof(act));
    act.sa_handler = &sigint_handler_func;
    sigaction(SIGINT, &act, NULL);
}