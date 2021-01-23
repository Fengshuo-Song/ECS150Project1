#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CMDLINE_MAX 512
struct Command
{
	char *program;
	char *argv[17];
	char *destfile;
};

void pipeline(struct Command *command1, struct Command *command2)
{
	int fd[2];
	pipe(fd);
	if (fork() != 0) { /* Parent */
		/* No need for read access */
		close(fd[0]);
		/* Replace stdout with pipe */
		dup2(fd[1], STDOUT_FILENO);
		/* Close now unused FD */
		close(fd[1]);
		/* Parent becomes process1 */
		execvp(command1->program, command1->argv);
	} else { /* Child */
		/* No need for write access */
		close(fd[1]);
		/* Replace stdin with pipe */
		dup2(fd[0], STDIN_FILENO);
		/* Close now unused FD */
		close(fd[0]);
		/* Child becomes process2 */
		execvp(command2->program, command2->argv);
	}
}

void tokentocommand(struct Command *command, char *token)
{
	char *lineptr = token;
	int outredir = 0;//flag for output redirection
	int argnum = 0;

	while (*lineptr) {

		/* Replace white spaces with null char*/
		while (*lineptr && *lineptr == ' ') {
			*lineptr = '\0';
			lineptr++;
		}

		/* Read an argument */
		if (*lineptr && *lineptr != '>' && outredir == 0) {
			command->argv[argnum] = lineptr;
			argnum++;
		}
		while (*lineptr && *lineptr != ' ' && *lineptr != '>') {
			lineptr++;
		}

		/* Read destination file for output redirection */
		if (*lineptr && *lineptr == '>') {
			outredir = 1;
			*lineptr = '\0';
			lineptr++;
			while (*lineptr && *lineptr == ' ') {
				*lineptr = '\0';
				lineptr++;
			}
			command->destfile = lineptr;
		}
		
	}

	if (argnum >= 17) {
		printf("error:exceed size\n");
	} else {
		command->argv[argnum] = NULL;
		command->program = command->argv[0];
		if (outredir == 1) {
			int fd;
			fd = open(command->destfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
			dup2(fd, STDOUT_FILENO);
			close(fd);
		}
	}
}

void parse(struct Command *command, char *cmd)
{
	char *token;
	token = strtok(cmd, "|");
	tokentocommand(command, token);
	command++;
	while (token != NULL) {
		token = strtok(NULL, "|");
		if (token == NULL) {
			break;
		}
		tokentocommand(command, token);
		command++;
	}
}

int process(struct Command *command)
{
        pid_t pid;
        pid = fork();
        if (pid == 0) {
                /* Child */
                execvp(command->program, command->argv);
                perror("execvp");
                exit(1);
        } else if (pid > 0) {
                /* Parent */
		int status;
		waitpid(pid, &status, 0);
		return WEXITSTATUS(status);
	} else {
		perror("fork");
		exit(1);
	}

	return 0;
}

void initialization(struct Command *command) {
	int i = 0;
	for (i = 0; i < 5; i++) {
		command->program = NULL;
		command->destfile = NULL;
		int j;
		for (j = 0; j < 17; j++) {
			command->argv[j] = NULL;
		}
		i++;
	}
}

int main(void)
{
        char cmd[CMDLINE_MAX];
	char line[CMDLINE_MAX];
	struct Command *command;
	struct Command commandlist[5];
	command = commandlist;
	char cwd[100];
	getcwd(cwd, sizeof(cwd));
	int saved_stdin = dup(STDIN_FILENO);
	int saved_stdout = dup(STDOUT_FILENO);
	int saved_stderr = dup(STDERR_FILENO);

        while (1) {
                char *nl;
                int exstatus;
		chdir(cwd);
		initialization(commandlist);
		
                /* Print prompt */
                printf("sshell$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

		/* Parse input */
		strcpy(line, cmd);
		parse(commandlist, line);

		/* Execute command */
		if (!strcmp(command->program, "exit")) {
			exstatus = 0;
			fprintf(stderr, "Bye...\n+ completed '%s' [%d]\n", cmd, exstatus);
                        break;
		} else if (!strcmp(command->program, "pwd")){
			getcwd(cwd, sizeof(cwd));
			fprintf(stderr,"%s\n", cwd);
		} else if (!strcmp(command->program, "cd")) {
			chdir(command->argv[1]);
			getcwd(cwd, sizeof(cwd));
		} else {
			/* Regular command */
			exstatus = process(command);
		}
		
		/* Reset fds */
		dup2(saved_stdin, STDIN_FILENO);
		dup2(saved_stdout, STDOUT_FILENO);
		dup2(saved_stderr, STDERR_FILENO);

		/* Print message */
                fprintf(stderr, "+ completed '%s' [%d]\n",
                        cmd, exstatus);
        }

        return EXIT_SUCCESS;
}
