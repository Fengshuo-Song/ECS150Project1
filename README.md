# ECS150Project1

## Group Members:
Hengze Ye\
Fengshuo Song

## Implementation:
The function “tokentocommand” changes the types of each subcommand, which is separated by “|” from string to struct Command. It takes 2 arguments: the first one is the receiver, and the second argument is the subcommand that need to be processed. We replace all spaces by ‘\0’, read each argument and store them in a 2D array. When there is a “>”, it finds the output filename and then links the file descriptor to standard output. 
The function “parse” splits a whole command by “|”, and manipulates each subcommands by the function “tokentocommand”. It takes 2 arguments: the first one is the receiver, and the second argument is the command that need to be processed.
The function “process” executes the command. It takes one argument of type struct Command, and then returns what the command returns if successful.
The function “initialization” initializes the command by assigning each term to be NULL.

## Test Methods:
We used the reference shell to test different cases of command input and compare the output to our shell. We also added several printers to in our code to see where the bugs come from.

## Sources:
We learned how to use strtok() function from the website https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
We also learned getcwd(), chdir(), strcpy() from GNU library. 
