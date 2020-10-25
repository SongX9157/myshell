/*
* This code implements a simple shell program
* It supports the internal shell command "exit",
* backgrounding processes with "&", input redirection
* with "<" and output redirection with ">".
* However, this is not complete.
*gcc -c myshell.c lex.yy.c
*gcc -o main myshell.o lex.yy.o -lfl
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

extern char **getaline();

/*
 * Handle exit signals from child processes
 */
void sig_handler(int signal)
{
    int status;
    int result = wait(&status);

    //printf("Wait returned %d\n", result);
}

/*
 * Check for ampersand as the last argument
 */
int ampersand(char **args)
{
    int i;

    for(i = 1; args[i] != NULL; i++) ;

    if(args[i-1][0] == '&')
    {
        free(args[i-1]);
        args[i-1] = NULL;
        return 1;
    }
    else
    {
        return 0;
    }

    //return 0;
}


/*
 * Check for internal commands
 * Returns true if there is more to do, false otherwise
 */
int internal_command(char **args)
{
    if(strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }

    return 0;
}

/*
 * Do the command
 */
int do_command(char **args, int block,
               int input, char *input_filename,
               int output, char *output_filename)
{

    int result;
    int result2;
    pid_t child_id;
    int status;

    // Fork the child process
    child_id = fork();

    // Check for errors in fork()
    switch(child_id)
    {
    case EAGAIN:
        perror("Error EAGAIN: ");
        return;
    case ENOMEM:
        perror("Error ENOMEM: ");
        return;
    }

    if(child_id == 0)
    {

        // Set up redirection in the child process
        if(input)
            freopen(input_filename, "r", stdin);

        if(output==1)
            freopen(output_filename, "w+", stdout);
        if(output==2)
            freopen(output_filename, "a", stdout);

        // Execute the command
        result = execvp(args[0], args);

        exit(-1);
    }
    // Wait for the child process to complete, if necessary
    if(block)
    {
        //printf("Waiting for child, pid = %d\n", child_id);
        //result = waitpid(child_id, &status, 0);
        result2 = checkProcess(child_id);
        return result2;

    }
    else printf("pid = %d\n",child_id);

}

/*
 *check for 'start' running result, success or fail
 */
int checkProcess(pid_t child_id)
{
    int status;
    waitpid(child_id, &status, 0);
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }
    else
    {
        return -1;
    }
}

/*
 * Check for input redirection
 */
int redirect_input(char **args, char **input_filename)
{
    int i;
    int j;

    for(i = 0; args[i] != NULL; i++)
    {

        // Look for the <
        if(args[i][0] == '<')
        {
            free(args[i]);

            // Read the filename
            if(args[i+1] != NULL)
            {
                *input_filename = args[i+1];

            }
            else
            {
                return -1;
            }

            // Adjust the rest of the arguments in the array
            for(j = i; args[j-1] != NULL; j++)
            {
                args[j] = args[j+2];
            }
            return 1;
        }
    }

    return 0;
}

/*
 * Check for output redirection
 */
int redirect_output(char **args, char **output_filename)
{
    int i;
    int j;

    for(i = 0; args[i] != NULL; i++)
    {

        // Look for the >
        if(args[i][0] == '>')
        {
            if(args[i+1][0] == '>')
            {
                free(args[i]);
                free(args[i+1]);
                if(args[i+2] != NULL)
                {
                    *output_filename = args[i+2];
                }
                else
                {
                    return -1;
                }

                for(j = i; args[j-2] != NULL; j++)
                {
                    args[j] = args[j+3];
                }
                return 2;
            }
            else
            {
                free(args[i]);

                // Get the filename
                if(args[i+1] != NULL)
                {
                    *output_filename = args[i+1];
                }
                else
                {
                    return -1;
                }

                // Adjust the rest of the arguments in the array
                for(j = i; args[j-1] != NULL; j++)
                {
                    args[j] = args[j+2];
                }

                return 1;
            }
        }
    }

    return 0;
}

/*
 *check for the ||
 */
int or_operator(char **args, char **start, char **end)
{
    int i;
    int j;
    int k;
    int t;

    for(i = 0; args[i] != NULL; i++)
    {

        // Look for the double |
        if(args[i][0] == '|')
        {
            if(args[i+1][0] == '|')
            {
                for(k=0; k<i; k++)
                {
                    start[k]=args[k];
                }
                free(args[i]);
                free(args[i+1]);
                if(args[i+2] != NULL)
                {
                    for(k=i+2; args[k]!=NULL; k++)
                    {
                        end[t]=args[k];
                        t++;
                    }
                }
                else
                {
                    return -1;
                }

                for(j = i; args[j-2] != NULL; j++)
                {
                    args[j] = args[j+3];
                }
                return 1;
            }
        }
    }

    return 0;
}

