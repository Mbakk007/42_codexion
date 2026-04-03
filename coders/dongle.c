/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 20:45:01 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/03 10:38:49 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	dongles_destroy(t_sim *sim)
{
	int	i;

	if (!sim->dongles)
		return ;
	i = 0;
	while (i < sim->params.n_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].mtx);
		i++;
	}
	free(sim->dongles);
	sim->dongles = NULL;
}

int	dongles_take_two(t_coder *c)
{
	int				left, right;
	int				win;
	long			min_avail, now, a_avail, b_avail, now_ms_value;
	struct timespec	timestamp;
	int				ret;
	int				a, b;

	left = c->id - 1;
	right = c->id % c->sim->params.n_coders;

	pthread_mutex_lock(&c->sim->sched_mtx);
	if (!queue_push(c->sim, c->id))
	{
		pthread_mutex_unlock(&c->sim->sched_mtx);
		return (0);
	}
	while (!sim_should_stop(c->sim))
	{
		win = queue_pick_winner(c->sim);
		if (win >= 0 && c->sim->queue->requests[win].coder_id == c->id)
		{
			if (left == right)
			{
				pthread_mutex_lock(&c->sim->dongles[left].mtx);
				now_ms_value = now_ms();
				if (now_ms_value < c->sim->dongles[left].available_at_ms)
				{
					min_avail = c->sim->dongles[left].available_at_ms;
					pthread_mutex_unlock(&c->sim->dongles[left].mtx);
				}
				else
				{
					min_avail = now_ms_value;
					queue_remove(c->sim, c->id);
					pthread_mutex_unlock(&c->sim->sched_mtx);
					log_event(c->sim, c->id, "has taken a dongle");
					pthread_mutex_unlock(&c->sim->dongles[left].mtx);
					return (1);
				}
			}
			else
			{
				/* Always lock in index order to avoid deadlocks */
				if (left < right)
				{
					a = left;
					b = right;
				}
				else
				{
					a = right;
					b = left;
				}
				pthread_mutex_lock(&c->sim->dongles[a].mtx);
				pthread_mutex_lock(&c->sim->dongles[b].mtx);
				now_ms_value = now_ms();
				a_avail = c->sim->dongles[a].available_at_ms;
				b_avail = c->sim->dongles[b].available_at_ms;
				if (now_ms_value < a_avail || now_ms_value < b_avail)
				{
					if (a_avail < b_avail)
						min_avail = a_avail;
					else
						min_avail = b_avail;
					pthread_mutex_unlock(&c->sim->dongles[b].mtx);
					pthread_mutex_unlock(&c->sim->dongles[a].mtx);
				}
				else
				{
					min_avail = now_ms_value;
					queue_remove(c->sim, c->id);
					pthread_mutex_unlock(&c->sim->sched_mtx);
					log_event(c->sim, c->id, "has taken a dongle");
					log_event(c->sim, c->id, "has taken a dongle");
					pthread_mutex_unlock(&c->sim->dongles[b].mtx);
					pthread_mutex_unlock(&c->sim->dongles[a].mtx);
					return (1);
				}
			}
			now = now_ms();
			if (min_avail <= now)
				min_avail = now + 1;
			timestamp.tv_sec = min_avail / 1000;
			timestamp.tv_nsec = (min_avail % 1000) * 1000000;
			ret = pthread_cond_timedwait(&c->sim->sched_cond, &c->sim->sched_mtx, &timestamp);
			(void)ret;
			continue ;
		}
		pthread_cond_wait(&c->sim->sched_cond, &c->sim->sched_mtx);
	}
	queue_remove(c->sim, c->id);
	pthread_mutex_unlock(&c->sim->sched_mtx);
	return (0);
}

void	dongles_release_two(t_coder *c)
{
	int left;
	int right;
	long next;

	left = c->id - 1;
	right = c->id % c->sim->params.n_coders;
	next = now_ms() + c->sim->params.dongle_cooldown;

	if (left == right)
	{
		c->sim->dongles[left].available_at_ms = next;
		pthread_mutex_unlock(&c->sim->dongles[left].mtx);
	}
	else
	{
		c->sim->dongles[left].available_at_ms = next;
		pthread_mutex_unlock(&c->sim->dongles[left].mtx);

		c->sim->dongles[right].available_at_ms = next;
		pthread_mutex_unlock(&c->sim->dongles[right].mtx);
	}
	pthread_mutex_lock(&c->sim->sched_mtx);
	pthread_cond_broadcast(&c->sim->sched_cond);
	pthread_mutex_unlock(&c->sim->sched_mtx);
}
