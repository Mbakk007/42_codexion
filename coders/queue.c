/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/02 17:53:23 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/03 10:40:18 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	queue_destroy(t_sim *sim)
{
	if (!sim->queue)
		return ;
	free(sim->queue->requests);
	free(sim->queue);
	sim->queue = NULL;
}

int	queue_push(t_sim *sim, int coder_id)
{
	int	i;

	/* already queued? don't add a duplicate */
	i = 0;
	while (i < sim->queue->len)
	{
		if (sim->queue->requests[i].coder_id == coder_id)
			return (1);
		i++;
	}
	if (sim->queue->len >= sim->queue->cap)
		return (0);
	sim->queue->requests[sim->queue->len].coder_id = coder_id;
	sim->queue->requests[sim->queue->len].arrival_key = sim->next_arrival_key++;
	sim->queue->len++;
	return (1);
}

void	queue_remove(t_sim *sim, int coder_id)
{
	int	i;

	i = 0;
	while (i < sim->queue->len)
	{
		if (sim->queue->requests[i].coder_id == coder_id)
		{
			while (i + 1 < sim->queue->len)
			{
				sim->queue->requests[i] = sim->queue->requests[i + 1];
				i++;
			}
			sim->queue->len--;
			return ;
		}
		i++;
	}
}

int	queue_pick_winner(t_sim *sim)
{
	int			i;
	int			best;
	long		best_key;
	long		key;
	t_request	*r;

	if (sim->queue->len <= 0)
		return (-1);
	best = 0;
	r = &sim->queue->requests[0];
	best_key = scheduler_key_ms(sim, r->coder_id, r->arrival_key);
	i = 1;
	while (i < sim->queue->len)
	{
		r = &sim->queue->requests[i];
		key = scheduler_key_ms(sim, r->coder_id, r->arrival_key);
		if (key < best_key || (key == best_key
				&& r->arrival_key < sim->queue->requests[best].arrival_key))
			best_key = key, best = i;
		i++;
	}
	return (best);
}
