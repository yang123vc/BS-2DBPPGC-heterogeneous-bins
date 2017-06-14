#include <stddef.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <list>
#include <tuple>

#include "classes_BPGC_HetBins.h"

vector<PIEZA>
set_available_pzas (NODE &father, vector<PIEZA> &all_pzas);

bool
StoppingCriteria (list<NODE>::iterator &father, list<NODE> &tree);

void
LastBinRefinement (list<NODE> &BS_tree);

// Main function. This Function creates the whole tree.
//
void
TREE::build_solution (const char *bin_filename, vector<PIEZA> &pzas)
{
  alpha = 2;
  beta = 3;

  list<NODE>::iterator father;
  NODE InitialNode;
  NODE *aaa = NULL;
  InitialNode.initialize_node (bin_filename, 0, "M");
  InitialNode.set_ID_pzas_disp (pzas);
  InitialNode.set_level (0);
  InitialNode.set_pred (*aaa);
  vector<PIEZA> pzas_avail;
  double ApzaMax = pzas[0].getArea (); //Área de la pza máxima.
  pzas_avail = set_available_pzas (InitialNode, pzas);
  BS_tree.push_back (InitialNode);
  father = BS_tree.begin (); //Pointer the father node in the tree.
  bool stop = false;
//    vector<int> ID_closed;
  int count = 1;
  while (!stop)
    {
      int No_Childs = 10; //Number of different first pieces
      int No_Rots = 3; //  2 (rotations of first piece) * 2 (Mirror(Yes / No))

      //Find available pieces for child nodes.
      pzas_avail = set_available_pzas (*father, pzas);
      if (pzas_avail.empty () && father->getOpen ()) //If there is no more pieces available for that father, close it, and move to the next one.
	{
	  father->setOpen (false);
	  ++father;
	}
      if (pzas_avail.size () < No_Childs && !pzas_avail.empty ())
	No_Childs = pzas_avail.size (); //If less than NoChilds available pieces, recalculate.

      //=====================================
      //Create NoChilds*NoRots*2 children and keep alpha best
      if (!pzas_avail.empty ())
	create_child (bin_filename, No_Childs, No_Rots, *father, pzas_avail,
		      ApzaMax);
      //=====================================

      int nodestot = 0;
      int l = 0;
      while (nodestot < beta)
	{
	  l++;
	  double aa = alpha * NH;
	  nodestot = pow (aa, l);
	}
      if (BS_tree.back ().get_level () < l)
	{
	  ++father; //No global evaluation for the first level children.
	  count++;
	  continue;
	}

      // =======================================================================
      // Global evaluation at the end of each level, and for nodes that still
      // have pieces to place.
      //

      list<NODE>::iterator next_father;
      next_father = father;
      ++next_father; //next father should now point to the next element in the tree after father.
      if (next_father->get_level () != father->get_level ()) //We are at the end of a level.
	{
	  // Renumber nodes ID so we have no duplicates ID's when deleting nodes
	  // in global evaluation

	  int level = next_father->get_level ();
	  list<NODE>::iterator node_level;
	  node_level = next_father;
	  int id = level * 100;

	  // Renumber nodes so we do not get duplicates from different branches.
	  //
	  for (node_level = next_father; node_level != BS_tree.end ();
	      node_level++)
	    {
	      node_level->setID (++id);
	    }

	  //==============================
	  stop = StoppingCriteria (next_father, BS_tree);
	  if (!stop)
	    {
	      cout << "\nGE LEVEL: " << count;
	      global_eval (bin_filename, pzas);
	      count++;
	    }
	}
      //========================================================================
      ++father;
    }
  LastBinRefinement (BS_tree);
}

list<NODE>
TREE::get_tree ()
{
  return BS_tree;
}

void
TREE::del_node (list<NODE>::iterator &nodo)
{
  BS_tree.erase (nodo);
}

//Function that performs a local evaluation based on node utilization.
//It will return tue if the node is accepted as a child, and false if not accepted.
double
Local_of (NODE &node);

bool
TREE::local_eval (vector<NODE> &children, NODE &node, int types,
		  double &ApzaMax)
{
  bool is_accepted = false;

  //Local evaluation: suma [(Area de pza en bin)^2/Amax)]/Area bin.
  //=================================================================
  double ap = 0;
  for (int i = 0; i < node.getPI ().size (); i++)
    {
      PIEZA p = *node.getPI ()[i];
      double apot = pow (p.getArea (), 2);
      ap = ap + apot; //Suma de potencias de áreas de pzas que están en el bin.
    }
  double area_node = node.getL () * node.getW ();
//    double node_util = ap / (area_node * ApzaMax);
  double node_util = ap / (area_node);
  node.set_localevaluation (node_util);
  //======================================================================
  if (children.empty ())
    {
      return true;
    }
  if (children.size () < alpha * types)
    {
      is_accepted = true;
    }
  //==================================================================
  double worst_util = children[children.size () - 1].get_localevaluation ();

//	//Children vector is ordered in descending order of its local evaluation objective function
//    double node_util = node.getPropUtil();
//    double worst_util = children[children.size()-1].getPropUtil();
  if (node_util >= worst_util)
    is_accepted = true;
  //Last check: node is not repeated in any of the previous children entries:
  if (is_accepted)
    for (int i = 0; i < children.size (); i++)
      {
	double of_i = children[i].get_localevaluation ();
	if (equal_double (node_util, of_i, TOL)) //In case there are nodes with same utilization, break ties with same number of pieces.
	  {
	    if (node.getNumPiezas () <= children[i].getNumPiezas ()) //Keep node with more pieces in it.
	      is_accepted = false;
	  }
      }
  return is_accepted;
}

