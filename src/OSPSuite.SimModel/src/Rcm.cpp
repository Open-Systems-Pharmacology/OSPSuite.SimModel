#include "SimModel/Rcm.h"
#include "ErrorData.h"
#include "SimModel/MathHelper.h"
#include <set>

namespace SimModelNative
{

using namespace std;

vector<unsigned int> Rcm::GenRcm(vector<vector<bool> > matrix)
{
	int * adj = NULL;
	int * adj_row = NULL;
	int * permutation = NULL;
	vector<unsigned int> newOrdering;

	try
	{
		//---- fill inputs required by core RCM algo
		int adjacencyNumber = 0;
		createAdjacencyInfo(matrix, adjacencyNumber, adj, adj_row);

		//---- get rcm permutation of the indices (1-based);
		unsigned int nodesNumber = (unsigned int)matrix.size();
		permutation = new int[nodesNumber];
		genrcm(nodesNumber, adjacencyNumber, adj_row, adj, permutation);

		//---- create permutations as vector (0-based!)
		newOrdering = getPermutationVectorFrom(permutation, nodesNumber);

		//---- clean up
		delete[] permutation; permutation = NULL;
		delete[] adj; adj = NULL;
		delete[] adj_row; adj_row = NULL;

		//just in case: make sure we really have got a permutation of {0, 1, ... nodesNumber-1}
		checkPermutation(newOrdering, nodesNumber);
	}
	catch(...)
	{
		if (adj) delete[] adj;

		if (adj_row) delete[] adj_row;

		if (permutation) delete[] permutation;

		throw;
	}

	return newOrdering;
}

//check that <permutation> contains some permutation of {0, 1, ... nodesNumber-1}
void Rcm::checkPermutation(const std::vector<unsigned int> & permutation, unsigned int nodesNumber)
{
	const char * ERROR_SOURCE = "Rcm::checkPermutation";
	const string errMsgPrefix="Rcm produced invalid indices permutation: ";

	if (permutation.size() != nodesNumber)
		throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE, errMsgPrefix+"number of permutated indices does not match the number of nodes");

	if (nodesNumber==0)
		return; //nothing to check anymore

	//---- created an ordered set from the permutation vector
	//     the set should be {0,1, ..., nodesNumber-1)
	set<unsigned int> permutationSet;
	unsigned int i;

	for (i=0; i<nodesNumber; i++)
		permutationSet.insert(permutation[i]);

	for (i=0; i<nodesNumber; i++)
	{
		if (permutationSet.find(i)==permutationSet.end())
			throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE, errMsgPrefix+"index "+MathHelper::ToString(i)+" not available");
	}
}

vector<unsigned int> Rcm::getPermutationVectorFrom(const int * permutation, unsigned int nodesNumber)
{
	vector<unsigned int> permutationVec;

	for(unsigned int i=0; i<nodesNumber; i++)
		permutationVec.push_back(permutation[i]-1); //convert indices from 1-based to 0-based

	return permutationVec;
}

void Rcm::createAdjacencyInfo(std::vector<std::vector<bool> > & matrix,
							  int & adjacencyNumber,
		                      int * & adj,
							  int * & adj_row)
{
    unsigned int nodesNumber = (unsigned int)matrix.size();

	//set matrix = matrix+matrix'
	createSymmetricMatrix(matrix);

	//get number of nonzero diagonal elements
	adjacencyNumber = getAdjacencyNumber(matrix);

	adj = new int[adjacencyNumber];
	adj_row = new int[nodesNumber+1];

	int nextAdjacencyElementPosition = 0; //

	adj_row[0] = 1; //s rcm.h for details and example

	for (size_t rowIdx=0; rowIdx<nodesNumber; rowIdx++)
	{
		for(size_t colIdx = 0; colIdx<nodesNumber; colIdx++)
		{
			if ((rowIdx == colIdx) || !matrix[rowIdx][colIdx])
				continue; //ignore diagonal and zero elements

			adj[nextAdjacencyElementPosition] = (unsigned int)colIdx+1; //column number of the next ajacency entry (1 based !)

			nextAdjacencyElementPosition ++;
		}

		adj_row[rowIdx+1] = nextAdjacencyElementPosition+1;
	}
}

