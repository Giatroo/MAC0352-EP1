#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "headers.h"
#include "topics.h"
#include "util.h"

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

int main(int argc, char **argv) {
    /* Os sockets. Um que será o socket que vai escutar pelas conexões
     * e o outro que vai ser o socket específico de cada conexão */
    int listenfd, connfd;
    /* Informações sobre o socket (endereço e porta) ficam nesta struct */
    struct sockaddr_in servaddr;
    /* Retorno da função fork para saber quem é o processo filho e
     * quem é o processo pai */
    pid_t childpid;
    /* Armazena linhas recebidas do cliente */
    unsigned char recvline[MAXLINE + 1];
    /* Armazena o tamanho da string lida do cliente */
    ssize_t n;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <Porta>\n", argv[0]);
        fprintf(stderr, "Vai rodar um servidor MQTT na porta <Porta> TCP\n");
        exit(1);
    }

    /* Criação de um socket. É como se fosse um descritor de arquivo.
     * É possível fazer operações como read, write e close. Neste caso o
     * socket criado é um socket IPv4 (por causa do AF_INET), que vai
     * usar TCP (por causa do SOCK_STREAM), já que o MQTT funciona sobre
     * TCP, e será usado para uma aplicação convencional sobre a Internet
     * (por causa do número 0) */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket :(\n");
        exit(2);
    }

    /* Agora é necessário informar os endereços associados a este
     * socket. É necessário informar o endereço / interface e a porta,
     * pois mais adiante o socket ficará esperando conexões nesta porta
     * e neste(s) endereços. Para isso é necessário preencher a struct
     * servaddr. É necessário colocar lá o tipo de socket (No nosso
     * caso AF_INET porque é IPv4), em qual endereço / interface serão
     * esperadas conexões (Neste caso em qualquer uma -- INADDR_ANY) e
     * qual a porta. Neste caso será a porta que foi passada como
     * argumento no shell (atoi(argv[1]))
     */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }

    /* Como este código é o código de um servidor, o socket será um
     * socket passivo. Para isto é necessário chamar a função listen
     * que define que este é um socket de servidor que ficará esperando
     * por conexões nos endereços definidos na função bind. */
    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    printf("[Servidor no ar. Aguardando conexões na porta %s]\n", argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");

    inicialize_topics();

    /* O servidor no final das contas é um loop infinito de espera por
     * conexões e processamento de cada uma individualmente */
    for (;;) {
        /* O socket inicial que foi criado é o socket que vai aguardar
         * pela conexão na porta especificada. Mas pode ser que existam
         * diversos clientes conectando no servidor. Por isso deve-se
         * utilizar a função accept. Esta função vai retirar uma conexão
         * da fila de conexões que foram aceitas no socket listenfd e
         * vai criar um socket específico para esta conexão. O descritor
         * deste novo socket é o retorno da função accept. */
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1) {
            perror("accept :(\n");
            exit(5);
        }

        /* Agora o servidor precisa tratar este cliente de forma
         * separada. Para isto é criado um processo filho usando a
         * função fork. O processo vai ser uma cópia deste. Depois da
         * função fork, os dois processos (pai e filho) estarão no mesmo
         * ponto do código, mas cada um terá um PID diferente. Assim é
         * possível diferenciar o que cada processo terá que fazer. O
         * filho tem que processar a requisição do cliente. O pai tem
         * que voltar no loop para continuar aceitando novas conexões.
         * Se o retorno da função fork for zero, é porque está no
         * processo filho. */
        if ((childpid = fork()) == 0) {
            /**** PROCESSO FILHO ****/
            fprintf(stdout, "[Uma conexão aberta (PID = %d)]\n", getpid());
            /* Já que está no processo filho, não precisa mais do socket
             * listenfd. Só o processo pai precisa deste socket. */
            close(listenfd);

            /* Agora pode ler do socket e escrever no socket. Isto tem
             * que ser feito em sincronia com o cliente. Não faz sentido
             * ler sem ter o que ler. Ou seja, neste caso está sendo
             * considerado que o cliente vai enviar algo para o servidor.
             * O servidor vai processar o que tiver sido enviado e vai
             * enviar uma resposta para o cliente (Que precisará estar
             * esperando por esta resposta)
             */

            /* ========================================================= */
            /* ========================================================= */
            /*                         EP1 INÍCIO                        */
            /* ========================================================= */
            /* ========================================================= */
            /* TODO: É esta parte do código que terá que ser modificada
             * para que este servidor consiga interpretar comandos MQTT  */
            while ((n = read(connfd, recvline, MAXLINE)) > 0) {
                recvline[n] = 0;

                fprintf(stdout,
                        "[Cliente conectado no processo filho %d enviou:] \n",
                        getpid());

                print_in_hex(recvline, n);

                FixedHeader *fixed_header;
                fixed_header = interpret_fixed_header(recvline);

                switch (fixed_header->type) {
                    case CONNECT_PACKAGE:
                        fprintf(stdout, "Connect case\n");

                        int encoded_len;
                        byte *encoded_str = encode_connack(&encoded_len);

                        write(connfd, encoded_str, encoded_len);
                        break;
                    case CONNACK_PACKAGE:
                        fprintf(stdout, "Connack case\n");
                        fprintf(stderr, "Server shouldn't receive a connack"
                                        "package\n");
                        exit(1);
                        break;
                    case PUBLISH_PACKAGE:
                        fprintf(stdout, "Publish case\n");

                        /* Reading the publish package */
                        PublishHeader *pub_header;
                        pub_header = interpret_publish_header(
                            recvline, fixed_header->remaning_length, n);

                        /* Sending the message to every subscriptor of the
                         * topic */
                        int return_code = send_msg_to_topic(
                            pub_header->recvline, pub_header->recvline_len,
                            pub_header->topic_value);

                        if (return_code == -1)
                            fprintf(stderr, "Tópico não encontrado.\n");

                        /* Deallocating the publish struct */
                        free(pub_header->msg);
                        free(pub_header->recvline);
                        free(pub_header->topic_value);
                        free(pub_header);
                        break;
                    case SUBSCRIBE_PACKAGE:
                        fprintf(stdout, "Subscribe case\n");

                        /* Reading the subscribe package */
                        SubscribeHeader *sub_header;
                        sub_header = interpret_subscribe_header(
                            recvline, fixed_header->remaning_length);

                        /* Adding the client to a topic */
                        int topic_idx;
                        topic_idx =
                            add_client_to_topic(sub_header->topic_value);
                        print_clients_in_topics();

                        if (topic_idx == -1) {
                            fprintf(stdout, "No space for new topic\n");
                            /* Sending the suback package */
                            byte suback_package[6] = { 0x90, 0x04, 0x00,
                                                       0x01, 0x00, 0x97 };
                            write(connfd, suback_package, 6);
                        } else {
                            /* Sending the suback package */
                            byte suback_package[6] = { 0x90, 0x04, 0x00,
                                                       0x01, 0x00, 0x00 };
                            write(connfd, suback_package, 6);

                            /* Creating a new process to check if any message
                             * was sent to the topic */
                            pid_t father_pid = getpid();
                            if ((childpid = fork()) == 0) {
                                while (1) {
                                    if (process_comusumed[father_pid] == 0) {
                                        write(connfd, topic_package[topic_idx],
                                              topic_package_len[topic_idx]);
                                        process_comusumed[father_pid] = 1;
                                    }
                                    usleep(100);
                                }
                            }
                        }

                        /* Deallocating the subscribe struct */
                        free(sub_header->topic_value);
                        free(sub_header);
                        break;
                    case PINGREQ_PACKAGE:
                        fprintf(stdout, "Pingreq case\n");
                        /* Hard coding the PINGRESP */
                        byte response[2] = { 0xd0, 0x00 };
                        write(connfd, response, 2);
                        fprintf(stdout, "Sending pingresp...\n");
                        break;
                    case DISCONNECT_PACKAGE:
                        fprintf(stdout, "Disconnect case\n");
                        remove_client_from_topic();
                        break;
                }

                free(fixed_header);
            }
            /* ========================================================= */
            /* ========================================================= */
            /*                         EP1 FIM                           */
            /* ========================================================= */
            /* ========================================================= */

            /* Após ter feito toda a troca de informação com o cliente,
             * pode finalizar o processo filho */
            printf("[Uma conexão fechada]\n");
            exit(0);
        } else
            /**** PROCESSO PAI ****/
            /* Se for o pai, a única coisa a ser feita é fechar o socket
             * connfd (ele é o socket do cliente específico que será tratado
             * pelo processo filho) */
            close(connfd);
    }

    free_topics();
    exit(0);
}
