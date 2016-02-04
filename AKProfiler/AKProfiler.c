//AKP_DO_NOT_PROFILE
/*
#define main _profiler_main
*/
#ifndef _AKPROFILER_H_
#define _AKPROFILER_H_
#include <acknex.h>
#include <stdio.h>
#define AKP_MAX_SCRIPT_SIZE 100000
char* AKP_DO_NOT_PROFILE = "//AKP_DO_NOT_PROFILE";
#define AKP_INSERTION_TYPE_RETURN_BEGIN 1
#define AKP_INSERTION_TYPE_RETURN_END 2
#define AKP_INSERTION_TYPE_WAIT_BEGIN 3
#define AKP_INSERTION_TYPE_WAIT_END 4
#define AKP_INSERTION_TYPE_FUNCTION_BEGIN 5
#define AKP_INSERTION_TYPE_FUNCTION_END 6
#define AKP_INSERTION_TYPE_INCLUDE_END 7
/* 
 * Runtime functions
 */
#ifdef AKP_DO_NOT_STARTUP //Only load when included the second time

// http://www.opserver.de/ubb7/ubbthreads.php?ubb=showflat&Number=284592#Post284592
long __stdcall GetTickCount();
#define PRAGMA_API GetTickCount;kernel32!GetTickCount

long AKP_first_function_call_time = 0;
var AKP_log_filehandle;

void AKP_onClose() {
	file_close(AKP_log_filehandle);
	sys_exit("Close by logger");
}

void AKP_profiler_function_start(char* functionName) {
	if(AKP_first_function_call_time == 0) {
		AKP_first_function_call_time = GetTickCount();
		AKP_log_filehandle = file_open_write("akp.log");
		on_close = AKP_onClose;
	}
	long time = GetTickCount() - AKP_first_function_call_time;
	char lineToWrite[128];
	sprintf(lineToWrite, "%s\tstart\t%i\n", functionName, time);
	file_str_write(AKP_log_filehandle, lineToWrite);
}

void AKP_profiler_function_stop(char* functionName) {
	long time = GetTickCount() - AKP_first_function_call_time;
	char lineToWrite[128];
	sprintf(lineToWrite, "%s\tstop\t%i\n", functionName, time);
	file_str_write(AKP_log_filehandle, lineToWrite);
}

void AKP_profiler_function_pause(char* functionName) {
	long time = GetTickCount() - AKP_first_function_call_time;
	char lineToWrite[128];
	sprintf(lineToWrite, "%s\tpause\t%i\n", functionName, time);
	file_str_write(AKP_log_filehandle, lineToWrite);
}

void AKP_profiler_function_resume(char* functionName) {
	long time = GetTickCount() - AKP_first_function_call_time;
	char lineToWrite[128];
	sprintf(lineToWrite, "%s\tresume\t%i\n", functionName, time);
	file_str_write(AKP_log_filehandle, lineToWrite);
}
#endif
/**
 * Parser functions
 */
typedef struct AKP_Parser_Insertion {
	int position;
	int insType;
	char functionName[64];
	struct AKP_Parser_Insertion *next;
}
AKP_Parser_Insertion;

AKP_Parser_Insertion* AKP_parser_insert(AKP_Parser_Insertion* head, int position, int insType, char* functionName) {
	AKP_Parser_Insertion* newInseration = (AKP_Parser_Insertion*) sys_malloc(sizeof(AKP_Parser_Insertion));
	newInseration->position = position;
	newInseration->insType = insType;
	strcpy(newInseration->functionName, functionName);
	newInseration->next = NULL;
	if(head == NULL) {
		head = newInseration;
	} else {
		AKP_Parser_Insertion* current = head;
		AKP_Parser_Insertion* previous = NULL;
		while(current != NULL) {
			if(current->position > newInseration->position) break;
			previous = current;
			current = current->next;
		}
		if(current == NULL) {
			//Node is the last node in the list
			previous->next = newInseration;
		} else {
			if(previous) {
				newInseration->next = previous->next;
				previous->next = newInseration;
			} else {
				newInseration->next = current;
				head = newInseration;
			}
		}
	}
	return head;
}

int AKP_is_whitespace(char character) {
	return (character == ' ' ||character == '\n' || character == '\t' || character == '\v' || character == '\f' || character == '\r');
}

int AKP_parse_whitespace_inverse(char* azCode, int currentParsePos ) {
	int i = currentParsePos;
	while(currentParsePos > 0 && AKP_is_whitespace(azCode[i])) {
		i--;
	}
	return i;
}

