/*
 * Copyright (c) 2012 Jeremy Pepper
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of message_queue nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "message_queue.h"
#include "common_types.h"
#include <string.h>
// #include <inttypes.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <stdio.h>
// #include <fcntl.h>
// #include <errno.h>

mymutex_ptr g_mtx_queue = NULL;
mymutex_ptr g_mtx_rmlist = NULL;
#define MQ_MAX_REMOVE_LIST	20
my_list_node* g_remove_list[MQ_MAX_REMOVE_LIST] = { NULL };

static inline int max(int x, int y) {
	return x > y ? x : y;
}

int message_queue_rmlist_add(my_list_node* node);
int message_queue_rmlist_free(void* data);

int message_queue_init(struct message_queue *queue, int message_size, int max_depth) {
	if (queue == NULL)
		return -1;
	if (g_mtx_queue == NULL)
	{
		g_mtx_queue = my_mutex_init();
		g_mtx_rmlist = my_mutex_init();
		memset(g_remove_list, 0, sizeof(g_remove_list));
	}
	memset(queue, 0, sizeof(*queue));
	queue->m_msg_size = message_size;
	return 0;
}

void *message_queue_message_alloc(struct message_queue *queue) {
	void* msg = my_malloc(queue->m_msg_size);
	if (msg == NULL)
	{
		my_printf("[%s] failed to malloc message, %d.\n", __func__, queue->m_msg_size);
		return NULL;
	}
	return msg;
}

void *message_queue_message_alloc_blocking(struct message_queue *queue) {
	return NULL;
}

void message_queue_message_free(struct message_queue *queue, void *message) {
	message_queue_rmlist_free(message);
}

void message_queue_write(struct message_queue *queue, void *message) {
	my_list_node* node = my_malloc(sizeof(my_list_node));
	if (node == NULL)
	{
		my_printf("[%s] failed to malloc node, %ld.\n", __func__, sizeof(my_list_node));
		return;
	}
	memset(node, 0, sizeof(*node));
	node->data = message;
	my_mutex_lock(g_mtx_queue);
	if (queue->m_tail == NULL)
	{
		queue->m_tail= node;
		queue->m_head = node;
	}
	else
	{
		queue->m_tail->next = node;
		node->prev = queue->m_tail;
		queue->m_tail = node;
	}
	my_mutex_unlock(g_mtx_queue);
}

void *message_queue_tryread(struct message_queue *queue) {
	my_list_node* cur = NULL;
	my_mutex_lock(g_mtx_queue);
	cur = queue->m_tail;
	if (cur != NULL)
	{
		queue->m_tail = cur->prev;
		if (queue->m_tail != NULL)
			queue->m_tail->next = NULL;
		else
		{
			queue->m_head = NULL;
			queue->m_tail = NULL;
		}
	}
	my_mutex_unlock(g_mtx_queue);
	if (cur != NULL)
	{
		message_queue_rmlist_add(cur);
		return cur->data;
	}
	return NULL;
}

void *message_queue_read(struct message_queue *queue) {
	my_list_node* cur = NULL;
	do {
		my_mutex_lock(g_mtx_queue);
		cur = queue->m_tail;
		if (cur != NULL)
		{
			queue->m_tail = cur->prev;
			if (queue->m_tail != NULL)
				queue->m_tail->next = NULL;
			else
			{
				queue->m_head = NULL;
				queue->m_tail = NULL;
			}
		}
		my_mutex_unlock(g_mtx_queue);
		my_usleep(1000);
	} while(cur == NULL);
	message_queue_rmlist_add(cur);
	return cur->data;
}

void message_queue_destroy(struct message_queue *queue) {
	my_mutex_lock(g_mtx_queue);
	my_list_node* cur = queue->m_head;
	my_list_node* tmp = NULL;
	while(cur != NULL)
	{
		tmp = cur;
		cur = cur->next;
		if (tmp->data != NULL)
			my_free(tmp->data);
		my_free(tmp);
	}
	memset(queue, 0, sizeof(*queue));
	my_mutex_unlock(g_mtx_queue);
}

int message_queue_rmlist_add(my_list_node* node)
{
	my_mutex_lock(g_mtx_rmlist);
	for(int i = 0; i < MQ_MAX_REMOVE_LIST; i++)
	{
		if (g_remove_list[i] == NULL)
		{
			g_remove_list[i] = node;
			my_mutex_unlock(g_mtx_rmlist);
			return 0;
		}
	}
	my_mutex_unlock(g_mtx_rmlist);
	my_printf("[%s] exceed buffer.\n", __func__);
	return 1;
}

int message_queue_rmlist_free(void* data)
{
	if (data == NULL)
		return 2;
	my_mutex_lock(g_mtx_rmlist);
	for(int i = 0; i < MQ_MAX_REMOVE_LIST; i++)
	{
		if (g_remove_list[i] != NULL && g_remove_list[i]->data == data)
		{
			my_free(data);
			my_free(g_remove_list[i]);
			g_remove_list[i] = NULL;
			my_mutex_unlock(g_mtx_rmlist);
			return 0;
		}
	}
	my_mutex_unlock(g_mtx_rmlist);
	my_printf("[%s] not found.\n", __func__);
	return 1;
}