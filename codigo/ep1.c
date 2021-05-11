#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "headers.h"
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
        fprintf(stderr, "Vai rodar um servidor de echo na porta <Porta> TCP\n");
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
            printf("[Uma conexão aberta]\n");
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

                printf("[Cliente conectado no processo filho %d enviou:] \n'",
                       getpid());
                fflush(stdout);
                for (int i = 0; i < n; ++i)
                    fprintf(stdout, "%02x ", recvline[i]);
                fprintf(stdout, "'\n");

                int index = 0;
                FixedHeader *fixed_header;
                fixed_header = interpret_fixed_header(recvline, &index);

                switch (fixed_header->type) {
                    case CONNECT_PACKAGE:
                        fprintf(stdout, "Connect case\n");

                        ConnackVarHeader *connack_header =
                            malloc(sizeof(ConnackVarHeader));
                        connack_header->ack_flags = 0x00;
                        connack_header->reason_code = 0x00;
                        connack_header->topic_alias_maximum_value = 10;
                        sprintf(connack_header->client_id, "%d", getpid());
                        connack_header->client_id_len = strlen(connack_header->client_id);

                        int encoded_len;
                        byte *encoded_str = encode_connack(connack_header, &encoded_len);

                        write(connfd, encoded_str, encoded_len);

                        break;
                    case CONNACK_PACKAGE:
                        fprintf(stdout, "Connack case\n");
                        break;
                    case PUBLISH_PACKAGE:
                        fprintf(stdout, "Publish case\n");
                        break;
                    case PUBACK_PACKAGE:
                        fprintf(stdout, "Pubback case\n");
                        break;
                    case PUBREC_PACKAGE:
                        fprintf(stdout, "Pubrec case\n");
                        break;
                    case PUBREL_PACKAGE:
                        fprintf(stdout, "Pubrel case\n");
                        break;
                    case PUBCOMP_PACKAGE:
                        fprintf(stdout, "Pubcomp case\n");
                        break;
                    case SUBSCRIBE_PACKAGE:
                        fprintf(stdout, "Subscribe case\n");

                        fprintf(stdout, "remaning_length: %lu\n", fixed_header->remaning_length);

                        SubscribeHeader *sub_header;
                        sub_header = interpret_subscribe_header(recvline, &index, fixed_header->remaning_length);

                        byte suback_package[6];
                        suback_package[0] = 0x90;
                        suback_package[1] = 0x04;
                        suback_package[2] = 0x00;
                        suback_package[3] = 0x01;
                        suback_package[4] = 0x00;
                        suback_package[5] = 0x00;

                        write(connfd, suback_package, 6);

                        break;
                    case SUBACK_PACKAGE:
                        fprintf(stdout, "Suback case\n");
                        break;
                    case UNSUBSCRIBE_PACKAGE:
                        fprintf(stdout, "Unsubscribe case\n");
                        break;
                    case UNSUBACK_PACKAGE:
                        fprintf(stdout, "Unsuback case\n");
                        break;
                    case PINGREQ_PACKAGE:
                        fprintf(stdout, "Pingreq case\n");
                        break;
                    case PINGRESP_PACKAGE:
                        fprintf(stdout, "Pingresp case\n");
                        break;
                    case DISCONNECT_PACKAGE:
                        fprintf(stdout, "Disconnect case\n");
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
    exit(0);
}
