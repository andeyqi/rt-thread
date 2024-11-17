#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "littleshell.h"
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"

#define CONSOLE_FROME "Core1# "
#define NEW_LINE      "\r\n"

#pragma section="FSymTab"

static struct littleshell_syscall * _syscall_table_begin = NULL;
static struct littleshell_syscall * _syscall_table_end = NULL;

static struct little_shell _shell;
static struct little_shell * shell = &_shell;

extern uint8_t uartgetchar(uint8_t* pdata);

void littleshell_system_function_init(const void* begin, const void* end)
{
    _syscall_table_begin = (struct littleshell_syscall*) begin;
    _syscall_table_end = (struct littleshell_syscall*) end;
}


void littleshell_system_init(void)
{
    littleshell_system_function_init(__section_begin("FSymTab"),
                               __section_end("FSymTab"));
}

struct littleshell_syscall* littleshell_syscall_lookup(const char* name)
{
    struct littleshell_syscall* index;

    for (index = _syscall_table_begin; index < _syscall_table_end; index++)
    {
        if (strcmp(index->name, name) == 0)
            return index;
    }
    return NULL;
}

static int str_common(const char *str1, const char *str2)
{
    const char *str = str1;

    while ((*str != 0) && (*str2 != 0) && (*str == *str2))
    {
        str ++;
        str2 ++;
    }

    return (str - str1);
}

static const char *shell_get_prompt(void)
{
    static char shell_prompt[MAX_CLI_ARGS_BUF_LEN + 1] = {0};
    
    strcpy(shell_prompt, CONSOLE_FROME);
    
    #if defined(DFS_USING_WORKDIR)
     extern void getcwd(char * buff, int len);
    /* get current working directory */
    getcwd(&shell_prompt[strlen(CONSOLE_FROME)], MAX_CLI_ARGS_BUF_LEN - strlen(shell_prompt));
    #endif
    
    strcat(shell_prompt, ">");

    return shell_prompt;
}


static void shell_auto_complete(char *prefix)
{
    struct littleshell_syscall* index;
    const char *name_ptr, *cmd_name;
    int length, min_length;

    min_length = 0;
    name_ptr = NULL;
    
    printf(NEW_LINE);
    if (*prefix == '\0')
    {
#if ( LTSHELL_USING_SHOW_HELP == 1 )
        printf("shell commands:"NEW_LINE);
        for (index = _syscall_table_begin; index < _syscall_table_end; index++)
        {
            printf("%-16s - %s"NEW_LINE, index->name, index->desc);
        }
#endif
        printf("%s%s", shell_get_prompt(), prefix);
        return;
    }

    for (index = _syscall_table_begin; index < _syscall_table_end; index++)
    {
        if (strncmp(index->name, prefix,strlen(prefix)) == 0)
        {
            cmd_name = index->name;
            if(min_length == 0)
            {
                 /* set name_ptr */
                 name_ptr = cmd_name;
                 /* set initial length */
                 min_length = strlen(name_ptr);
            }
            length =str_common(name_ptr, cmd_name);
            if (length < min_length)
                min_length = length;
            printf("%s"NEW_LINE, cmd_name);
        }
    }
    /* auto complete string */
    if (name_ptr != NULL)
    {
        strncpy(prefix, name_ptr, min_length);
    }
    printf("%s%s", shell_get_prompt(), prefix);
    return ;
}
    
/*** Parse command line
 * return pointer to start of new command line
 */ 
static void littleshell_parse_args(const char *cmd, struct littleshell_arg *arg)
{
    char *p;

    if (arg) {
        arg->argc = 0;
        if (cmd) {
            p = arg->_buf;
            while (*cmd && arg->argc < MAX_CLI_ARGS_NUM && (p - arg->_buf < MAX_CLI_ARGS_BUF_LEN)) {
                while(*cmd && (*cmd == ' ' || *cmd == '\t'))/* Ignore spaces and backspace */
                    cmd++;

                arg->argv[arg->argc] = p;/* Write command point values */
                while (*cmd && (*cmd != ' ' && *cmd != '\t') && (p - arg->_buf < MAX_CLI_ARGS_BUF_LEN)) {/* Write command buffer */                     
                        *p++ = *cmd++;
                }
                *p++ = '\0';

                if (*(arg->argv[arg->argc]) == '\0') /* anomaly detection */
                    break;
                arg->argc++;
            }
        }
    }
}
    
/*** filter out leading and tailing spaces, discard comments
 * return pointer to start of new command line
 */
    