//Function that decides if a node enters the tree or not based on global evaluation
//It will keep the best beta in the BS_tree.
//double ConsSol(const char *argv,vector<PIEZA> item, unsigned long &size);//item pass by copy since I don't want to modify its contents ( this heuristic will place items in bins, and I don't want the pieces to be modified )

void
KeepBestNodes (NODE &, vector<GE_COMP> &, vector<int> &, vector<int> &, int);

double
Construct_Solution (const char *, vector<PIEZA>, double, double &, double &,
		    double &, vector<NODE> &);

void
TREE::global_eval (const char *bin_filename, vector<PIEZA> &item)
{

  // node_eval: pointer to the node we are evaluating
  //
  list<NODE>::iterator node_eval;

  // ini_level: pointer to initial node on level, to know the limits to which
  // make comparisons.
  //
  list<NODE>::iterator ini_level;

  node_eval = BS_tree.end ();
  --node_eval; // node_eval points to the last element on the tree.

  ini_level = BS_tree.end ();
  --ini_level; // ini_level points to the last element on the tree (that corresponds to a limit on the level).

  int last_level = node_eval->get_level ();

  // virtual objective function when following that branch.
  //
  double of = 0;

  // the estimated solution following that branch
  //
  vector<NODE> bin_sol;

  // vector of best values of the objective function after global eval.
  //
  vector<GE_COMP> best_of;

  // node id's that stay after global evaluation
  //
  vector<int> keep_id;

  vector<int> no_pzas_disp;
  double Amax = item[0].getArea ();
  while (node_eval->get_level () == last_level) //Global evaluation done through all elements in a level.
    {
      int id = node_eval->getID ();
      cout << "\nNode: " << node_eval->getID ();

//            int id =  node_eval->getID();
//            cout<<" Predecessor: "<<(node_eval->get_Pred())->getID()<<"  "<<node_eval->getSize()<<"\n";
//            cout<<"Pieces: ";
//            for(int i  = 0; i<node_eval->getPI().size();i++)
//            {
//                
//                int id = node_eval->getPI()[i]->getID();
//                cout<<id<<" ";
//                
//            }
//            cout<<"\n";

      vector<PIEZA> pzas_avail;
      pzas_avail = set_available_pzas (*node_eval, item);
      NODE node;
      node = *node_eval;
      node_eval->set_globaleval (-1);
      //Find value of the objective function up to that point.
      double level = 0;
      double AreaPz = 0;
      double AreaBin = 0;

      //Calculate Total Area of Pieces
      //======================================
      for (int i = 0; i < item.size (); i++)
	AreaPz = AreaPz + item[i].getArea ();
      //======================================

      while (node.get_Pred () != NULL)
	{
	  //===========================================
	  // calculate Total Area of Bins
	  //===========================================
	  AreaBin = AreaBin + node.getW () * node.getL ();
	  level++;
	  //============================================
	  node = *node.get_Pred ();
	}

      //======================================================
      if (!pzas_avail.empty ())
	{
	  // estimate solution from this node
	  //
	  Construct_Solution (bin_filename, pzas_avail, level, AreaBin, Amax,
			      of, bin_sol);
	  of = AreaPz / of;
	  node_eval->set_globaleval (of);

	  // setting the best global solution
	  //
	  if (best_global_evaluation < of)
	    {
	      best_global_evaluation = of;
	      best_global_level =  node_eval->get_level();

	      // add current solution
	      //
	      best_global_branch.clear();
	      best_global_branch = bin_sol;

	      // add estimated solution
	      //
	      node = *node_eval;
	      while (node.get_Pred () != NULL)
		{
		  best_global_branch.push_back(node);
		  node = *node.get_Pred ();
		}
	    }

	  // modify best_of and keep_id so it stores the id's of the nodes to
	  // keep
	  //
	  KeepBestNodes (*node_eval, best_of, keep_id, no_pzas_disp, beta);
	}
      cout << "\tGE Value: " << node_eval->get_globaleval () << "\n";
      --node_eval;        //Next node to perform global eval. (previous in tree)
    }

  // delete nodes with id not in Keep_id
  //
  while (ini_level->getID () != node_eval->getID ())
    {
      if (find (keep_id.begin (), keep_id.end (), ini_level->getID ())
	  == keep_id.end ())
	{
	  // ini_level points to a node that needs to be eliminated (if it is
	  // not a closed node)
	  //
	  if (ini_level->getOpen ())          //!ini_level->getIDdisp().empty())
	    {
	      del_node (ini_level);
	      ini_level = BS_tree.end ();
	    }
	}
      ini_level--;
    }

  cout << "Nodes kept: ";
  for (int i = 0; i < keep_id.size (); i++)
    cout << keep_id[i] << " ";
  cout << "\n";
}

