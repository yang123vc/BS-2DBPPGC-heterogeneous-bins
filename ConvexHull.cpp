// Implementation of Andrew's monotone chain 2D convex hull algorithm.
// Asymptotic complexity: O(n log n).
// Practical performance: 0.5-1.0 seconds for n=1000000 on a 1GHz machine.

#include "classes_BPGC_HetBins.h"
double
calcular_area_exacta (vector<PUNTO> &poligono);
bool
lexicographic (PUNTO A, PUNTO B);
// 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
// Returns a positive value, if OAB makes a counter-clockwise turn,
// negative for clockwise turn, and zero if the points are collinear.
double
cross (PUNTO &O, PUNTO &A, PUNTO &B)
{
  return (A.coordx - O.coordx) * (B.coordy - O.coordy)
      - (A.coordy - O.coordy) * (B.coordx - O.coordx);
}

// Returns a list of points on the convex hull in counter-clockwise order.
// Note: the last point in the returned list is the same as the first one.
vector<PUNTO>
convex_hull (vector<PUNTO> P)
{
  unsigned long n = P.size ();
  int k = 0;
  vector<PUNTO> H (2 * n);
  //Initialize vector H to (-100, -100)
  for (int i = 0; i < H.size (); i++)
    {
      H[i].coordx = -100;
      H[i].coordy = -100;
    }
  // Sort points lexicographically
  sort (P.begin (), P.end (), lexicographic);

  // Build lower hull
  for (int i = 0; i < n; ++i)
    {
      while (k >= 2 && cross (H[k - 2], H[k - 1], P[i]) <= 0)
	k--;
      H[k++] = P[i];
    }

  // Build upper hull
  for (int i = n - 2, t = k + 1; i >= 0; i--)
    {
      while (k >= t && cross (H[k - 2], H[k - 1], P[i]) <= 0)
	k--;
      H[k++] = P[i];
    }

  H.resize (k);
  return H;
}

//This function returns the convex hull (as a piece) of all the pieces in one section of the bin.
PIEZA
create_convexhull_in_section (IRR_BIN &sect)
{
  PIEZA convexhull;
  vector<PIEZA*> *pb = sect.get_pzas (); //pb pieces in bin.
  vector<PUNTO> ptos_sect; //vector que contiene todos los puntos de las piezas que están en la sección. Para construir el convex hull.
  vector<PUNTO> ptos_pza; //Puntos de cada pieza.
  vector<PUNTO> ch_sect;
  vector<PUNTO>::iterator it_pto;
  vector<PIEZA*>::iterator it_pza;
  //Create the convex hull of the pieces in the bin.
  //===============================================================
  convexhull.setID ((-1) * sect.getID ());	//convex hull ID = -ID sect.
  convexhull.setbin (sect.getID ());//Indicates the bin in which these pieces are placed.
  for (it_pza = pb->begin (); it_pza != pb->end (); it_pza++)
    {
      ptos_pza = *(*it_pza)->obtener_puntos ();
      for (it_pto = ptos_pza.begin (); it_pto != ptos_pza.end (); it_pto++)
	{
	  ptos_sect.push_back (*it_pto);
	}
    }
  ch_sect = convex_hull (ptos_sect);//Returns points in the convex hull counter clockwise. Last point equals to first.
  for (int i = 0; i < ch_sect.size () - 1; i++)	//Delete all duplicates created by numerical errors with convexhull.
    {
      if (equal_pto (ch_sect[i], ch_sect[i + 1]))
	{
	  ch_sect.erase (ch_sect.begin () + i);
	}
    }
  ch_sect.back ();	//Move to last point in vector
  ch_sect.pop_back (); //Delete last entry

  for (int i = 0; i < ch_sect.size (); i++)
    convexhull.add_point (ch_sect[i]); //adds convexhull points to convexhull (piece).
  convexhull.initialize_piece ();
  //Redefine edges, since ptos iniciales are placed in origin.
  convexhull.ini_edges ();
  return convexhull;
}

double
convexhull_Utilization (PIEZA P1, PIEZA P2)
{
  double Util;
  vector<PUNTO> ptos_ch, ptos_p1, ptos_p2;
  vector<PUNTO> ch;
  vector<PUNTO>::iterator it_pto;
  ptos_p1 = *P1.obtener_puntos (); //puntos pieza 1
  ptos_p2 = *P2.obtener_puntos (); //puntos pieza 2

  //Create the convex hull of P1 U P2
  for (it_pto = ptos_p1.begin (); it_pto != ptos_p1.end (); it_pto++)
    ptos_ch.push_back (*it_pto);
  for (it_pto = ptos_p2.begin (); it_pto != ptos_p2.end (); it_pto++)
    ptos_ch.push_back (*it_pto);
  ch = convex_hull (ptos_ch);
  ch.back (); //Move to last point in vector
  ch.pop_back (); //Delete last entry
  //=================================

  //Calculate utilization of convex hull.
  double area_ch = calcular_area_exacta (ch);
  Util = (P1.getArea () + P2.getArea ()) / area_ch;
  //=========================================
  return Util;
}

double
convexhull_Height (PIEZA P1, PIEZA P2)
{
  double alto = 0;
  vector<PUNTO> ptos_ch, ptos_p1, ptos_p2;
  vector<PUNTO> ch;
  vector<PUNTO>::iterator it_pto;
  ptos_p1 = *P1.obtener_puntos ();	//puntos pieza 1
  ptos_p2 = *P2.obtener_puntos ();	//puntos pieza 2

  //Create the convex hull of P1 U P2
  for (it_pto = ptos_p1.begin (); it_pto != ptos_p1.end (); it_pto++)
    ptos_ch.push_back (*it_pto);
  for (it_pto = ptos_p2.begin (); it_pto != ptos_p2.end (); it_pto++)
    ptos_ch.push_back (*it_pto);
  ch = convex_hull (ptos_ch);
  ch.back ();	//Move to last point in vector
  ch.pop_back (); //Delete last entry
  //=================================
  for (int i = 0; i < ch.size (); i++)
    {
      if (ch[i].coordy > alto)
	alto = ch[i].coordy;
    }
  //=========================================
  return alto;
}

double
convexhull_Length (PIEZA P1, PIEZA P2)
{
  double ancho = 0;
  vector<PUNTO> ptos_ch, ptos_p1, ptos_p2;
  vector<PUNTO> ch;
  vector<PUNTO>::iterator it_pto;
  ptos_p1 = *P1.obtener_puntos ();	//puntos pieza 1
  ptos_p2 = *P2.obtener_puntos ();	//puntos pieza 2

  //Create the convex hull of P1 U P2
  for (it_pto = ptos_p1.begin (); it_pto != ptos_p1.end (); it_pto++)
    ptos_ch.push_back (*it_pto);
  for (it_pto = ptos_p2.begin (); it_pto != ptos_p2.end (); it_pto++)
    ptos_ch.push_back (*it_pto);
  ch = convex_hull (ptos_ch);
  ch.back ();	//Move to last point in vector
  ch.pop_back (); //Delete last entry
  //=================================
  for (int i = 0; i < ch.size (); i++)
    {
      if (ch[i].coordx > ancho)
	ancho = ch[i].coordx;
    }
  //=========================================
  return ancho;
}