int AKP_parse_whitespace(char* azCode, int currentParsePos) {
	int i = currentParsePos;
	while(azCode[i] != '\0' && AKP_is_whitespace(azCode[i])) {
		i++;
	}
	return i;
}

//returns the parser position after the wait instruction. If no wait was parsed, returns the same currentParsePos
int AKP_parse_wait(char* azCode, int currentParsePos, AKP_Parser_Insertion** parseInsertList) {
	char* azWait = "wait";
	int i = 0;
	//parsed "wait" characters
	while(azCode[currentParsePos + i] != '\0' && azWait[i] != '\0') {
		if(azCode[currentParsePos + i] != azWait[i]) return currentParsePos;
		i++;
	}
	if(azWait[i] != '\0') return currentParsePos;
	//Wait was not completly parsed
	i = currentParsePos + i;
	i = AKP_parse_whitespace(azCode, i);
	//Parse parenthese open
	if(azCode[i] != '(') return currentParsePos;
	i++;
	//Parse content between parentheses
	while(azCode[i] != '\0') {
		if(azCode[i] == ')') break;
		i++;
	}
	if(azCode[i] != ')') return currentParsePos;
	i++;
	i = AKP_parse_whitespace(azCode, i);
	//Parse semicolon
	if(azCode[i] != ';') return currentParsePos;
	i++;
	*parseInsertList = AKP_parser_insert(*parseInsertList, currentParsePos, AKP_INSERTION_TYPE_WAIT_BEGIN, "");
	*parseInsertList = AKP_parser_insert(*parseInsertList, i, AKP_INSERTION_TYPE_WAIT_END, "");
	return i;
}

//returns the parser position after the return instruction. If no return was parsed, returns the same currentParsePos
int AKP_parse_return(char* azCode, int currentParsePos, AKP_Parser_Insertion** parseInsertList) {
	char* azReturn = "return";
	int i = 0;
	//parsed "wait" characters
	while(azCode[currentParsePos + i] != '\0' && azReturn[i] != '\0') {
		if(azCode[currentParsePos + i] != azReturn[i]) return currentParsePos;
		i++;
	}
	if(azReturn[i] != '\0') return currentParsePos;
	//Wait was not completly parsed
	i = currentParsePos + i;
	int parsedWhiteSpace = i;
	i = AKP_parse_whitespace(azCode, i);
	parsedWhiteSpace = (i != parsedWhiteSpace);
	//Parse parenthese open if there is
	if(azCode[i] != '(' && !parsedWhiteSpace) return currentParsePos;
	if(!parsedWhiteSpace) i++;
	//Parse content between parentheses
	while(azCode[i] != '\0') {
		if(azCode[i] == ';') break;
		i++;
	}
	if(azCode[i] != ';') return currentParsePos;
	i++;
	*parseInsertList = AKP_parser_insert(*parseInsertList, currentParsePos, AKP_INSERTION_TYPE_RETURN_BEGIN, "");
	*parseInsertList = AKP_parser_insert(*parseInsertList, i, AKP_INSERTION_TYPE_RETURN_END, "");
	return i;
}

void AKP_parse_file(char* fileName);

//returns the parser position after the include instruction. If no include was parsed, returns the same currentParsePos
int AKP_parse_include(char* azCode, int currentParsePos, AKP_Parser_Insertion** parseInsertList) {
	char* azInclude = "#include";
	int i = 0;
	//parsed "wait" characters
	while(azCode[currentParsePos + i] != '\0' && azInclude[i] != '\0') {
		if(azCode[currentParsePos + i] != azInclude[i]) return currentParsePos;
		i++;
	}
	if(azInclude[i] != '\0') return currentParsePos;
	//Wait was not completly parsed
	i = currentParsePos + i;
	i = AKP_parse_whitespace(azCode, i);
	//Parse parenthese open
	if(azCode[i] != '"') return currentParsePos;
	i++;
	//Parse content between ""
	int includeNameStart = i;
	char includeFileName[264];
	while(azCode[i] != '\0') {
		if(azCode[i] == '"') break;
		includeFileName[i - includeNameStart] = azCode[i];
		i++;
	}
	includeFileName[i - includeNameStart] = '\0';
	int includeNameEnd = i;
	if(azCode[i] != '"') return currentParsePos;
	i++;
	*parseInsertList = AKP_parser_insert(*parseInsertList, includeNameEnd, AKP_INSERTION_TYPE_INCLUDE_END, includeFileName);
	AKP_parse_file(includeFileName);
	return i;
}

