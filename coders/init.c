/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 17:23:27 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/03 10:48:50 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	sim_resources_init(t_sim *sim)
{
	int	i;
	int j;

	if (!queue_init(sim) || !dongles_init(sim) || !scheduler_init(sim))
		return (0);
	sim->next_arrival_key = 0;
	i = 0;
	while (i < sim->params.n_coders)
	{
		if (pthread_mutex_init(&sim->coders[i].state_mtx, NULL) != 0)
		{
			j = 0;
			while (j < i)
				pthread_mutex_destroy(&sim->coders[j++].state_mtx);
			scheduler_destroy(sim);
			dongles_destroy(sim);
			queue_destroy(sim);
			return (0);
		}
		sim->coders[i].last_compile_start_ms = sim->start_ms;
		sim->coders[i].compile_count = 0;
		i++;
	}
	return (1);
}

void	sim_resources_destroy(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->params.n_coders)
	{
		pthread_mutex_destroy(&sim->coders[i].state_mtx);
		i++;
	}
	scheduler_destroy(sim);
	dongles_destroy(sim);
	queue_destroy(sim);
}

int	dongles_init(t_sim *sim)
{
	int	i;
	int j;

	sim->dongles = malloc(sizeof(t_dongle) * sim->params.n_coders);
	if (!sim->dongles)
		return (0);
	i = 0;
	while (i < sim->params.n_coders)
	{
		sim->dongles[i].available_at_ms = now_ms();
		if (pthread_mutex_init(&sim->dongles[i].mtx, NULL) != 0)
		{
			j = 0;
			while (j < i)
				pthread_mutex_destroy(&sim->dongles[j++].mtx);
			free(sim->dongles);
			sim->dongles = NULL;
			return (0);
		}
		i++;
	}
	return (1);
}

int	scheduler_init(t_sim *sim)
{
	if (pthread_mutex_init(&sim->sched_mtx, NULL) != 0)
		return (0);
	if (pthread_cond_init(&sim->sched_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->sched_mtx);
		return (0);
	}
	return (1);
}

int	queue_init(t_sim *sim)
{
	sim->queue = malloc(sizeof(t_queue));
	if (!sim->queue)
		return (0);
	sim->queue->requests = malloc(sizeof(t_request) * sim->params.n_coders);
	if (!sim->queue->requests)
	{
		free(sim->queue);
		sim->queue = NULL;
		return (0);
	}
	sim->queue->len = 0;
	sim->queue->cap = sim->params.n_coders;
	return (1);
}