int Rcm::getAdjacencyNumber(const vector<vector<bool> > & matrix)
{
	int adjacencyNumber = 0;

	size_t nodesNumber = matrix.size();

	for (size_t rowIdx=0; rowIdx<nodesNumber; rowIdx++)
	{
		for(size_t colIdx = 0; colIdx<nodesNumber; colIdx++)
		{
			if (rowIdx == colIdx)
				continue; //ignore diagonal elements

			if(!matrix[rowIdx][colIdx])
				continue;

            adjacencyNumber++;
		}
	}

	return adjacencyNumber;
}

void Rcm::createSymmetricMatrix(std::vector<std::vector<bool> > & matrix)
{
	const char * ERROR_SOURCE = "Rcm::createSymmetricMatrix";

	size_t nodesNumber = matrix.size();

	if (nodesNumber == 0)
		return; //nothing to do

	for (size_t rowIdx=0; rowIdx<nodesNumber; rowIdx++)
	{
		if (matrix[rowIdx].size() != nodesNumber)
			throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE, "Matrix with invalid structure passed");

		for(size_t colIdx = 0; colIdx<nodesNumber; colIdx++)
		{
			matrix[rowIdx][colIdx] = matrix[rowIdx][colIdx] || matrix[colIdx][rowIdx];
			matrix[colIdx][rowIdx] = matrix[colIdx][rowIdx] || matrix[rowIdx][colIdx];
		}
	}
}
//================================= Core RCM implementation =========================================
//More info can be found in Rcm.h

void Rcm::genrcm ( int node_num, int adj_num, int adj_row[], int adj[], int perm[] )

//    GENRCM finds the reverse Cuthill-Mckee ordering for a general graph.
//
//  Discussion:
//
//    For each connected component in the graph, the routine obtains
//    an ordering by calling RCM.
//
//  Modified:
//
//    05 January 2007
//
//  Parameters:
//
//    Input, int NODE_NUM, the number of nodes.
//
//    Input, int ADJ_NUM, the number of adjacency entries.
//
//    Input, int ADJ_ROW[NODE_NUM+1].  Information about row I is stored
//    in entries ADJ_ROW(I) through ADJ_ROW(I+1)-1 of ADJ.
//
//    Input, int  ADJ[ADJ_NUM], the adjacency structure.
//    For each row, it contains the column indices of the nonzero entries.
//
//    Output, int  PERM[NODE_NUM], the RCM ordering.
//
//  Local Parameters:
//
//    Local, int  LEVEL_ROW[NODE_NUM+1], the index vector for a level
//    structure.  The level structure is stored in the currently unused
//    spaces in the permutation vector PERM.
//
//    Local, int MASK[NODE_NUM], marks variables that have been numbered.
//
{
  int i;
  int iccsze;
  int level_num;
  int *level_row;
  int *mask;
  int num;
  int root;

  level_row = new int[node_num+1];
  mask = new int[node_num];

  for ( i = 0; i < node_num; i++ )
  {
    mask[i] = 1;
  }

  num = 1;

  for ( i = 0; i < node_num; i++ )
  {
//
//  For each masked connected component...
//
    if ( mask[i] != 0 )
    {
      root = i + 1;
//
//  Find a pseudo-peripheral node ROOT.  The level structure found by
//  ROOT_FIND is stored starting at PERM(NUM).
//
      root_find ( &root, adj_num, adj_row, adj, mask, &level_num,
        level_row, perm+num-1, node_num );
//
//  RCM orders the component using ROOT as the starting node.
//
      rcm ( root, adj_num, adj_row, adj, mask, perm+num-1, &iccsze,
        node_num );

      num = num + iccsze;
//
//  We can stop once every node is in one of the connected components.
//
      if ( node_num < num )
      {
        delete [] level_row;
        delete [] mask;
        return;
      }
    }
  }

  delete [] level_row;
  delete [] mask;

  return;
}


void Rcm::root_find ( int *root, int adj_num, int adj_row[], int adj[], int mask[],
  int *level_num, int level_row[], int level[], int node_num )

