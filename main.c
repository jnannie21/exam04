#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int			g_pipe_des[2];

int			ft_strlen(char *str)
{
	int		i = 0;

	while (str[i])
		i++;
	return (i);
}

void		fatal_error(void)
{
	write(2, "error: fatal\n", ft_strlen("error: fatal\n"));
	exit(EXIT_FAILURE);
}

char		**add_arg(char **command_argv, char *arg)
{
	int		i = 0;
	char	**temp;

	while (command_argv && command_argv[i])
		i++;
	temp = command_argv;
	if (!(command_argv = malloc(sizeof(char *) * (i + 2))))
		fatal_error();
	i = 0;
	while (temp && temp[i])
	{
		command_argv[i] = temp[i];
		i++;
	}
	free(temp);
	command_argv[i] = arg;
	command_argv[i + 1] = 0;
	return (command_argv);
}

int			execute(char **command_argv, char **envp, int in_pipe)
{
	int		pid;
	int		exit_status = 0;
	int		ret = 0;
	int		pipe_in = 0;
	int		pipe_out = 1;

	if (strcmp(command_argv[0], "cd") == 0)
	{
		if (!command_argv[1] || command_argv[2])
		{
			write(2, "error: cd: bad arguments\n", ft_strlen("error: cd: bad arguments\n"));
			ret = 1;
		}
		else if ((chdir(command_argv[1])) == -1)
		{
			write(2, "error: cd: cannot change directory to ", ft_strlen("error: cd: cannot change directory to "));
			write(2, command_argv[1], ft_strlen(command_argv[1]));
			write(2, "\n", 1);
			ret = 1;
		}
	}
	else
	{
		if (g_pipe_des[0])
		{
			pipe_in = g_pipe_des[0];
			g_pipe_des[0] = 0;
		}
		if (in_pipe)
		{
			if (pipe(g_pipe_des) == -1)
				fatal_error();
			pipe_out = g_pipe_des[1];
		}
		pid = fork();
		if (pid < 0)
			fatal_error();
		else if (pid == 0)
		{
			if (pipe_in != 0)
			{
				if (dup2(pipe_in, 0) == -1)
					fatal_error();
				close(pipe_in);
			}
			if (pipe_out != 1)
			{
				if (dup2(pipe_out, 1) == -1)
					fatal_error();
				close(pipe_out);
			}
			execve(command_argv[0], command_argv, envp);
			write(2, "error: cannot execute ", ft_strlen("error: cannot execute "));
			write(2, command_argv[0], ft_strlen(command_argv[0]));
			write(2, "\n", 1);
			exit(1);
		}
		if (pipe_in != 0)
			close(pipe_in);
		if (pipe_out != 1)
			close(pipe_out);
		waitpid(pid, &exit_status, 0);
		if (WIFEXITED(exit_status))
			ret = WEXITSTATUS(exit_status);
	}
	return (ret);
}

int			main(int argc, char **argv, char **envp)
{
	char	**command_argv;
	int		ret = 0;

	command_argv = 0;
	while (argc)
	{
		argv++;
		if (*argv == 0)
		{
			if (command_argv)
				ret = execute(command_argv, envp, 0);
			free(command_argv);
			command_argv = 0;
			break ;
		}
		else if (!strcmp(*argv, "|"))
		{
			if (command_argv)
				ret = execute(command_argv, envp, 1);
			free(command_argv);
			command_argv = 0;
		}
		else if (!strcmp(*argv, ";"))
		{
			if (command_argv)
				ret = execute(command_argv, envp, 0);
			free(command_argv);
			command_argv = 0;
		}
		else
			command_argv = add_arg(command_argv, *argv);
	}
	return (ret);
}
