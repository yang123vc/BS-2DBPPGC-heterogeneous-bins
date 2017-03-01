#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <list>
#include <tuple>
#include <vector>

#include "classes_BPGC_HetBins.h"

void
dibujar_nodes (vector<NODE> &bins, char *instance, char *name, double t,
	       int escalado);

void
print_best_solution (
    const std::tuple<list<NODE> &, list<NODE> &, double &> &best_solution)
{
  //
  // best global evaluation
  //

  std::cout << "Best global evaluation = " << std::get<2>(best_solution) << endl;

  //
  // best global solution
  //

  std::cout << setw(80) << setfill('=') <<  "Best global solution" << std::endl;
  std::cout << std::string(80, '=') << endl;
  std::cout << std::left << std::setw(20) << "ID"
	    << std::left << std::setw(20) << "Global eval"
	    << std::left << std::setw(20) << "Local eval"
	    << std::left << std::setw(20) << "Waste" << endl;
  std::cout << std::string(80, '=') << endl;

  std::list<NODE> node_list = std::get<0>(best_solution);
  for (std::list<NODE>::const_iterator it = node_list.begin (), end =
      node_list.end (); it != end; ++it)
    {
      const NODE &node = *it;
      std::cout << node.getID () << node.get_globaleval ()
	  << node.get_localevaluation () << node.get_waste () << std::endl;
    }

  //
  // best estimated solution
  //

  std::cout << setw(80) << setfill('=') <<  "Best estimated solution" << std::endl;
  std::cout << std::string(80, '=') << endl;
  std::cout << std::left << std::setw(20) << "ID"
  	    << std::left << std::setw(20) << "Global eval"
  	    << std::left << std::setw(20) << "Local eval"
  	    << std::left << std::setw(20) << "Waste" << endl;
  std::cout << std::string(80, '=') << endl;

  node_list = std::get<1>(best_solution);
  for (std::list<NODE>::const_iterator it = node_list.begin (), end =
      node_list.end (); it != end; ++it)
    {
      const NODE &node = *it;
      std::cout << node.getID () << node.get_globaleval ()
      	  << node.get_localevaluation () << node.get_waste () << std::endl;
    }
}

void
PrintSolution (char *instance_name, TREE &bs_sol, double runtime,
	       double &AreaPz)
{

  list<NODE>::iterator it_child;
  list<NODE> tree;
  tree = bs_sol.get_tree ();
  int last_level;
  it_child = tree.end ();
  it_child--;
  //Identify solution node
  //======================
  //Identify solution node
  //======================
  int bestID = -1;
  double bestUsage = -GRANDE;
  while (it_child != tree.begin ())
    {

      if (it_child->get_IDdisp ().empty ())
	{
	  //Find %Utilization on that branch
	  NODE branch;
	  branch = *it_child;
	  double AreaB = 0;
	  int level = 0;
	  while (branch.get_Pred () != NULL)
	    {
	      //OF: %Utilization = Area Pzas /(Area Bins)
	      AreaB = AreaB + branch.getL () * branch.getW ();
	      //============================================
	      branch = *branch.get_Pred ();
	    }
	  double Util = AreaPz / AreaB;
	  if (bestUsage < Util)
	    {
	      //Keep the info for the branch with best OF
	      bestUsage = Util;
	      bestID = it_child->getID ();
	    }
	}
      it_child--;
    }
  //======================
  it_child = tree.end ();
  it_child--;
  vector<NODE> branch_sol;
  while (it_child != tree.begin () && branch_sol.empty ())
    {
      if (it_child->getID () == bestID)
	{
	  NODE child;
	  child = *it_child;
	  while (child.get_Pred () != NULL)    //While not at FATHER level
	    {
	      branch_sol.push_back (child);
	      child = *child.get_Pred ();
	    }
	  reverse (branch_sol.begin (), branch_sol.end ());
	  char name[200];
	  sprintf (name, "SOL");
	  dibujar_nodes (branch_sol, name, instance_name, runtime, 1);
	}
      it_child--;
    }
}

void
PrintTree (char *instance_name, TREE &bs_sol, double runtime)
{
  int ins = 0;

  //Print tree
  list<NODE> tree;
  tree = bs_sol.get_tree ();
  list<NODE>::iterator it_child;
  it_child = tree.end ();
  it_child--;    //Child points to the last element of the tree
  while (it_child != tree.begin ())
    {
      if (it_child->get_IDdisp ().empty ())
	{
	  vector<NODE> branch_sol;
	  NODE child;
	  child = *it_child;
	  while (child.get_Pred () != NULL)    //While not at FATHER level
	    {
	      branch_sol.push_back (child);
	      child = *child.get_Pred ();
	    }
	  ins++;
	  reverse (branch_sol.begin (), branch_sol.end ());
	  char name[200];
	  sprintf (name, "TREE(%d)", ins);
	  dibujar_nodes (branch_sol, instance_name, name, runtime, 1);
	}
      it_child--;
    }
  printf ("\n");
  //============
}