//    ROOT_FIND finds a pseudo-peripheral node.
//
//  Discussion:
//
//    The diameter of a graph is the maximum distance (number of edges)
//    between any two nodes of the graph.
//
//    The eccentricity of a node is the maximum distance between that
//    node and any other node of the graph.
//
//    A peripheral node is a node whose eccentricity equals the
//    diameter of the graph.
//
//    A pseudo-peripheral node is an approximation to a peripheral node;
//    it may be a peripheral node, but all we know is that we tried our
//    best.
//
//    The routine is given a graph, and seeks pseudo-peripheral nodes,
//    using a modified version of the scheme of Gibbs, Poole and
//    Stockmeyer.  It determines such a node for the section subgraph
//    specified by MASK and ROOT.
//
//    The routine also determines the level structure associated with
//    the given pseudo-peripheral node; that is, how far each node
//    is from the pseudo-peripheral node.  The level structure is
//    returned as a list of nodes LS, and pointers to the beginning
//    of the list of nodes that are at a distance of 0, 1, 2, ...,
//    NODE_NUM-1 from the pseudo-peripheral node.
//
//  Modified:
//
//    05 January 2007
//
//  Parameters:
//
//    Input/output, int *ROOT.  On input, ROOT is a node in the
//    the component of the graph for which a pseudo-peripheral node is
//    sought.  On output, ROOT is the pseudo-peripheral node obtained.
//
//    Input, int ADJ_NUM, the number of adjacency entries.
//
//    Input, int ADJ_ROW[NODE_NUM+1].  Information about row I is stored
//    in entries ADJ_ROW(I) through ADJ_ROW(I+1)-1 of ADJ.
//
//    Input, int ADJ[ADJ_NUM], the adjacency structure.
//    For each row, it contains the column indices of the nonzero entries.
//
//    Input, int MASK[NODE_NUM], specifies a section subgraph.  Nodes
//    for which MASK is zero are ignored by FNROOT.
//
//    Output, int *LEVEL_NUM, is the number of levels in the level structure
//    rooted at the node ROOT.
//
//    Output, int LEVEL_ROW(NODE_NUM+1), LEVEL(NODE_NUM), the
//    level structure array pair containing the level structure found.
//
//    Input, int NODE_NUM, the number of nodes.
//
{
  int iccsze;
  int j;
  int jstrt;
  int k;
  int kstop;
  int kstrt;
  int level_num2;
  int mindeg;
  int nabor;
  int ndeg;
  int node;
//
//  Determine the level structure rooted at ROOT.
//
  level_set ( *root, adj_num, adj_row, adj, mask, level_num,
    level_row, level, node_num );
//
//  Count the number of nodes in this level structure.
//
  iccsze = level_row[*level_num] - 1;
//
//  Extreme case:
//    A complete graph has a level set of only a single level.
//    Every node is equally good (or bad).
//
  if ( *level_num == 1 )
  {
    return;
  }
//
//  Extreme case:
//    A "line graph" 0--0--0--0--0 has every node in its only level.
//    By chance, we've stumbled on the ideal root.
//
  if ( *level_num == iccsze )
  {
    return;
  }
//
//  Pick any node from the last level that has minimum degree
//  as the starting point to generate a new level set.
//
  for ( ; ; )
  {
    mindeg = iccsze;

    jstrt = level_row[*level_num-1];
    *root = level[jstrt-1];

    if ( jstrt < iccsze )
    {
      for ( j = jstrt; j <= iccsze; j++ )
      {
        node = level[j-1];
        ndeg = 0;
        kstrt = adj_row[node-1];
        kstop = adj_row[node] - 1;

        for ( k = kstrt; k <= kstop; k++ )
        {
          nabor = adj[k-1];
          if ( 0 < mask[nabor-1] )
          {
            ndeg = ndeg + 1;
          }
        }

        if ( ndeg < mindeg )
        {
          *root = node;
          mindeg = ndeg;
        }
      }
    }
//
//  Generate the rooted level structure associated with this node.
//
    level_set ( *root, adj_num, adj_row, adj, mask, &level_num2,
      level_row, level, node_num );
//
//  If the number of levels did not increase, accept the new ROOT.
//
    if ( level_num2 <= *level_num )
    {
      break;
    }

    *level_num = level_num2;
//
//  In the unlikely case that ROOT is one endpoint of a line graph,
//  we can exit now.
//
    if ( iccsze <= *level_num )
    {
      break;
    }
  }

  return;
}


void Rcm::rcm ( int root, int adj_num, int adj_row[], int adj[], int mask[],
  int perm[], int *iccsze, int node_num )

