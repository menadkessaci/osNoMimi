#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> //new included library for chdir
#include <sys/stat.h> //new included library for stat
#include "shellmemory.h"
#include "shell.h"

//PCB Strcuture to hold the process information - also implements the Queue
struct PCB {
    int pid;           // Process ID
    char *script_addr; // Address of script in shell memory
    int script_len;    // Length of script in shell memory
    int current_instr; // Current instruction to execute
	struct PCB* next_pcb; //Pointer to next PCB
	int numberOfInstructions; //Number of inscructions in the process
	int isHead; //Boolean isHead or notHead
};

//Sort for array
void sort(int x, int y, int z, int* arr) {
    int temp;
    if (x > y) {
        temp = x;
        x = y;
        y = temp;
    }
    if (y > z) {
        temp = y;
        y = z;
        z = temp;
    }
    if (x > y) {
        temp = x;
        x = y;
        y = temp;
    }
    arr[0] = x;
    arr[1] = y;
    arr[2] = z;
}

int MAX_ARGS_SIZE = 7;

int badcommand(){
	printf("%s\n", "Unknown Command");
	return 1;
}

int badcommandTooManyTokens(){
	printf("%s\n", "Bad command: Too many tokens");
	return 2;
}

int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad command: File not found");
	return 3;
}

int badcommandMkdir(){
	printf("%s\n", "Bad command: my_mkdir");
	return 4;
}

int badcommandCd(){
	printf("%s\n", "Bad command: my_cd");
	return 5;
}

int badCommandSameFile()
{
	printf("%s\n", "Bad command: same file name");
	return 6;
}

int help();
int quit();
int set(char* var, char* value);
int print(char* var);
int run(char* script);
int exec(char* command_args[], int args_size); //added exec
int echo(char* var);
int my_ls();
int my_mkdir(char* dirname);
int my_touch(char* filename);
int my_cd(char* dirname);

// Interpret commands and their arguments
int interpreter(char* command_args[], int args_size){
	if (args_size < 1){
		return badcommand();
	}

	if (args_size > MAX_ARGS_SIZE){
		return badcommandTooManyTokens();
	}

	for (int i=0; i<args_size; i++){ //strip spaces new line etc
		command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	}

	if (strcmp(command_args[0], "help")==0){
	    //help
	    if (args_size != 1) return badcommand();
	    return help();
	
	} else if (strcmp(command_args[0], "quit")==0) {
		//quit
		if (args_size != 1) return badcommand();
		return quit();

	} else if (strcmp(command_args[0], "set")==0) {
		//set
		if (args_size < 3) return badcommand();	
		int total_len = 0;
		for(int i=2; i<args_size; i++){
			total_len+=strlen(command_args[i])+1;
		}
		char *value = (char*) calloc(1, total_len);
		char spaceChar = ' ';
		for(int i = 2; i < args_size; i++){
			strncat(value, command_args[i], strlen(command_args[i]));
			if(i < args_size-1){
				strncat(value, &spaceChar, 1);
			}
		}
		int errCode = set(command_args[1], value);
		free(value);
		return errCode;
	
	} else if (strcmp(command_args[0], "print")==0) {
		if (args_size != 2) return badcommand();
		return print(command_args[1]);
	
	} else if (strcmp(command_args[0], "run")==0) {
		if (args_size != 2) return badcommand();
		return run(command_args[1]);

	}else if (strcmp(command_args[0], "exec")==0) {
		//exec script policy
		if(args_size < 3 || args_size > 5)
		{
			return badcommand();
		}
		else if(args_size == 3) //Should have same behavior that Run command, just use run.
		{
			return run(command_args[1]);
		}
		//exec command_args[scripts] policy
		else
		{
			return exec(command_args, args_size);
		}
	
	} else if (strcmp(command_args[0], "echo")==0){
		if (args_size > 2) return badcommand();
		return echo(command_args[1]);

	} else if (strcmp(command_args[0], "my_ls")==0) {
		if (args_size > 1) return badcommand();
		return my_ls();
	
	} else if (strcmp(command_args[0], "my_mkdir")==0) {
		if (args_size > 2) return badcommand();
		return my_mkdir(command_args[1]);
	
	} else if (strcmp(command_args[0], "my_touch")==0) {
		if (args_size > 2) return badcommand();
		return my_touch(command_args[1]);
	
	} else if (strcmp(command_args[0], "my_cd")==0) {
		if (args_size > 2) return badcommand();
		return my_cd(command_args[1]);
	
	} else return badcommand();
}

