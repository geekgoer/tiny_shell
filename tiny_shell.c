/*************************************************************************
	> File Name: main.c
	> Author: BeYC
	> Mail:
	> Created Time: 2023年12月22日 星期五 21时48分45秒
 ************************************************************************/

#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#define INIT_TEXTSIZE 64
#define SPLIT_CHAR "\r\n\a\t "
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

char * lsh_readline();
char ** lsh_split_line(char * line );
int exce(char ** tokens);
int lsh_help(char ** split_line);
int lsh_cd(char ** split_line);
int lsh_exit(char ** split_line);
int lsh_launch(char ** args);

char *buffer_str[] = {
	"cd",
	"help",
	"exit"
};

int (* added_command[]) (char **) = {
	&lsh_cd,
	&lsh_help,
	&lsh_exit
};

int lsh_cd(char ** args)
{
    if(args[1] == NULL) {
        perror("args lose : cd ");
        exit(EXIT_FAILURE);
    }
    if(chdir(args[1]) != 0) {
        perror("cd failed ");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int lsh_exit(char ** args) {
    return 1;
}

int lsh_help(char ** args) {
    printf("this is help !\n ");
    return 0;
}

char * lsh_readline() {
	char * line = NULL;
	ssize_t  size = 0 ;
	if(getline(&line,&size,stdin) == -1 ){
		perror("readline ");
	}
	return line;
}

void print(char * str)
{
    for(int i = 0;i < sizeof(str)/sizeof(char) -1; i ++) {
        printf("%c",str[i]);
    }
    puts("");
}

char ** lsh_split_line(char * line ) {
	int size = INIT_TEXTSIZE, pos = 0;
	char ** tokens = malloc(size * sizeof(char *));
	char *token, **backupToken;

	if(!tokens) {
		perror("split error ");
		exit(EXIT_FAILURE);
	}

	token = strtok(line , SPLIT_CHAR);
	while(token != NULL) {
		tokens[pos++] = token;

//        printf("token pos is %d ",pos);
//        print(tokens[pos]);

		if(pos >= size) {
			size += INIT_TEXTSIZE;
			backupToken = tokens;
			tokens = realloc(tokens,size);
			if(!tokens) {
                free(backupToken);
				perror("split memory ");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL,SPLIT_CHAR);
	}
	tokens[pos] = NULL;
	return tokens;
}

int getBufLen()
{
    return sizeof(buffer_str) / sizeof(char *);
}

int lsh_launch(char ** args)
{
    pid_t pid;
    int status ;

//    printf("args len is %ld  \n",sizeof(args)/sizeof(char *));
//    for(int i = 0;i < sizeof(args) / sizeof(char *) ; i ++ ) {
//        printf("i is %d ",i);
//        for(int j = 0; args[i][j] != 0 ; j ++) {
//            printf("%c",args[i][j]);
//        }
//    }

    pid = fork();
    if(pid == 0) {
        if(args[0] == NULL) {
            perror("launch ");
            return EXIT_FAILURE;
        }
        if(execvp(args[0],args) == -1) {
            perror("lauch error ");
            return EXIT_FAILURE;
        }
    }else if(pid < 0) {
        perror("launch pid < 0");
    }else {
        do {
            waitpid(pid,&status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 0;
}

int exce(char ** split_line) {
	if(split_line[0] == NULL) {
        fprintf(stderr,"blank command ");
		return 0;
	}

	for (int i = 0;i < getBufLen() ; i ++) {
        if(strstr(buffer_str[i],split_line[0])) {
            return added_command[i](split_line);
        }
	}

	return lsh_launch(split_line);
}


void lsh_loop() {
    int status ;
    do {
        printf(">");
        char * line = lsh_readline();
        char ** split_line = lsh_split_line(line);
        status = exce(split_line);
        free(line);
        free(split_line);
    }while(!status);
}

int main()
{
	lsh_loop();
	return 0;
}
