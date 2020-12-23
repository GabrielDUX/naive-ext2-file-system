#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>




char* 
join(char *s1, char *s2)
{
  char *result = (char *)malloc(strlen(s1)+strlen(s2)+1);
  strcpy(result, s1);
  strcat(result, s2);
  return result;
}