////=====================================================================
////Funciones que ya no pertenecen a la clase pero las necesitamos.
////=====================================================================

vector<PIEZA>
set_available_pzas (NODE &father, vector<PIEZA> &all_pzas)
{
  vector<PIEZA> avail_pzas;
  vector<int> avail_id = father.get_IDdisp ();
  for (int i = 0; i < avail_id.size (); i++)
    {
      int id = avail_id[i];
      int p = 0;
      while (all_pzas[p].getID () != id)
	p++;
      avail_pzas.push_back (all_pzas[p]);
    }

  return avail_pzas;

}

void
KeepBestNodes (NODE &node_eval, vector<GE_COMP> &best_of, vector<int> &keep_id,
	       vector<int>&no_pzas, int b)
{
    if (best_of.empty ())
    {
        GE_COMP gg;
        gg.fractional_util = node_eval.get_globaleval ();
        gg.prop_used_current_bin = node_eval.getPropUtil ();
        keep_id.push_back (node_eval.getID ());
        no_pzas.push_back ((node_eval.get_IDdisp ()).size ());
        best_of.push_back (gg);
        //		best_of.push_back(node_eval.get_globaleval());
        //		keep_id.push_back(node_eval.getID());
        //		no_pzas.push_back((node_eval.get_IDdisp()).size());
        return;
    }
    bool is_accepted = false;
    if (best_of.size () < b)
    {
        is_accepted = true;
    }
    //best_of vector is ordered in descending order of its utilization
    GE_COMP node_of;
    node_of.fractional_util = node_eval.get_globaleval ();
    node_of.prop_used_current_bin = node_eval.getPropUtil ();
    int pzas_disp_node = node_eval.get_IDdisp ().size ();
    int last_post = best_of.size () - 1;
    double b_of = best_of[last_post].fractional_util;
    
    //Primer criterio de aceptación: Debe ser mayor que el peor aceptado en términos de su valor en la GE (más un parámetro para empates)
    double aux1 = b_of + PARAM_LAST_BIN;
    double aux2 =b_of - PARAM_LAST_BIN;
    double aux3 = best_of[last_post].prop_used_current_bin - TOL2;

    if (node_of.fractional_util >= aux1)
    {
        is_accepted = true;
    }
    else
    {
        //Segundo Criterio: Aceptar nodo si su %Util es
        if (node_of.fractional_util >=aux2
            && node_of.prop_used_current_bin > aux3)//(9/3/17): estaba la comparación <
        {
            is_accepted = true;
        }
    }
    
    //Insert accepted node in descending order of the value obtained in the global evaluation (fractional_util), and in case of a tie, first node with best %Util
    if (is_accepted)
    {
        int pos = 0;
        while ((pos <= last_post && (node_of.fractional_util<= best_of[pos].fractional_util - PARAM_LAST_BIN))
                || ((node_of.fractional_util<= best_of[pos].fractional_util + PARAM_LAST_BIN)
                        && (node_of.prop_used_current_bin< best_of[pos].prop_used_current_bin)
                        && (pos <= last_post)))
        {
            pos++;
        }
        best_of.insert (best_of.begin () + pos, node_of);
        keep_id.insert (keep_id.begin () + pos, node_eval.getID ());
        no_pzas.insert (no_pzas.begin () + pos, pzas_disp_node);
        //Deleting non accepted nodes.
        if (best_of.size () > b)
        {
            best_of.pop_back ();
            keep_id.pop_back ();
            no_pzas.pop_back ();
        }
    }
}

//
//Stopping criteria: Stop when there are no more pieces to place in any node.
bool
StoppingCriteria (list<NODE>::iterator &node, list<NODE> &tree)
{
  bool stop = false;
  list<NODE>::iterator it_n;
  it_n = node;
  while (it_n != tree.end ())
    {
      if (it_n->get_IDdisp ().empty ()) // current has no pieces to place.
	{
	  it_n->setOpen (false);

	}
      else
	it_n->setOpen (true);

      it_n++;
    }
  it_n = node;
  while (!it_n->getOpen () && it_n != tree.end ())
    {
      it_n++;
    }
  if (it_n == tree.end ())
    stop = true;

  return stop;
}