//  Purpose:
//
//    RCM renumbers a connected component by the reverse Cuthill McKee algorithm.
//
//  Discussion:
//
//    The connected component is specified by a node ROOT and a mask.
//    The numbering starts at the root node.
//
//    An outline of the algorithm is as follows:
//
//    X(1) = ROOT.
//
//    for ( I = 1 to N-1)
//      Find all unlabeled neighbors of X(I),
//      assign them the next available labels, in order of increasing degree.
//
//    When done, reverse the ordering.
//
//  Modified:
//
//    09 August 2013
//
//  Parameters:
//
//    Input, int ROOT, the node that defines the connected component.
//    It is used as the starting point for the RCM ordering.
//
//    Input, int ADJ_NUM, the number of adjacency entries.
//
//    Input, int ADJ_ROW[NODE_NUM+1].  Information about row I is stored
//    in entries ADJ_ROW(I) through ADJ_ROW(I+1)-1 of ADJ.
//
//    Input, int ADJ[ADJ_NUM], the adjacency structure.
//    For each row, it contains the column indices of the nonzero entries.
//
//    Input/output, int MASK[NODE_NUM], a mask for the nodes.  Only
//    those nodes with nonzero input mask values are considered by the
//    routine.  The nodes numbered by RCM will have their mask values
//    set to zero.
//
//    Output, int PERM[NODE_NUM], the RCM ordering.
//
//    Output, int *ICCSZE, the size of the connected component
//    that has been numbered.
//
//    Input, int NODE_NUM, the number of nodes.
//
//  Local Parameters:
//
//    Workspace, int DEG[NODE_NUM], a temporary vector used to hold
//    the degree of the nodes in the section graph specified by mask and root.
//
{
	const char * ERROR_SOURCE ="Rcm::rcm";

  int *deg;
  int fnbr;
  int i;
  int j;
  int jstop;
  int jstrt;
  int k;
  int l;
  int lbegin;
  int lnbr;
  int lperm;
  int lvlend;
  int nbr;
  int node;
//
//  If node_num out of bounds, something is wrong.
//
  if ( node_num < 1 )
  {
	  throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE, "Unacceptable input value of NODE_NUM");
//    cerr << "RCM - Fatal error!\n";
//    cerr << "  Unacceptable input value of NODE_NUM = " << node_num << "\n";
  }
//
//  If the root is out of bounds, something is wrong.
//
  if ( root < 1 || node_num < root )
  {
	  throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE, "Unacceptable input value of ROOT");
//    cerr << "RCM - Fatal error!\n";
//    cerr << "  Unacceptable input value of ROOT = " << root << "\n";
//    cerr << "  Acceptable values are between 1 and " << node_num << ", inclusive.\n";
  }
//
//  Allocate memory for the degree array.
//
  deg = new int[node_num];
//
//  Find the degrees of the nodes in the component specified by MASK and ROOT.
//
  degree ( root, adj_num, adj_row, adj, mask, deg, iccsze, perm, node_num );
//
//  If the connected component size is less than 1, something is wrong.
//
  if ( *iccsze < 1 )
  {
	  throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE, 
		  "RCM - Fatal error!\nConnected component size ICCSZE returned from DEGREE as "+MathHelper::ToString(*iccsze));
//    cerr << "RCM - Fatal error!\n";
//    cerr << "  Connected component size ICCSZE returned from DEGREE as "
//         << *iccsze << "\n";
  }
//
//  Set the mask value for the root.
//
  mask[root-1] = 0;
//
//  If the connected component is a singleton, there is no ordering necessary.
//
  if ( *iccsze == 1 )
  {
    delete [] deg;
    return;
  }
