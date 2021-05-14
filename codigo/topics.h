#ifndef TOPICS_H
#define TOPICS_H

#define MAX_TOPICS 8
#define MAX_CLIENTS 300000
#define MAX_TOPIC_NAME_LEN 128
#define TOPIC_DIR "topics"
#define MAX_MSG_LEN 129

#include "util.h"

extern string *topic_names;

extern int **topic_subs;
extern int *topic_num_subs;

extern ustring *topic_package;
extern int *topic_package_len;

extern int *process_comusumed;

extern int client_topic;

void inicialize_topics();
void free_topics();

int find_topic(string topic_name);

int add_topic(string topic_name);
void remove_topic(int topic_idx);
int add_client_to_topic(string topic_name);
void remove_client_from_topic();

int send_msg_to_topic(ustring msg, int msg_len, string topic_name);

void print_topics();
void print_clients_in_topics();

#endif /* ifndef TOPICS_H */