int AKP_parse_function_body(char* azCode, int currentParsePos, AKP_Parser_Insertion** parseInsertList) {
	//Parses backwards
	if(azCode[currentParsePos] != '}') return currentParsePos;
	int functionBodyEndPos = currentParsePos;
	int functionBodyStartPos = -1;
	int i = currentParsePos - 1;
	int scopeCloseCount = 1;
	while(i > 0) {
		i = AKP_parse_whitespace_inverse(azCode, i);
		if(azCode[i] == '}')
		      scopeCloseCount++; else if(azCode[i] == '{') {
			scopeCloseCount--;
			if(scopeCloseCount == 0) {
				functionBodyStartPos = i + 1;
				break;
			}
		}
		i--;
	}
	if(functionBodyStartPos == -1) return currentParsePos;
	i--;
	i = AKP_parse_whitespace_inverse(azCode, i);
	if(azCode[i] != ')') return currentParsePos;
	//Parse content between parentheses
	while(i > 0) {
		if(azCode[i] == '(') break;
		i--;
	}
	if(azCode[i] != '(') return currentParsePos;
	i--;
	i = AKP_parse_whitespace(azCode, i);
	int functionNameEndPos = i + 1;
	while(i > 0 && !AKP_is_whitespace(azCode[i])) {
		i--;
	}
	int functionNameStartPos = i + 1;
	char functionName[64];
	for (i = functionNameStartPos; i < functionNameEndPos; i++) {
		functionName[i - functionNameStartPos] = azCode[i];
	}
	functionName[i - functionNameStartPos] = '\0';
	if(strcmp(functionName, "while") == 0 || strcmp(functionName, "if") == 0 || strcmp(functionName, "switch") == 0 || strcmp(functionName, "for") == 0)
	  	return currentParsePos;
	*parseInsertList = AKP_parser_insert(*parseInsertList, functionBodyStartPos, AKP_INSERTION_TYPE_FUNCTION_BEGIN, functionName);
	*parseInsertList = AKP_parser_insert(*parseInsertList, functionBodyEndPos, AKP_INSERTION_TYPE_FUNCTION_END, "");
	//Todo copy function name + ignore for/while/switch
	return currentParsePos + 1;
}

void AKP_parse_lite_c(char* azCode, char* resultScript) {
	AKP_Parser_Insertion* parseInsertList = NULL;
	int parsePos = 0;
	while(azCode[parsePos] != '\0') {
		int nextParsePos;
		parsePos = AKP_parse_whitespace(azCode, parsePos);
		//Skip whitespace
		nextParsePos = AKP_parse_wait(azCode, parsePos, &parseInsertList);
		if(nextParsePos == parsePos) {
			//No return found
			nextParsePos = AKP_parse_return(azCode, parsePos, &parseInsertList);
		}
		if(nextParsePos == parsePos) {
			nextParsePos = AKP_parse_include(azCode, parsePos, &parseInsertList);
		}
		if(nextParsePos == parsePos) {
			//No wait found
			nextParsePos = AKP_parse_function_body(azCode, nextParsePos, &parseInsertList);
		}
		parsePos = nextParsePos;
		parsePos++;
	}
	//Create new script
	AKP_Parser_Insertion* it = parseInsertList;
	parsePos = 0;
	int resultPos = 0;
	char* currentFunction = "";
	char* azDefineNoStartup = "#define AKP_DO_NOT_STARTUP\n";
	for (resultPos = 0; resultPos < 27; resultPos++) {
		resultScript[resultPos] = azDefineNoStartup[resultPos];
	}
	while(azCode[parsePos] != '\0') {
		if(it != NULL) {
			if(it->position == parsePos) {
				if(it->insType == AKP_INSERTION_TYPE_FUNCTION_BEGIN) {
					currentFunction = it->functionName;
					resultPos += sprintf(&(resultScript[resultPos]), "{AKP_profiler_function_start(\"%s\");}", currentFunction);
				} else if(it->insType == AKP_INSERTION_TYPE_FUNCTION_END) {
					resultPos += sprintf(&(resultScript[resultPos]), "{AKP_profiler_function_stop(\"%s\");}", currentFunction);
				} else if(it->insType == AKP_INSERTION_TYPE_WAIT_BEGIN) {
					resultPos += sprintf(&(resultScript[resultPos]), "{{AKP_profiler_function_pause(\"%s\");}", currentFunction);
				} else if(it->insType == AKP_INSERTION_TYPE_WAIT_END) {
					resultPos += sprintf(&(resultScript[resultPos]), "{AKP_profiler_function_resume(\"%s\");}}", currentFunction);
				} else if(it->insType == AKP_INSERTION_TYPE_RETURN_BEGIN) {
					resultPos += sprintf(&(resultScript[resultPos]), "{{AKP_profiler_function_stop(\"%s\");}", currentFunction);
				} else if(it->insType == AKP_INSERTION_TYPE_RETURN_END) {
					resultPos += sprintf(&(resultScript[resultPos]), "}");
				} else if(it->insType == AKP_INSERTION_TYPE_INCLUDE_END) {
					resultPos += sprintf(&(resultScript[resultPos]), ".akp.c");
				}
				it = it->next;
			}
		}
		resultScript[resultPos] = azCode[parsePos];
		parsePos++;
		resultPos++;
	}
	resultScript[resultPos] = '\0';
	//free linked list parseInsertList
	AKP_Parser_Insertion *prev = parseInsertList;
	AKP_Parser_Insertion *cur = parseInsertList;
	while(cur) {
		prev = cur;
		cur = prev->next;
		free(prev);
	}
}

