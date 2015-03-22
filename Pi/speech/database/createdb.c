#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

char* dbname="../dictionary.db";

void createPhonemeList(sqlite3* db);
void createWordList(sqlite3* db);
void remove_newline_ch(char *line);
char *str_replace(char *orig, char *rep, char *with);

int main()
{
	sqlite3* db;
	int result=sqlite3_open(dbname,&db) ;
	if (result != SQLITE_OK) 
	{
		printf("Failed to open database \n\r");
   		sqlite3_close(db) ;
		return 1;
	}
	printf("Opened db %s OK\n\r",dbname) ;

	createPhonemeList(db);
	createWordList(db);
	return 0;
  
}

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep
    int len_with; // length of with
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    if (!orig)
        return NULL;
    if (!rep)
        rep = "";
    len_rep = strlen(rep);
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

void remove_newline_ch(char *line)
{
    int new_line = strlen(line) -1;
    if (line[new_line] == '\n')
        line[new_line] = '\0';
}

void createPhonemeList(sqlite3* db)
{
	char* create = "create table CMU_Phonemes(phoneme varchar(5));";
	char *zErrMsg = 0;
	char insert[100];
	int rc;

	rc = sqlite3_exec(db, create, NULL, 0, &zErrMsg);
	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	
	FILE *fp;
	int c;
	char line[80];
	fp = fopen("cmudict-0.7b.symbols","r");
	while(fgets(line, 80, fp) != NULL)
	{
		remove_newline_ch(line);
		sprintf(insert, "insert into CMU_Phonemes(phoneme) values ('%s');", line);
		printf("%s\n", insert);
		rc = sqlite3_exec(db, insert, NULL, 0, &zErrMsg);
		if( rc!=SQLITE_OK ){
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}	
	}
	fclose(fp);
}

void createWordList(sqlite3* db)
{
	char* createW = "create table CMU_Words(wordid int, word varchar(255));";
	char* createX = "create table CMU_WXP(wordid int, phoneme varchar(5), sort int);";
	char *zErrMsg = 0;
	char *es_line;
	char *tok;
	char insert[255];
	char three[4];
	int rc;
	int sort;
	int wordid = 0;

	rc = sqlite3_exec(db, createW, NULL, 0, &zErrMsg);
	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	
	rc = sqlite3_exec(db, createX, NULL, 0, &zErrMsg);
	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	
	FILE *fp;
	int c;
	char line[255];
	fp = fopen("cmudict-0.7b","r");
	while(fgets(line, 255, fp) != NULL)
	{
		strncpy(three, line, 4);
		three[3] = 0;
		if(strncmp(three, ";;;", 3) != 0)
		{
			remove_newline_ch(line);
			tok = strtok(line, " ");
				
			if(es_line = str_replace(tok, "'", "''"))
			{
				sprintf(insert, "insert into CMU_Words(wordid, word) values (%d, '%s');", wordid, es_line);
				printf("%s\n", insert);
				rc = sqlite3_exec(db, insert, NULL, 0, &zErrMsg);
				if( rc!=SQLITE_OK ){
					fprintf(stderr, "SQL error: %s\n", zErrMsg);
					sqlite3_free(zErrMsg);
				}
				free(es_line);
				
				
				sort = 0;
				while(tok = strtok(NULL, " "))
				{
					sprintf(insert, "insert into CMU_WXP(wordid, phoneme, sort) values (%d, '%s', %d);", wordid, tok, sort++);
					printf("%s\n", insert);
					rc = sqlite3_exec(db, insert, NULL, 0, &zErrMsg);
					if( rc!=SQLITE_OK ){
						fprintf(stderr, "SQL error: %s\n", zErrMsg);
						sqlite3_free(zErrMsg);
					}
				}
			}
			wordid++;	
		}
	}
	fclose(fp);
}
