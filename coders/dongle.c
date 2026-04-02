/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 20:45:01 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/02 21:33:05 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	try_take_two(t_coder *c, int left, int right, long *out_min_avail)
{
	long	now;
	int		a;
	int		b;
	long	a_avail;
	long	b_avail;

	a = left;
	b = right;
	if (a > b)
		(a = right, b = left);
	pthread_mutex_lock(&c->sim->dongles[a].mtx);
	pthread_mutex_lock(&c->sim->dongles[b].mtx);
	now = now_ms();
	a_avail = c->sim->dongles[a].available_at_ms;
	b_avail = c->sim->dongles[b].available_at_ms;
	if (now < a_avail || now < b_avail)
	{
		*out_min_avail = (a_avail < b_avail) ? a_avail : b_avail;
		pthread_mutex_unlock(&c->sim->dongles[b].mtx);
		pthread_mutex_unlock(&c->sim->dongles[a].mtx);
		return (0);
	}
	*out_min_avail = now;
	return (1);
}

int	dongles_init(t_sim *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->params.n_coders);
	if (!sim->dongles)
		return (0);
	i = 0;
	while (i < sim->params.n_coders)
	{
		sim->dongles[i].available_at_ms = now_ms();
		if (pthread_mutex_init(&sim->dongles[i].mtx, NULL) != 0)
			return (0);
		i++;
	}
	return (1);
}

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
	int				left;
	int				right;
	int				win;
	long			min_avail;
	struct timespec	timestamp;
	int				ret;
	long			now;

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
			if (try_take_two(c, left, right, &min_avail))
			{
				queue_remove(c->sim, c->id);
				pthread_mutex_unlock(&c->sim->sched_mtx);
				log_event(c->sim, c->id, "has taken a dongle");
				log_event(c->sim, c->id, "has taken a dongle");
				return (1);
			}
			now = now_ms();
			if (min_avail <= now)
				min_avail = now + 1;
			timestamp.tv_sec = min_avail / 1000;
			timestamp.tv_nsec = (min_avail % 1000) * 1000000;
			ret = pthread_cond_timedwait(&c->sim->sched_cond,
					&c->sim->sched_mtx, &timestamp);
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

	c->sim->dongles[left].available_at_ms = next;
	pthread_mutex_unlock(&c->sim->dongles[left].mtx);

	c->sim->dongles[right].available_at_ms = next;
	pthread_mutex_unlock(&c->sim->dongles[right].mtx);

	pthread_mutex_lock(&c->sim->sched_mtx);
	pthread_cond_broadcast(&c->sim->sched_cond);
	pthread_mutex_unlock(&c->sim->sched_mtx);
}