typedef struct AKP_Parsed_File {
	char fileName[264];
	struct AKP_Parsed_File *next;
}
AKP_Parsed_File;

AKP_Parsed_File* AKP_parser_add_file(AKP_Parsed_File* head, char* fileName) {
	AKP_Parsed_File* newParsedFile = (AKP_Parsed_File*) sys_malloc(sizeof(AKP_Parsed_File));
	strcpy(newParsedFile->fileName, fileName);
	newParsedFile->next = head;
	return newParsedFile;
}

BOOL AKP_parser_file_included(AKP_Parsed_File* head, char* filename) {
	AKP_Parsed_File* current = head;
	while(current) {
		if(strcmp(current->fileName, filename) == 0) return true;
		current = current->next;
	}
	return false;
}

void AKP_parser_cleanup_includes(AKP_Parsed_File* head) {
	AKP_Parsed_File* current = head;
	while(current) {
		char targetFileName[264];
		sprintf(targetFileName, "%s.akp.c", current->fileName);
		file_delete(targetFileName);
		current = current->next;
	}
}

AKP_Parsed_File* parsedFileList = NULL;

void AKP_parse_file(char* sourceFileName) {
	if(AKP_parser_file_included(parsedFileList, sourceFileName)) return;
	parsedFileList = AKP_parser_add_file(parsedFileList, sourceFileName);
	char *fileContent = sys_malloc(AKP_MAX_SCRIPT_SIZE);
	var fileHandleSrc = file_open_read(sourceFileName);
	file_str_readto(fileHandleSrc,fileContent, "", AKP_MAX_SCRIPT_SIZE);
	file_close(fileHandleSrc);
	int i = 0;
	int shouldInclude = 0;
	while(fileContent[i] != '\0' && AKP_DO_NOT_PROFILE[i] != '\0') {
		if(fileContent[i] != AKP_DO_NOT_PROFILE[i]) {
			shouldInclude = 1;
			break;
		}
		i++;
	}
	if(AKP_DO_NOT_PROFILE[i] == '\0') shouldInclude = 0;
	
	char targetFileName[264];
	sprintf(targetFileName, "%s.akp.c", sourceFileName);
	
	var fileHandleDest = file_open_write(targetFileName);
	
	if(!shouldInclude) {
		file_str_write(fileHandleDest, fileContent);
		file_close(fileHandleDest);
	} else {
		char* resultContent = sys_malloc(AKP_MAX_SCRIPT_SIZE);
		AKP_parse_lite_c(fileContent, resultContent);
		file_str_write(fileHandleDest, resultContent);
		sys_free(resultContent);
	}
	sys_free(fileContent);
	file_close(fileHandleDest);
}

#ifndef AKP_DO_NOT_STARTUP
//Parse main script file (getting location from first command line parameter)
void AKP_init_startup() {
	char sourceFileName[260];
	//260 = max. recommanded file name length windows
	int i;
	for (i = 0; (_chr(command_str))[i] != '\0' && (_chr(command_str))[i] != ' '; i++) {
		sourceFileName[i] = (_chr(command_str))[i];
	}
	sourceFileName[i] = '\0';
	AKP_parse_file(sourceFileName);
	char targetFileName[264];
	sprintf(targetFileName, "%s.akp.c", sourceFileName);
	exec_wait("%EXE_DIR%\\acknex.exe",targetFileName);
	AKP_parser_cleanup_includes(parsedFileList);
	sys_exit("Restarting with profiler");
}
#endif
#endif