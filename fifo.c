#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "perseus-sdr.h"
#include "fifo.h"

int make_fifo(const char *fifo_name)
{
	int fd = open(fifo_name, O_RDONLY);
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags);
	return fd;
}

static 
void*	fifo_thread_fn (void *p)
{
	FIFO_TH *pd = (FIFO_TH *)p;
	char buf[256];
	
	FILE *fd = fdopen(pd->fd, "r");
	
	while (1) {
		
		char *x = fgets(buf, sizeof(buf), fd);

		if (x && strlen(x) > 1 ) {
			buf[strlen(x)-1] = 0 ; // trim endline chararacter
			
			int nf;
			if (strcmp(buf, "quit") == 0) break;
			if (sscanf (buf, "%d", &nf) == 1 && pd->descr) {
				fprintf (stderr, ">>>>> NF: %d\n", nf);
				perseus_set_ddc_center_freq(pd->descr, (double) nf, 0);
			}
		}
	}
	return 0;
}

int		run_fifo(FIFO_TH *ftd)
{
	
	if (pthread_create(&ftd->th, NULL, &fifo_thread_fn, ftd)!= 0) {
		return 1;
	} else 
		return  0;
}


#if defined TEST_MODULE

int main ()
{
	FIFO_TH d = {0};
	d.fd = make_fifo("/tmp/xyz");
	
	fprintf (stderr, "FIFO desc: %d\n", d.fd);
	
	run_fifo (&d);
	
	pthread_join (d.th, 0);
}


#endif