int help(){

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
	printf("%s\n", help_string);
	return 0;
}

int quit(){
	printf("%s\n", "Bye!");
	exit(0);
}

int set(char* var, char* value){
	char *link = "=";
	char buffer[1000];
	strcpy(buffer, var);
	strcat(buffer, link);
	strcat(buffer, value);
	mem_set_value(var, value);
	return 0;

}

int print(char* var){
	char *value = mem_get_value(var);
	if(value == NULL) value = "\n";
	printf("%s\n", value); 
	return 0;
}

int run(char* script){
	//Added
	int instructionNumber = 100;
	int count = 0;

	int errCode = 0;
	char line[1000];
	FILE *p = fopen(script,"rt");  // the program is in a file
	if(p == NULL) return badcommandFileDoesNotExist();
	fgets(line,999,p);
	//count += 1;
	while(1){

		//errCode = parseInput(line);	// which calls interpreter()
		count += 1;

		//Encode Instruction Number 
		instructionNumber += 1;
		char instrNum[3]; //Buffer for instruction number
		snprintf(instrNum, 4, "%d", instructionNumber); //Encode instruction number into a string (into buffer)

		//Set instructionNum-InstructionPair in Memory
		set(instrNum, line);

		memset(line, 0, sizeof(line));		//Set all characters of line to '0'.
		if(feof(p))
		{
			//Take care of instruction number here (and PID).
			break;
		}
		fgets(line,999,p);
	}
    fclose(p);

	//Create PCB for loaded file
	struct PCB *new_pcb = malloc(sizeof(struct PCB)); // Allocate memory for the new PCB
	// Initialize the fields of the new PCB
	new_pcb->pid = 100;
	new_pcb->script_addr = mem_get_value("101");
	new_pcb->script_len = count;
	new_pcb->current_instr = 1;
	new_pcb->next_pcb = NULL;
	new_pcb->numberOfInstructions = count;
	new_pcb->isHead = 1;


	//Scheduler here - loading file done - PCB created and alone in Queue
	struct PCB* queue = new_pcb; //Must be done in a while loop - go through all PCBs
	while(queue->current_instr <= queue->numberOfInstructions)
	{
		char instruction[4];
		snprintf(instruction, 4, "%d", queue->pid + queue->current_instr);
		
		char* buff;
		buff = mem_get_value(instruction);
		//printf("%s\n", buff);
		//printf("%s\n", mem_get_value(instruction));
		
		if(strcmp(buff, "quit") == 0)
		{
			//printf("%s", "We are Here!");
			buff = "quit\n";
			parseInput(buff);
			break;
		}

		//echo bug at end of script - found during exec development
		if(strstr(script, ".txt") != NULL && strstr(buff, "echo") != NULL && queue->current_instr == queue->numberOfInstructions) //Only at end of script
		{
			//printf("%s", "We are Here!");
			strncat(buff, "\n", 2);
			parseInput(buff);
			break;
		}

		//Catch echo bug in non-.txt files
		if(strstr(script, ".txt") == NULL && strstr(buff, "echo") != NULL && queue->current_instr == queue->numberOfInstructions) //Only at end of script
		{
			//printf("%s", "We are Here!");
			strncat(buff, "\n", 2);
			parseInput(buff);
			break;
		}

		//printf("%s\n", buff);
		parseInput(buff);
		queue->current_instr += 1;
	}
	
	free(new_pcb); //Removing script from memory

	return errCode;
}

