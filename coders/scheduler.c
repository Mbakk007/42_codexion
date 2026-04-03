/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 17:53:43 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/03 11:15:52 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	scheduler_destroy(t_sim *sim)
{
	pthread_cond_destroy(&sim->sched_cond);
	pthread_mutex_destroy(&sim->sched_mtx);
}

long	scheduler_key_ms(t_sim *sim, int coder_id, long arrival_key)
{
	long	last;

	if (sim->params.scheduler == FIFO)
		return (arrival_key);
	pthread_mutex_lock(&sim->coders[coder_id - 1].state_mtx);
	last = sim->coders[coder_id - 1].last_compile_start_ms;
	pthread_mutex_unlock(&sim->coders[coder_id - 1].state_mtx);
	return (last + sim->params.t_burnout);
}
