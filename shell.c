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

char inputString[2050];
char *commandArgs[513];

bool emptyInputFlag = false;
bool commentInputFlag = false;
bool exitCommandFlag = false;
bool statusCommandFlag = false;
bool cdCommandFlag = false;

bool externalCommandFlag = false;
bool bgProcessFlag = false;

bool inputRedirFlag = false;
char inputRedirLoc[512];
bool outputRedirFlag = false;
char outputRedirLoc[512];

int fgStatusCode = 0;

int bgProcessArray[513];
bool bgProcessNum = -5;
bool allowBGFlag == true;

bool exitFlag = false;

void initShellState();
void resetCommandStateVariables();
void getInput();
void parseInput();
void killBGProcesses();
void checkBGProcesses();

void catchSIGTSTP();


int main()
{
	// signal stuff goes here


	// call sigaction

	initShellState();

	while(exitFlag != true)
	{
		resetCommandStateVariables();
		getInput();
		//dollarSignExpansion();
		parseInput();

		if (emptyInputFlag == true)
		{
			continue;
		}
		else if (commentInputFlag == true)
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
			if (WIFEXITED(fgStatusCode) != 0)
			{
				printf("exit value %d\n", WEXITSTATUS(fgStatusCode));
			}
			else if (WIFSIGNALED(fgStatusCode) != 0)
			{
				printf("terminated by signal %d\n", WTERMSIG(fgStatusCode));
			}
			fflush(stdout);
		}
		else if (cdCommandFlag == true)
		{
			//change dir
		}
		else if (externalCommandFlag == true)
		{
			pid_t spawnPid = -5
			spawnPid = fork();
			switch(spawnPid)
			{
				case -1:
					printf("fork failed\n");
					fflush(stdout);
				case 0: // child
					// signal stuff for children to ignore ctrl+z
					if (bgProcessFlag == true && allowBGFlag == false)
					{
						exit(0);
					}
					// foreground
					if (bgProcessFlag == false)
					{
						// signal stuff for foreground and ctrl+c
						if (inputRedirFlag == true)
						{
							// code for input redirection
						}
						if (outputRedirFlag == true)
						{
							// code for output redirection
						}
					}
					// background and background is allowed
					else
					{
						if (inputRedirFlag == true)
						{
							// code for input redirection
						}
						if (outputRedirFlag == true)
						{
							// code for output redirection
						}
					}
					// exec
					printf("invalid command\n");
					fflush(stdout);
					exit(1);
				default: // parent process
					if (bgProcessFlag == true && allowBGFlag == true)
					{
						// add to BGarray
						bgProcessArray[bgProcessNum] = spawnPid;
						// increment BGarray
						bgProcessNum++;
						printf("background pid is %d\n", spawnPid);
						fflush(stdout);
					}
					else if (bgProcessFlag == false)
					{
						// wait for child process
						pid_t pidOutput = waitpid(bgProcessArray[i], &fgStatusCode, 0);
						// if signal terminated printout
						if (WIFSIGNALED(fgStatusCode) != 0)
						{
							printf("terminated by signal %d\n", WTERMSIG(fgStatusCode));
							fflush(stdout);
						}

					}
			}
		}
		else
		{
			// bad things happened
		}
		checkBGProcesses();
	}
	return 0;
}

void initShellState()
{

	memset(inputString, sizeof(inputString), '\0');

	int i;
	for (i = 0; i < 513; ++i)
	{
		commandArgs[i] = NULL;
		bgProcessArray[i] = -5
	}

	bool bgProcessNum = 0;
	bool allowBGFlag == true;

	bool emptyInputFlag = false;
	bool commentInputFlag = false;
	bool exitCommandFlag = false;
	bool statusCommandFlag = false;
	bool cdCommandFlag = false;

	bool externalCommandFlag = false;
	bool bgProcessFlag = false;

	bool inputRedirFlag = false;
	memset(inputRedirLoc, sizeof(outputRedirLoc), '\0');
	
	bool outputRedirFlag = false;
	memset(outputRedirLoc, sizeof(outputRedirLoc), '\0');

	int fgStatusCode = -5;

	bool exitFlag = false;
}

