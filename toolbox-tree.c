//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox-tree.c
//
// Copyright (c) 2015 Francois Oligny-Lemieux
// All rights reserved
//
//      Created: 12.Apr.2015
//
//  Description: 
//      To store filesystem folders/files
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "toolbox.h"

#if C_TOOLBOX_GENERIC_TREE == 1
#	include "toolbox-tree.h"
#	include "toolbox-errors.h"
#	include "toolbox-basic-types.h"

#	include <string.h>
#	include <stdlib.h>


int genericTree_Constructor(genericTree_T * genericTree)
{
	if ( genericTree == NULL )
	{
		return -1;
	}

	genericTree->should_always_be_0xAFAF7878 = 0xAFAF7878;
	genericTree->current_iterator = NULL;
	genericTree->accept_duplicate_names = 1;
	genericTree->accept_duplicate_unique_ids = 0;
	genericTree->allow_zero_unique_ids = 0;
	genericTree->next_unique_id = 1;
   genericTree->max_name_length = 65000; // 0 for unlimited
   genericTree->itemAmount = 0;
   memset(&genericTree->top, 0, sizeof(genericTree->top));
   genericTree->top.tree = genericTree;

	return 1;
}

// will insert as a child of level, alphabetically sorted between childs
int genericTree_Insert(genericTree_T * genericTree,
                       treeItem_T * level, // level to insert on
                       const char * name,
                       uint64_t unique_id, 
                       void * client,
                       treeItem_T ** new_element)
{
	treeItem_T * newItem;
	treeItem_T * loopItem;
	treeItem_T * prvLoopItem;
//	treeItem_T * iterator = NULL;

	unsigned int strLen;
	int i = 0;

   int inserted = 0;

	if ( genericTree == NULL )
	{
		return -1;
	}
	if ( genericTree->should_always_be_0xAFAF7878 != 0xAFAF7878 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
	}
	if ( name == NULL )
	{
		return -2;
	}
    
	newItem = (treeItem_T*)malloc(sizeof(treeItem_T));
	if ( newItem == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}
	memset(newItem, 0, sizeof(treeItem_T));

	strLen = (unsigned int)strlen(name);
	if ( genericTree->max_name_length > 0 && strLen >= genericTree->max_name_length )
	{
		free(newItem);
		return TOOLBOX_ERROR_YOU_PASSED_A_STRING_TOO_BIG;
	}
	flexstrcpy(&newItem->name, name);

	if ( unique_id == 0 )
	{
		if ( genericTree->allow_zero_unique_ids == 0 )
		{
			// generate one
			unique_id = genericTree->next_unique_id;
			genericTree->next_unique_id++;
		}
		else
		{
			// keep the zero
		}
	}
	else if ( genericTree->next_unique_id <= unique_id )
	{	genericTree->next_unique_id = unique_id + 1;
	}
	newItem->unique_id = unique_id;
	newItem->client = client;

   loopItem = level->childs;
   if (loopItem == NULL)
   {
      level->childs = newItem;
      newItem->parent = level;            
      goto end_of_func_success;
   }

	while ( loopItem )
	{
		i = flexstrcasecmp(&loopItem->name, name);
		if ( i < 0 )
		{	// candidate > iterator
         
         // continue looping 
         /*
         // insert after
         if (loopItem->next)
         {
            newItem->next = loopItem->next;
            loopItem->next->prev = newItem;
         }
         loopItem->next = newItem;
         newItem->prev = loopItem;
         newItem->parent = loopItem->parent;
         inserted = 1;
         break;*/
		}
		else if ( i > 0 )
		{	// candidate < iterator
         // insert before
         if (loopItem->prev)
         {
            loopItem->prev->next = newItem;
            newItem->prev = loopItem->prev;
         }
         else
         {  // loopItem->prev == NULL
            if (loopItem->parent == NULL)
            {  return -20; // corruption
            }
            loopItem->parent->childs = newItem; // replace parent's first child
         }
         
         newItem->next = loopItem;
         loopItem->prev = newItem;
         newItem->parent = loopItem->parent;
         inserted = 1;
         break;
		}
		else
		{
			i = flexstrcmp(&loopItem->name, name);
			if ( i < 0 )
			{	// candidate > iterator
            /*
            // insert after
            if (loopItem->next)
            {
               newItem->next = loopItem->next;
               loopItem->next->prev = newItem;
            }
            loopItem->next = newItem;
            newItem->prev = loopItem;
            newItem->parent = loopItem->parent;
            inserted = 1;
            break;*/
			}
			else if ( i > 0 )
			{	// candidate < iterator
            // continue looping
            
            if (loopItem->prev)
            {
               loopItem->prev->next = newItem;
               newItem->prev = loopItem->prev;
            }
            else
            {  // loopItem->prev == NULL
               if (loopItem->parent == NULL)
               {
                  free(newItem);
                  return -21; // corruption error
               }
               loopItem->parent->childs = newItem;
            }
            
            newItem->next = loopItem;
            loopItem->prev = newItem;
            newItem->parent = loopItem->parent;
            inserted = 1;
            break;
			}
			else
			{
				// already exists!!!
				if ( genericTree->accept_duplicate_names )
				{  
               // insert after
               if (loopItem->next)
               {
                  newItem->next = loopItem->next;
                  loopItem->next->prev = newItem;
               }
               loopItem->next = newItem;
               newItem->prev = loopItem;
               newItem->parent = loopItem->parent;
               inserted = 1;
               break;
				}
				else
				{
					free(newItem);
					return TOOLBOX_ERROR_ALREADY_EXISTS;
				}
			}
		}
      prvLoopItem = loopItem;
      loopItem = loopItem->next;
	}
	
	if ( inserted == 0 )
	{
      // append at end
      loopItem = prvLoopItem;
      if (loopItem->next)
      {        
         return -22; // internal implementation error
      }
      if (loopItem->parent == NULL)
      {
          return -23; // corruption error
      }

      loopItem->next = newItem;
      newItem->prev = loopItem;
      newItem->parent = loopItem->parent;
      inserted = 1;
   }

end_of_func_success:
	if ( new_element ) *new_element = newItem;
   genericTree->itemAmount++;
   newItem->tree = genericTree;

	return 1;

}

