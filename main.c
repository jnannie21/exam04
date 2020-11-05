#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <string.h>

int			g_pipe_des[2];
int			g_stdout;
int			g_stdin;

int			ft_strlen(char *str)
{
	int		i = 0;

	while (str[i])
		i++;
	return (i);
}

char		*ft_strdup(char *str)
{
	char	*res;
	int		i = 0;

	res = malloc(sizeof(char) * ft_strlen(str) + 1);
	while (str[i])
	{
		res[i] = str[i];
		i++;
	}
	res[i] = '\0';
	return (res);
}

void		fatal_error(void)
{
	write(2, "error: fatal", ft_strlen("error: fatal"));
	write(2, "\n", 1);
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
	command_argv[i] = ft_strdup(arg);
	command_argv[i + 1] = 0;
	return (command_argv);
}

void		execute(char **command_argv, char **envp, int in_pipe)
{
	int		pid;

	if (g_pipe_des[0])
	{
		if (dup2(g_pipe_des[0], 0) == -1)
			fatal_error();
		close(g_pipe_des[0]);
		g_pipe_des[0] = 0;
	}
	else if (dup2(g_stdin, 0) == -1)
			fatal_error();
	if (in_pipe)
	{
		if (pipe(g_pipe_des) == -1)
			fatal_error();
		if (dup2(g_pipe_des[1], 1) == -1)
			fatal_error();
		close(g_pipe_des[1]);
	}
	else if (dup2(g_stdout, 1) == -1)
			fatal_error();
	if (!command_argv)
		return ;
	if (strcmp(command_argv[0], "cd") == 0)
	{
		if (!command_argv[1] || command_argv[2])
			write(2, "error: cd: bad arguments\n", ft_strlen("error: cd: bad arguments\n"));
		else if (chdir(command_argv[1]) == -1)
		{
			write(2, "error: cd: cannot change directory to ", ft_strlen("error: cd: cannot change directory to "));
			write(2, command_argv[1], ft_strlen(command_argv[1]));
			write(2, "\n", 1);
		}
	}
	else
	{
		pid = fork();
		if (pid < 0)
			fatal_error();
		else if (pid == 0)
		{
			execve(command_argv[0], command_argv, envp);
			write(2, "error: cannot execute ", ft_strlen("error: cannot execute "));
			write(2, command_argv[0], ft_strlen(command_argv[0]));
			write(2, "\n", 1);
			exit(-1);
		}
	}
	if (!in_pipe)
		while (wait(NULL) != -1)
			;
}

void		free_argv(char **command_argv)
{
	int		i = 0;

	while (command_argv && command_argv[i])
	{
		free(command_argv[i]);
		i++;
	}
	free(command_argv);
}

int			main(int argc, char **argv, char **envp)
{
	char	**command_argv;

	command_argv = 0;
	argv++;
	g_stdin = dup(0);
	g_stdout = dup(1);
	while (1)
	{
		if (*argv == 0)
		{
			if (command_argv)
				execute(command_argv, envp, 0);
			free_argv(command_argv);
			break ;
		}
		else if (**argv == '|' && *(*argv + 1) == '\0')
		{
			execute(command_argv, envp, 1);
			free_argv(command_argv);
			command_argv = 0;
		}
		else if (**argv == ';' && *(*argv + 1) == '\0')
		{
			execute(command_argv, envp, 0);
			free_argv(command_argv);
			command_argv = 0;
		}
		else
			command_argv = add_arg(command_argv, *argv);
		argv++;
	}
	dup2(g_stdin, 0);
	dup2(g_stdout, 1);
	close(g_stdin);
	close(g_stdout);
	return (!argc);
}
