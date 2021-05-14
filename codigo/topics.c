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

int **topic_subs;
int *topic_num_subs;

ustring *topic_package;
int *topic_package_len;

int *process_comusumed;

int client_topic;

void inicialize_topics() {
    topic_names = global_malloc(MAX_TOPICS * sizeof(string));

    topic_subs = global_malloc(MAX_TOPICS * sizeof(int *));
    topic_num_subs = global_malloc(MAX_TOPICS * sizeof(int));

    topic_package = global_malloc(MAX_TOPICS * sizeof(ustring *));
    topic_package_len = global_malloc(MAX_TOPICS * sizeof(int));

    process_comusumed = global_malloc(MAX_CLIENTS * sizeof(int));

    for (int i = 0; i < MAX_TOPICS; ++i) {
        topic_names[i] = global_malloc(MAX_TOPIC_NAME_LEN * sizeof(char));
        topic_names[i][0] = 0; /* empty topic */

        topic_subs[i] = global_malloc(MAX_CLIENTS * sizeof(int));

        topic_package[i] = global_malloc(MAX_MSG_LEN * sizeof(uchar));
        topic_package[i][0] = 0;

        topic_num_subs[i] = 0;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        process_comusumed[i] = 1;

        for (int j = 0; j < MAX_TOPICS; j++) topic_subs[j][i] = -1;
    }

    client_topic = -1;

    fprintf(stdout, "Iniciando topicos\n");
}

void free_topics() {
    for (int i = 0; i < MAX_TOPICS; ++i) {
        global_free(topic_names[i], MAX_TOPIC_NAME_LEN * sizeof(char));
        global_free(topic_subs[i], MAX_CLIENTS * sizeof(int));
        global_free(topic_package[i], MAX_MSG_LEN * sizeof(uchar));
    }

    global_free(topic_names, MAX_TOPICS * sizeof(string));
    global_free(topic_subs, MAX_TOPICS * sizeof(int *));
    global_free(topic_num_subs, MAX_TOPICS * sizeof(int));
    global_free(topic_package, MAX_TOPICS * sizeof(ustring *));
    global_free(topic_package_len, MAX_TOPICS * sizeof(int));
    global_free(process_comusumed, MAX_CLIENTS * sizeof(int));
}

/* returns the topic index in the topic_names array or -1 if it's not present in
 * it */
int find_topic(string topic_name) {
    for (int i = 0; i < MAX_TOPICS; ++i) {
        if (topic_names[i][0] == 0) continue;
        if (strcmp(topic_name, topic_names[i]) == 0) return i;
    }

    return -1;
}

int add_topic(string topic_name) {
    int i;
    /* Search for a please to add the topic */
    for (i = 0; i < MAX_TOPICS && topic_names[i][0] != 0; ++i)
        ;

    /* return -1 if topic_names is full */
    if (i == MAX_TOPICS) return -1;

    fprintf(stdout, "Adicionando um novo tópico\n");

    memcpy(topic_names[i], topic_name, strlen(topic_name));
    topic_names[i][strlen(topic_name)] = 0;
    return i;
}

void remove_topic(int topic_idx) {
    topic_names[topic_idx][0] = 0; // just mark it as an empty topic
}

int add_client_to_topic(string topic_name) {
    pid_t pid = getpid();
    int topic_idx = find_topic(topic_name);

    if (pid > MAX_CLIENTS) {
        fprintf(stderr, "HUGE ERROR: pid (%d) can't be greater than %d.\n", pid,
                MAX_CLIENTS);
        exit(1);
    }

    if (topic_idx == -1) {
        if ((topic_idx = add_topic(topic_name)) == -1)
            return -1; /* Maximum number of topics exceeded */
    }

    fprintf(stdout, "Client_topic = %d\n", topic_idx);
    client_topic = topic_idx;

    topic_subs[topic_idx][pid] = pid;
    topic_num_subs[topic_idx]++;
    return topic_idx;
}

void remove_client_from_topic() {
    if (client_topic == -1) return;

    int topic_idx = client_topic;

    fprintf(stdout, "Removendo cliente %d do tópico %s.\n", getpid(),
            topic_names[topic_idx]);

    client_topic = -1;

    topic_subs[topic_idx][getpid()] = -1;
    topic_num_subs[topic_idx]--;

    if (topic_num_subs[topic_idx] == 0) { remove_topic(topic_idx); }
}

int send_msg_to_topic(ustring msg, int msg_len, string topic_name) {
    int topic_idx = find_topic(topic_name);
    int pid;

    if (topic_idx == -1) return -1;

    memcpy(topic_package[topic_idx], msg, msg_len);
    topic_package_len[topic_idx] = msg_len;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        pid = topic_subs[topic_idx][i];
        if (pid == -1) continue;
        process_comusumed[pid] = 0;
    }

    return 0;
}

void print_topics() {
    fprintf(stdout, "Current topics:\n");

    for (int i = 0; i < MAX_TOPICS; ++i) {
        if (topic_names[i][0] == 0) continue;

        fprintf(stdout, "Topic[%d] : %s\n", i, topic_names[i]);
    }

    fprintf(stdout, "\n");
}

void print_clients_in_topics() {
    fprintf(stdout, "Current clients:\n");

    for (int i = 0; i < MAX_TOPICS; ++i) {
        if (topic_names[i][0] == 0) continue;

        fprintf(stdout, "Topic[%d] - %s:\n", i, topic_names[i]);

        for (int j = 0; j < MAX_CLIENTS; j++) {
            if (topic_subs[i][j] == -1) continue;

            fprintf(stdout, "\t%d\n", topic_subs[i][j]);
        }

        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\n");
}
