#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct TableHeader {
    char *name;
    int type;
};

struct Column {
    void *value;
};

struct Row {
    struct Column **columns;
};

struct Table {
    struct TableHeader **header;
    struct Row **row;
    int rowCount;
    int columnCount;
};

int* getTableKeys(char **columns) {

}

struct Column *createColumn(void *value) {
    struct Column *tb = malloc(sizeof(struct Column*));
    tb->value = value;
    return tb;
}

struct Row *parseRow(struct Table *ta, struct Row *r, char *row) {
    int len = strlen(row);
    int lookForDelimiter = 1;
    int lookForQuote = 0;
    int type = ta->header[0]->type;
    int columnAt = 0;
    char *temp = malloc(sizeof(char)*strlen(row));
    for(int i=0; i<len; i++) {
        if(row[i] == '"' && lookForQuote) {
            lookForQuote = 0;
            continue;
        } else if(row[i] == '"' && !lookForQuote) {
            lookForQuote = 1;
            continue;
        }
        
        if(row[i] == ';' && lookForDelimiter && lookForQuote == 0) {
            struct Column *column = malloc(sizeof(struct Column*));
            
            r->columns = malloc(sizeof(struct Column*)*strlen(row));
            printf("%s\n", temp);
            if(type == 0) {
                printf("Masuk int\n");
                int intVal = atoi(temp);
                int *ptr = &intVal;
                column->value = ptr;
                r->columns[columnAt++] = column;
            } else {
                printf("Masuk str\n");
                column->value = temp;
                r->columns[columnAt++] = column;
            }
            type = ta->header[columnAt]->type;
            temp[0] = '\0'; // cut substring
        } else {
            // push to buffer
            strncat(temp, &row[i], 1);
        }
    }
}

struct Row *getRowByIndex(struct Table *table, int index) {
    return table->row[index];
}

int loadFromFile(struct Table *ta, char *file) {
    int i = 0;
    char isHeader = 1;
    char *delim = "'\n'";
    char *split, *endToken;
    split = strtok_r(file, delim, &endToken);
    while (split != NULL) {
        if(isHeader == 1) {
            int columnCount = 0;

            // parse header, input e.g. idi;ages;names;
            char *headerColumn;
            char *headerDelim = ";";
            char *endToken2;

            char *newSplit = strdup(split);
            
            headerColumn = strtok_r(newSplit, headerDelim, &endToken2);
            ta->header = malloc(sizeof(struct TableHeader*)*strlen(headerColumn));
            while(headerColumn != NULL) {
                struct TableHeader *tb = malloc(sizeof(struct TableHeader*));
                char *buffer = malloc(sizeof(char)*strlen(headerColumn));

                strcpy(buffer, headerColumn);
                int type = 0;
                if(buffer[strlen(buffer)-1] == 's') {
                    type = 1;
                }

                // set name and type 
                tb->type = type;
                buffer[strlen(buffer)-1] = '\0';
                tb->name = buffer;
                ta->header[columnCount++] = tb;

                headerColumn = strtok_r(NULL, headerDelim, &endToken2);
            }
            ta->columnCount = columnCount;
            isHeader = 0;
        } else {
            struct Row *r = malloc(sizeof(struct Row*));
            r->columns = malloc(sizeof(struct Column*)*ta->columnCount*10);
            parseRow(ta, r, split);
            ta->row = malloc(sizeof(struct Row*)*100);
            // printf("%d\n", *((int*) r->columns[0]->value));
            ta->row[ta->rowCount] = r;
            ta->rowCount++;
        }
        split = strtok_r(NULL, delim, &endToken);
    }
    for(int i=0;i<ta->columnCount;i++) {
        printf("Header %d %s %d\n", i, ta->header[i]->name, ta->header[i]->type);
    }
    void *ptr = ta->row[0]->columns[0];
    printf("Column count %d\n", ta->columnCount);
    printf("Row count %d\n", ta->rowCount);
    printf("%s\n", file);
}

struct DatabaseServer {

};

int main() {
    struct Table myTable = {};
    char buffer[100];
    strcpy(buffer, "idi;ages;names;\n1;3213;\n1;3213;");
    loadFromFile(&myTable, buffer);
}