/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 10:53:32 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/02 19:32:50 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	reached_quota(t_coder *c)
{
	int	done;

	if (c->sim->params.must_compile_count <= 0)
		return (0);
	pthread_mutex_lock(&c->state_mtx);
	done = (c->compile_count >= c->sim->params.must_compile_count);
	pthread_mutex_unlock(&c->state_mtx);
	return (done);
}

static int	coder_cycle(t_coder *c)
{
	if (!dongles_take_two(c))
		return (0);
	if (sim_should_stop(c->sim) || reached_quota(c))
		return (dongles_release_two(c), 0);
	pthread_mutex_lock(&c->state_mtx);
	c->last_compile_start_ms = now_ms();
	pthread_mutex_unlock(&c->state_mtx);
	pthread_mutex_lock(&c->sim->sched_mtx);
	pthread_cond_broadcast(&c->sim->sched_cond);
	pthread_mutex_unlock(&c->sim->sched_mtx);
	log_event(c->sim, c->id, "is compiling");
	ms_sleep(c->sim, c->sim->params.t_compile);
	pthread_mutex_lock(&c->state_mtx);
	c->compile_count++;
	pthread_mutex_unlock(&c->state_mtx);
	dongles_release_two(c);
	if (sim_should_stop(c->sim) || reached_quota(c))
		return (0);
	log_event(c->sim, c->id, "is debugging");
	ms_sleep(c->sim, c->sim->params.t_debug);
	if (sim_should_stop(c->sim) || reached_quota(c))
		return (0);
	log_event(c->sim, c->id, "is refactoring");
	ms_sleep(c->sim, c->sim->params.t_refactor);
	return (1);
}

void	*coder_routine(void *arg)
{
	t_coder	*c;

	c = (t_coder *)arg;
	while (!sim_should_stop(c->sim) && !reached_quota(c))
	{
		if (!coder_cycle(c))
			break ;
	}
	return (NULL);
}