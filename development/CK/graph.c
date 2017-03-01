/*
 * graph.c:  Functions to manipulate the object dependency graph (ODG)
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 15/11/98  ramon       1.0    First release
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * The dependencies between objects in the Alliance Cache Kernel are
 * kept track of in an unweighted directed graph.  We use a special
 * representation for this, which is derived from the conventional edge
 * list representation.  In our model, the vertrices (object headers)
 * are in linked lists (one linked list per object type.)  All
 * dependencies are kept track of in the Object Dependency Array, which is
 * an array of edges.  In this array, the edges are sorted by the address
 * of the parent (tail).  Edges point from parent to dependant.  Moreover,
 * each object header contains a pointer to its first edge, and the number
 * of edges (dependencies) it has in the array.  Thus, the ODA actually
 * consists of many subarrays, of which the lengths are kept track of in
 * the object headers.
 * Functions for adding and removing dependencies in the graph are quite
 * similar to the functions used to manipulate the scheduler priority
 * heap.
 *
 * Indication of usage:
 * - To create a new object, we allocate memory for the object header,
 *   and insert it at the end of the appropiate linked list.  We then load
 *   all of its dependencies in the ODA using the graph functions.
 * - Removing an object is done recursively;  All the object's dependant
 *   objects, and their dependencies, are removed recursively.  Then the
 *   memory itself can be de-allocated.
 *
 * Future optimisations to the CK internal object model:
 * - The reason for keeping linked lists of the objects is that the
 *   objects might need to be searched (ie, kernels can be searched for
 *   Module ID, etc.)  Therefore, it is possible that the current
 *   linked-list construction will be replaced in the future by a
 *   structure optimised for searching (perhaps a binary tree... I have to
 *   have a look in Knuth for this one.  For now, linked lists will do.)
 */

#include <typewrappers.h>
#include "ckobjects.h"
#include "depgraph.h"

/*
 * Now we get the Object Dependency Graph itself.  It is an array of EDGEs
 * (or dependencies.)  lastDep points to the last used dependency in the
 * ODG.
 */

PUBLIC EDGE ODA[MAXDEPS];        /* Object Dependency Array  */
LOCAL  EDGE *lastDep = ODA - 1;  /* The last used dependency */


/*
 * Functions to add/remove dependencies go here.
 * NOTE:  Make sure CK is locked using mutexes, these could mess each
 *        other up pretty much (yuck)
 */

PUBLIC VOID CKaddObjectDep(VERTEX *parent, VERTEX *child)
/*
 * Add an object dependency to the ODG
 *
 * INPUT:
 *     parent:  parent object
 *     child:   dependant object
 *
 * RETURNS:
 *     none
 */
{
    EDGE *currDep = lastDep++;    /* Add a new dependency to the ODG    */

    parent->dependencies++;       /* Add a new dependency to the parent */

    while((currDep > ODA - 1) && (currDep->parent > parent)) {
        if(currDep->parent != (currDep-1)->parent) {
            currDep->parent->firstDependency = currDep+1;
        }
        currDep[1].parent = currDep->parent;  /* If it isn't the right  */
        currDep[1].child  = currDep->child;   /* place yet, move up the */
        currDep--;                            /* current dep and try    */
    }                                         /* again                  */

    currDep++;                    /* We found the right place for dep   */
    currDep->parent = parent;
    currDep->child  = child;

    if((currDep == ODA) || ((currDep-1)->parent != parent)) {
        parent->firstDependency = currDep;
    }
}


PUBLIC VOID CKremoveObjectDep(EDGE *dep)
/*
 * Remove a dependency in the ODG
 *
 * INPUT:
 *     dep:  pointer to dependency to remove
 *
 * RETURNS:
 *     none
 */
{
    EDGE *currDep = dep;

    dep->parent->dependencies--; /* Remove a dependency from the parent */

    while(currDep < lastDep) {
        currDep->parent = currDep[1].parent;  /* Just move everything   */
        currDep->child  = currDep[1].child;   /* Down a place           */
        if((currDep == ODA) || currDep->parent != (currDep-1)->parent) {
            currDep->parent->firstDependency = currDep;
        }
        currDep++;
    }

    lastDep--;                   /* Remove a dependency from the ODG    */
}
