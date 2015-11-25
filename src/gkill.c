#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define DEFAULT_TIMEOUT 10

void print_help(FILE *fp, char *app)
{
	fprintf(fp, "Usage: %s (<options>) <pid>\n", app);
	fprintf(fp, "\n");
	fprintf(fp, " -a enable SIGABRT\n");
	fprintf(fp, " -c enable SIGCONT\n");
	fprintf(fp, " -h Print this help\n");
	fprintf(fp, " -v Print version\n");
	fprintf(fp, " -t Timeout between signals (default: %d)\n", DEFAULT_TIMEOUT);
	fprintf(fp, "\n");
	fprintf(fp, "The normal sequence of signals is SIGTERM,SIGCONT,SIGABRT,SIGKILL\n");
	fprintf(fp, "\n");
}

int IsRunning(pid_t pid)
{
	struct stat info;
	char *fname = NULL;
	int value = 1; //Assume running
	if (asprintf(&fname, "/proc/%d", pid) == 0)
		abort();
	int ret = stat(fname, &info);
	if (ret < 0)
	{
		value = 0;
	}
	free(fname);
	return value;
}

int WaitForExit(pid_t pid, int timeout)
{
	time_t until = time(NULL) + timeout;
	
	while(until > time(NULL))
	{
		if (IsRunning(pid) == 0)
			return 0;		
	
		usleep(100);
	}
	
	if (IsRunning(pid) == 0)
		return 0;
	return 1;
}

int SignalAndWait(pid_t pid, int signal, int timeout)
{
	printf("Sending signal %d To Pid: %d\n", signal, pid);
	if (kill(pid, signal) < 0)
	{
		printf("kill pid %d failed with error %s\n", pid, strerror(errno));
		return 0;
	}
	
	return WaitForExit(pid, timeout);
}

int main(int argc, char **argv)
{
	const char *ops = "achvt?";
	int opt = 0;
	int timeout = DEFAULT_TIMEOUT;
	int fails = 0;
	int enable_abort = 0;
	int enable_cont = 0;
	
	while( (opt = getopt(argc, argv, ops)) != -1)
	{
		switch(opt)
		{
			case 'a':
				enable_abort = 1;
				break;
			case 'c':
				enable_cont = 1;
				break;
			case 'h':
				print_help(stdout, argv[0]);
				exit(EXIT_SUCCESS);
				break;
			case 'v':
				printf("Version: %s\n", PACKAGE_VERSION);
				exit(EXIT_SUCCESS);
				break;
			case 't':
				timeout = atoi(optarg);
				break;
			default:
				printf("Unknown option '%c'\n", opt);
				print_help(stderr, argv[0]);
				exit(EXIT_FAILURE);
				break;
		}
	}

	if (optind == argc)
	{
		print_help(stderr, argv[0]);
		exit(EXIT_FAILURE);
	}
	
	while(optind < argc)
	{
		pid_t pid = atoi(argv[optind]);
		optind++;
		if (IsRunning(pid) == 0)
		{
			printf("Process %d not running\n", pid);
			continue;
		}
		
		printf("Gracfully killing process %d\n", pid);
	
		if (SignalAndWait(pid, SIGTERM, timeout) == 0)
		{
			printf("Process %d quit after SIGTERM\n", pid);
			continue;
		}
		
		if (enable_cont)
		{
			if (SignalAndWait(pid, SIGCONT, timeout) == 0)
			{
				printf("Process %d quit after SIGCONT\n", pid);
				continue;
			}
		}

		if (enable_abort)
		{
			if (SignalAndWait(pid, SIGABRT, timeout) == 0)
			{
				printf("Process %d quit after SIGABRT\n", pid);
				continue;
			}
		}

		if (SignalAndWait(pid, SIGKILL, timeout) == 0)
		{
			printf("Process %d stopped by SIGKILL\n", pid);
			continue;
		}
	
		printf("Error: process %d did not exit after SIGKILL\n", pid);
		fails++;
	}
	
	if (fails)
	{
		printf("Failed to kill %d processes\n", fails);
	}
	
	return fails;
}

