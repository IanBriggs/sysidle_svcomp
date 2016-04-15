extern void __VERIFIER_error();
extern void __VERIFIER_atomic_begin();
extern void __VERIFIER_atomic_end();

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <poll.h>   /* ADDED MISSING INCLUDE */
#include <unistd.h> /* ADDED MISSING INCLUDE */
#include "fake.h"

#define CONFIG_NR_CPUS 3 /* SET TO 3 SIMULATED CORES */
#define NR_CPUS CONFIG_NR_CPUS
#define CONFIG_RCU_FANOUT 16
#define CONFIG_RCU_FANOUT_LEAF 8

#define CONFIG_NO_HZ_FULL_SYSIDLE
#define CONFIG_NO_HZ_FULL_SYSIDLE_SMALL 4
#define CONFIG_PREEMPT_RCU

#define HZ 1000

int nr_cpu_ids = NR_CPUS;
int rcu_fanout_leaf = CONFIG_RCU_FANOUT_LEAF;

#include "rcutree.h"

struct rcu_state rcu_preempt_state;
struct rcu_state rcu_sched_state;

struct rcu_data *rcu_preempt_data_array;
struct rcu_data *rcu_sched_data_array;

struct rcu_dynticks *rcu_dynticks_array;

int tick_do_timer_cpu;

void rcu_kick_nohz_cpu(int cpu)
{
	printf("Informed CPU %d of end of full-system idle.\n", cpu);
}

static int rcu_gp_in_progress(struct rcu_state *rsp)
{
	return 0;
}

#include "sysidle.h"

int goflag = 1;
int nthreads = CONFIG_NR_CPUS; /* SET TO MATCH CONFIGURATION */

struct thread_arg {
	int me;
};

void do_fqs(struct rcu_state *rsp, struct rcu_data *rdp_in)
{
	int i;
	bool isidle;
	unsigned long maxj;
	int old_full_sysidle_state;
	int new_full_sysidle_state;
	struct rcu_data *rdp;

	/* FQS scan for sched-RCU. */
	isidle = 1;
	maxj = jiffies - ULONG_MAX / 4;
	for (i = 0; i < nthreads; i++) {
		rdp = &rdp_in[i];
		if ((random() & 0xff) == 0)
			poll(NULL, 0, 20);
		rcu_sysidle_check_cpu(rdp, &isidle, &maxj);
	}
	old_full_sysidle_state = ACCESS_ONCE(full_sysidle_state);
	rcu_sysidle_report(rsp, isidle, maxj, 0);
	new_full_sysidle_state = ACCESS_ONCE(full_sysidle_state);
	if (old_full_sysidle_state != new_full_sysidle_state)
		printf("%lu: System transition from state %d to %d\n",
		       jiffies,
		       old_full_sysidle_state, new_full_sysidle_state);
	old_full_sysidle_state = ACCESS_ONCE(full_sysidle_state);
	if (old_full_sysidle_state == RCU_SYSIDLE_FULL)
		poll(NULL, 0, 5);
	else
		poll(NULL, 0, 1);
	if (rcu_sys_is_idle() &&
	    old_full_sysidle_state != RCU_SYSIDLE_FULL_NOTED) {
		printf("%lu: System fully idle\n", jiffies);
	} else {
		new_full_sysidle_state = ACCESS_ONCE(full_sysidle_state);
		if (new_full_sysidle_state != RCU_SYSIDLE_FULL_NOTED &&
		    old_full_sysidle_state != new_full_sysidle_state)
			printf("%lu: System transition from state %d to %d\n",
			       jiffies,
			       old_full_sysidle_state, new_full_sysidle_state);
	}
}

void *timekeeping_cpu(void *arg)
{
	int i;
	struct thread_arg *tap = (struct thread_arg *)arg;

	my_smp_processor_id = tap->me;
	while (ACCESS_ONCE(goflag)) {
		jiffies++;

		/* FQS scan for RCU-preempt and then RCU-sched. */
		do_fqs(&rcu_preempt_state, rcu_preempt_data_array);
		do_fqs(&rcu_sched_state, rcu_sched_data_array);
	}
	return NULL; /* RETURN ADDED TO SILENCE WARNING */
}

void *other_cpu(void *arg)
{
	int nest;
	struct rcu_dynticks *rdtp;
	struct thread_arg *tap = (struct thread_arg *)arg;

	my_smp_processor_id = tap->me;
	rdtp = &rcu_dynticks_array[tap->me];
	while (ACCESS_ONCE(goflag)) {
		/* busy period. */
		WARN_ON_ONCE(full_sysidle_state > RCU_SYSIDLE_LONG);
		poll(NULL, 0, random() % 10 + 1);
		WARN_ON_ONCE(full_sysidle_state > RCU_SYSIDLE_LONG);

		/* idle entry. */
		rcu_sysidle_enter(rdtp, 0);
		poll(NULL, 0, 1);

		/* Interrupts from idle. */
		nest = 0;
		while (random() & 0x100) {
			rcu_sysidle_exit(rdtp, 1);
			nest++;
		}
		poll(NULL, 0, 1);
		while (nest-- > 0) {
			rcu_sysidle_enter(rdtp, 1);
		}

		/* idle period. */
		poll(NULL, 0, random() % 100 + 1);

		/* idle exit. */
		rcu_sysidle_exit(rdtp, 0);
	}
	return NULL; /* RETURN ADDED TO SILENCE WARNING */
}

