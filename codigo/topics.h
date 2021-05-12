#ifndef TOPICS_H
#define TOPICS_H

#define MAX_TOPICS 8
#define MAX_SUBSCRIBERS 3000

#include "util.h"

extern string topic_names[MAX_TOPICS];
extern int topic_subscribers[MAX_TOPICS][MAX_SUBSCRIBERS];
extern int topic_cur_subscribers[MAX_TOPICS];


void inicialize_topics();
int find_topic(string topic_name);
int add_topic(string topic_name);
int add_client_to_topic(string topic_name);

void print_topics();
void print_clients_in_topics();

void update_topics_to_file();

#endif /* ifndef TOPICS_H */