//
//  Carry out the reordering.
//
//  LBEGIN and LVLEND point to the beginning and
//  the end of the current level respectively.
//
  lvlend = 0;
  lnbr = 1;

  while ( lvlend < lnbr )
  {
    lbegin = lvlend + 1;
    lvlend = lnbr;

    for ( i = lbegin; i <= lvlend; i++ )
    {
//
//  For each node in the current level...
//
      node = perm[i-1];
      jstrt = adj_row[node-1];
      jstop = adj_row[node] - 1;
//
//  Find the unnumbered neighbors of NODE.
//
//  FNBR and LNBR point to the first and last neighbors
//  of the current node in PERM.
//
      fnbr = lnbr + 1;

      for ( j = jstrt; j <= jstop; j++ )
      {
        nbr = adj[j-1];

        if ( mask[nbr-1] != 0 )
        {
          lnbr = lnbr + 1;
          mask[nbr-1] = 0;
          perm[lnbr-1] = nbr;
        }
      }
//
//  If no neighbors, skip to next node in this level.
//
      if ( lnbr <= fnbr )
      {
        continue;
      }
//
//  Sort the neighbors of NODE in increasing order by degree.
//  Linear insertion is used.
//
      k = fnbr;

      while ( k < lnbr )
      {
        l = k;
        k = k + 1;
        nbr = perm[k-1];

        while ( fnbr < l )
        {
          lperm = perm[l-1];

          if ( deg[lperm-1] <= deg[nbr-1] )
          {
            break;
          }

          perm[l] = lperm;
          l = l - 1;
        }
        perm[l] = nbr;
      }
    }
  }
//
//  We now have the Cuthill-McKee ordering.  
//  Reverse it to get the Reverse Cuthill-McKee ordering.
//
  i4vec_reverse ( *iccsze, perm );
//
//  Free memory.
//
  delete [] deg;

  return;
}

void Rcm::level_set ( int root, int adj_num, int adj_row[], int adj[], int mask[],
  int *level_num, int level_row[], int level[], int node_num )

//    LEVEL_SET generates the connected level structure rooted at a given node.
//
//  Discussion:
//
//    Only nodes for which MASK is nonzero will be considered.
//
//    The root node chosen by the user is assigned level 1, and masked.
//    All (unmasked) nodes reachable from a node in level 1 are
//    assigned level 2 and masked.  The process continues until there
//    are no unmasked nodes adjacent to any node in the current level.
//    The number of levels may vary between 2 and NODE_NUM.
//
//  Modified:
//
//    05 January 2007
//
//  Parameters:
//
//    Input, int ROOT, the node at which the level structure
//    is to be rooted.
//
//    Input, int ADJ_NUM, the number of adjacency entries.
//
//    Input, int ADJ_ROW[NODE_NUM+1].  Information about row I is stored
//    in entries ADJ_ROW(I) through ADJ_ROW(I+1)-1 of ADJ.
//
//    Input, int ADJ[ADJ_NUM], the adjacency structure.
//    For each row, it contains the column indices of the nonzero entries.
//
//    Input/output, int MASK[NODE_NUM].  On input, only nodes with nonzero
//    MASK are to be processed.  On output, those nodes which were included
//    in the level set have MASK set to 1.
//
//    Output, int *LEVEL_NUM, the number of levels in the level
//    structure.  ROOT is in level 1.  The neighbors of ROOT
//    are in level 2, and so on.
//
//    Output, int LEVEL_ROW[NODE_NUM+1], LEVEL[NODE_NUM], the rooted
//    level structure.
//
//    Input, int NODE_NUM, the number of nodes.
//
{
  int i;
  int iccsze;
  int j;
  int jstop;
  int jstrt;
  int lbegin;
  int lvlend;
  int lvsize;
  int nbr;
  int node;

  mask[root-1] = 0;
  level[0] = root;
  *level_num = 0;
  lvlend = 0;
  iccsze = 1;
//
//  LBEGIN is the pointer to the beginning of the current level, and
//  LVLEND points to the end of this level.
//
  for ( ; ; )
  {
    lbegin = lvlend + 1;
    lvlend = iccsze;
    *level_num = *level_num + 1;
    level_row[*level_num-1] = lbegin;
//
//  Generate the next level by finding all the masked neighbors of nodes
//  in the current level.
//
    for ( i = lbegin; i <= lvlend; i++ )
    {
      node = level[i-1];
      jstrt = adj_row[node-1];
      jstop = adj_row[node] - 1;

      for ( j = jstrt; j <= jstop; j++ )
      {
        nbr = adj[j-1];

        if ( mask[nbr-1] != 0 )
        {
          iccsze = iccsze + 1;
          level[iccsze-1] = nbr;
          mask[nbr-1] = 0;
        }
      }
    }
//
//  Compute the current level width (the number of nodes encountered.)
//  If it is positive, generate the next level.
//
    lvsize = iccsze - lvlend;

    if ( lvsize <= 0 )
    {
      break;
    }
  }

  level_row[*level_num] = lvlend + 1;
//
//  Reset MASK to 1 for the nodes in the level structure.
//
  for ( i = 0; i < iccsze; i++ )
  {
    mask[level[i]-1] = 1;
  }

  return;
}