/*
*check for the &&
*/
int and_operator(char **args, char **start, char **end)
{
    int i;
    int j;
    int k;
    int t;

    for(i = 0; args[i] != NULL; i++)
    {

        // Look for double &
        if(args[i][0] == '&')
        {
            if(args[i+1][0] == '&')
            {
                for(k=0; k<i; k++)
                {
                    start[k]=args[k];
                }
                free(args[i]);
                free(args[i+1]);
                if(args[i+2] != NULL)
                {
                    for(k=i+2; args[k]!=NULL; k++)
                    {
                        end[t]=args[k];
                        t++;
                    }
                }
                else
                {
                    return -1;
                }

                for(j = i; args[j-2] != NULL; j++)
                {
                    args[j] = args[j+3];
                }
                return 1;
            }
        }
    }

    return 0;
}

/*
*check for the ;
*/
int operator2(char **args, char **start, char **end)
{
    int i;
    int j;
    int k;
    int t;
    for(i = 0; args[i] != NULL; i++)
    {

        // Look for the ;
        if(args[i][0] == ';')
        {
            for(k=0; k<i; k++)
            {
                start[k]=args[k];
            }
            free(args[i]);
            if(args[i+1] != NULL)
            {
                for(k=i+1; args[k]!=NULL; k++)
                {
                    end[t]=args[k];
                    t++;
                }
            }
            else
            {
                return -1;
            }

            for(j = i; args[j-1] != NULL; j++)
            {
                args[j] = args[j+2];
            }
            return 1;
        }
    }
    return 0;
}


/*
 *myProcess which is used to do the redirect
 */
int myProcess(int block,char **args,char *input_filename,char *output_filename, int a)
{
    int input,output,result;

    input = redirect_input(args, &input_filename);

    output = redirect_output(args, &output_filename);
    switch(output)
    {
    case -1:
        printf("Syntax error!\n");
        return -1;
        break;
    case 0:
        break;
    case 1:
        if(a==1)
            printf("Redirecting output to: %s\n", output_filename);
        break;
    case 2:
        if(a==1)
            printf("Redirecting output1 to: %s\n", output_filename);
        break;
    }


    switch(input)
    {
    case -1:
        printf("Syntax error!\n");
        return -1;
        break;
    case 0:
        break;
    case 1:
        if(a==1)
            printf("Redirecting input from: %s\n", input_filename);
        break;
    }
    //execute the command
    result=do_command(args, block,
                      input, input_filename,
                      output, output_filename
                     );
    return result;
}

/*
 *check the number of pipe
 */
int findPipe(char **args)
{
    int i;
    int num=0;
    for(i = 0; args[i] != NULL; i++)
    {
        if((args[i][0]=='|')&&(args[i-1][0]!='|')&&args[i+1][0]!='|')
            num++;
    }
    return num;
}

/*
 * process the pipe
 */
int processPipe(int num, char **args,int out)
{
    //printf("Processing the pipe...\n");
    int k=0,t=0,i=0,j=0,h=0;
    char **start, **end, **newarg;

    int fds[2];
    int status;
    pid_t child_id;

    for(k=0; args[k]!=NULL; k++);

    start=(char**)malloc(sizeof(char *) * k);
    newarg=(char**)malloc(sizeof(char *) * k);


    //find the first command named start
    for(i = 0; args[i] != NULL; i++)
    {
        if(num==0)
        {
            //printf("only one remain\n");
            //when the number of pipe is zero which means it is the last recursion, the new args is the first command
            for(i=0; i<k; i++)
            {
                start[i]=args[i];
                start[i+1]=NULL;
            }
            break;
        }

        if((args[i][0]=='|')&&(args[i+1][0]!='|'))
        {
            for(k=0; k<i; k++)
            {
                start[k]=args[k];
                //printf("cmd 1: %s\n", start[k]);
            }
            start[k]=NULL;
            if((args[i+1] != NULL))
            {
                for(k=i+1; args[k]!=NULL; k++)
                {
                    newarg[h]=args[k];
                    //printf("%d\n",k);
                    //printf("new arg: %s\n", newarg[h]);
                    h++;
                }
                newarg[h]=NULL;

                break;
            }
            //else break;


        }
    }

    //deal with the first command

    if(start[0]!=NULL)
    {
        //printf("pipe\n");
        for(i=0; start[i]!=NULL; i++);
        //printf("%d\n",i);
        j=i;
        /*
        for(i=0; start[i]!=NULL; i++)
        {
        printf("%d\n",i);
        printf("first command: %s\n",start[i]);
        }
        */
        pipe(fds);
        child_id=fork();

        if(child_id == 0)
        {
            close(fds[0]);
            if(num==0)
                dup2(out,1);
            else
            {
                close(1);
                dup(fds[1]);
            }
            //printf("123\n");
            close(fds[1]);
            find_operator(start,0);
            exit(-1);
        }

        else
        {
            //printf(".Waiting for child, pid = %d\n", child_id);
            waitpid(child_id, &status, 0);
            close(fds[1]);
            close(0);
            dup(fds[0]);
            close(fds[0]);
        }
    }

    if(newarg[0]!=NULL) //if have another pipes
    {
        //recursion to get the first command before the first |
        processPipe(num-1,newarg,out);
    }
    //else {printf("finish\n");}
}

/*
 *find and handler the operator, then execute the command
 */
