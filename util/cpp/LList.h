/*
 * LList.h
 *
 * This file is part of the IHMC Util Library
 * Copyright (c) 1993-2016 IHMC.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 (GPLv3) as published by the Free Software Foundation.
 *
 * U.S. Government agencies and organizations may redistribute
 * and/or modify this program under terms equivalent to
 * "Government Purpose Rights" as defined by DFARS 
 * 252.227-7014(a)(12) (February 2014).
 *
 * Alternative licenses that allow for use within commercial products may be
 * available. Contact Niranjan Suri at IHMC (nsuri@ihmc.us) for details.
 *
 * Base linked list class
 */

#ifndef INCL_LLIST_H
#define INCL_LLIST_H

#include <stdlib.h>

#pragma warning (disable:4786)

namespace NOMADSUtil
{

    template <class T>
    class LList
    {
        public:
            /**
            Constructors and Destructor for Linked List
            - SourceList: Pointer to list to copy
            */
             LList (void);
             LList (const LList<T> &SourceList);
             virtual ~LList (void);

             /**
             Get List first element
             @Return: Number of elements in the List
             */
             int getCount (void);

             /**
             Get List first element
             - El: Pointer to first list element
             @Return: 1 if it was possible to return an element 
                      0 otherwise
             */
             int getFirst (T &El);

             /**
             Get List next element
             - El: Pointer to next element
             @Return: 1 if it was possible to return an element
                      0 otherwise
             */
             int getNext (T &El);

             /**
             Add element of type T to list
             - el: Element to add
             */
             void add (T el);

             /**
             Remove an element el from the list
             - SourceList: T el: Element to remove
             @Return: 1 if the elemnt was found and removed
                      0 otherwise
             */
             int remove (T el);

             /**
             Remove an element el from the list
             */
             void removeAll (void);

             /**
             Replace old elements elOld with new element elNew
             - T elOld: Old element
             - T elNew: New element
             @Return: 1 If it was possible
                      0 if it was not possible
             */
             int replace (T elOld, T elNew);

             /**
             Reset the list accessor to the first element
             */
             void resetGet (void);

             /**
             Search for an element el
             - El: Pointer to element to look for
             @Return: 1 if the element was found
                      0 otherwise
             */
             int search (T &El);
 
             /**
            Copy operator, returns value by reference
             - SourceList List to copy
             @Return: Copied List
             */
             virtual LList<T> & operator = (LList<T> &SourceList);
             
             int length;

        protected:
             struct Node {
                  T el;
                  Node * pNextNode;
             };
             Node *pRoot;
             Node *pGetNextNode;
    };

    template <class T> LList<T>::LList (void)
    {
        pRoot = NULL;
        pGetNextNode = NULL;
        length = 0;
    }

    template <class T> LList<T>::LList (const LList<T> &SourceList)
    {
        Node *pSourceNode, *pNewNode, *pTempNode;

        pRoot = NULL;
        pGetNextNode = NULL;

        pSourceNode = SourceList.pRoot;
        if (pSourceNode)
        {
             pRoot = new Node;
             pRoot -> el = pSourceNode -> el;
             pRoot -> pNextNode = NULL;
             pTempNode = pRoot;
             pSourceNode = pSourceNode -> pNextNode;
             while (pSourceNode)
             {
                  pNewNode = new Node;
                  pNewNode -> el = pSourceNode -> el;
                  pNewNode -> pNextNode = NULL;
                  pTempNode -> pNextNode = pNewNode;
                  pTempNode = pNewNode;
                  pSourceNode = pSourceNode -> pNextNode;
             }
        }
    }

    template <class T> LList<T>::~LList (void)
    {
        removeAll();
    }

    template <class T> void LList<T>::add (T el)
    {
        Node *pNewNode, *pTempNode;
        length++;
        pNewNode = new Node;
        pNewNode->el = el;
        pNewNode->pNextNode = NULL;

        if (pRoot == NULL)
        {
            pRoot = pNewNode;
            pGetNextNode = pRoot;
        }
        else
        {
            pTempNode = pRoot;
            while (pTempNode->pNextNode != NULL)
            {
                pTempNode = pTempNode->pNextNode;
            }
            pTempNode->pNextNode = pNewNode;
            if (pGetNextNode == NULL)
            {
                pGetNextNode = pNewNode;
            }
        }
    }

