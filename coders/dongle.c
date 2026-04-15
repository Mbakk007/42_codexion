/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 20:45:01 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/15 19:29:25 by ael-bakk         ###   ########.fr       */
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

void	dongles_release_two(t_coder *c)
{
	int		left;
	int		right;
	long	next;

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

int	dongles_take_two(t_coder *c)
{
	int				win;
	long			min_avail;
	struct timespec	ts;

	pthread_mutex_lock(&c->sim->sched_mtx);
	if (!queue_push(c->sim, c->id))
		return (pthread_mutex_unlock(&c->sim->sched_mtx), 0);
	while (!sim_should_stop(c->sim))
	{
		win = queue_pick_winner(c->sim);
		if (win >= 0 && c->sim->queue->requests[win].coder_id == c->id)
		{
			min_avail = try_take_dongles(c);
			if (min_avail == -1)
				return (1);
			wait_until(c, min_avail, &ts);
			continue ;
		}
		pthread_cond_wait(&c->sim->sched_cond, &c->sim->sched_mtx);
	}
	queue_remove(c->sim, c->id);
	pthread_mutex_unlock(&c->sim->sched_mtx);
	return (0);
}

void	wait_until(t_coder *c, long min_avail, struct timespec *ts)
{
	long	now;

	now = now_ms();
	if (min_avail <= now)
		min_avail = now + 1;
	ts->tv_sec = min_avail / 1000;
	ts->tv_nsec = (min_avail % 1000) * 1000000;
	pthread_cond_timedwait(&c->sim->sched_cond,
		&c->sim->sched_mtx, ts);
}
