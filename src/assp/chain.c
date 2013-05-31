/***********************************************************************
*                                                                      *
* This file is part of Michel Scheffers's miscellanous functions       *
* library.                                                             *
*                                                                      *
* Copyright (C) 1996 - 2010  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel, Germany                       *
*                            ms@ipds.uni-kiel.de                       *
*                                                                      *
* This library is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, either version 3 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* This library is distributed in the hope that it will be useful,      *
* but WITHOUT ANY WARRANTY; without even the implied warranty of       *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
* GNU General Public License for more details.                         *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this library. If not, see <http://www.gnu.org/licenses/>. *
*                                                                      *
*----------------------------------------------------------------------*
*                                                                      *
* File:     chain.c                                                    *
* Contents: Functions for handling double-linked lists.                *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: chain.c,v 1.2 2010/02/05 14:29:12 mtms Exp $ */

#include <stdio.h>     /* NULL */
#include <stdlib.h>

#include "misc.h"     /* LINK freeLinkFunc() */

/*DOC

Function 'firstLink'

This function returns a pointer to the first link ('head') of the 
double-linked list referred to by "chain". "chain" may point to any 
link in that list.

DOC*/

LINK *firstLink(LINK *chain)
{
  if(chain != NULL) {
    while(chain->prev != NULL)
      chain = chain->prev;
  }
  return(chain);
}

/*DOC

Function 'lastLink'

This function returns a pointer to the last link ('tail') of the 
double-linked list referred to by "chain". "chain" may point to any 
link in that list.

DOC*/

LINK *lastLink(LINK *chain)
{
  if(chain != NULL) {
    while(chain->next != NULL)
      chain = chain->next;
  }
  return(chain);
}

/*DOC

Function 'numLinks'

This function returns the number of links in the double-linked list 
referred to by "chain". "chain" may point to any link in that list.
It returns -1 if the chain is corrupted i.e. somewhere closed in itself.

DOC*/

long numLinks(LINK *chain)
{
  long n=0;

  if(chain != NULL) {
    chain = firstLink(chain);
    n = 1;
    while(chain->next != NULL) {
      n++;
      if(n < 0) /* overflow */
	return(-1);
      chain = chain->next;
    }
  }
  return(n);
}

/*DOC

Function 'appendChain'

This function appends the chain whose first link is pointed to by 
"chain" to the chain pointed to by "head". If "head" refers to a NULL 
pointer, "chain" will become the new "head". 
"head" will always be updated to point to a pointer to the first link 
in the chain.
The function returns the pointer "chain" or NULL upon error.

DOC*/

LINK *appendChain(LINK **head, LINK *chain)
{
  LINK *last;

  if(head == NULL || chain == NULL)
    return(NULL);
  chain->prev = NULL;                              /* ensure validity */
  if(*head == NULL) {                           /* create a new chain */
    *head = chain;
  }
  else {
    last = lastLink(*head);
    last->next = chain;
    chain->prev = last;
    *head = firstLink(last);
  }
  return(chain);
}

/*DOC

Function 'appendLink'

This function adds the link pointed to by "link" at the end of the 
chain pointed to by "head". If "head" refers to a NULL pointer, a new 
chain will be created with "link" as its first link. 
"head" will always be updated to point to a pointer to the first link 
in the chain.
The function returns the pointer "link" or NULL upon error.

DOC*/

LINK *appendLink(LINK **head, LINK *link)
{
  if(head == NULL || link == NULL)
    return(NULL);
  link->prev = link->next = NULL;                  /* ensure validity */
  return(appendChain(head, link));
}

/*DOC

Function 'insChainBefore'

This function inserts the chain whose first link is pointed to by 
"chain" in the chain pointed to by "head" between the link pointed to 
by "pos" and its predecessor. 
If "pos" is a NULL pointer, "head" may not be a NULL pointer and the 
chain is prepended to "head", if necessary by creating a new chain. 
Otherwise the insertion will be carried out and if "head" is not a NULL 
pointer it will be updated to refer to a pointer to the first link of 
the chain.
The function returns the pointer "chain" or NULL upon error.

DOC*/

LINK *insChainBefore(LINK **head, LINK *pos, LINK *chain)
{
  LINK *last=NULL;

  if((pos == NULL && head == NULL) || chain == NULL)
    return(NULL);
  chain->prev = NULL;                              /* ensure validity */
  last = lastLink(chain);
  if(pos == NULL) {
    if(*head != NULL) {                            /* prepend to head */
      pos = *head;
      pos->prev = last;
      last->next = pos;
    }
    /* else create a new chain */
    *head = chain; /* always the new head */
  }
  else {
    chain->prev = pos->prev;
    if(pos->prev != NULL)
      pos->prev->next = chain;
    pos->prev = last;
    last->next = pos;
    if(head != NULL)
      *head = firstLink(chain);
  }
  return(chain);
}

