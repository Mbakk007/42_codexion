/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 11:49:49 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/01 21:17:29 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	destroy_dongles(t_simulation *sim);
static int	init_dongles(t_simulation *sim);
static int	init_coders(t_simulation *sim);

void	destroy_simulation(t_simulation *sim)
{
	int	i;

	if (sim->simulation_running && sim->coders)
	{
		i = 0;
		while (i < sim->params.number_of_coders)
			pthread_join(sim->coders[i++].thread, NULL);
		pthread_join(sim->monitor_thread, NULL);
	}
	free(sim->coders);
	sim->coders = NULL;
	destroy_dongles(sim);
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->state_mutex);
}

static void	destroy_dongles(t_simulation *sim)
{
	int	i;
	int	num;

	num = sim->params.number_of_coders;
	if (num < 2)
		num = 2;
	i = 0;
	while (sim->dongles && i < num)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		pthread_cond_destroy(&sim->dongles[i].cond);
		free(sim->dongles[i].wait_queue.coders);
		i++;
	}
	free(sim->dongles);
	sim->dongles = NULL;
}

int	init_simulation(t_simulation *sim, int argc, char **argv)
{
	int	i;

	sim->coders = NULL;
	sim->dongles = NULL;
	sim->simulation_running = 0;
	sim->someone_burned_out = 0;
	sim->all_done = 0;
	sim->has_started = 0;
	if (!parse_args(&sim->params, argc, argv))
		return (-1);
	pthread_mutex_init(&sim->log_mutex, NULL);
	pthread_mutex_init(&sim->state_mutex, NULL);
	if (init_dongles(sim) < 0 || init_coders(sim) < 0)
		return (destroy_simulation(sim), -1);
	sim->start_time = get_timestamp_ms();
	sim->simulation_running = 1;
	i = 0;
	while (i < sim->params.number_of_coders)
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_thread, &sim->coders[i++]) != 0)
			return (destroy_simulation(sim), -1);
	if (pthread_create(&sim->monitor_thread, NULL,
			(void *(*)(void *))monitor_simulation, sim) != 0)
		return (destroy_simulation(sim), -1);
	return (0);
}

static int	init_dongles(t_simulation *sim)
{
	int	i;
	int	num;

	num = sim->params.number_of_coders;
	if (num < 2)
		num = 2;
	sim->dongles = malloc(sizeof(t_dongle) * num);
	i = 0;
	while (i < num)
	{
		sim->dongles[i].id = i + 1;
		sim->dongles[i].is_held = 0;
		sim->dongles[i].release_time = 0;
		sim->dongles[i].sim = sim;
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		pthread_cond_init(&sim->dongles[i].cond, NULL);
		sim->dongles[i].wait_queue.coders = malloc(sizeof(t_priority_coder)
				* num);
		if (!sim->dongles[i].wait_queue.coders)
			return (destroy_simulation(sim), -1);
		sim->dongles[i].wait_queue.size = 0;
		sim->dongles[i].wait_queue.capacity = num;
		i++;
	}
	return (0);
}

static int	init_coders(t_simulation *sim)
{
	int		i;
	int		num;
	t_coder	*c;

	num = sim->params.number_of_coders;
	if (num < 2)
		num = 2;
	sim->coders = malloc(sizeof(t_coder) * sim->params.number_of_coders);
	if (!sim->coders)
		return (-1);
	i = 0;
	while (i < sim->params.number_of_coders)
	{
		c = &sim->coders[i];
		c->id = i + 1;
		c->state = WAITING;
		c->compiles_completed = 0;
		c->last_compile_start = 0;
		c->deadline = 0;
		c->left_dongle = &sim->dongles[i % num];
		c->right_dongle = &sim->dongles[(i + 1) % num];
		c->sim = sim;
		i++;
	}
	return (0);
}
