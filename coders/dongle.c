/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 11:10:15 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/02 19:42:46 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	try_take_two(t_coder *c, int left, int right)
{
	long	now;
	int		a;
	int		b;

	now = now_ms();
	a = left;
	b = right;
	if (a > b)
		(a = right, b = left);

	if (pthread_mutex_trylock(&c->sim->dongles[a].mtx) != 0)
		return (0);
	if (pthread_mutex_trylock(&c->sim->dongles[b].mtx) != 0)
		return (pthread_mutex_unlock(&c->sim->dongles[a].mtx), 0);
	if (now < c->sim->dongles[left].available_at_ms
		|| now < c->sim->dongles[right].available_at_ms)
	{
		pthread_mutex_unlock(&c->sim->dongles[b].mtx);
		pthread_mutex_unlock(&c->sim->dongles[a].mtx);
		return (0);
	}
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
		sim->dongles[i].available_at_ms = sim->start_ms;
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
	int	left;
	int	right;
	int	win;

	left = c->id - 1;
	right = c->id % c->sim->params.n_coders;
	pthread_mutex_lock(&c->sim->sched_mtx);
	if (!queue_push(c->sim, c->id))
		return (pthread_mutex_unlock(&c->sim->sched_mtx), 0);
	while (!sim_should_stop(c->sim))
	{
		win = queue_pick_winner(c->sim);
		if (win >= 0 && c->sim->queue->requests[win].coder_id == c->id
			&& try_take_two(c, left, right))
			return (queue_remove(c->sim, c->id),
				pthread_mutex_unlock(&c->sim->sched_mtx), log_event(c->sim,
					c->id, "has taken a dongle"), log_event(c->sim, c->id,
					"has taken a dongle"), 1);
		pthread_cond_wait(&c->sim->sched_cond, &c->sim->sched_mtx);
	}
	return (queue_remove(c->sim, c->id),
		pthread_mutex_unlock(&c->sim->sched_mtx), 0);
}

void	dongles_release_two(t_coder *c)
{
	int		left;
	int		right;
	long	next;

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