/*DOC

Function 'insLinkBefore'

This function inserts the link pointed to by "link" in the chain 
pointed to by "head" between the link pointed to by "pos" and its 
predecessor. 
If "pos" is a NULL pointer, "head" may not be a NULL pointer and "link" 
will be inserted as first link in the chain, if necessary by creating a 
new chain. Otherwise the insertion will be carried out and if "head" is 
not a NULL pointer it will be updated to refer to a pointer to the 
first link of the chain.
The function returns the pointer "link" or NULL upon error.

DOC*/

LINK *insLinkBefore(LINK **head, LINK *pos, LINK *link)
{
  if(link == NULL)
    return(NULL);
  link->prev = link->next = NULL;                  /* ensure validity */
  return(insChainBefore(head, pos, link));
}

/*DOC

Function 'insChainBehind'

This function inserts the chain whose first link is pointed to by 
"chain" in the chain pointed to by "head" between the link pointed to 
by "pos" and its successor. 
If "pos" is a NULL pointer, "head" may not be a NULL pointer and 
"chain" will be appended to that chain or, if "head" points to a NULL 
pointer, a new chain will be created. Otherwise the insertion will 
be carried out and if "head" is not a NULL pointer it will be updated 
to refer to a pointer to the first link of the chain.
The function returns the pointer "chain" or NULL upon error.

DOC*/

LINK *insChainBehind(LINK **head, LINK *pos, LINK *chain)
{
  LINK *last=NULL;

  if((pos == NULL && head == NULL) || chain == NULL)
    return(NULL);
  chain->prev = NULL;                              /* ensure validity */
  if(pos == NULL)
    return(appendChain(head, chain));
  else {
    last = lastLink(chain);
    last->next = pos->next;
    if(pos->next != NULL)
      pos->next->prev = last;
    pos->next = chain;
    chain->prev = pos;
    if(head != NULL)
      *head = firstLink(pos);
  }
  return(chain);
}

/*DOC

Function 'insLinkBehind'

This function inserts the link pointed to by "link" in the chain 
pointed to by "head" between the link pointed to by "pos" and its 
successor. 
If "pos" is a NULL pointer, "head" may not be a NULL pointer and "link" 
will be appended as last link in the chain or, if "head" points to a 
NULL pointer, a new chain will be created. Otherwise the insertion will 
be carried out and if "head" is not a NULL pointer it will be updated 
to refer to a pointer to the first link of the chain.
The function returns the pointer "link" or NULL upon error.

DOC*/

LINK *insLinkBehind(LINK **head, LINK *pos, LINK *link)
{
  if(link == NULL)
    return(NULL);
  link->prev = link->next = NULL;                  /* ensure validity */
  return(insChainBehind(head, pos, link));
}

/*DOC

Function 'detachChain'

This function removes the sub-chain specified by the pointers "first" 
and "last" from the chain pointed to by "head" without changing the 
contents of the links of that sub-chain. If "first" was the head of the 
chain, "head" will be updated.
The function returns the pointer "first" or NULL upon error.

Note that upon return the pointers first->prev and last->next still 
refer to links in the original chain.

DOC*/

LINK *detachChain(LINK **head, LINK *first, LINK *last)
{
  if(head == NULL || first == NULL || last == NULL)
    return(NULL);
  if(first->prev != NULL)
    first->prev->next = last->next;
  else if(first == *head) /* was head of the chain */
    *head = last->next;
  else
    return(NULL);
  if(last->next != NULL)
    last->next->prev = first->prev;
  return(first);
}

/*DOC

Function 'detachLink'

This function removes the link pointed to by "link" from the chain 
pointed to by "head" without changing the contents of the link. If 
"link" was the head of the chain, "head" will be updated.
The function returns the pointer "link" or NULL upon error.

DOC*/

LINK *detachLink(LINK **head, LINK *link)
{
  return(detachChain(head, link, link));
}

/*DOC

Function 'deleteLink'

This function removes the link pointed to by "link" from the chain 
pointed to by "head". If dynamic memory allocation has been used to 
create the link, a function "freeLink" must be provided to return all 
allocated memory, i.e. any private memory in the link and that for the 
link itself. If "freeLink" is NULL, the contents of "link" will remain
unchanged exept for 'prev' and 'next' which will be set to NULL. 
If "link" was the head of the chain, "head" will be updated.
The function returns a pointer to the next link in the chain which will 
be NULL if this was the last link.

DOC*/

LINK *deleteLink(LINK **head, LINK *link, freeLinkFunc freeLink)
{
  LINK *next;

  if(head == NULL || link == NULL)
    return(NULL);
  next = link->next;
  if(detachLink(head, link) == NULL)
    return(NULL);
  if(freeLink != NULL)
    freeLink((void *)link);
  else {
    link->prev = link->next = NULL;
  }
  return(next);
}

/*DOC

Function 'deleteChain'

This function deletes all links from the double-linked list pointed to 
by "head" and returns all allocated memory using the function "freeLink" 
(see 'deleteLink' for more details). If "head" does not really point to 
the head of the chain, the link it is pointing to and all trailing ones 
will be deleted.

DOC*/

void deleteChain(LINK **head, freeLinkFunc freeLink)
{
/*   *head = firstLink(*head); */       /* make sure we're at the beginning */
  while(*head != NULL)
    deleteLink(head, *head, freeLink);
  return;
}