int littleshell_interpret(const char *line)
{
    struct littleshell_arg arg = {0};
//  const struct cli_command *cmd;

    struct littleshell_syscall* fun_index = NULL;
    littleshell_parse_args(line, &arg);

    fun_index = littleshell_syscall_lookup(arg.argv[0]);
    if(fun_index)
    {
        fun_index->func(arg.argc,arg.argv);
    }
    //printf("[%d][%s]\n",arg.argc,arg._buf);
    return 0;
}

    
/*** filter out leading and tailing spaces, discard comments
 * return pointer to start of new command line
 */
static char * littleshell_process_line(char *p)
{
    char *s;
    char *x;

    if (!p)
        return NULL;
    // skip leading spaces
    while (p && (*p == ' ' || *p == '\t'))
        p++;
    for (s = x = p; *p; x++, p++) {
        switch(*p) {
        case '\\':
            p++;
            if (*p) {
                switch(*p) {
                case 'n':
                    *x = '\n';
                    break;
                case 'r':
                    *x = '\r';
                    break;
                case 't':
                    *x = '\t';
                    break;
                case 'b':
                    *x = '\b';
                    break;
                default:
                    if (*p >= '0' && *p <= '9')
                        *x = *p - '0';
                    else
                        *x = *p;
                    break;
                }
            }
            break;
        default:
            if (*p == '\r' || *p == '\n' || *p == '#') *p = '\0';
            *x = *p;
            break;
        }
        if (*p == 0)
            break;
    }
    // trim tailing spaces
    p--;
    while (p > s && (*p == ' ' || *p == '\t'))
        *p-- = '\0';
    return s;
}   

#ifdef LTSHELL_USING_HISTORY

static void shell_handle_history(struct little_shell *shell)
{

    printf("\033[2K\r");
    printf("%s%s", shell_get_prompt(), shell->line);
}

static void shell_push_history(struct little_shell *shell)
{
    if (shell->line_position != 0)
    {
        /* push history */
        if (shell->history_count >= MAX_HISTROY_NUMS)
        {
            /* if current cmd is same as last cmd, don't push */
            if (memcmp(&shell->cmd_history[MAX_HISTROY_NUMS - 1], shell->line, MAX_CLI_ARGS_BUF_LEN))
            {
                /* move history */
                int index;
                for (index = 0; index < MAX_HISTROY_NUMS - 1; index ++)
                {
                    memcpy(&shell->cmd_history[index][0],
                           &shell->cmd_history[index + 1][0], MAX_CLI_ARGS_BUF_LEN);
                }
                memset(&shell->cmd_history[index][0], 0, MAX_CLI_ARGS_BUF_LEN);
                memcpy(&shell->cmd_history[index][0], shell->line, shell->line_position);

                /* it's the maximum history */
                shell->history_count = MAX_HISTROY_NUMS;
            }
        }
        else
        {
            /* if current cmd is same as last cmd, don't push */
            if (shell->history_count == 0 || memcmp(&shell->cmd_history[shell->history_count - 1], shell->line, MAX_CLI_ARGS_BUF_LEN))
            {
                shell->current_history = shell->history_count;
                memset(&shell->cmd_history[shell->history_count][0], 0, MAX_CLI_ARGS_BUF_LEN);
                memcpy(&shell->cmd_history[shell->history_count][0], shell->line, shell->line_position);

                /* increase count and set current history position */
                shell->history_count ++;
            }
        }
    }
    shell->current_history = shell->history_count;
}

#endif
/*** read a line datas from command line.
 */

