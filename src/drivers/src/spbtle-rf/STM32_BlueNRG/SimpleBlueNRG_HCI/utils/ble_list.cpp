/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
* File Name          : ble_list.c
* Author             : AMS - HEA&RF BU
* Version            : V1.0.0
* Date               : 19-July-2012
* Description        : Circular Linked List Implementation.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
#include <tos/interrupt.hpp>

#ifdef __cplusplus
 extern "C" {
#endif

/******************************************************************************
 * Include Files
******************************************************************************/
#include <hal_types.h>
#include "ble_list.h"
#include <stddef.h>

/******************************************************************************
 * Function Definitions
******************************************************************************/
void list_init_head (tListNode * listHead)
{
  listHead->next = listHead;
  listHead->prev = listHead;
}

uint8_t list_is_empty (tListNode * listHead)
{
  uint8_t return_value;

    tos::int_guard ig;
  if(listHead->next == listHead)
  {
    return_value = TRUE;
  }
  else
  {
    return_value = FALSE;
  }

  return return_value;
}

void list_insert_head (tListNode * listHead, tListNode * node)
{
    tos::int_guard ig;

  node->next = listHead->next;
  node->prev = listHead;
  listHead->next = node;
  (node->next)->prev = node;
}

void list_insert_tail (tListNode * listHead, tListNode * node)
{
    tos::int_guard ig;
  node->next = listHead;
  node->prev = listHead->prev;
  listHead->prev = node;
  (node->prev)->next = node;
}

void list_remove_node (tListNode * node)
{
    tos::int_guard ig;
  (node->prev)->next = node->next;
  (node->next)->prev = node->prev;
}

void list_remove_head (tListNode * listHead, tListNode ** node )
{
    tos::int_guard ig;
  *node = listHead->next;
  list_remove_node (listHead->next);
  (*node)->next = NULL;
  (*node)->prev = NULL;
}

void list_remove_tail (tListNode * listHead, tListNode ** node )
{
    tos::int_guard ig;
  *node = listHead->prev;
  list_remove_node (listHead->prev);
  (*node)->next = NULL;
  (*node)->prev = NULL;
}

void list_insert_node_after (tListNode * node, tListNode * ref_node)
{
    tos::int_guard ig;
  node->next = ref_node->next;
  node->prev = ref_node;
  ref_node->next = node;
  (node->next)->prev = node;
}

void list_insert_node_before (tListNode * node, tListNode * ref_node)
{
    tos::int_guard ig;
  node->next = ref_node;
  node->prev = ref_node->prev;
  ref_node->prev = node;
  (node->prev)->next = node;
}

int list_get_size (tListNode * listHead)
{
    tos::int_guard ig;
  int size = 0;
  tListNode * temp;

  temp = listHead->next;
  while (temp != listHead)
  {
    size++;
    temp = temp->next;
  }
  return (size);
}

void list_get_next_node (tListNode * ref_node, tListNode ** node)
{
    tos::int_guard ig;
  *node = ref_node->next;
}

void list_get_prev_node (tListNode * ref_node, tListNode ** node)
{
    tos::int_guard ig;
  *node = ref_node->prev;
}


#ifdef __cplusplus
 }
#endif
