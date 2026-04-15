/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 09:35:58 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/15 19:28:50 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	try_take_dongles(t_coder *c)
{
	int	left;
	int	right;

	left = c->id - 1;
	right = c->id % c->sim->params.n_coders;
	if (left == right)
		return (try_take_one(c, left));
	return (try_take_two(c, left, right));
}

long	try_take_one(t_coder *c, int idx)
{
	long	now;
	long	avail;

	pthread_mutex_lock(&c->sim->dongles[idx].mtx);
	now = now_ms();
	avail = c->sim->dongles[idx].available_at_ms;
	if (now >= avail)
	{
		queue_remove(c->sim, c->id);
		pthread_mutex_unlock(&c->sim->sched_mtx);
		log_event(c->sim, c->id, "has taken a dongle");
		pthread_mutex_unlock(&c->sim->dongles[idx].mtx);
		return (-1);
	}
	pthread_mutex_unlock(&c->sim->dongles[idx].mtx);
	return (avail);
}

static void	get_indices(int left, int right, int *a, int *b)
{
	if (left < right)
		*a = left;
	else
		*a = right;
	if (left < right)
		*b = right;
	else
		*b = left;
}

static long	get_min_time(t_sim *sim, int a, int b)
{
	if (sim->dongles[a].available_at_ms
		< sim->dongles[b].available_at_ms)
		return (sim->dongles[a].available_at_ms);
	return (sim->dongles[b].available_at_ms);
}

long	try_take_two(t_coder *c, int left, int right)
{
	int		a;
	int		b;
	long	now;

	get_indices(left, right, &a, &b);
	pthread_mutex_lock(&c->sim->dongles[a].mtx);
	pthread_mutex_lock(&c->sim->dongles[b].mtx);
	now = now_ms();
	if (now >= c->sim->dongles[a].available_at_ms
		&& now >= c->sim->dongles[b].available_at_ms)
	{
		queue_remove(c->sim, c->id);
		pthread_mutex_unlock(&c->sim->sched_mtx);
		log_event(c->sim, c->id, "has taken a dongle");
		log_event(c->sim, c->id, "has taken a dongle");
		pthread_mutex_unlock(&c->sim->dongles[b].mtx);
		pthread_mutex_unlock(&c->sim->dongles[a].mtx);
		return (-1);
	}
	pthread_mutex_unlock(&c->sim->dongles[b].mtx);
	pthread_mutex_unlock(&c->sim->dongles[a].mtx);
	return (get_min_time(c->sim, a, b));
}
