/* Por Prof. Daniel Batista <batista@ime.usp.br>
 * Em 4/4/2021
 *
 * Um código simples de um servidor de eco a ser usado como base para
 * o EP1. Ele recebe uma linha de um cliente e devolve a mesma linha.
 * Teste ele assim depois de compilar:
 *
 * ./ep1-servidor-exemplo 8000
 *
 * Com este comando o servidor ficará escutando por conexões na porta
 * 8000 TCP (Se você quiser fazer o servidor escutar em uma porta
 * menor que 1024 você precisará ser root ou ter as permissões
 * necessárias para rodar o código com 'sudo').
 *
 * Depois conecte no servidor via telnet. Rode em outro terminal:
 *
 * telnet 127.0.0.1 8000
 *
 * Escreva sequências de caracteres seguidas de ENTER. Você verá que o
 * telnet exibe a mesma linha em seguida. Esta repetição da linha é
 * enviada pelo servidor. O servidor também exibe no terminal onde ele
 * estiver rodando as linhas enviadas pelos clientes.
 *
 * Obs.: Você pode conectar no servidor remotamente também. Basta
 * saber o endereço IP remoto da máquina onde o servidor está rodando
 * e não pode haver nenhum firewall no meio do caminho bloqueando
 * conexões na porta escolhida.
 */

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

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

enum PackageType {
    CONNECT_PACKAGE = 0x1,
    CONNACK_PACKAGE = 0x2,
    PUBLISH_PACKAGE = 0x3,
    PUBACK_PACKAGE = 0x4,
    PUBREC_PACKAGE = 0x5,
    PUBREL_PACKAGE = 0x6,
    PUBCOMP_PACKAGE = 0x7,
    SUBSCRIBE_PACKAGE = 0x8,
    SUBACK_PACKAGE = 0x9,
    UNSUBSCRIBE_PACKAGE = 0xa,
    UNSUBACK_PACKAGE = 0xb,
    PINGREQ_PACKAGE = 0xc,
    PINGRESP_PACKAGE = 0xd,
    DISCONNECT_PACKAGE = 0xe
};

typedef struct {
    u_int8_t type_flags;
    u_int8_t remaning_length;
} FixedHeader;

typedef struct {
    u_int16_t protocol_name_len; // 0x00 0x04
    unsigned char name[4]; // 'MQTT'
    u_int8_t version; // 0x05
    u_int8_t flags; // 0x02 (ignored)
    u_int16_t keep_alive;
    u_int8_t property_length;
    u_int8_t receive_maximum_id;
    u_int16_t receive_maximum_value;
} ConnectVarHeader;

typedef struct {
    u_int8_t ack_flags;
    u_int8_t reason_code;
    u_int8_t property_length;
    u_int8_t topic_alias_maximum_id;
    u_int16_t topic_alias_maximum_value;
    u_int8_t assigned_client_id;
    u_int16_t client_id_len;
    u_int8_t *client_id;
    u_int8_t receive_maximum_id;
    u_int16_t receive_maximum_value;
} ConnackVarHeader;

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
    char recvline[MAXLINE + 1];
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
                for (int i = 0; i < n; ++i) fprintf(stdout, "%02x ", recvline[i]);
                fprintf(stdout, "'\n");

                FixedHeader *package_header = malloc(sizeof(FixedHeader));
                memcpy(package_header, recvline, sizeof(*package_header));


                fprintf(stdout, "\nType_flags : %02x\nRemaning length: %02x\n",
                        package_header->type_flags, package_header->remaning_length);
                u_int8_t type = package_header->type_flags >> 4;
                u_int8_t flags = package_header->type_flags & 0x0F;
                fprintf(stdout, "type: %01x flags: %01x\n", type, flags);



                switch (type) {
                    case CONNECT_PACKAGE:
                        fprintf(stdout, "Connect case\n");

                        ConnectVarHeader *con_header = malloc(sizeof(ConnectVarHeader));
                        memcpy(con_header, &recvline[2], sizeof(*con_header));
                        con_header->protocol_name_len = ntohs(con_header->protocol_name_len);
                        con_header->keep_alive = ntohs(con_header->keep_alive);
                        con_header->receive_maximum_value = ntohs(con_header->receive_maximum_value);

                        FixedHeader *fixed_header = malloc(sizeof(FixedHeader));
                        fixed_header->type_flags = 0x02;
                        fixed_header->remaning_length = 2;

                        ConnackVarHeader *connack_header = malloc(sizeof(ConnackVarHeader));
                        connack_header->ack_flags = 0x00;
                        connack_header->reason_code = 0x00;
                        connack_header->property_length = 2;
                        connack_header->topic_alias_maximum_id = 0x22;
                        connack_header->topic_alias_maximum_value = 0x000a;
                        connack_header->assigned_client_id = 0x12;
                        connack_header->client_id_len = 2;
                        /* connack_header->client_id = */

                        fprintf(stdout, "pid: %d\n", getpid());


                        /* char conn_ack[4] = { 0x20, 0x02, 0x00, 0x00 }; */
                        /* write(connfd, conn_ack, 4); */


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

                        memset(recvline, 0, MAXLINE);
                }
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
