/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 17:53:43 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/02 19:29:34 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	scheduler_init(t_sim *sim)
{
	if (pthread_mutex_init(&sim->sched_mtx, NULL) != 0)
		return (0);
	if (pthread_cond_init(&sim->sched_cond, NULL) != 0)
		return (0);
	return (1);
}

void	scheduler_destroy(t_sim *sim)
{
	pthread_cond_destroy(&sim->sched_cond);
	pthread_mutex_destroy(&sim->sched_mtx);
}

long	scheduler_key_ms(t_sim *sim, int coder_id, long arrival_ms)
{
	long	last;

	if (sim->params.scheduler == FIFO)
		return (arrival_ms);
	last = sim->coders[coder_id - 1].last_compile_start_ms;
	return (last + sim->params.t_burnout);
}