void littleshell_main_entry(void *pvParameters)
{ 
    char *p;
    littleshell_system_init();

    #if defined(DFS_USING_WORKDIR)
     extern void fs_init(void);
     fs_init();
    #endif
    
    printf("%s",shell_get_prompt());
    while(1)
    {    
        uint8_t ch = 100;
        //if (linelen >= sizeof(line))
        //    continue;
        if(uartgetchar(&ch))
        {
            /*
             * handle control key
             * up key  : 0x1b 0x5b 0x41
             * down key: 0x1b 0x5b 0x42
             * right key:0x1b 0x5b 0x43
             * left key: 0x1b 0x5b 0x44
             */
            if (ch == 0x1b)
            {
                shell->stat = WAIT_SPEC_KEY;
                continue;
            }
            else if (shell->stat == WAIT_SPEC_KEY)
            {
                if (ch == 0x5b)
                {
                    shell->stat = WAIT_FUNC_KEY;
                    continue;
                }
    
                shell->stat = WAIT_NORMAL;
            }
            else if (shell->stat == WAIT_FUNC_KEY)
            {
                shell->stat = WAIT_NORMAL;
    
                if (ch == 0x41) /* up key */
                {
#ifdef  LTSHELL_USING_HISTORY
                    /* prev history */
                    if (shell->current_history > 0)
                        shell->current_history --;
                    else
                    {
                        shell->current_history = 0;
                        continue;
                    }
    
                    /* copy the history command */
                    memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                           MAX_CLI_ARGS_BUF_LEN);
                    shell->line_curpos = shell->line_position = strlen(shell->line);
                    shell_handle_history(shell);
#endif
                    continue;
                }
                else if (ch == 0x42) /* down key */
                {
#ifdef LTSHELL_USING_HISTORY
                    /* next history */
                    if (shell->current_history < shell->history_count - 1)
                        shell->current_history ++;
                    else
                    {
                        /* set to the end of history */
                        if (shell->history_count != 0)
                            shell->current_history = shell->history_count - 1;
                        else
                            continue;
                    }
    
                    memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                           MAX_CLI_ARGS_BUF_LEN);
                    shell->line_curpos = shell->line_position = strlen(shell->line);
                    shell_handle_history(shell);
#endif
                    continue;
                }
                else if (ch == 0x44) /* left key */
                {
                    if (shell->line_curpos)
                    {
                        printf("\b");
                        shell->line_curpos --;
                    }
    
                    continue;
                }
                else if (ch == 0x43) /* right key */
                {
                    if (shell->line_curpos < shell->line_position)
                    {
                        printf("%c", shell->line[shell->line_curpos]);
                        shell->line_curpos ++;
                    }
    
                    continue;
                }
            }

            /* received null or error */
            if (ch == '\0' || ch == 0xFF) continue;
            /* handle tab key */
            else if (ch == '\t')
            {
                int i;
                /* move the cursor to the beginning of line */
                for (i = 0; i < shell->line_curpos; i++)
                    printf("\b");

                /* auto complete */
                shell_auto_complete(&shell->line[0]);
                /* re-calculate position */
                shell->line_curpos = shell->line_position = strlen(shell->line);

                continue;
            }
            /* handle backspace key */
            else if (ch == 0x7f || ch == 0x08)
            {
                /* note that shell->line_curpos >= 0 */
                if (shell->line_curpos == 0)
                    continue;

                shell->line_position--;
                shell->line_curpos--;

                if (shell->line_position > shell->line_curpos)
                {
                    int i;

                    memmove(&shell->line[shell->line_curpos],
                               &shell->line[shell->line_curpos + 1],
                               shell->line_position - shell->line_curpos);
                    shell->line[shell->line_position] = 0;

                    printf("\b%s  \b", &shell->line[shell->line_curpos]);

                    /* move the cursor to the origin position */
                    for (i = shell->line_curpos; i <= shell->line_position; i++)
                        printf("\b");
                }
                else
                {
                    printf("\b \b");
                    shell->line[shell->line_position] = 0;
                }

                continue;
            }

            /* handle end of line, break */
            if (ch == '\r' || ch == '\n')
            {
                if (shell->line_position != 0) 
                {
                    shell_push_history(shell);
                    p = littleshell_process_line(shell->line);
                    if (*p)
                    {
                        printf(NEW_LINE);

                        littleshell_interpret(p);
                        //printf("[%s]\n",p);
                        //TODO deal with strings,end of deal echo console string
                        
                    }
                }
                printf(NEW_LINE);
                printf("%s",shell_get_prompt());

                memset(shell->line, 0, sizeof(shell->line));
                shell->line_curpos = shell->line_position = 0;
                continue;
            }

            /* it's a large line, discard it */
            if (shell->line_position >= MAX_CLI_ARGS_BUF_LEN)
                shell->line_position = 0;

            /* normal character */
            if (shell->line_curpos < shell->line_position)
            {
                int i;

                memmove(&shell->line[shell->line_curpos + 1],
                           &shell->line[shell->line_curpos],
                           shell->line_position - shell->line_curpos);
                shell->line[shell->line_curpos] = ch;
                printf("%s", &shell->line[shell->line_curpos]);

                /* move the cursor to new position */
                for (i = shell->line_curpos; i < shell->line_position; i++)
                    printf("\b");
            }
            else
            {
                shell->line[shell->line_position] = ch;
                printf("%c", ch);
            }

            ch = 0;
            shell->line_position ++;
            shell->line_curpos++;
            if (shell->line_position >= MAX_CLI_ARGS_BUF_LEN)
            {
                /* clear command line */
                shell->line_position = 0;
                shell->line_curpos = 0;
            }   
        }
        else
        {
            vTaskDelay(10);
        }
    }   
}