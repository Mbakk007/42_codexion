/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-bakk <ael-bakk@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 17:40:36 by ael-bakk          #+#    #+#             */
/*   Updated: 2026/03/30 01:39:30 by ael-bakk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <time.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <limits.h>
# include <string.h>

// FIFO: first arrived, first served
// EDF: serve the coder closest to burnout_ms
typedef enum e_sched
{
    FIFO_SCHED = 0,
    EDF_SCHED = 1
} t_sched;

// args struct
typedef struct args
{
    int     number_of_coders;
    long    time_to_burnout_ms;
    long    time_to_compile_ms;
    long    time_to_debug_ms;
    long    time_to_refactor_ms;
    int     number_of_compiles_required;
    long    dongle_cooldown_ms;
    t_sched sched;
} t_args;







#endif