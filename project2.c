#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#define DELIM " \n"
//#define BUFFER = 100;

char **tokenize(char *args);
int executeCommand(char **args);


int main(int argc, char **argv){
  
  //Path Variables
  char path[100];
  
  
  //Input Variables
  
  char *inputLine = (char *)calloc(256, sizeof(char));
  // char *inputLine;
  char **args;
  
  char *history[10] = {NULL};
  int historyStore = 0;
  int historyCount = 0;
  int historyUsed = 0; //check if up arrow was used to get a command so the last else doesnt fuck with inputLine
  
  char input[100];
  int status = 1;
  char *token;
  
  int c = 0;
  int x = 0;
  int w = 0;
  
  int i;
  
  struct termios origConfig;
  tcgetattr(0, &origConfig);
  
  struct termios newConfig = origConfig;
  
  newConfig.c_lflag &= ~(ICANON|ECHO);
  newConfig.c_cc[VMIN] = 1;
  newConfig.c_cc[VTIME] = 2;
  tcsetattr(0, TCSANOW, &newConfig);
  
  while(status)
    {
      //inputLine = (char *)calloc(256, sizeof(char));
      // Print Prompt
      getcwd(path, 100);
      printf("%s> ", path);
      
      fflush(stdout);
      
      do{
	read(0, &input, 10);
	if(input[0] == 27 /*&& input[1] == 91 && input[2] == 65)*/){
	  if(input[1] == 91 && input[2] == 65){
	    historyUsed = 1;
	    if(history[historyCount] != NULL){
	      //printf("%d\n", strlen(inputLine));
	      while(w < strlen(inputLine)){
		printf("\b \b");
		w++;
	      }
	      w = 0;
	      
	      printf("%s", history[historyCount]);
	      inputLine = strdup(history[historyCount]);
	      x = strlen(history[historyCount]);
	      
	      if(historyCount > 0){
		historyCount--;
	      }
	    }
	  }
	  
	  else if(/*input[0] == 27 &&*/ input[1] == 91 && input[2] == 66){
	    //###########################################################
	    historyUsed = 1;
	    if(history[historyCount] != NULL){
	      //printf("TEST: %s\n ::", inputLine);
	      while(w < strlen(inputLine)){
		printf("\b \b");
		w++;
	      }
	      w = 0;
	      
	      printf("%s", history[historyCount+1]);
	      inputLine = strdup(history[historyCount+1]);
	      x = strlen(history[historyCount+1]);
	      
	      if(historyCount < historyStore-2){
		historyCount++;
	      }
	      
	    }
	    //############################################################
	  }
	  else if((/*input[0] == 27 &&*/ input[1] == 91 && input[2] == 67)){
	    printf("arrow right\n");
	  }
	  else if((/*input[0] == 27 &&*/ input[1] == 91 && input[2] == 68)){
	    printf("arrow left\n");
	  }
	}
	else{
	  //printf("%c",input[0]);
	  if(/*historyUsed != 1 && */input[0] != '\n' && input[0]){
	    printf("%c", input[0]);
	    inputLine[x] = input[0];
	    x++;
	  }
	}
	//ADD IN LATER IF NEEDED
	fflush(stdout);
      } while(input[0] != '\n');
      printf("\n");
      inputLine[x] = '\0';
      x = 0;
      
      //printf("testing: %s:\n", inputLine);
      
      if(strlen(inputLine) == 0){
	continue;
      }
      

      history[historyStore] = strdup(inputLine);
      
      if(historyStore < 10){
	historyStore++;
      }
      else{
	while(historyStore > 1){
	  history[historyStore] = strdup(history[historyStore-1]);
	  if (historyStore > 1){
	    historyStore--;
	  }
	}
	historyStore = 0;
      }
      historyCount = historyStore - 1;
  
  //################################################################################
  //Tokenize Arguments
  args = tokenize(inputLine);
  
      //Check if cd directory
      if(strcmp(args[0],"cd") == 0 && args[1] != NULL){
        if(chdir(args[1]) == -1){
	  printf("No such file or directory\n");
	}
        continue;
      }
      else if(strcmp(args[0],"cd") == 0){
        //chdir("..");
        chdir(getenv("HOME"));
        continue;
      }

      //Check if Exit
      if(strcmp(args[0], "exit") == 0){
        break;
      }

      //Execute Command
      executeCommand(args);


      //memset(inputLine, 0, 256);
      inputLine[0]='\0';

      historyUsed = 0;
      c = 0;

    }
  tcsetattr(0, TCSANOW, &origConfig);
  return 0;
}

/**********************************************************************
 *                          F U N C T I O N S                         *
 **********************************************************************/

char **tokenize(char *line){

  char **tokens = malloc(256*sizeof(char *)); //BUFFER
  char *token;

  int position = 0;

  token = strtok(line, " ");

  while(token != NULL){
    int i =0;

    while(i < sizeof(token)){
      if(token[i] == '\n'){
        token[i] = '\0';
      }
      i++;
    }
   
    tokens[position] = token;
    token = strtok(NULL, DELIM);

    position++;
  }

  tokens[position] = NULL;

  return tokens;
}


int executeCommand(char **args){

  pid_t pid, wpid;
  int status;

  pid = fork();
  if(pid == 0){
    if(execvp(args[0], args) == -1){
      perror("child failed");
    }
    exit(EXIT_FAILURE);
  }
  else if(pid < 0){
    //Error Forking @_@
    perror("fork failed -_-");
  }
  else {
    // Parent
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 0;
}
