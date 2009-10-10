#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "index.h"

#define FILELEN 1024
#define BUFFLEN 1024000

int *
search_index (char *query, int *resnum)
{

  // count space
  char *ch_p;
  int query_num = 1;
  for (ch_p = query; *ch_p != '\0'; ch_p++) {
    if (*ch_p == ' ') query_num++;
  }


  // DEBUG
  printf("Input query num: %d\n", query_num);


  char *buff = malloc(sizeof(char) * BUFFLEN);
  if (buff == NULL) {
    fprintf(stderr, "Failed to allocate memory.\n");
    exit(1);
  }

  char *query_list[ query_num ];
  int df[ query_num ];
  int *invlist[ query_num ];


  int i;
  char *token;
  for (i = 0; i < query_num; i++) {
    if (i == 0) {
      token = strtok(query, " ");
    } else {
      token = strtok(NULL, " ");
    }
  
    query_list[ i ] = malloc(sizeof(char) * (strlen(token) + 1));
    if (query_list[ i ] == NULL) {
      fprintf(stderr, "Failed to allocate memory.\n");
      exit(1);
    }
    strcpy(query_list[ i ], token);
  }

  for (i = 0; i < query_num; i++) {

    char filename[ FILELEN ];
    filename[ 0 ] = '\0';
    strcpy(filename, INDEX_DIR);

    // Append '/' unless exists
    int len = strlen(filename);
    if (filename[ len - 1] != '/') {
      filename[ len ] = '/';
      filename[ len + 1 ] = '\0';
    }
    strcat(filename, query_list[ i ]);

    // TEST
    fprintf(stderr, "filename=%s\n", filename);

    // If file does NOT exist
    struct stat sb;
    if (stat(filename, &sb) < 0) {
      /* MUST free things */
      /* Not implemented yet. */

      *resnum = -1;
      return NULL;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
      fprintf(stderr, "Failed to open %s\n", filename);
      exit(1);
    }

    fprintf(stderr, "Read index from %s done.\n", filename);

    if (fgets(buff, BUFFLEN, fp) != NULL) {
      char *token = strtok(buff, " ");
      df[ i ] = atoi(token);

      invlist[ i ] = malloc(sizeof(int) * df[ i ]);
      if (invlist[ i ] == NULL) {
	fprintf(stderr, "Failed to allocate memroy for invlist.\n");
	exit(1);
      }

      int count = 0;
      while ((token = strtok(NULL, " ")) != NULL && count < df[ i ] ) {
	invlist[ i ][ count ] = atoi( token );
	count++;
      }

    } else {
      fprintf(stderr, "Failed to read line.\n");
    }
    fclose(fp);
  }


  // Print inverted list
  fprintf(stderr, "\ninverted lists for input query ===\n");
  for (i = 0; i < query_num; i++) {

    int j;
    fprintf(stderr, "%s -> ", query_list[ i ]);
    fprintf(stderr, "%d: ", df[ i ]);

    for (j = 0; j < df[ i ]; j++) {
      fprintf(stderr, "%d ", invlist[ i ][ j ]);
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
  

  /* Intersection */
  // Create order list by bubble sort
  int length_order_list[ query_num ];
  for (i = 0; i < query_num; i++) {
    length_order_list[ i ] = i;
  }
  for (i = 0; i < query_num; i++) {
    int j;
    for (j = 1; j < query_num - i; j++) {

      if (df[ length_order_list[j] ] <
	  df[ length_order_list[j - 1] ]) {

	// swap i and j
	int swap = length_order_list[ j ];
	length_order_list[ j ] = length_order_list[ j - 1 ];
	length_order_list[ j - 1 ] = swap;
      }
    }
  }

  // Print ordered list
  fprintf(stderr, "ordered list===\n");
  for (i = 0; i < query_num; i++) {
    int idx = length_order_list[ i ];
    int j;
    fprintf(stderr, "%s -> ", query_list[ idx ]);
    fprintf(stderr, "%d: ", df[ idx ]);

    for (j = 0; j < df[ idx ]; j++) {
      fprintf(stderr, "%d ", invlist[ idx ][ j ]);
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");


  // Intersection by linear merge
  int result_num = 0;
  if (query_num == 1) {
    result_num = df[ 0 ];
  }
  for (i = 1; i < query_num; i++) {

    int idx1 = length_order_list[ i - 1 ];
    int idx2 = length_order_list[ i ];


    // Get intersection between invlist[ idx1 ] and invlist[ idx2 ]
    // and store result into invlist[ idx1 ]

    // DEBUG PRINT
    if (i == 1) {
      fprintf(stderr, "Intersection of %s and %s: ", query_list[ idx1 ], query_list[ idx2 ]);
    } else {
      fprintf(stderr, "Intersection of last result and %s: ", query_list[ idx2 ]);
    }

    int i1 = 0; // pointer for invlist[ idx1 ]
    int i2 = 0; // pointer for invlist[ idx2 ]
    int r  = 0; // pointer for result (invlist[ idx2 ])

    while (i1 < df[ idx1 ] &&
	   i2 < df[ idx2 ]) {

      if (invlist[ idx1 ][ i1 ] <
	  invlist[ idx2 ][ i2 ]) {
	i1++;

      } else if (invlist[ idx1 ][ i1 ] >
		 invlist[ idx2 ][ i2 ]) {
	i2++;

      } else {
	// Hit
	invlist[ idx2 ][ r ] = invlist[ idx2 ][ i2 ];
	r++;

	i1++;
	i2++;
      }
    }

    result_num = r;
    df[ idx2 ] = result_num;


    // DEBUG PRINT
    int j;
    for (j = 0; j < result_num; j++) {
      fprintf(stderr, "%d ", invlist[ idx2 ][ j ]);
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");

  
  // Print result
  fprintf(stderr, "search result ===\n");
  fprintf(stderr, "result num: %d\n", result_num);


  int *result_list = malloc(sizeof(int) * result_num);
  if (result_list == NULL) {
    fprintf(stderr, "ERROR: Failed to allocate memory for result_list.\n");
    exit(1);
  }

  for (i = 0; i < result_num; i++) {
    result_list[ i ] = invlist[ length_order_list[query_num - 1] ][ i ];
  }


// Free allocated memory
  for (i = 0; i < query_num; i++) {
    free(query_list[ i ]);
    free(invlist[ i ]);
  }
  free(buff);

  *resnum = result_num;
  return result_list; 
}


#ifdef INDEX_MAIN
int
main (int argc, char *argv[])
{

  char query[ 1024 ];
  strcpy(query, "hoge fuga piyo");
  int resnum;
  
  int *reslist = search_index(query, &resnum);


  int i;
  fprintf(stderr, "result: ");
  for (i = 0; i < resnum; i++) {
    fprintf(stderr, "%d ", reslist[ i ]);
  }
  fprintf(stderr, "\n");

  return 0;
}
#endif