// written 29.Jan.2008
int genericTree_Remove(genericTree_T * genericTree, treeItem_T * remItem)
{
   int iret;

	if ( genericTree == NULL )
	{
		return -1;
	}

	if ( remItem == NULL )
	{
		return -2;
	}

   iret = 1;
   while ( remItem->childs != NULL && iret > 0 )
   {
      iret = genericTree_Delete(genericTree, remItem->childs);
   }
   
   if ( remItem->parent != NULL && remItem->parent->childs == remItem )
	{
		remItem->parent->childs = remItem->next;
	}

   if (remItem->prev)
   {
      remItem->prev->next = remItem->next;
   }

   if (remItem->next)
   {
      remItem->next->prev = remItem->prev;
   }

   if (remItem == genericTree->top.childs)
   {
      genericTree->top.childs = remItem->next;
   }

	free(remItem);
   genericTree->itemAmount--;

	return 1;
}


// deletes and clean clients
int genericTree_Delete(genericTree_T * genericTree, treeItem_T * remItem)
{
   int iret;

	if ( genericTree == NULL )
	{
		return -1;
	}

	if ( remItem == NULL )
	{
		return -2;
	}
   
   iret = 1;
   while ( remItem->childs != NULL && iret > 0 )
   {
      iret = genericTree_Delete(genericTree, remItem->childs);
   }
   
   if ( remItem->parent != NULL && remItem->parent->childs == remItem )
   {
	   remItem->parent->childs = remItem->next;
   }

   if (remItem->prev)
   {
      remItem->prev->next = remItem->next;
   }

   if (remItem->next)
   {
      remItem->next->prev = remItem->prev;
   }

   if (remItem == genericTree->top.childs)
   {
      genericTree->top.childs = remItem->next;
   }

   if (remItem->client) free(remItem->client);
   remItem->client = NULL;
   free(remItem);
   genericTree->itemAmount--;


   return 1;
}

int genericTree_Destructor(genericTree_T * genericTree)
{
   treeItem_T * remItem;

	if ( genericTree == NULL )
	{
		return -1;
	}

   remItem = genericTree->top.childs;
   while (remItem != NULL)
   {
      genericTree_Delete(genericTree, remItem);
      remItem = genericTree->top.childs;
   }

   genericTree->should_always_be_0xAFAF7878 = 0;

	return 1;
}



#endif //C_TOOLBOX_BINARY_TREE

