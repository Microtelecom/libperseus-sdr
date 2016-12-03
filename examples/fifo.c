#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "perseus-sdr.h"
#include "fifo.h"

FIFO_TH fifo_data = {0};


int make_fifo(const char *fifo_name, perseus_descr *descr)
{
	strcpy (fifo_data.fifo_name, fifo_name);
	fifo_data.descr = descr;
	fifo_data.fd = fopen(fifo_data.fifo_name, "rw+");
	return fifo_data.fd != 0;
}

static 
void*	fifo_thread_fn (void *p)
{
	FIFO_TH *pd = (FIFO_TH *)p;
	
	while (1) {
		char buf[256] = {0};
		char *x = fgets(buf, sizeof(buf), pd->fd);
		
		if (x == 0) {
			perror ("fifo_thread_fn");
			break;
		}
		if (x && strlen(x) > 1 ) {
			buf[strlen(x)-1] = 0 ; // trim endline chararacter
			
			int nf;
			double nff;
			int att;

			if (strcmp(buf, "quit") == 0) break;
			if (sscanf (buf, "%lf", &nff) == 1 && pd->descr) {
				fprintf (stderr, ">>>>> NFF: %lf\n", nff*1000000.0);
				perseus_set_ddc_center_freq(pd->descr, (double) nff*1000000.0, 0);
			} else
			if (sscanf (buf, "%d", &nf) == 1 && pd->descr) {
				fprintf (stderr, ">>>>> NF: %d\n", nf);
				perseus_set_ddc_center_freq(pd->descr, (double) nf, 0);
			} else
			if (sscanf (buf, "att %d", &att) == 1 && pd->descr) {
				fprintf (stderr, ">>>>> ATT: %d\n", att);
				perseus_set_attenuator_n(pd->descr, att);
			} else
			{}
		}
	}
	fprintf (stderr, "Thread quitting...\n");
	return 0;
}

int	run_fifo()
{
	if (pthread_create(&(fifo_data.th), NULL, &fifo_thread_fn, &fifo_data)!= 0) {
		return 1;
	} else 
		return  0;
}

void stop_fifo ()
{
	void *trc;

	FILE *pf = fopen(fifo_data.fifo_name, "w+");
	fprintf (pf, "%s\n", "quit");
	fprintf (stderr, "Quit sent\n");
	fclose (pf);

	pthread_join (fifo_data.th, &trc);
}

#if defined TEST_MODULE

int main ()
{
	make_fifo("/tmp/pctrl",0);
	
	run_fifo ();
	fprintf (stderr, "running....\n");
	sleep (5);
	fprintf (stderr, "exiting....\n");
	stop_fifo();
}


#endif

