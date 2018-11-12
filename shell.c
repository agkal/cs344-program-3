#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

// type def to use bool as a data type
typedef int bool;
enum { false, true };

char inputString[2048];
char *commandArgs[512];

bool emptyInputFlag = false;
bool exitCommandFlag = false;
bool statusCommandFlag = false;
bool cdCommandFlag = false;

bool externalCommandFlag = false;
bool bgProcessFlag = false;
bool inputRedirectionFlag = false;
char inputRedirectionLocation[512];
bool outputRedirectionFlag = false;
char outputRedirectionLocation[512];

int fgStatusCode = 0;

int bgProeccessArray[512];
bool bgProcessNum = -5;

bool exitFlag = false;

void initShellState();
void resetCommandStateVariables();
void checkBGProcesses();
void getInput();
void dollarSignExpansion();
void parseInput();
void killBGProcesses();


int main()
{
	// signal stuff goes here


	// call sigaction

	setShellState();

	while(exitFlag != true)
	{
		resetCommandStateVariables();
		checkBGProcesses();
		getInput();
		//dollarSignExpansion();
		parseInput();

		if (emptyInputFlag == true)
		{
			continue;
		}
		else if (exitCommandFlag == true)
		{
			killBGProcesses();
			exitFlag = true;
		}
		else if (statusCommandFlag == true)
		{
			printf("The last forground process ended with status code: %d\n", fgStatusCode);
			fflush(stdout);
		}
		else if (cdCommandFlag == true)
		{
			//change dir
		}
		else if (externalCommandFlag == true)
		{
			pid_t spawnpid = -5
			spawnpid = fork();
			switch(spawnpid)
			{
				case -1:
					printf("fork failed\n");
					fflush(stdout);
				case 0: // child
					if (bgProcessFlag == false )
					{
						// signal stuff for foreground
					}

					if (inputRedirectionFlag == true)
					{
						// code for input redirection
					}
					if (outputRedirectionFlag == true)
					{
						// code for output redirection
					}

					if (bgProcessFlag == true)
					{
						// exec code
					}
					else
					{
						// exec code
					}
				default: // parent process
					if (bgProcessFlag == true)
					{
						// add to BGarray
						// increment BGarray
					}
					else
					{
						// wait for child process
						// get exit status
					}
			}
		}
		else
		{
			// bad things happened
		}
	}
	return 0;
}