    template <class T> int LList<T>::getCount (void)
    {
        Node *pTempNode;
        int iCount = 0;

        pTempNode = pRoot;
        while (pTempNode)
        {
            iCount++;
            pTempNode = pTempNode->pNextNode;
        }
        return iCount;
    }

    template <class T> int LList<T>::getFirst (T &el)
    {
        if (pRoot != NULL)
        {
            el = pRoot->el;
            pGetNextNode = pRoot->pNextNode;
            return 1;
        }
        else
        {
            return 0;
        }
    }

    template <class T> int LList<T>::getNext (T &el)
    {
        if (pGetNextNode != NULL)
        {
            el = pGetNextNode->el;
            pGetNextNode = pGetNextNode->pNextNode;
            return 1;
        }
        return 0;
    }

    template <class T> int LList<T>::remove (T el)
    {
        int iResult = 0;
        Node *pPrevNode, *pCurrNode, *pTempNode;

        if (pRoot)
        {
             if (pRoot -> el == el)
             {
                  pTempNode = pRoot;
                  pRoot = pRoot -> pNextNode;
                  delete pTempNode;
                  length--;
                  iResult = 1;
             }
             else
             {
                  pPrevNode = pRoot;
                  pCurrNode = pRoot -> pNextNode;
                  while ((pCurrNode) && !(iResult))
                  {
                       if (pCurrNode -> el == el)
                       {
                            pPrevNode -> pNextNode = pCurrNode -> pNextNode;
                            delete pCurrNode;
                            length--;
                            iResult = 1;
                       }
                       else
                       {
                            pPrevNode = pCurrNode;
                            pCurrNode = pCurrNode -> pNextNode;
                       }
                  }
             }
        }
        return iResult;
    }

    template <class T> void LList<T>::removeAll (void)
    {
        Node *pTempNode;

        while (pRoot)
        {
             pTempNode = pRoot;
             pRoot = pRoot -> pNextNode;
             delete pTempNode;
        }
    }
 
    template <class T> int LList<T>::replace (T elOld, T elNew)
    {
        int iResult = 0;
        Node *pTempNode;

        if (pRoot)
        {
             if (pRoot -> el == elOld)
             {
                  pRoot -> el = elNew;
                  iResult = 1;
             }
             else
             {
                  pTempNode = pRoot -> pNextNode;
                  while ((pTempNode) && !(iResult))
                  {
                       if (pTempNode -> el == elOld)
                       {
                            pTempNode -> el = elNew;
                            iResult = 1;
                       }
                       else
                       {
                            pTempNode = pTempNode -> pNextNode;
                       }
                  }
             }

        }
        return iResult;
    }

    template <class T> void LList<T>::resetGet (void)
    {
        pGetNextNode = pRoot;
    }

    template <class T> int LList<T>::search (T &el)
    {
        Node *pTempNode;
        int iFound;

        pTempNode = pRoot;
        iFound = 0;
        while ((pTempNode) && (!iFound)) {
            if (pTempNode -> el == el) {
                el = pTempNode -> el;
                iFound = 1;
            }
            else {
                pTempNode = pTempNode -> pNextNode;
            }
        }
        return iFound;
    }

    template <class T> LList<T> & LList<T>::operator = (LList<T> & SourceList)
    {
        Node *pTempNode;

        pTempNode = pRoot;
        while (pRoot)
        {
             pTempNode = pRoot;
             pRoot = pRoot -> pNextNode;
             delete pTempNode;
        }
        pRoot = NULL;
        pTempNode = SourceList.pRoot;

        while (pTempNode)
        {
             add (pTempNode->el);
             pTempNode = pTempNode->pNextNode;
        }
        return *this;
    }
}

#endif // INCL_LIST_H
