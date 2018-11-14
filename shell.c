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
#include <errno.h>

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
int bgProcessNum = -5;
bool allowBGFlag = true;

bool exitFlag = false;

void initShellState();
void resetCommandStateVariables();
void getInput();
void parseInput();
void killBGProcesses();
void checkBGProcesses();

void catchSIGTSTP(int signo);


int main()
{
	// signal code heavily referenced from in class powerpoint
	// signal stuff goes here
	struct sigaction SIGINT_action = {0};
	SIGINT_action.sa_handler = SIG_DFL;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;

	struct sigaction SIGTSTP_action = {0};
	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;

	struct sigaction ignore_action = {0};
	ignore_action.sa_handler = SIG_IGN;

	sigaction(SIGINT, &ignore_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);


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
			if (commandArgs[1] != NULL)
			{
				chdir(commandArgs[1]);
			}
			else
			{
				chdir(getenv("HOME"));
			}
		}
		else if (externalCommandFlag == true)
		{
			pid_t spawnPid = -5;
			spawnPid = fork();
			switch(spawnPid)
			{
				case -1:
					printf("fork failed\n");
					fflush(stdout);
				case 0: ;// child
					int fdin = 0;
					int fdout = 0;
					int daijoubu = 0;

					// signal stuff for children to ignore ctrl+z
					sigaction(SIGTSTP, &ignore_action, NULL);

					if (bgProcessFlag == true && allowBGFlag == false)
					{
						exit(-5);
					}
					// foreground
					if (bgProcessFlag == false)
					{
						// signal stuff for foreground and ctrl+c
						sigaction(SIGINT, &SIGINT_action, NULL);

						if (inputRedirFlag == true)
						{
							if (strcmp(inputRedirLoc, "") != 0)
							{
								fdin = open(inputRedirLoc, O_RDONLY);
								if (fdin == -1)
								{
									fprintf(stderr, "cannot open %s for input\n", inputRedirLoc);
									fflush(stderr);
									exit(1);
								}
								daijoubu = dup2(fdin, STDIN_FILENO);
								if (daijoubu == -1)
								{
									fprintf(stderr, "error with dup2\n");
									fflush(stderr);
									exit(1);
								}
								fcntl(fdin, F_SETFD, FD_CLOEXEC);
							}
						}

						if (outputRedirFlag == true)
						{
							if (strcmp(outputRedirLoc, "") != 0)
							{
								fdout = open(outputRedirLoc, O_WRONLY | O_CREAT | O_TRUNC, 0666);
								if (fdout == -1)
								{
									fprintf(stderr, "cannot open %s for output\n", outputRedirLoc);
									fflush(stderr);
									exit(1);
								}
								daijoubu = dup2(fdout, STDOUT_FILENO);
								if (daijoubu == -1)
								{
									fprintf(stderr, "error with dup2\n");
									fflush(stderr);
									exit(1);
								}
								fcntl(fdout, F_SETFD, FD_CLOEXEC);
							}
						}
					}
					// background and background is allowed
					else
					{
						fdin = open("/dev/null", O_RDONLY);
						if (fdin == -1)
								{
									fprintf(stderr, "cannot open /dev/null for input\n");
									fflush(stderr);
									exit(1);
								}
						daijoubu = dup2(fdin, STDIN_FILENO);
						if (daijoubu == -1)
								{
									fprintf(stderr, "error with dup2\n");
									fflush(stderr);
									exit(1);
								}
						fcntl(fdin, F_SETFD, FD_CLOEXEC);

						fdout = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0666);
						if (fdout == -1)
								{
									fprintf(stderr, "cannot open /dev/null for output\n");
									fflush(stderr);
									exit(1);
								}
						daijoubu = dup2(fdout, STDOUT_FILENO);
						if (daijoubu == -1)
								{
									fprintf(stderr, "error with dup2\n");
									fflush(stderr);
									exit(1);
								}
						fcntl(fdout, F_SETFD, FD_CLOEXEC);

						if (inputRedirFlag == true)
						{
							if (strcmp(inputRedirLoc, "") != 0)
							{
								fdin = open(inputRedirLoc, O_RDONLY);
								if (fdin == -1)
								{
									fprintf(stderr, "cannot open %s for input\n", inputRedirLoc);
									fflush(stderr);
									exit(1);
								}
								daijoubu = dup2(fdin, STDIN_FILENO);
								if (daijoubu == -1)
								{
									fprintf(stderr, "error with dup2\n");
									fflush(stderr);
									exit(1);
								}
								fcntl(fdin, F_SETFD, FD_CLOEXEC);
							}
						}

						if (outputRedirFlag == true)
						{
							if (strcmp(outputRedirLoc, "") != 0)
							{
								fdout = open(outputRedirLoc, O_WRONLY | O_CREAT | O_TRUNC, 0666);
								if (fdout == -1)
								{
									fprintf(stderr, "cannot open %s for output\n", outputRedirLoc);
									fflush(stderr);
									exit(1);
								}
								daijoubu = dup2(fdout, STDOUT_FILENO);
								if (daijoubu == -1)
								{
									fprintf(stderr, "error with dup2\n");
									fflush(stderr);
									exit(1);
								}
								fcntl(fdout, F_SETFD, FD_CLOEXEC);
							}
						}
					}

					int execStatus = execvp(commandArgs[0], commandArgs);
					if (execStatus == -1)
					{
						fprintf(stderr, "%s: no such file or directory\n", commandArgs[0]);
						fflush(stderr);
						exit(1);
					}

					fprintf(stderr, "invalid command\n");
					fflush(stderr);
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
						pid_t pidOutput = waitpid(spawnPid, &fgStatusCode, 0);
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
			printf("undefined action\n");
			fflush(stdout);
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
		bgProcessArray[i] = -5;
	}

	bool bgProcessNum = 0;
	bool allowBGFlag = true;

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
	inputString[strlen(inputString)-1] = '\0';
}

void parseInput()
{
	bool isBGLastArg = false;
	int numArgs = 0;
	int pid = getpid();

	if (strcmp(inputString, "") == 0)
	{
		emptyInputFlag = true;
		commandArgs[0] = strdup("");
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
					sprintf(commandArgs[numArgs], "%s%d",
						commandArgs[numArgs][curChar], pid);//,
						//commandArgs[numArgs][curChar + 2]);
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
			if (pidOutput > 0)
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

void catchSIGTSTP(int signo)
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