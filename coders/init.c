/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 17:23:27 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/02 19:18:01 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	sim_resources_init(t_sim *sim)
{
	int	i;

	if (!queue_init(sim) || !dongles_init(sim) || !scheduler_init(sim))
		return (0);
	i = 0;
	while (i < sim->params.n_coders)
	{
		if (pthread_mutex_init(&sim->coders[i].state_mtx, NULL) != 0)
			return (0);
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
