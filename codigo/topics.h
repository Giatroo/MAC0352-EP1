#ifndef TOPICS_H
#define TOPICS_H

#define MAX_TOPICS 8
#define MAX_CLIENTS 300000
#define MAX_TOPIC_NAME_LEN 128
#define TOPIC_DIR "topics"
#define MAX_MSG_LEN 129

#include "util.h"

// extern string topic_names[MAX_TOPICS];
// extern int topic_subscribers[MAX_TOPICS][MAX_SUBSCRIBERS];
// extern int topic_cur_subscribers[MAX_TOPICS];
extern string *topic_names;
extern int **topic_subscribers;
extern int *topic_cur_subscribers;

extern pthread_mutex_t *process_mutex;

extern ustring *topic_msg;
extern int *topic_len;

extern int *process_comusumed;

void inicialize_topics();
void free_topics();
int find_topic(string topic_name);
int add_topic(string topic_name);
int add_client_to_topic(string topic_name);

int send_msg_to_topic(ustring msg, int msg_len, string topic_name);
void send_msg_to_client(ustring msg, int msg_len, int pid);

void print_topics();
void print_clients_in_topics();

void publish_msg(string topic_name, string msg);
string get_msg(string topic_name);

#endif /* ifndef TOPICS_H */
