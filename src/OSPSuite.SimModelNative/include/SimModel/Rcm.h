
#ifndef _Rcm_H_
#define _Rcm_H_


#include <vector>

namespace SimModelNative
{

//Rearrange matrix rows and columns using Reverse Cuthill McKee ordering
//
//Core RCM algorithm implementation taken from http://people.sc.fsu.edu/~jburkardt/m_src/rcm/rcm.html 
//
//More on RCM with some examples:
// - http://www.mathworks.de/de/help/matlab/ref/symrcm.html
// - http://ciprian-zavoianu.blogspot.de/2009/01/project-bandwidth-reduction.html
class Rcm
{
public:
	//Returns the reverse Cuthill-McKee ordering of the (undirected) graph whose
	//adjacency matrix is given by "<matrix>+<matrix>'" 
	//
	//<matrix> (input argument) is NxN adjacency matrix of a (directed) graph (not necessary symmetric)
	//
	// Remarks:
	// - "matrix'" is the transpose of "matrix"
	// - <matrix> is modified within the function, that's why passed by value and not as ref.
	// - returned indices are in range [0..N-1]
	std::vector<unsigned int> GenRcm(std::vector<std::vector<bool> > matrix);

protected:

	//create matrix+matrix' and replace the original matrix with it
	void createSymmetricMatrix(std::vector<std::vector<bool> > & matrix);

	//get number of nonzero nondiagonal elements
	int getAdjacencyNumber(const std::vector<std::vector<bool> > & matrix);

	//create inputs needed by core rcm algorithm
	void createAdjacencyInfo(std::vector<std::vector<bool> > & matrix,
		                     int & adjacencyNumber,
							 int * & adj,
							 int * & adj_row);

	//copy permutated indices from double array returned by rcm into vector
	std::vector<unsigned int> getPermutationVectorFrom(const int * permutation, unsigned int nodesNumber);

    //check if all indices are included into the permutation
	void checkPermutation(const std::vector<unsigned int> & permutation, unsigned int nodesNumber);

private:
	//================================= Core RCM implementation =========================================

//  Author:
//
//    Original FORTRAN77 version by Alan George, Joseph Liu.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Alan George, Joseph Liu,
//    Computer Solution of Large Sparse Positive Definite Systems,
//    Prentice Hall, 1981.
//
//    Norman Gibbs, William Poole, Paul Stockmeyer,
//    An Algorithm for Reducing the Bandwidth and Profile of a Sparse Matrix,
//    SIAM Journal on Numerical Analysis,
//    Volume 13, pages 236-250, 1976.
//
//    Norman Gibbs,
//    Algorithm 509: A Hybrid Profile Reduction Algorithm,
//    ACM Transactions on Mathematical Software,
//    Volume 2, pages 378-387, 1976.
//    Paul Bratley, Bennett Fox, Linus Schrage,
//    A Guide to Simulation,
//    Springer Verlag, pages 201-202, 1983.
//
//    Albert Nijenhuis, Herbert Wilf,
//    Combinatorial Algorithms,
//    Academic Press, 1978, second edition,
//    ISBN 0-12-519260-6.


/*
	genrcm - core rcm algo implementation for NxN symmetric adjacency matrix (boolean matrix)

	Inputs:
		node_num - number of rows/columns of the matrix
		adj_num  - number of nondiagonal nonzero elements of the matrix

		adj - adjacency info (row indices) of the input matrix stored as 1-dimensional ongoing sequence
		      (= row indices of the nonzero nondiagonal elements)
			  Indices of each row start at 1 (!)

			  adj has adj_num elements

		adj_row - information about row I is stored in entries adj_row[I-1]-1 through adj_row[I]-2 of adj 
				  
				  adj_row has node_num+1 elements. 
				  First element is always = 1, last element is always adj_num+1

  ===== Example =====================
  Given 10x10 matrix below. Legend: "D" = diagonal element (ignored by the algo), "." = 0, "X" = 1

          1 2 3 4 5 6 7 8 9 10
        1 D . . X . X . . . .
        2 . D X . X . X . . X
        3 . X D X X . . . . .
        4 X . X D . X . . X .
        5 . X X . D . X . . .
        6 X . . X . D X X . .
        7 . X . . X X D X . .
        8 . . . . . X X D . .
        9 . . . X . . . . D .
       10 . X . . . . . . . D

  Input:
   node_num = 10 
   
   adj_num  = 28 (28 nonzero nondiagonal elements)
   
   static int adj_save[adj_num] = 
   {
    4, 6,           //1st row: 4-th and 6-th nondiagonal element are nonzero
    3, 5, 7, 10,
    2, 4, 5,
    1, 3, 6, 9,
    2, 3, 7,
    1, 4, 7, 8,
    2, 5, 6, 8,
    6, 7,
    4,
    2 
   };

   static int adj_row_save[NODE_NUM+1] = {1, 3, 7, 10, 14, 17, 21, 25, 27, 28, 29};
 
   (==> adj[0]..adj[1]   contains adjacency info for the row #1  ==> {4, 6}
        adj[2]..adj[5]   contains adjacency info for the row #2  ==> {3, 5, 7, 10}
		...
		adj[26]..adj[26] contains adjacency info for the row #9  ==> {4}
		adj[27]..adj[27] contains adjacency info for the row #10 ==> {2})
*/

//finds the reverse Cuthill-Mckee ordering for a general graph
void genrcm ( int node_num, int adj_num, int adj_row[], int adj[], int perm[] );

//finds a pseudo-peripheral node
void root_find ( int *root, int adj_num, int adj_row[], int adj[], int mask[],
  int *level_num, int level_row[], int level[], int node_num );

//renumbers a connected component by the reverse Cuthill McKee algorithm
void rcm ( int root, int adj_num, int adj_row[], int adj[], int mask[],
  int perm[], int *iccsze, int node_num );

//generates the connected level structure rooted at a given node
void level_set ( int root, int adj_num, int adj_row[], int adj[], int mask[], 
  int *level_num, int level_row[], int level[], int node_num );

//computes the degrees of the nodes in the connected component
void degree ( int root, int adj_num, int adj_row[], int adj[], int mask[], 
  int deg[], int *iccsze, int ls[], int node_num );

//reverses the elements of an I4VEC
void i4vec_reverse ( int n, int a[] );

};

}//.. end "namespace SimModelNative"


#endif //_Rcm_H_

