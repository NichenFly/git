#include "builtin.h"
#include "cache.h"
#include "config.h"
#include "parse-options.h"
#include "sequencer.h"
#include "rebase-interactive.h"

static const char * const builtin_rebase_helper_usage[] = {
	N_("git rebase--helper [<options>]"),
	NULL
};

int cmd_rebase__helper(int argc, const char **argv, const char *prefix)
{
	struct replay_opts opts = REPLAY_OPTS_INIT;
	unsigned flags = 0, keep_empty = 0, rebase_merges = 0, autosquash = 0;
	int abbreviate_commands = 0, rebase_cousins = -1;
	enum {
		CONTINUE = 1, ABORT, MAKE_SCRIPT, SHORTEN_OIDS, EXPAND_OIDS,
		CHECK_TODO_LIST, REARRANGE_SQUASH, ADD_EXEC, EDIT_TODO, PREPARE_BRANCH,
		COMPLETE_ACTION
	} command = 0;
	struct option options[] = {
		OPT_BOOL(0, "ff", &opts.allow_ff, N_("allow fast-forward")),
		OPT_BOOL(0, "keep-empty", &keep_empty, N_("keep empty commits")),
		OPT_BOOL(0, "allow-empty-message", &opts.allow_empty_message,
			N_("allow commits with empty messages")),
		OPT_BOOL(0, "rebase-merges", &rebase_merges, N_("rebase merge commits")),
		OPT_BOOL(0, "rebase-cousins", &rebase_cousins,
			 N_("keep original branch points of cousins")),
		OPT_BOOL(0, "autosquash", &autosquash,
			 N_("move commits thas begin with squash!/fixup!")),
		OPT__VERBOSE(&opts.verbose, N_("be verbose")),
		OPT_CMDMODE(0, "continue", &command, N_("continue rebase"),
				CONTINUE),
		OPT_CMDMODE(0, "abort", &command, N_("abort rebase"),
				ABORT),
		OPT_CMDMODE(0, "make-script", &command,
			N_("make rebase script"), MAKE_SCRIPT),
		OPT_CMDMODE(0, "shorten-ids", &command,
			N_("shorten commit ids in the todo list"), SHORTEN_OIDS),
		OPT_CMDMODE(0, "expand-ids", &command,
			N_("expand commit ids in the todo list"), EXPAND_OIDS),
		OPT_CMDMODE(0, "check-todo-list", &command,
			N_("check the todo list"), CHECK_TODO_LIST),
		OPT_CMDMODE(0, "rearrange-squash", &command,
			N_("rearrange fixup/squash lines"), REARRANGE_SQUASH),
		OPT_CMDMODE(0, "add-exec-commands", &command,
			N_("insert exec commands in todo list"), ADD_EXEC),
		OPT_CMDMODE(0, "edit-todo", &command,
			    N_("edit the todo list during an interactive rebase"),
			    EDIT_TODO),
		OPT_CMDMODE(0, "prepare-branch", &command,
			    N_("prepare the branch to be rebased"), PREPARE_BRANCH),
		OPT_CMDMODE(0, "complete-action", &command,
			    N_("complete the action"), COMPLETE_ACTION),
		OPT_END()
	};

	sequencer_init_config(&opts);
	git_config_get_bool("rebase.abbreviatecommands", &abbreviate_commands);

	opts.action = REPLAY_INTERACTIVE_REBASE;
	opts.allow_ff = 1;
	opts.allow_empty = 1;

	argc = parse_options(argc, argv, NULL, options,
			builtin_rebase_helper_usage, PARSE_OPT_KEEP_ARGV0);

	flags |= keep_empty ? TODO_LIST_KEEP_EMPTY : 0;
	flags |= abbreviate_commands ? TODO_LIST_ABBREVIATE_CMDS : 0;
	flags |= rebase_merges ? TODO_LIST_REBASE_MERGES : 0;
	flags |= rebase_cousins > 0 ? TODO_LIST_REBASE_COUSINS : 0;
	flags |= command == SHORTEN_OIDS ? TODO_LIST_SHORTEN_IDS : 0;

	if (rebase_cousins >= 0 && !rebase_merges)
		warning(_("--[no-]rebase-cousins has no effect without "
			  "--rebase-merges"));

	if (command == CONTINUE && argc == 1)
		return !!sequencer_continue(&opts);
	if (command == ABORT && argc == 1)
		return !!sequencer_remove_state(&opts);
	if (command == MAKE_SCRIPT && argc > 1)
		return !!sequencer_make_script(stdout, argc, argv, flags);
	if ((command == SHORTEN_OIDS || command == EXPAND_OIDS) && argc == 1)
		return !!transform_todos(flags);
	if (command == CHECK_TODO_LIST && argc == 1)
		return !!check_todo_list();
	if (command == REARRANGE_SQUASH && argc == 1)
		return !!rearrange_squash();
	if (command == ADD_EXEC && argc == 2)
		return !!sequencer_add_exec_commands(argv[1]);
	if (command == EDIT_TODO && argc == 1)
		return !!edit_todo_list(flags);
	if (command == PREPARE_BRANCH && argc == 2)
		return !!prepare_branch_to_be_rebased(&opts, argv[1]);
	if (command == COMPLETE_ACTION && argc == 6)
		return !!complete_action(&opts, flags, argv[1], argv[2], argv[3],
					 argv[4], argv[5], autosquash);

	usage_with_options(builtin_rebase_helper_usage, options);
}