void resetCommandStateVariables()
{
	memset(inputString, sizeof(inputString), '\0');

	int i;
	for (i = 0; i < 513; ++i)
	{
		//free(commandArgs[i]);
		commandArgs[i] = NULL;
	}

	bool emptyInputFlag = false;
	bool commentInputFlag = false;
	bool exitCommandFlag = false;
	bool statusCommandFlag = false;
	bool cdCommandFlag = false;

	bool externalCommandFlag = false;
	bool bgProcessFlag = false;

	bool inputRedirFlag = false;
	memset(inputRedirLoc, sizeof(outputRedirLoc), '\0');
	
	bool outputRedirFlag = false;
	memset(outputRedirLoc, sizeof(outputRedirLoc), '\0');

	bool exitFlag = false;
}

void getInput()
{
	printf(": ");
	fflush(stdout);
	// from stackoverflow on how signals can stop fgets mid system call
	// https://stackoverflow.com/questions/46146240/why-does-alarm-cause-fgets-to-stop-waiting
	do {
		errno = 0;
		fgets(inputString, sizeof(inputString), stdin);
	} while (EINTR == errno);
	inputString[strlen(userInput)-1] = '\0'
}

void parseInput()
{
	bool isBGLastArg = false;
	int numArgs = 0;
	int pid = getpid();

	if (strcmp(inputString, "") == 0)
	{
		emptyInputFlag = true;
	}

	char *token = strtok(inputString, " ");

	while (token != NULL)
	{
		if (strcmp(token, "<") == 0)
		{
			isBGLastArg = false;
			inputRedirFlag = true;
			token = strtok(NULL, " ");
			strcpy(inputRedirLoc, token);
		}
		else if (strcmp(token, ">") == 0)
		{
			isBGLastArg = false;
			token = strtok(NULL, " ");
			strcpy(outputRedirLoc, token);
		}
		else if (strcmp(token, "&") == 0)
		{
			isBGLastArg = true;
		}
		else
		{
			isBGLastArg = false;
			commandArgs[numArgs] = strdup(token);

			// simple way to do $$ expansion
			// borrowed from friend
			int curChar;
			for (curChar = 0; commandArgs[numArgs][curChar]; ++curChar)
			{
				// works because two chars are beign compared not strings
				if (commandArgs[numArgs][curChar] == '$'
					&& commandArgs[numArgs][curChar + 1] == '$')
				{
					commandArgs[numArgs][curChar] = '\0';
					sprintf(commandArgs[numArgs], "%s%d%s",
						commandArgs[numArgs][curChar], pid,
						commandArgs[numArgs][curChar + 2]);
				}
			}

			numArgs++;
		}
		// next argument
		token = strtok(NULL, " ");
	}
	if(isBGLastArg = true)
	{
		bgProcessFlag = true;
	}

	if (commandArgs[0][0] == '#')
	{
		commentInputFlag = true;
	}
	else if (strcmp(commandArgs[0], "exit") == 0)
	{
		exitCommandFlag = true;
	}
	else if (strcmp(commandArgs[0], "status") == 0)
	{
		statusCommandFlag = true;
	}
	else if (strcmp(commandArgs[0], "cd") == 0)
	{
		cdCommandFlag = true;
	}
	else
	{
		externalCommandFlag = true;
	}
}

void killBGProcesses()
{
	int i;
	for (i = 0; i < 513; ++i)
	{
		if (bgProcessArray[i] != -5)
		{
			kill(bgProcessArray[i], SIGTERM);
			int childExitMethod = -5;
			pid_t pidOutput = waitpid(bgProcessArray[i], &childExitMethod, 0);
		}
	}
}

void checkBGProcesses()
{
	int i;
	for (i = 0; i < 513; ++i)
	{
		if (bgProcessArray[i] != -5)
		{
			int childExitMethod = -5;
			pid_t pidOutput = waitpid(bgProcessArray[i], &childExitMethod, WNOHANG);
			if (pidOuput > 0)
			{
				printf("background pid %d is done: ", bgProcessArray[i]);
				if (WIFEXITED(childExitMethod) != 0)
				{
					printf("exit value %d\n", WEXITSTATUS(childExitMethod));
				}
				else if (WIFSIGNALED(childExitMethod))
				{
					printf("terminated by signal %d\n", WTERMSIG(childExitMethod));
				}
			}
		}
	}
	fflush(stdout);
}

void catchSIGTSTP()
{
	// signal code referenced from signals slides
	if (allowBGFlag == true)
	{
		allowBGFlag == false;
		char* message = "Entering foreground-only mode (& is now ignored)\n";
		write(1, message, 49);
		fflush(stdout);
	}
	else
	{
		allowBGFlag == true;
		char* message = "Exiting foreground-only mode\n";
		write (1, message, 29);
		fflush(stdout);
	}
}