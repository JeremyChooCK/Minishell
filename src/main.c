/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jegoh <jegoh@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/21 21:45:15 by jegoh             #+#    #+#             */
/*   Updated: 2023/11/23 20:53:46 by jgyy             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "minishell.h"

void sigint_handler(int sig)
{
	(void)sig;
    write(1, "\nminishell> ", 12);
}

void	sigquit_handler(int sig)
{
    (void)sig;
}

void	setup_signal_handlers()
{
    struct sigaction sa;

    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sa.sa_handler = sigquit_handler;
    sigaction(SIGQUIT, &sa, NULL);
}

char	*getpath(t_list *data)
{
	char	**splitpath;
	char	*path;
	char	*temp;
	char	*joinedpath;
	int		i;
	int		j;

	i = 0;
	path = getenv("PATH");
	if (path == NULL)
		path = "/usr/local/bin:/usr/bin:/bin";
	splitpath = ft_split(path, ':');
	if (!splitpath)
		return (NULL);
	while (splitpath[i++] != NULL)
	{
		temp = ft_strjoin(splitpath[i], "/");
		joinedpath = ft_strjoin(temp, data->commandsarr[0]);
		free(temp);
		if (access(joinedpath, X_OK) == 0)
		{
			while (splitpath[i])
				free(splitpath[i++]);
			free(splitpath);
			return (joinedpath);
		}
		free(joinedpath);
	}
	j = 0;
	while (splitpath[j])
		free(splitpath[j++]);
	free(splitpath);
	return (NULL);
}

void	getcmd(t_list *data)
{
	data->commandsarr = ft_split(data->prompt, ' ');
	if (data->commandsarr == NULL)
	{
		fprintf(stderr, "Error splitting command input.\n");
		return ;
	}
}

void	executecommands(t_list *data, char **envp)
{
	int	id;

	data->execcmds[0] = data->path;
	data->path = NULL;
	data->execcmds[1] = data->commandsarr[1];
	data->execcmds[2] = NULL;
	id = fork();
	if (id == 0)
		execve(data->execcmds[0], data->execcmds, envp);
	else
		wait(NULL);
}

int	checkempty(char *s)
{
	size_t	i;

	i = 0;
	while (s[i] == ' ' || s[i] == '\t')
		i++;
	return (i == ft_strlen(s));
}

void	ft_add_to_history(t_list *data, char *command)
{
	t_history	*new_history;
	t_history	*last;

	new_history = malloc(sizeof(t_history));
	if (!new_history)
	{
		perror("Failed to allocate memory for history");
		return ;
	}
	new_history->command = ft_strdup(command);
	new_history->next = NULL;
	new_history->prev = NULL;
	if (data->history == NULL)
		data->history = new_history;
	else
	{
		last = data->history;
		while (last->next != NULL)
			last = last->next;
		last->next = new_history;
		new_history->prev = last;
	}
}

void	ft_display_history(t_list *data)
{
	t_history	*current;
	int			count;

	current = data->history;
	count = 0;
	while (current != NULL)
	{
		printf("%d %s\n", ++count, current->command);
		current = current->next;
	}
}

int	checkdir(char *path)
{
	char cwd[PATH_MAX];

    // If no path given, default to the HOME directory
    if (path == NULL || ft_strcmp(path, "~") == 0)
	{
        path = getenv("HOME");
        if (path == NULL)
		{
            printf("cd: HOME not set\n");
            return 1;
        }
    }
	else if (ft_strcmp(path, "-") == 0)
	{
        // Implement the 'cd -' functionality to go to the previous directory
        path = getenv("OLDPWD");
        if (path == NULL)
		{
            printf("cd: OLDPWD not set\n");
            return (1);
        }
        printf("%s\n", path); // Print the new directory
    }
    // Save the current directory before changing it
    if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
        perror("cd: getcwd failed");
        return (1);
    }
    // Change to the new directory
    if (chdir(path) != 0)
	{
        perror("cd");
        return (1);
    }
    return (0);
}

void	ft_display_prompt(t_list *data, char **envp)
{
	// char	buf[1000];
	int		i;

	while (1)
	{
		data->prompt = readline("minishell> ");
		if (!data->prompt)
			break ;
		if (checkempty(data->prompt) == 0)
		{
			ft_add_to_history(data, data->prompt);
			add_history(data->prompt);
			getcmd(data);
			if (ft_strcmp(data->commandsarr[0], "cd") == 0)
				checkdir(data->commandsarr[1]);
			else if (ft_strcmp(data->commandsarr[0], "history") == 0)
				ft_display_history(data);
			else
			{
				data->path = getpath(data);
				if (data->prompt && *data->prompt)
					add_history(data->prompt);
				if (data->path)
				{
					executecommands(data, envp);
					free(data->path);
				}
				else
					printf("Command not found: %s\n", data->commandsarr[0]);
				i = 0;
				while (data->commandsarr[i])
					free(data->commandsarr[i++]);
				free(data->commandsarr);
			}
		}
		free(data->path);
		free(data->prompt);
	}
}

int	main(int argc, char **argv, char **envp)
{
	t_list	*data;

	(void)argv;
	setup_signal_handlers();
	if (argc > 1)
	{
		printf("No arguments are required for minishell\n");
		return (1);
	}
	data = malloc(sizeof(t_list));
	if (!data)
	{
		printf("Memory allocation failed\n");
		return (1);
	}
	printf("# Still testing signals, fast double press ctrl + c to quit.\n");
	ft_display_prompt(data, envp);
	free(data);
	return (0);
}