int find_operator(char **args,int a)
{
    int i,k,l,t;
    int result;
    int block;
    int op;
    int op2;
    int op3;
    char *output_filename;
    char *input_filename;
    char **start;
    char **end;


    // Check for an ampersand
    block = (ampersand(args) == 0);

    for(k=0; args[k]!=NULL; k++);
    start=(char**)malloc(sizeof(char *) * k);
    end=(char**)malloc(sizeof(char *) * k);
    //printf("test length: %d\n", k);

    op = or_operator(args,start,end);
    op2 = and_operator(args,start,end);
    op3 = operator2(args,start,end);

    //printf("test start: %s\n", start);
    /*
    printf("testop: %d\n", op);
    printf("testop: %d\n", op2);
    printf("testop: %d\n", op3);
    */
    //check for ||
    if(op==1)
    {
        //printf("contain ||\n");
        for(l=0; start[l]!=NULL; l++);
        //printf("test start length: %d\n", l);
        for(t=0; end[t]!=NULL; t++);
        //printf("test end length: %d\n", t);
        switch(op)
        {
        case -1:
            if(a==1)
            {
                printf("Syntax error!\n");
            }
            return -1;
            break;
        case 0:
            if(a==1)
            {
                printf("no || contains");
            }
            return -1;

        case 1:
            for(i=0; i<l; i++)
            {
                if(a==1)
                {
                    printf("the first part: %s\n", start[i]);
                }
            }
            for(i=0; i<t; i++)
            {
                if(a==1)
                {
                    printf("the second part: %s\n", end[i]);
                }
            }
            result=myProcess(block,start,input_filename,output_filename,a);
            //printf("result: %d\n",result);
            if(result!=0)
            {
                myProcess(block,end,input_filename,output_filename,a);
            }
            return 1;

        case 2:
            break;
        }
    }

//check for &&
    else if(op2==1)
    {
        //printf("contain &&\n");
        //op2 = and_operator(args,start,end);
        //printf("test start: %s\n", start);
        for(l=0; start[l]!=NULL; l++);
        //printf("test start length: %d\n", l);
        for(t=0; end[t]!=NULL; t++);
        //printf("test end length: %d\n", t);
        switch(op2)
        {
        case -1:
            if(a==1)
            {
                printf("Syntax error!\n");
            }
            return -1;
            break;
        case 0:
            if(a==1)
            {
                printf("no && contains");
            }
            return -1;

        case 1:
            for(i=0; i<l; i++)
            {
                if(a==1)
                {
                    printf("test start: %s\n", start[i]);
                }
            }
            for(i=0; i<t; i++)
            {
                if(a==1)
                {
                    printf("test end: %s\n", end[i]);
                }
            }
            result=myProcess(block,start,input_filename,output_filename,a);
            //printf("result: %d\n",result);
            if(result==0)
            {
                myProcess(block,end,input_filename,output_filename,a);
            }
            return 1;

        case 2:
            break;
        }
    }

//check for ;
    else if(op3==1)
    {
        //printf("contain ;\n");
        //printf("test start: %s\n", start);
        for(l=0; start[l]!=NULL; l++);
        //printf("test start length: %d\n", l);
        for(t=0; end[t]!=NULL; t++);
        //printf("test end length: %d\n", t);
        switch(op3)
        {
        case -1:
            if(a==1)
            {
                printf("Syntax error!\n");
            }
            return -1;
            break;
        case 0:
            if(a==1)
            {
                printf("no && contains");
            }
            return -1;

        case 1:
            for(i=0; i<l; i++)
            {
                if(a==1)
                {
                    printf("the first part: %s\n", start[i]);
                }
            }
            for(i=0; i<t; i++)
            {
                if(a==1)
                {
                    printf("the second part: %s\n", end[i]);
                }
            }
            result=myProcess(block,start,input_filename,output_filename,a);
            //printf("result: %d\n",result);

            myProcess(block,end,input_filename,output_filename,a);

            return 1;

        case 2:
            break;
        }
    }

    else
    {
        myProcess(block,args,input_filename,output_filename,a);
        return 1;
    }
}

/*
 * The main shell function
 */
main()
{
    char **args;
    int result;
    int block;
    int num;
    int in=dup(STDIN_FILENO);
    int out=dup(STDOUT_FILENO);
    struct sigaction newact,oldact;
    // Set up the signal handler
    //sigset(SIGCHLD, sig_handler);

    newact.sa_handler = sig_handler;
    newact.sa_flags = SA_RESTART;
    sigaction(SIGCHLD,&newact,&oldact);


    // Loop forever
    while(1)
    {
        // Print out the prompt and get the input
        printf("->");
        args = getaline();

        // No input, continue
        if(args[0] == NULL)
            continue;

        // Check for internal shell commands, such as exit
        if(internal_command(args))
            continue;

        //find the number of pipe
        num=findPipe(args);
        //printf("contains %d pipe(s)\n",num);

        if(num>0)//have pipe
        {
            processPipe(num,args,out);
            //printf("7878\n");
            dup2(in,STDIN_FILENO);
            dup2(out,STDOUT_FILENO);
        }
        else//no pipe
            find_operator(args,1);

    }
}