int exec(char* command_args[], int args_size)
{
	if(args_size == 4 && strcmp(command_args[3], "FCFS") == 0)
	{
		if(strcmp(command_args[1], command_args[2]) == 0)
		{
			badCommandSameFile();
			return 0;
		}

		//Execute 2 scripts according to FCFS policy
		run(command_args[1]);
		run(command_args[2]);
	}

	if(args_size == 5 && strcmp(command_args[4], "FCFS") == 0)
	{
		if(strcmp(command_args[1], command_args[2]) == 0 || strcmp(command_args[2], command_args[3]) == 0
		|| strcmp(command_args[1], command_args[3]) == 0)
		{
			badCommandSameFile();
			return 0;
		}
		
		//Execute 3 scripts according to FCFS policy
		run(command_args[1]);
		run(command_args[2]);
		run(command_args[3]);
	}

	if(args_size == 4 && strcmp(command_args[3], "SJF") == 0)
	{
		if(strcmp(command_args[1], command_args[2]) == 0)
		{
			badCommandSameFile();
			return 0;
		}
		
		//Execute 2 scripts according to SJF policy
		//printf("%s\n", "Executing SJF on 2 scripts!");

		int c1 = 0; //Count of number of instructions
		int c2 = 0;

		char line[1000];
		FILE *p = fopen(command_args[1],"rt"); //Open first script
		if(p == NULL) return badcommandFileDoesNotExist();
		fgets(line,999,p);
		while(1){
			c1 += 1; //Count line

			memset(line, 0, sizeof(line));		//Set all characters of line to '0'.
			if(feof(p))
			{
				//Take care of instruction number here (and PID).
				break;
			}
			fgets(line,999,p);
		}
		fclose(p);

		//Same counting for second file
		FILE *p2 = fopen(command_args[2],"rt"); //Open first script
		if(p2 == NULL) return badcommandFileDoesNotExist();
		fgets(line,999,p2);
		while(1){
			c2 += 1; //Count line

			memset(line, 0, sizeof(line));		//Set all characters of line to '0'.
			if(feof(p2))
			{
				//Take care of instruction number here (and PID).
				break;
			}
			fgets(line,999,p2);
		}
		fclose(p2);

		//Compare the counts and run scripts in the right order
		if(c1 > c2)
		{
			run(command_args[2]);
			run(command_args[1]);
		}
		else if(c2 > c1)
		{
			run(command_args[1]);
			run(command_args[2]);
		}
		else //Equal so run whatever
		{
			run(command_args[1]);
			run(command_args[2]);
		}

	}

	if(args_size == 5 && strcmp(command_args[4], "SJF") == 0)
	{
		if(strcmp(command_args[1], command_args[2]) == 0 || strcmp(command_args[2], command_args[3]) == 0
		|| strcmp(command_args[1], command_args[3]) == 0)
		{
			badCommandSameFile();
			return 0;
		}
		
		//Execute 3 scripts according to SJF policy
		//printf("%s\n", "Executing SJF on 3 scripts!");

		int c1 = 0; //Count of number of instructions
		int c2 = 0;
		int c3 = 0;

		char line[1000];
		FILE *p = fopen(command_args[1],"rt"); //Open first script
		if(p == NULL) return badcommandFileDoesNotExist();
		fgets(line,999,p);
		while(1){
			c1 += 1; //Count line

			memset(line, 0, sizeof(line));		//Set all characters of line to '0'.
			if(feof(p))
			{
				//Take care of instruction number here (and PID).
				break;
			}
			fgets(line,999,p);
		}
		fclose(p);
		//Set PCB for script 1


		//Same counting for second file
		FILE *p2 = fopen(command_args[2],"rt"); //Open first script
		if(p2 == NULL) return badcommandFileDoesNotExist();
		fgets(line,999,p2);
		while(1){
			c2 += 1; //Count line

			memset(line, 0, sizeof(line));		//Set all characters of line to '0'.
			if(feof(p2))
			{
				//Take care of instruction number here (and PID).
				break;
			}
			fgets(line,999,p2);
		}
		fclose(p2);

		//Same counting for third file
		FILE *p3 = fopen(command_args[3],"rt"); //Open first script
		if(p3 == NULL) return badcommandFileDoesNotExist();
		fgets(line,999,p3);
		while(1){
			c3 += 1; //Count line

			memset(line, 0, sizeof(line));		//Set all characters of line to '0'.
			if(feof(p3))
			{
				//Take care of instruction number here (and PID).
				break;
			}
			fgets(line,999,p3);
		}
		fclose(p3);

		//Order the counts into an array of integers
		int sorted[3];
		sort(c1, c2, c3, sorted);

		//Run the scripts in the right order
		for(int i = 0; i < 3; i++)
		{
			if(sorted[i] == c1)
			{
				c1 = -1;
				run(command_args[1]);
				continue;
			}
			if(sorted[i] == c2)
			{
				c2 = -1;
				run(command_args[2]);
				continue;
			}
			if(sorted[i] == c3)
			{
				c3 = -1;
				run(command_args[3]);
				continue;
			}
		}
	}

	if(args_size == 4 && strcmp(command_args[3], "RR") == 0)
	{
		if(strcmp(command_args[1], command_args[2]) == 0)
		{
			badCommandSameFile();
			return 0;
		}
		
		//Execute 2 scripts according to RR policy
		printf("%s\n", "Executing RR on 2 scripts!");

		//Loading files in Memory:
		//FILE 1:
		//Added
		int instructionNumber = 100;
		int count = 0;

		int errCode = 0;
		char line[1000];
		FILE *p = fopen(command_args[1],"rt");  // the program is in a file
		if(p == NULL) return badcommandFileDoesNotExist();
		fgets(line,999,p);
		//count += 1;
		while(1){

			//errCode = parseInput(line);	// which calls interpreter()
			count += 1;

			//Encode Instruction Number 
			instructionNumber += 1;
			char instrNum[3]; //Buffer for instruction number
			snprintf(instrNum, 4, "%d", instructionNumber); //Encode instruction number into a string (into buffer)

			//Set instructionNum-InstructionPair in Memory
			set(instrNum, line);

			memset(line, 0, sizeof(line));		//Set all characters of line to '0'.
			if(feof(p))
			{
				//Take care of instruction number here (and PID).
				break;
			}
			fgets(line,999,p);
		}
		fclose(p);

		//Create PCB for loaded file
		struct PCB *new_pcb = malloc(sizeof(struct PCB)); // Allocate memory for the new PCB
		// Initialize the fields of the new PCB
		new_pcb->pid = 100;
		new_pcb->script_addr = mem_get_value("101");
		new_pcb->script_len = count;
		new_pcb->current_instr = 1;
		new_pcb->next_pcb = NULL;
		new_pcb->numberOfInstructions = count;
		new_pcb->isHead = 1;

		//FILE 2:
		//Added
		int instructionNumber2 = 200;
		int count2 = 0;

		int errCode2 = 0;
		char line2[1000];
		FILE *p2 = fopen(command_args[2],"rt");  // the program is in a file
		if(p2 == NULL) return badcommandFileDoesNotExist();
		fgets(line2,999,p2);
		//count += 1;
		while(1){

			//errCode = parseInput(line);	// which calls interpreter()
			count2 += 1;

			//Encode Instruction Number 
			instructionNumber2 += 1;
			char instrNum2[3]; //Buffer for instruction number
			snprintf(instrNum2, 4, "%d", instructionNumber2); //Encode instruction number into a string (into buffer)

			//Set instructionNum-InstructionPair in Memory
			set(instrNum2, line2);

			memset(line2, 0, sizeof(line2));		//Set all characters of line to '0'.
			if(feof(p2))
			{
				//Take care of instruction number here (and PID).
				break;
			}
			fgets(line2,999,p2);
		}
		fclose(p2);

		//Create PCB for loaded file2
		struct PCB *new_pcb2 = malloc(sizeof(struct PCB)); // Allocate memory for the new PCB
		// Initialize the fields of the new PCB
		new_pcb2->pid = 200;
		new_pcb2->script_addr = mem_get_value("201");
		new_pcb2->script_len = count2;
		new_pcb2->current_instr = 1;
		new_pcb2->next_pcb = new_pcb;
		new_pcb2->numberOfInstructions = count2;
		new_pcb2->isHead = 0;

		//LINKING PCBs
		new_pcb->next_pcb = new_pcb2;

		//Setting timer & total number of instructions in both PCBs
		int timer = 0;
		int totalInstructions = new_pcb->numberOfInstructions + new_pcb2->numberOfInstructions;
		int processed = 0;

		//RR with timer = 2
		struct PCB *curr = new_pcb;
		do
		{
			/* code */			
			//Get next 1-instruction
			char instruction[4];
			snprintf(instruction, 4, "%d", curr->pid + curr->current_instr);
			char* buff;
			buff = mem_get_value(instruction);
			
			if(strcmp(buff, "quit") == 0)
			{
				buff = "quit\n";
				parseInput(buff);
				processed += 1;
				break;
			}

			
			//echo bug at end of script - found during exec development
			if(/*strstr(buff, "echo") != NULL &&*/ curr->current_instr == curr->numberOfInstructions) //Only at end of script
			{
				strncat(buff, "\n", 2);
				parseInput(buff);
				processed += 1;
				curr->current_instr += 1;

				curr->next_pcb->isHead = 1; //Change head if not already head
				curr->next_pcb->next_pcb = curr->next_pcb; //Make it point to itself
				curr = curr->next_pcb;
				continue; //Next iteration with new current and new circular behavior
			}

			//Parse it (execute) if the instruction exists
			if(buff != NULL)
			{
				parseInput(buff);
				processed += 1;
				curr->current_instr += 1;
			}
			else //There is no more instruction in the current PCB
			{
				curr = curr->next_pcb;
				curr->next_pcb = curr;
				
				//curr->next_pcb->isHead = 1; //Change head if not already head
				//curr->next_pcb->next_pcb = curr->next_pcb; //Make it point to itself
				//curr = curr->next_pcb;
				continue; //Next iteration with new current and new circular behavior
			}

			//Get next 2-instruction
			char instruction2[4];
			snprintf(instruction2, 4, "%d", curr->pid + curr->current_instr);
			char* buff2;
			buff2 = mem_get_value(instruction2);
			
			
			if(strcmp(buff2, "quit") == 0)
			{
				buff2 = "quit\n";
				parseInput(buff2);
				processed += 1;
				break;
			}

			
			//echo bug at end of script - found during exec development
			if(/*strstr(buff2, "echo") != NULL && */curr->current_instr == curr->numberOfInstructions) //Only at end of script
			{
				strncat(buff2, "\n", 2);
				parseInput(buff2);
				processed += 1;
				curr->current_instr += 1;

				curr->next_pcb->isHead = 1; //Change head if not already head
				curr->next_pcb->next_pcb = curr->next_pcb; //Make it point to itself
				curr = curr->next_pcb;
				continue; //Next iteration with new current and new circular behavior
			}


			//Parse it (execute) if the instruction exists
			if(buff2 != NULL)
			{
				parseInput(buff2);
				processed += 1;
				curr->current_instr += 1;

			}
			else //There is no more instruction in the current PCB
			{
				curr = curr->next_pcb;
				curr->next_pcb = curr;
				//curr->next_pcb->isHead = 1; //Change head if not already head
				//curr->next_pcb->next_pcb = curr->next_pcb; //Make it point to itself
				//curr = curr->next_pcb;
				continue; //Next iteration with new current and new circular behavior
			}

			curr = curr->next_pcb;

		} while (processed < totalInstructions);
	}

	if(args_size == 5 && strcmp(command_args[4], "RR") == 0)
	{
		if(strcmp(command_args[1], command_args[2]) == 0 || strcmp(command_args[2], command_args[3]) == 0
		|| strcmp(command_args[1], command_args[3]) == 0)
		{
			badCommandSameFile();
			return 0;
		}
		
		//Execute 3 scripts according to RR policy
		//printf("%s\n", "Executing RR on 3 scripts!");

		//Load 3 files in Memory
		
		//FILE 1:
		//Added
		int instructionNumber = 100;
		int count = 0;

		int errCode = 0;
		char line[1000];
		FILE *p = fopen(command_args[1],"rt");  // the program is in a file
		if(p == NULL) return badcommandFileDoesNotExist();
		fgets(line,999,p);
		//count += 1;
		while(1){

			//errCode = parseInput(line);	// which calls interpreter()
			count += 1;

			//Encode Instruction Number 
			instructionNumber += 1;
			char instrNum[3]; //Buffer for instruction number
			snprintf(instrNum, 4, "%d", instructionNumber); //Encode instruction number into a string (into buffer)

			//Set instructionNum-InstructionPair in Memory
			set(instrNum, line);

			memset(line, 0, sizeof(line));		//Set all characters of line to '0'.
			if(feof(p))
			{
				//Take care of instruction number here (and PID).
				break;
			}
			fgets(line,999,p);
		}
		fclose(p);

		//Create PCB for loaded file 1
		struct PCB *new_pcb = malloc(sizeof(struct PCB)); // Allocate memory for the new PCB
		// Initialize the fields of the new PCB
		new_pcb->pid = 100;
		new_pcb->script_addr = mem_get_value("101");
		new_pcb->script_len = count;
		new_pcb->current_instr = 1;
		new_pcb->next_pcb = NULL;
		new_pcb->numberOfInstructions = count;
		new_pcb->isHead = 1;

		//FILE 2:
		//Added
		int instructionNumber2 = 200;
		int count2 = 0;

		int errCode2 = 0;
		char line2[1000];
		FILE *p2 = fopen(command_args[2],"rt");  // the program is in a file
		if(p2 == NULL) return badcommandFileDoesNotExist();
		fgets(line2,999,p2);
		//count += 1;
		while(1){

			//errCode = parseInput(line);	// which calls interpreter()
			count2 += 1;

			//Encode Instruction Number 
			instructionNumber2 += 1;
			char instrNum2[3]; //Buffer for instruction number
			snprintf(instrNum2, 4, "%d", instructionNumber2); //Encode instruction number into a string (into buffer)

			//Set instructionNum-InstructionPair in Memory
			set(instrNum2, line2);

			memset(line2, 0, sizeof(line2));		//Set all characters of line to '0'.
			if(feof(p2))
			{
				//Take care of instruction number here (and PID).
				break;
			}
			fgets(line2,999,p2);
		}
		fclose(p2);

		//Create PCB for loaded file2
		struct PCB *new_pcb2 = malloc(sizeof(struct PCB)); // Allocate memory for the new PCB
		// Initialize the fields of the new PCB
		new_pcb2->pid = 200;
		new_pcb2->script_addr = mem_get_value("201");
		new_pcb2->script_len = count2;
		new_pcb2->current_instr = 1;
		new_pcb2->next_pcb = NULL;
		new_pcb2->numberOfInstructions = count2;
		new_pcb2->isHead = 0;

		//FILE 3:
		//Added
		int instructionNumber3 = 300;
		int count3 = 0;

		int errCode3 = 0;
		char line3[1000];
		FILE *p3 = fopen(command_args[3],"rt");  // the program is in a file
		if(p3 == NULL) return badcommandFileDoesNotExist();
		fgets(line3,999,p3);
		//count += 1;
		while(1){

			//errCode = parseInput(line);	// which calls interpreter()
			count3 += 1;

			//Encode Instruction Number 
			instructionNumber3 += 1;
			char instrNum3[3]; //Buffer for instruction number
			snprintf(instrNum3, 4, "%d", instructionNumber3); //Encode instruction number into a string (into buffer)

			//Set instructionNum-InstructionPair in Memory
			set(instrNum3, line3);

			memset(line3, 0, sizeof(line3));		//Set all characters of line to '0'.
			if(feof(p3))
			{
				//Take care of instruction number here (and PID).
				break;
			}
			fgets(line3,999,p3);
		}
		fclose(p3);

		//Create PCB for loaded file3
		struct PCB *new_pcb3 = malloc(sizeof(struct PCB)); // Allocate memory for the new PCB
		// Initialize the fields of the new PCB
		new_pcb3->pid = 300;
		new_pcb3->script_addr = mem_get_value("301");
		new_pcb3->script_len = count3;
		new_pcb3->current_instr = 1;
		new_pcb3->next_pcb = NULL;
		new_pcb3->numberOfInstructions = count3;
		new_pcb3->isHead = 0;

		//Link PCBs accordingly (circular implementation)
		new_pcb->next_pcb = new_pcb2;
		new_pcb2->next_pcb = new_pcb3;
		new_pcb3->next_pcb = new_pcb;

		//Run parsing of instructions and end of files handling
		struct PCB *curr = new_pcb;
		int processed2 = 0;
		int totalInstructions2 = new_pcb->numberOfInstructions + new_pcb2->numberOfInstructions + new_pcb3->numberOfInstructions;
		int numPCBsLeft = 3;
		
		do
		{
			/* code */			
			//Get next 1-instruction
			char instruction[4];
			snprintf(instruction, 4, "%d", curr->pid + curr->current_instr);
			char* buff;
			buff = mem_get_value(instruction);
			
			//Parse it (execute) if the instruction exists
			if(buff != NULL && strcmp(buff, "quit") != 0)
			{
				parseInput(buff);
				processed2 += 1;
				curr->current_instr += 1;
			}
			if(strcmp(buff, "quit") == 0)
			{
				buff = "quit\n";
				parseInput(buff);
				processed2 += 1;
				break;
			}
			

			//echo bug at end of script - found during exec development
			if(curr->current_instr == curr->numberOfInstructions) //Only at end of script
			{
				//strncat(buff, "\n", 2);
				//parseInput(buff);
				//processed2 += 1;
				//curr->current_instr += 1;

				if(numPCBsLeft == 3)
				{
					curr->next_pcb->next_pcb->next_pcb = curr->next_pcb;
					curr = curr->next_pcb;
					numPCBsLeft = 2;
					continue;
				}
				if(numPCBsLeft == 2)
				{
					curr->next_pcb->next_pcb = curr->next_pcb;
					curr = curr->next_pcb;
					numPCBsLeft = 1;
					continue;
				}
				//continue; //Next iteration with new current and new circular behavior
			}


			//Get next 2-instruction
			char instruction2[4];
			snprintf(instruction2, 4, "%d", curr->pid + curr->current_instr);
			char* buff2;
			buff2 = mem_get_value(instruction2);
			
			//Parse it (execute) if the instruction exists
			if(buff2 != NULL && strcmp(buff2, "quit") != 0)
			{
				parseInput(buff2);
				processed2 += 1;
				curr->current_instr += 1;

			}
			if(strcmp(buff2, "quit") == 0)
			{
				buff2 = "quit\n";
				parseInput(buff2);
				processed2 += 1;
				break;
			}
			
			//echo bug at end of script - found during exec development
			if(curr->current_instr == curr->numberOfInstructions) //Only at end of script
			{
				//strncat(buff2, "\n", 2);
				//parseInput(buff2);
				//processed2 += 1;
				//curr->current_instr += 1;

				if(numPCBsLeft == 3)
				{
					curr->next_pcb->next_pcb->next_pcb = curr->next_pcb;
					curr = curr->next_pcb;
					numPCBsLeft = 2;
					continue;
				}
				if(numPCBsLeft == 2)
				{
					curr->next_pcb->next_pcb = curr->next_pcb;
					curr = curr->next_pcb;
					numPCBsLeft = 1;
					continue;
				}
				//continue; //Next iteration with new current and new circular behavior
			}

			curr = curr->next_pcb;

		} while (processed2 < totalInstructions2 /*|| numPCBsLeft > 0*/);

	}

	if(args_size == 4 && strcmp(command_args[3], "AGING") == 0)
	{
		if(strcmp(command_args[1], command_args[2]) == 0)
		{
			badCommandSameFile();
			return 0;
		}
		
		//Execute 2 scripts according to AGING policy
		printf("%s\n", "Executing AGING on 2 scripts!");
	}

	if(args_size == 5 && strcmp(command_args[4], "AGING") == 0)
	{
		if(strcmp(command_args[1], command_args[2]) == 0 || strcmp(command_args[2], command_args[3]) == 0
		|| strcmp(command_args[1], command_args[3]) == 0)
		{
			badCommandSameFile();
			return 0;
		}
		
		//Execute 3 scripts according to AGING policy
		printf("%s\n", "Executing AGING on 3 scripts!");
	}

	//ERROR MESSAGE IF WRONG POLICIES GIVEN
	if(args_size == 4 && strcmp(command_args[3], "AGING") != 0  && strcmp(command_args[3], "RR") != 0
	&& strcmp(command_args[3], "SJF") != 0 && strcmp(command_args[3], "FCFS") != 0)
	{
		printf("ERROR: Wrong Policy Name!\n");
	}

	if(args_size == 5 && strcmp(command_args[4], "AGING") != 0 && strcmp(command_args[4], "RR") != 0
	&& strcmp(command_args[4], "SJF") != 0 && strcmp(command_args[4], "FCFS") != 0)
	{
		printf("ERROR: Wrong Policy Name!\n");
	}

	return 0;
}

int echo(char* var){
	if(var[0] == '$') print(++var);
	else printf("%s\n", var); 
	return 0; 
}

int my_ls(){
	int errCode = system("ls | sort");
	return errCode;
}

int my_mkdir(char *dirname){
	char *dir = dirname;
	if(dirname[0] == '$'){
		char *value = mem_get_value(++dirname);
		if(value == NULL || strchr(value, ' ') != NULL){
			return badcommandMkdir();
		}
		dir = value;
	}
	int namelen = strlen(dir);
	char* command = (char*) calloc(1, 7+namelen); 
	strncat(command, "mkdir ", 7);
	strncat(command, dir, namelen);
	int errCode = system(command);
	free(command);
	return errCode;
}

int my_touch(char* filename){
	int namelen = strlen(filename);
	char* command = (char*) calloc(1, 7+namelen); 
	strncat(command, "touch ", 7);
	strncat(command, filename, namelen);
	int errCode = system(command);
	free(command);
	return errCode;
}

int my_cd(char* dirname){
	struct stat info;
	if(stat(dirname, &info) == 0 && S_ISDIR(info.st_mode)) {
		//the object with dirname must exist and is a directory
		int errCode = chdir(dirname);
		return errCode;
	}
	return badcommandCd();
}
