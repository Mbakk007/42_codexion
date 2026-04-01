/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 11:10:15 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/04/01 11:24:48 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Releases dongles when called, signals waiting coders
void	release_dongles(t_coder *coder)
{
	t_dongle	*left;
	t_dongle	*right;

	left = coder->left_dongle;
	right = coder->right_dongle;
	pthread_mutex_lock(&left->mutex);
	left->is_held = 0;
	pthread_cond_signal(&left->cond);
	pthread_mutex_unlock(&left->mutex);
	pthread_mutex_lock(&right->mutex);
	right->is_held = 0;
	pthread_cond_signal(&right->cond);
	pthread_mutex_unlock(&right->mutex);
}

// Waits until both dongles are available, then locks them
void	wait_for_dongles(t_coder *coder)
{
	t_dongle	*left;
	t_dongle	*right;

	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		left = coder->left_dongle;
		right = coder->right_dongle;
	}
	else
	{
		left = coder->right_dongle;
		right = coder->left_dongle;
	}
	pthread_mutex_lock(&left->mutex);
	while (left->is_held)
		pthread_cond_wait(&left->cond, &left->mutex);
	left->is_held = 1;
	pthread_mutex_unlock(&left->mutex);
	log_taken_dongle(coder->left_dongle->sim, coder);
	pthread_mutex_lock(&right->mutex);
	while (right->is_held)
		pthread_cond_wait(&right->cond, &right->mutex);
	right->is_held = 1;
	pthread_mutex_unlock(&right->mutex);
	log_taken_dongle(coder->right_dongle->sim, coder);
}
