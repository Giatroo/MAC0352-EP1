#include "topics.h"

#include <stdio.h>
#include <unistd.h>

string topic_names[MAX_TOPICS];
int topic_subscribers[MAX_TOPICS][MAX_SUBSCRIBERS];
int topic_cur_subscribers[MAX_TOPICS];

void inicialize_topics() {
    /* inicialize every topic as NULL */
    for (int i = 0; i < MAX_TOPICS; ++i) {
        topic_names[i] = NULL;
        topic_cur_subscribers[i] = 0;
    }
}

/* returns the topic index in the topic_names array or -1 if it's not present in
 * it */
int find_topic(string topic_name) {
    for (int i = 0; i < MAX_TOPICS && topic_names[i] != NULL; ++i)
        if (strcmp(topic_name, topic_names[i]) == 0) return i;

    return -1;
}

int add_topic(string topic_name) {
    int i;
    for (i = 0; i < MAX_TOPICS && topic_names[i] != NULL; ++i)
        ;

    if (i == MAX_TOPICS) return -1; /* return -1 if topic_names is full */

    topic_names[i] = malloc((strlen(topic_name)) * sizeof(char));
    strcpy(topic_names[i], topic_name);
    return i;
}

int add_client_to_topic(string topic_name) {
    int topic_idx = find_topic(topic_name);
    if (topic_idx == -1)
        if ((topic_idx = add_topic(topic_name)) == -1)
            return -1; /* Maximum number of topics exceeded */

    topic_subscribers[topic_idx][topic_cur_subscribers[topic_idx]++] = getpid();
    return 0;
}

void print_topics() {
    fprintf(stdout, "Current topics:\n");
    for (int i = 0; i < MAX_TOPICS && topic_names[i] != NULL; ++i)
        fprintf(stdout, "Topic[%d] : %s\n", i, topic_names[i]);
    fprintf(stdout, "\n");
}

void print_clients_in_topics() {
    fprintf(stdout, "Current clients:\n");
    for (int i = 0; i < MAX_TOPICS && topic_names[i] != NULL; ++i) {
        fprintf(stdout, "Topic %s: ", topic_names[i]);
        for (int j = 0; j < topic_cur_subscribers[i]; j++) {
            fprintf(stdout, "%d ", topic_subscribers[i][j]);
        }
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\n");
}

