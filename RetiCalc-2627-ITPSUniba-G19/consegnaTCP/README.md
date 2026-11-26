# Esonero socket TCP — G19

**Sistema operativo del gruppo** — macOS  
**Linguaggio** — C11

- `client-TCP_G19` contiene il progetto Eclipse CDT del client.
- `serverTCP_G19` contiene il progetto Eclipse CDT del server.

## Compilazione con Eclipse CDT su macOS

1. Installa Eclipse IDE for C/C++ Developers e gli strumenti da riga di comando di Xcode (`xcode-select --install`).
2. In Eclipse, apri **File → Import → General → Existing Projects into Workspace** e seleziona `consegnaTCP` come directory radice.
3. Seleziona entrambi i progetti e conferma l'importazione. Usa **Project → Build Project** per ciascun progetto.

## Avvio e prova

Da due terminali aperti in `consegnaTCP`, avvia prima il server sulla porta 5050 (oppure scegli un'altra porta come argomento), poi il client.

```sh
./serverTCP_G19/Debug/server 5050
```

```sh
./client-TCP_G19/Debug/client
```

1. Sul client, inserisci `127.0.0.1` e poi `5050`. Il client stampa `connessione avvenuta` e il server registra l'IP del client. Per una prova su due macchine, inserisci l'indirizzo IPv4 della macchina che ospita il server.
2. Inserisci `10 20 5`. Il server stampa `la somma di 10+20-5=25`, il client stampa `25` e chiede altri tre numeri.
3. Inserisci `40 70 10`. Il server stampa `la somma di 40+70-10=100` e invia `QUIT`. Il client stampa `QUIT` e termina.
4. Avvia di nuovo il client. Il server continua ad accettare connessioni e gestisce una sessione alla volta. Per arrestarlo, premi **Ctrl+C** nel suo terminale.

## Compilazione da terminale

Da `consegnaTCP`, su macOS o Linux:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic serverTCP_G19/server.c -o serverTCP_G19/server
cc -std=c11 -Wall -Wextra -Wpedantic client-TCP_G19/client.c -o client-TCP_G19/client
```

Avvia `./serverTCP_G19/server 5050` e poi `./client-TCP_G19/client`. Su Windows con MinGW-w64, usa `gcc`, assegna l'estensione `.exe` agli eseguibili e aggiungi `-lws2_32` alla fine di entrambi i comandi.

## Protocollo

I messaggi sono righe di testo terminate da LF. Ogni richiesta contiene tre interi decimali con segno a 32 bit separati da spazi. Il server calcola `n1+n2-n3` in un intero a 64 bit e risponde con il risultato decimale oppure con `QUIT` se il risultato è 100. Una richiesta TCP malformata chiude la sessione corrente. Il client convalida l'input prima di inviarlo.