void Rcm::degree ( int root, int adj_num, int adj_row[], int adj[], int mask[],
  int deg[], int *iccsze, int ls[], int node_num )

//---------------------------------------------------------------------------
//
//  Purpose:
//
//    DEGREE computes the degrees of the nodes in the connected component.
//
//  Discussion:
//
//    The connected component is specified by MASK and ROOT.
//    Nodes for which MASK is zero are ignored.
//
//  Modified:
//
//    05 January 2007
//
//  Parameters:
//
//    Input, int ROOT, the node that defines the connected component.
//
//    Input, int ADJ_NUM, the number of adjacency entries.
//
//    Input, int ADJ_ROW[NODE_NUM+1].  Information about row I is stored
//    in entries ADJ_ROW(I) through ADJ_ROW(I+1)-1 of ADJ.
//
//    Input, int ADJ[ADJ_NUM], the adjacency structure.
//    For each row, it contains the column indices of the nonzero entries.
//
//    Input, int MASK[NODE_NUM], is nonzero for those nodes which are
//    to be considered.
//
//    Output, int DEG[NODE_NUM], contains, for each  node in the connected
//    component, its degree.
//
//    Output, int *ICCSIZE, the number of nodes in the connected component.
//
//    Output, int LS[NODE_NUM], stores in entries 1 through ICCSIZE the nodes
//    in the connected component, starting with ROOT, and proceeding
//    by levels.
//
//    Input, int NODE_NUM, the number of nodes.
//
{
  int i;
  int ideg;
  int j;
  int jstop;
  int jstrt;
  int lbegin;
  int lvlend;
  int lvsize;
  int nbr;
  int node;
//
//  The sign of ADJ_ROW(I) is used to indicate if node I has been considered.
//
  ls[0] = root;
  adj_row[root-1] = -adj_row[root-1];
  lvlend = 0;
  *iccsze = 1;
//
//  LBEGIN is the pointer to the beginning of the current level, and
//  LVLEND points to the end of this level.
//
  for ( ; ; )
  {
    lbegin = lvlend + 1;
    lvlend = *iccsze;
//
//  Find the degrees of nodes in the current level,
//  and at the same time, generate the next level.
//
    for ( i = lbegin; i <= lvlend; i++ )
    {
      node = ls[i-1];
      jstrt = -adj_row[node-1];
      jstop = abs ( adj_row[node] ) - 1;
      ideg = 0;

      for ( j = jstrt; j <= jstop; j++ )
      {
        nbr = adj[j-1];

        if ( mask[nbr-1] != 0 )
        {
          ideg = ideg + 1;

          if ( 0 <= adj_row[nbr-1] )
          {
            adj_row[nbr-1] = -adj_row[nbr-1];
            *iccsze = *iccsze + 1;
            ls[*iccsze-1] = nbr;
          }
        }
      }
      deg[node-1] = ideg;
    }
//
//  Compute the current level width.
//
    lvsize = *iccsze - lvlend;
//
//  If the current level width is nonzero, generate another level.
//
    if ( lvsize == 0 )
    {
      break;
    }
  }
//
//  Reset ADJ_ROW to its correct sign and return.
//
  for ( i = 0; i < *iccsze; i++ )
  {
    node = ls[i] - 1;
    adj_row[node] = -adj_row[node];
  }

  return;
}

void Rcm::i4vec_reverse ( int n, int a[] )

//---------------------------------------------------------------------------
//
//  Purpose:
//
//    I4VEC_REVERSE reverses the elements of an I4VEC.
//
//  Example:
//
//    Input:
//
//      N = 5,
//      A = ( 11, 12, 13, 14, 15 ).
//
//    Output:
//
//      A = ( 15, 14, 13, 12, 11 ).
//
//  Modified:
//
//    22 September 2005
//
//  Parameters:
//
//    Input, int N, the number of entries in the array.
//
//    Input/output, int A(N), the array to be reversed.
//
{
  int i;
  int j;

  for ( i = 0; i < n / 2; i++ )
  {
    j        = a[i];
    a[i]     = a[n-1-i];
    a[n-1-i] = j;
  }

  return;
}

}//.. end "namespace SimModelNative"
