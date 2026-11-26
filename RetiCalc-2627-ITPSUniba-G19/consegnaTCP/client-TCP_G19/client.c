#include "socket_tcp.h"


static int read_input(const char *prompt, char input[LINE_CAPACITY])
{
    fputs(prompt, stdout);
    fflush(stdout);
    if (!fgets(input, LINE_CAPACITY, stdin))
        return 0;

    char *end = strpbrk(input, "\r\n");
    if (end) {
        *end = '\0';
        return 1;
    }
    int byte = getchar();
    if (byte == '\n' || (byte == EOF && feof(stdin)))
        return 1;

    while (byte != EOF && (byte = getchar()) != '\n' && byte != EOF) {
    }
    return -1;
}

static int run_client(socket_tcp sock)
{
    char line[LINE_CAPACITY];
    if (!receive_line(sock, line) || strcmp(line, "connessione avvenuta")) {
        fputs("Saluto del server assente o non valido.\n", stderr);
        return EXIT_FAILURE;
    }
    puts(line);

    for (;;) {
        int read_status = read_input("Inserisci n1 n2 n3: ", line);
        if (!read_status)
            return EXIT_SUCCESS;

        int32_t values[3];
        if (read_status < 0 || !parse_integers(line, values, 3)) {
            fputs("Inserisci esattamente tre interi a 32 bit separati da spazi.\n", stderr);
            continue;
        }

        snprintf(line, sizeof line, "%" PRId32 " %" PRId32 " %" PRId32 "\n",
                 values[0], values[1], values[2]);
        if (!send_line(sock, line) || !receive_line(sock, line)) {
            fputs("Scambio con il server interrotto.\n", stderr);
            return EXIT_FAILURE;
        }
        puts(line);
        if (strcmp(line, "QUIT") == 0)
            return EXIT_SUCCESS;
    }
}

int main(void)
{
    if (!initialize_sockets()) {
        fputs("Impossibile inizializzare i socket.\n", stderr);
        return EXIT_FAILURE;
    }

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    char line[LINE_CAPACITY];
    int32_t port;
    int result = EXIT_FAILURE;

    if (read_input("IP del server (IPv4): ", line) != 1 ||
        inet_pton(AF_INET, line, &address.sin_addr) != 1) {
        fputs("Indirizzo IPv4 non valido.\n", stderr);
    } else if (read_input("Porta del server: ", line) != 1 ||
               !parse_integers(line, &port, 1) || port < 1 || port > 65535) {
        fputs("Porta non valida.\n", stderr);
    } else {
        address.sin_port = htons((uint16_t)port);
        socket_tcp sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == SOCKET_INVALID) {
            fputs("Impossibile creare il socket.\n", stderr);
        } else {
            if (connect(sock, (struct sockaddr *)&address, sizeof address) == 0)
                result = run_client(sock);
            else
                fputs("Connessione al server non riuscita.\n", stderr);
            CLOSE_SOCKET(sock);
        }
    }

    cleanup_sockets();
    return result;
}
