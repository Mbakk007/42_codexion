/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 10:53:32 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/01 11:18:38 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	start_compiling(t_coder *coder);
static void	start_debugging(t_coder *coder);
static void	start_refactoring(t_coder *coder);

void	*coder_thread(void *arg)
{
	t_coder			*coder;
	t_simulation	*sim;

	coder = (t_coder *)arg;
	sim = coder->left_dongle->sim;
	coder->deadline = get_elapsed_ms(sim) + sim->params.time_to_burnout_ms;
	while (sim->simulation_running)
	{
		if (get_elapsed_ms(sim) > coder->deadline)
		{
			coder->state = BURNED_OUT;
			log_message(sim, coder, "burned out");
			sim->someone_burned_out = 1;
			return (NULL);
		}
		wait_for_dongles(coder);
		start_compiling(coder);
		start_debugging(coder);
		start_refactoring(coder);
	}
	return (NULL);
}

// Simulate compiling
static void	start_compiling(t_coder *coder)
{
	t_simulation	*sim;

	sim = coder->left_dongle->sim;
	coder->state = COMPILING;
	log_compiling(sim, coder);
	sleep_ms(sim->params.time_to_compile_ms);
	coder->compiles_completed++;
	release_dongles(coder);
	coder->deadline = get_elapsed_ms(sim) + sim->params.time_to_burnout_ms;
}

// Simulate debugging
static void	start_debugging(t_coder *coder)
{
	t_simulation	*sim;

	sim = coder->left_dongle->sim;
	coder->state = DEBUGGING;
	log_debugging(sim, coder);
	sleep_ms(sim->params.time_to_debug_ms);
}

// Simulate refactoring
static void	start_refactoring(t_coder *coder)
{
	t_simulation	*sim;

	sim = coder->left_dongle->sim;
	coder->state = REFACTORING;
	log_refactoring(sim, coder);
	sleep_ms(sim->params.time_to_refactor_ms);
	if (coder->compiles_completed >= sim->params.number_of_compiles_required)
		sim->all_done = 1;
}
