#include "topics.h"

#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

string *topic_names;

int **topic_subscribers;
int *topic_cur_subscribers;

pthread_mutex_t *process_mutex;

ustring *topic_msg;
int *topic_len;

int *process_comusumed;

void inicialize_topics() {
    topic_names =
        mmap(NULL, MAX_TOPICS * sizeof(string), PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    topic_cur_subscribers =
        mmap(NULL, MAX_TOPICS * sizeof(int), PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    topic_subscribers =
        mmap(NULL, MAX_TOPICS * sizeof(int *), PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    topic_msg = mmap(NULL, MAX_TOPICS * sizeof(string), PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    topic_len = mmap(NULL, MAX_TOPICS * sizeof(int), PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    process_mutex =
        mmap(NULL, MAX_CLIENTS * sizeof(pthread_mutex_t),
             PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    process_comusumed =
        mmap(NULL, MAX_CLIENTS * sizeof(int), PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    for (int i = 0; i < MAX_TOPICS; ++i) {
        topic_names[i] =
            mmap(NULL, MAX_TOPIC_NAME_LEN * sizeof(char),
                 PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
        topic_names[i][0] = 0;

        topic_subscribers[i] =
            mmap(NULL, MAX_TOPIC_NAME_LEN * sizeof(int), PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, 0, 0);

        topic_msg[i] =
            mmap(NULL, MAX_MSG_LEN * sizeof(char), PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, 0, 0);
        topic_msg[i][0] = 0;

        topic_cur_subscribers[i] = 0;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_mutex_init(&process_mutex[i], NULL);
        pthread_mutex_lock(&process_mutex[i]);
        process_comusumed[i] = 1;
    }
    fprintf(stdout, "Iniciando topicos\n");
}

void free_topics() {
    int err;
    for (int i = 0; i < MAX_TOPICS; ++i) {
        err = munmap(topic_names[i], MAX_TOPIC_NAME_LEN * sizeof(char));

        if (err != 0)
            fprintf(
                stderr,
                "Um erro ocorreu ao tentar desalocar o mmap topic_names[%d].\n",
                i);

        err = munmap(topic_subscribers[i], MAX_TOPIC_NAME_LEN * sizeof(int));

        if (err != 0)
            fprintf(stderr,
                    "Um erro ocorreu ao tentar desalocar o mmap "
                    "topic_subscribers[%d].\n",
                    i);
    }

    err = munmap(topic_names, MAX_TOPICS * sizeof(string));
    if (err != 0)
        fprintf(stderr,
                "Um erro ocorreu, ao tentar desalocar o mmap topic_names.\n");
}

/* returns the topic index in the topic_names array or -1 if it's not present in
 * it */
int find_topic(string topic_name) {
    for (int i = 0; i < MAX_TOPICS && topic_names[i][0] != 0; ++i)
        if (strcmp(topic_name, topic_names[i]) == 0) return i;

    return -1;
}

int add_topic(string topic_name) {
    int i;
    for (i = 0; i < MAX_TOPICS && topic_names[i][0] != 0; ++i)
        ;

    if (i == MAX_TOPICS) return -1; /* return -1 if topic_names is full */

    fprintf(stdout, "Adicionando um novo tópico\n");

    memcpy(topic_names[i], topic_name, strlen(topic_name));
    topic_names[i][strlen(topic_name)] = 0;
    return i;
}

int add_client_to_topic(string topic_name) {
    int topic_idx = find_topic(topic_name);
    if (topic_idx == -1) {
        if ((topic_idx = add_topic(topic_name)) == -1)
            return -1; /* Maximum number of topics exceeded */
    }

    topic_subscribers[topic_idx][topic_cur_subscribers[topic_idx]++] = getpid();
    return topic_idx;
}

int send_msg_to_topic(ustring msg, int msg_len, string topic_name) {
    int topic_idx = find_topic(topic_name);
    int pid;

    if (topic_idx == -1) return -1;

    memcpy(topic_msg[topic_idx], msg, msg_len);
    topic_len[topic_idx] = msg_len;

    for (int i = 0; i < topic_cur_subscribers[topic_idx]; i++) {
        pid = topic_subscribers[topic_idx][i];
        process_comusumed[pid] = 0;
    }

    return 0;
}

void send_msg_to_client(ustring msg, int msg_len, int pid) {
    pthread_mutex_unlock(&process_mutex[pid]);
    pthread_mutex_lock(&process_mutex[pid]);
}

void print_topics() {
    fprintf(stdout, "Current topics:\n");
    for (int i = 0; i < MAX_TOPICS && topic_names[i][0] != 0; ++i) {
        fprintf(stdout, "Topic[%d] :", i);
        fflush(stdout);
        fprintf(stdout, " '%s'\n", topic_names[i]);
    }
    fprintf(stdout, "\n");
}

void print_clients_in_topics() {
    fprintf(stdout, "Current clients:\n");
    for (int i = 0; i < MAX_TOPICS && topic_names[i][0] != 0; ++i) {
        fprintf(stdout, "Topic '%s':\n", topic_names[i]);
        for (int j = 0; j < topic_cur_subscribers[i]; j++) {
            fprintf(stdout, "\t%d\n", topic_subscribers[i][j]);
        }
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\n");
}
