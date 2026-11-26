#include "socket_tcp.h"

static void serve_client(socket_tcp peer)
{
    if (!send_line(peer, "connessione avvenuta\n"))
        return;

    char line[LINE_CAPACITY];
    while (receive_line(peer, line)) {
        int32_t values[3];
        if (!parse_integers(line, values, 3)) {
            fputs("Richiesta non valida. Chiudo la sessione.\n", stderr);
            return;
        }

        int64_t result = (int64_t)values[0] + values[1] - values[2];
        printf("la somma di %" PRId32 "+%" PRId32 "-%" PRId32 "=%" PRId64 "\n",
               values[0], values[1], values[2], result);
        fflush(stdout);
        if (result == 100) {
            send_line(peer, "QUIT\n");
            return;
        }
        snprintf(line, sizeof line, "%" PRId64 "\n", result);
        if (!send_line(peer, line))
            return;
    }
}

static int listen_for_clients(uint16_t port)
{
    socket_tcp listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == SOCKET_INVALID) {
        fputs("Impossibile creare il socket di ascolto.\n", stderr);
        return EXIT_FAILURE;
    }

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    if (bind(listener, (struct sockaddr *)&address, sizeof address) != 0 ||
        listen(listener, SOMAXCONN) != 0) {
        fprintf(stderr, "Impossibile avviare l'ascolto sulla porta %u.\n", (unsigned)port);
        CLOSE_SOCKET(listener);
        return EXIT_FAILURE;
    }

    printf("Server in ascolto sulla porta %u.\n", (unsigned)port);
    fflush(stdout);

    for (;;) {
        struct sockaddr_in remote = {0};
        socket_length length = sizeof remote;
        socket_tcp peer = accept(listener, (struct sockaddr *)&remote, &length);
        if (peer == SOCKET_INVALID) {
            if (socket_call_interrupted())
                continue;
            fputs("Errore durante accept.\n", stderr);
            break;
        }

        char ip[INET_ADDRSTRLEN];
        const char *remote_ip = inet_ntop(AF_INET, &remote.sin_addr, ip, sizeof ip);
        printf("connesso con il client il cui nome IP: %s\n",
               remote_ip ? remote_ip : "sconosciuto");
        fflush(stdout);
        serve_client(peer);
        CLOSE_SOCKET(peer);
    }

    CLOSE_SOCKET(listener);
    return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
    int32_t port = 5050;
    if (argc > 2 || (argc == 2 &&
        (!parse_integers(argv[1], &port, 1) || port < 1 || port > 65535))) {
        fprintf(stderr, "Uso: %s [porta da 1 a 65535]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (!initialize_sockets()) {
        fputs("Impossibile inizializzare i socket.\n", stderr);
        return EXIT_FAILURE;
    }

    int status = listen_for_clients((uint16_t)port);
    cleanup_sockets();
    return status;
}
