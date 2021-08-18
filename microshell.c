


int exec_cmd(t_list *cmd, char **env)
{
	pid_t	pid;
	int	ret;
	int	status;
	int	pipe_open;

	ret = EXIT_FAILURE;
	pipe_open = 0;
	if (cmd->type == TYPE_PIPE || (cmd->previous && cmd->previous->type == TYPE_PIPE))
	{
		pipe_open = 1;
		if (pipe(cmd->pipes))
			return (exit_fatal());
	}
	pid = fork();
	if (pid < 0)
		return (exit_fatal());
	else if (pid == 0)
	{
		if (cmd->type == TYPE_PIPE
				&& dup2(cmd->pipes[SIDE_IN], STDOUT) < 0)
				return (exit_fatal());
		if (cmd->previous && cmd->previous->type == TYPE_PIPE
				&& dup2(cmd->previous->pipes[SIDE_OUT], STDin) < 0)
				return (exit_fatal());
		if ((ret = execve(cmd->args[0], cmd->args, env)) < 0)
		{
			show_error("error: cannot execute ");
			show_error(cmd->args[0]);
			show_error("\n");
		}
		exit(ret);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (pipe_open)
		{
			close(cmd->pipes[SIDE_IN]);
			if (!cmd->next || cmd->type == TYPE_BREAK)
				close(cmd->pipes[SIDE_OUT]);
		}
		if (cmd->previous->type == TYPE_PIPE)
			close(cmd->previous->pipes[SIDE_OUT]);
		if (WIFEXITED(status))
			ret = WEXITSTATUS(status);
	}
	return (ret);
}

int exec_cmds(t_list **cmds, char **env)
{
	t_list	*crt;
	int		ret;

	ret = EXIT_SUCCESS;
	list_rewind(cmds);
	while (*cmds)
	{
		crt = *cmds;
		if (strcmp("cd", crt->args[0]) == 0)
		{
			ret = EXIT_SUCCESS;
			if (crt->length < 2)
				ret = show_error("error: cd: bad arguments\n");
			else if (chdir(crt->args[1]))
			{
				ret = show_error("error: cd: cannot change directory to ");
				show_error(crt->args[1]);
				show_error("\n");
			}
		}
		else
			ret = exec_cmd(crt, env);
		if (!(*cmds)->next)
			break ;
		*cmds = (*cmds)->next;
	}
	return (ret);
}

int main(int argc, char *argv[], char *env[])
{
	t_list	*cmds;
	int		i;
	int		ret;

	ret = EXIT_SUCCESS;
	cmds = NULL;
	i = 1;
	while (i < argc)
		parse_arg(&cmds, argv[i++]);
	if (cmds)
		ret = exec_cmds(&cmds, env);
	list_clear(&cmds);
	if (TEST)
		while(1);
	return (ret);
}