int main(int argc, char *argv[])
{
	int i;
	struct thread_arg *ta_array;
	pthread_t *tids;

	/* Parse single optional argument, # cpus. */
	/****************************************************************/
        /* if (argc > 1) {					        */
	/* 	nr_cpu_ids = atoi(argv[1]);			        */
	/* 	nthreads = nr_cpu_ids;				        */
	/* 	printf("nr_cpu_ids set to %d\n", nr_cpu_ids);	        */
	/* }							        */
        /****************************************************************/

	/* Allocate arrays and initialize. */
	rcu_preempt_data_array =
		malloc(sizeof(rcu_preempt_data_array[0]) * nthreads);
	memset(rcu_preempt_data_array, 0,
	       sizeof(rcu_preempt_data_array[0]) * nthreads);
	rcu_sched_data_array =
		malloc(sizeof(rcu_sched_data_array[0]) * nthreads);
	memset(rcu_sched_data_array, 0,
	       sizeof(rcu_sched_data_array[0]) * nthreads);
	rcu_dynticks_array = malloc(sizeof(rcu_dynticks_array[0]) * nthreads);
	memset(rcu_dynticks_array, 0, sizeof(rcu_dynticks_array[0]) * nthreads);
	ta_array = malloc(sizeof(ta_array[0]) * nthreads);
	memset(rcu_dynticks_array, 0, sizeof(ta_array) * nthreads);
	tids = malloc(sizeof(tids[0]) * nthreads);
	for (i = 0; i < nthreads; i++) {
		rcu_preempt_data_array[i].dynticks = &rcu_dynticks_array[i];
		rcu_preempt_data_array[i].rsp = &rcu_preempt_state;
		rcu_preempt_data_array[i].cpu = i;
		rcu_sched_data_array[i].dynticks = &rcu_dynticks_array[i];
		rcu_sched_data_array[i].rsp = &rcu_sched_state;
		rcu_sched_data_array[i].cpu = i;
		atomic_set(&rcu_dynticks_array[i].dynticks, 1);
		atomic_set(&rcu_dynticks_array[i].dynticks_idle, 1);
		rcu_dynticks_array[i].dynticks_idle_nesting =
			DYNTICK_TASK_EXIT_IDLE;
		ta_array[i].me = i;
	}
	rcu_preempt_state.rda = rcu_preempt_data_array;
	rcu_sched_state.rda = rcu_sched_data_array;
	srandom(time(NULL));

	/* Smoke test. */
	/**********************************************************************/
        /* printf("Start smoke test.\n");				      */
	/* for (i = 1; i < nthreads; i++) {				      */
	/* 	my_smp_processor_id = i;				      */
	/* 	rcu_sysidle_enter(&rcu_dynticks_array[i], 0);		      */
	/* }								      */
	/* my_smp_processor_id = 0;					      */
	/* for (i = 0; i < 100; i++) {					      */
	/* 	jiffies++;						      */
	/* 	do_fqs(&rcu_preempt_state, rcu_preempt_data_array);	      */
	/* 	do_fqs(&rcu_sched_state, rcu_sched_data_array);		      */
	/* 	if (full_sysidle_state == RCU_SYSIDLE_FULL_NOTED)	      */
	/* 		break;						      */
	/* }								      */
	/* WARN_ON_ONCE(full_sysidle_state != RCU_SYSIDLE_FULL_NOTED);	      */
	/* for (i = 1; i < nthreads; i++) {				      */
	/* 	my_smp_processor_id = i;				      */
	/* 	rcu_sysidle_exit(&rcu_dynticks_array[i], 0);		      */
	/* }								      */
	/* printf("End of smoke test.\n");				      */
        /**********************************************************************/

	/* Stress test. */
	printf("Start stress test.\n");
	pthread_create(&tids[0], NULL, timekeeping_cpu, &ta_array[0]);
	for (i = 1; i < nthreads; i++) {
		pthread_create(&tids[i], NULL, other_cpu, &ta_array[i]);
	}
	sleep(10);
	ACCESS_ONCE(goflag) = 0;
	for (i = 0; i < nthreads; i++) {
		void *junk;

		if (pthread_join(tids[i], &junk) != 0) {
			perror("pthread_join()");
			abort();
		}
	}
	printf("End of stress test.\n");

	if (!(full_sysidle_state != RCU_SYSIDLE_FULL_NOTED ||
	      ((atomic_read(&rcu_preempt_data_array[1].dynticks->dynticks_idle) & 0x1) == 0 &&
	       (atomic_read(&rcu_preempt_data_array[2].dynticks->dynticks_idle) & 0x1) == 0))) {
	  __VERIFIER_error();
	}

	return 0; /* RETURN ADDED TO SILENCE WARNING */
}
