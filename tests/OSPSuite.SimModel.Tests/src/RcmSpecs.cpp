#ifdef _WINDOWS
#pragma warning( disable : 4691)
#endif

#include "SimModelManaged/ExceptionHelper.h"
#include "SimModelSpecs/SpecsHelper.h"
#include "SimModel/Rcm.h"

namespace UnitTests
{
	using namespace OSPSuite::BDDHelper;
	using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework;
	using namespace SimModelNET;

	//provide access to protected members via public inheritance
	class RcmExtender : public SimModelNative::Rcm
	{
	public:
		void createAdjacencyInfo(std::vector<std::vector<bool>> & matrix, int & adjacencyNumber, int * & adj, int * & adj_row)
		{
			SimModelNative::Rcm::createAdjacencyInfo(matrix, adjacencyNumber, adj, adj_row);
		}
	};

	ref class RcmWrapper
	{
	public:
		RcmExtender * Rcm;
		RcmWrapper(){Rcm=new RcmExtender();}
		~RcmWrapper(){delete Rcm;}
	};

	using namespace SimModelNative;
	using namespace std;

	public ref class concern_for_rcm abstract : ContextSpecification<RcmWrapper^>
    {
    public:
		virtual void GlobalContext() override
		{
			try
			{
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(System::Exception^ )
			{
				throw;
			}
			catch(...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}

	protected:
		vector<vector<bool>> * _matrix;

        virtual void Context() override
        {
			try
			{
				sut=gcnew RcmWrapper();

/*
				create example 10x10 matrix
				Legend: "D" = diagonal element (ignored by the algo), "." = 0, "X" = 1

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
*/
				_matrix = new vector<vector<bool>>();
				for(int i=0; i<10; i++)
				{
					_matrix->push_back(vector<bool> ());

					for (int j=0; j<10; j++)
					{
						(*_matrix)[i].push_back(false);
					}
				}

				(*_matrix)[0][3] = true; (*_matrix)[0][5] = true;
				(*_matrix)[1][2] = true; (*_matrix)[1][4] = true; (*_matrix)[1][6] = true; (*_matrix)[1][9] = true;
				(*_matrix)[2][1] = true; (*_matrix)[2][3] = true; (*_matrix)[2][4] = true;
				(*_matrix)[3][0] = true; (*_matrix)[3][2] = true; (*_matrix)[3][5] = true; (*_matrix)[3][8] = true;
				(*_matrix)[4][1] = true; (*_matrix)[4][2] = true; (*_matrix)[4][6] = true;
				(*_matrix)[5][0] = true; (*_matrix)[5][3] = true; (*_matrix)[5][6] = true; (*_matrix)[5][7] = true;
				(*_matrix)[6][1] = true; (*_matrix)[6][4] = true; (*_matrix)[6][5] = true; (*_matrix)[6][7] = true;
				(*_matrix)[7][5] = true; (*_matrix)[7][6] = true;
				(*_matrix)[8][3] = true;
				(*_matrix)[9][1] = true;
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(System::Exception^ )
			{
				throw;
			}
			catch(...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
        }
    };

	public ref class when_creating_rcm_core_inputs : public concern_for_rcm
    {
	protected:   
		int _adjacencyNumber;
		int * _adj, * _adj_row;

		virtual void Context() override
		{
			concern_for_rcm::Context();
		}

		virtual void Because() override
        {
			try
			{
				pin_ptr<int *> adj_ptr = &_adj;
				pin_ptr<int *> adj_row_ptr = &_adj_row;
				pin_ptr<int> _adjacencyNumberPtr = &_adjacencyNumber;

				sut->Rcm->createAdjacencyInfo(*_matrix, *_adjacencyNumberPtr, *adj_ptr, *adj_row_ptr);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(System::Exception^ )
			{
				throw;
			}
			catch(...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
        }

    public:
        [TestAttribute]
        void should_create_proper_rcm_inputs()
        {
			//--------------------------------------------------------------------------------
			// expected outputs for the adjacency info
			//--------------------------------------------------------------------------------
			const int adj_save[28] = {
				4, 6,           
				3, 5, 7, 10,
				2, 4, 5,
				1, 3, 6, 9,
				2, 3, 7,
				1, 4, 7, 8,
				2, 5, 6, 8,
				6, 7,
				4,
				2 };

			const int adj_row_save[11] = {1, 3, 7, 10, 14, 17, 21, 25, 27, 28, 29};
			//--------------------------------------------------------------------------------

			try
			{
				BDDExtensions::ShouldBeEqualTo(_adjacencyNumber, 28);
				
				int i;

				for(i=0; i<28; i++)
					BDDExtensions::ShouldBeEqualTo(_adj[i], adj_save[i]);

				for(i=0; i<11; i++)
					BDDExtensions::ShouldBeEqualTo(_adj_row[i], adj_row_save[i]);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
        }
    };

	public ref class when_getting_rcm_permutation : public concern_for_rcm
    {
	protected:   
		vector<unsigned int> * _permutation;

		virtual void Context() override
		{
			concern_for_rcm::Context();
			_permutation = new vector<unsigned int> ();
		}

		virtual void Because() override
        {
			try
			{
				*_permutation=sut->Rcm->GenRcm(*_matrix);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(System::Exception^ )
			{
				throw;
			}
			catch(...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
        }

    public:
        [TestAttribute]
        void should_return_correct_permutation()
        {
			const unsigned int permutation_save[10] = {8,0,7,5,3,6,4,2,1,9};
			try
			{
				int i;

				for(i=0; i<10; i++)
					BDDExtensions::ShouldBeEqualTo((*_permutation)[i], permutation_save[i]);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
        }
    };


}