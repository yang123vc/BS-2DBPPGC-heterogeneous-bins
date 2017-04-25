//
//==========================================================
// Beam search algorithm for the 2DBPPGC and Heterogeneus Bins.
//
//  Created by Marta Cabo Nodar on 14/06/16.
//  Copyright (c) 2016 Marta Cabo. All rights reserved.
//
//==========================================================

#include "classes_BPGC_HetBins.h"

#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)

//
// Declaración de funciones necesarias
//

vector<PIEZA>
read_pieces (char *name);

bool
orden_area (PIEZA i, PIEZA j);

void
dibujar_nodes (vector<NODE> &bins, char *instance, char *name, double t,
	       double usage, int escalado);

// dos argumentos el archivo de los Beans y el *.dat
//
int
main (int argc, const char * argv[])
{
  for (int k = 2; k < argc; k++) //For all instances.
    {
      //
      // reading instance name
      //

      char instance_name[120];
      strcpy (instance_name, argv[k]);

      //
      // reading pieces
      //

      vector<PIEZA> item;
      item = read_pieces (instance_name);

      //
      // ordering pieces by area
      //

      sort (item.begin (), item.end (), orden_area);
      double AreaPz = 0;
      for (int i = 0; i < item.size (); i++)
	{
	  AreaPz = AreaPz + item[i].getArea ();
	}

      //
      // computing solution (with time measure)
      //

      TREE bs_sol;

      clock_t start = clock ();
      bs_sol.build_solution (argv[1], item);
      clock_t finish = clock ();

      // running time in milliseconds.
      //
      double runtime = (finish - start) / CLOCKS_PER_MS;

      // running time in seconds, without loosing precision.
      //
      runtime = runtime / 1000;

      //
      // solution information
      //

      const std::tuple<vector<NODE> &, double &, int &> &best_solution =
            	  bs_sol.getBestSolution ();

      vector<NODE> &solution_branch = std::get<0>(best_solution);
      const double bestUsage = std::get<1>(best_solution);
      const int level = std::get<2>(best_solution);
      const int no_bins = solution_branch.size();

      //
      // printing solution to console
      //

      cout << endl;
      cout << "Instance: " << instance_name << endl;
      cout << "Level: " << level << endl;
      cout << "Number of bins: " << no_bins << endl;
      cout << "Bin Utilization: " << bestUsage << endl;
      cout << "Running time: " << runtime << endl;

      //
      // printing solution to *.tex
      //

      dibujar_nodes (solution_branch, (char *) "SOL", instance_name, runtime,
		     bestUsage, 1);

      //
      // printing solution to *.txt
      //

      ofstream fp;
      fp.open ("results.txt", ios_base::app);
      if (!fp.is_open ())
	{
	  cerr << "Unable to open Results file" << endl;
	  exit (1);
	}
      else
	{
	  fp << instance_name << "\t" << level << "\t" << no_bins << "\t"
	      << bestUsage << "\t" << runtime << "\t" << "[ ";

	  for (auto it = solution_branch.begin (); it < solution_branch.end ();
	      ++it)
	    {
	      fp << (*it).getSize() << " - ";
	    }
	  fp << (*(--solution_branch.end())).getSize() << " ]" << endl;
	}
    }
  return 0;
}
