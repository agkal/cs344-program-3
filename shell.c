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
	// signal stuff for SIGINT
	struct sigaction SIGINT_action = {0};
	SIGINT_action.sa_handler = SIG_DFL;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;

	// signal stuff for SIGTSTP
	struct sigaction SIGTSTP_action = {0};
	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;

	// signal stuff for ignoring signals
	struct sigaction ignore_action = {0};
	ignore_action.sa_handler = SIG_IGN;

	// call sigaction
	sigaction(SIGINT, &ignore_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	// init the shell state
	initShellState();

	// while the shell is not set to exit keep running
	while(exitFlag != true)
	{
		// following functions do as what their names say
		resetCommandStateVariables();
		getInput();
		//dollarSignExpansion();
		parseInput();

		// handle built in functions
		// empty
		if (emptyInputFlag == true)
		{
			continue;
		}
		// comment
		else if (commentInputFlag == true)
		{
			continue;
		}
		// exit
		else if (exitCommandFlag == true)
		{
			killBGProcesses();
			exitFlag = true;
		}
		// status
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
		// cd
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
		// external command
		else if (externalCommandFlag == true)
		{
			pid_t spawnPid = -5;
			spawnPid = fork();
			int fdin = 0;
			int fdout = 0;
			int daijoubu = 0;
			switch(spawnPid)
			{
				case -1: // fork failed
					printf("fork failed\n");
					fflush(stdout);
				case 0: ;// child
					

					// signal stuff for children to ignore ctrl+z
					sigaction(SIGTSTP, &ignore_action, NULL);

					// catch error with state, should never get into this if statement
					if (bgProcessFlag == true && allowBGFlag == false)
					{
						exit(-5);
					}
					// foreground
					if (bgProcessFlag == false)
					{
						// signal stuff for foreground and ctrl+c
						sigaction(SIGINT, &SIGINT_action, NULL);

						// input redirection
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
						// output redirection
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
						// default redirection for background processes
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

						// input redirection for background process
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

						// output redirection for background process
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

					// exec code handle if error
					int execStatus = execvp(commandArgs[0], commandArgs);
					if (execStatus == -1)
					{
						fprintf(stderr, "%s: no such file or directory\n", commandArgs[0]);
						fflush(stderr);
						exit(1);
					}
					// should never get here
					fprintf(stderr, "invalid command\n");
					fflush(stderr);
					exit(1);
				default: // parent process
					// if background
					if (bgProcessFlag == true && allowBGFlag == true)
					{
						// add to BGarray
						bgProcessArray[bgProcessNum] = spawnPid;
						// increment BGarray
						bgProcessNum++;
						printf("background pid is %d\n", spawnPid);
						fflush(stdout);
					}
					else if (bgProcessFlag == false) // if foreground
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
		// check to see if any background children have died before giving back controll to user
		checkBGProcesses();
	}
	return 0;
}

// sets all variables to initial state when shell first starts
void initShellState()
{

	memset(inputString, sizeof(inputString), '\0');

	int i;
	for (i = 0; i < 513; ++i)
	{
		commandArgs[i] = NULL;
		bgProcessArray[i] = -5;
	}

	bgProcessNum = 0;
	allowBGFlag = true;

	emptyInputFlag = false;
	commentInputFlag = false;
	exitCommandFlag = false;
	statusCommandFlag = false;
	cdCommandFlag = false;

	externalCommandFlag = false;
	bgProcessFlag = false;

	inputRedirFlag = false;
	memset(inputRedirLoc, sizeof(outputRedirLoc), '\0');
	
	outputRedirFlag = false;
	memset(outputRedirLoc, sizeof(outputRedirLoc), '\0');

	fgStatusCode = -5;

	exitFlag = false;
}

// resets all variables needed for each loop to work
void resetCommandStateVariables()
{
	memset(inputString, sizeof(inputString), '\0');

	int i;
	for (i = 0; i < 513; ++i)
	{
		//free(commandArgs[i]);
		commandArgs[i] = NULL;
	}

	emptyInputFlag = false;
	commentInputFlag = false;
	exitCommandFlag = false;
	statusCommandFlag = false;
	cdCommandFlag = false;

	externalCommandFlag = false;
	bgProcessFlag = false;

	inputRedirFlag = false;
	memset(inputRedirLoc, sizeof(outputRedirLoc), '\0');
	
	outputRedirFlag = false;
	memset(outputRedirLoc, sizeof(outputRedirLoc), '\0');

	exitFlag = false;
}

// gets input simple as that
void getInput()
{
	// from stackoverflow on how signals can stop fgets mid system call
	// https://stackoverflow.com/questions/46146240/why-does-alarm-cause-fgets-to-stop-waiting
	do {
		printf(": ");
		fflush(stdout);
		errno = 0;
		fgets(inputString, sizeof(inputString), stdin);
	} while (EINTR == errno);
	inputString[strlen(inputString)-1] = '\0';
}

// parses input and makes sense of it
void parseInput()
{
	bool isBGLastArg = false;
	int numArgs = 0;
	int pid = getpid();

	// if input string is empty
	// avoids seg fault
	if (strcmp(inputString, "") == 0)
	{
		emptyInputFlag = true;
		commandArgs[0] = strdup("");
	}

	char *token = strtok(inputString, " ");

	// strtok the string into workable pieces
	while (token != NULL)
	{
		// handle input redirection
		if (strcmp(token, "<") == 0)
		{
			isBGLastArg = false;
			inputRedirFlag = true;
			token = strtok(NULL, " ");
			strcpy(inputRedirLoc, token);
		}
		// handle output redirection
		else if (strcmp(token, ">") == 0)
		{
			isBGLastArg = false;
			outputRedirFlag = true;
			token = strtok(NULL, " ");
			strcpy(outputRedirLoc, token);
		}
		// handle background process
		else if (strcmp(token, "&") == 0)
		{
			isBGLastArg = true;
		}
		else
		{
			isBGLastArg = false;
			commandArgs[numArgs] = strdup(token);

			// simple way to do $$ expansion
			// Credit to Brent Irway
			int curChar;
			for (curChar = 0; commandArgs[numArgs][curChar]; ++curChar)
			{
				// works because two chars are beign compared not strings
				if (commandArgs[numArgs][curChar] == '$'
					&& commandArgs[numArgs][curChar + 1] == '$')
				{
					commandArgs[numArgs][curChar] = '\0';
					sprintf(commandArgs[numArgs], "%s%d",
						commandArgs[numArgs], pid);//,
						//commandArgs[numArgs][curChar + 2]);
				}
			}

			numArgs++;
		}
		// next argument
		token = strtok(NULL, " ");
	}

	// sets flags for what command was inputed based on what was put in commandArgs array
	if(isBGLastArg == true && allowBGFlag == true)
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

// function kills all background processes
// it gets called when exit is run
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

// checks background processes to see if any of them have terminated
// print message based on how they terminated
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

// function to catch SIGTSTP
// prints out message to terminal about foreground-only state
// and toggles forground-only state
void catchSIGTSTP(int signo)
{
	// signal code referenced from signals slides
	if (allowBGFlag == true)
	{
		allowBGFlag = false;
		char* message = "\nEntering foreground-only mode (& is now ignored)\n";
		write(1, message, 51);
		fflush(stdout);
	}
	else
	{
		allowBGFlag = true;
		char* message = "\nExiting foreground-only mode\n";
		write (1, message, 31);
		fflush(stdout);
	}
}