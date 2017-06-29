#include "classes_BPGC_HetBins.h"
vector<double>
calculate_rotations_FitBin_Mirror (NODE &b, PIEZA &p, int num_rot,
				   double epsilon);
double
calculate_rotation_match (EDGES &e1, EDGES &e2);
IRR_BIN
Redefine_NewSection (vector<PUNTO> &rbin, EDGES gc);
bool
right_angles (vector<double> angle);
bool
PieceInSect (PIEZA &candidate, IRR_BIN &current_sect);
vector<double>
calculate_rotations_FitSection_Mirror (vector<EDGES> &sect, PIEZA &p,
				       int num_rot, double epsilon);
bool
FitInSect_IniPcFinSec (vector<EDGES> e_pza, vector<PUNTO> ptos_Sec,
		       EDGES esec_match, EDGES epza_match);
bool
FitInSect_FinPcIniSec (vector<EDGES> e_pza, vector<PUNTO> ptos_Sec,
		       EDGES esec_match, EDGES epza_match);
int
BreakTie (vector<EDGES> ed_pza, vector<PUNTO> ptos_sect, EDGES esec,
	  EDGES epza);
void
trans_piece_ini_fin (PIEZA &item2, EDGES e1, EDGES e2);
void
trans_piece_fin_ini (PIEZA &item2, EDGES e1, EDGES e2);

//=============================================
void
OpenBin (NODE &bin, PIEZA &item)
{
  vector<double> angles = item.get_angles ();
  vector<EDGES> *edges = item.getEdges ();
  vector<double> try_rots;
  EDGES base_bin;
  base_bin.ini.coordx = bin.getL ();
  base_bin.ini.coordy = 0.0;
  base_bin.fin.coordx = 0.0;
  base_bin.fin.coordy = 0.0;
  base_bin.set_mod ();

  //int longest_edge = argmax_mod(*edges);//identifies the longest edge of the piece.
  //=============================================
  try_rots = calculate_rotations_FitBin_Mirror (bin, item, 10, ALPHA);//Rotations that match the piece and the bin.
  //For the rectangle instances:
  //=============================
  //try_rots = calculate_rotations_FitBin_Mirror(bin, item, 2, ALPHA);//Rotations that match the piece and the
  //=============================
  bool placed = false;
  int count_rot = 0;
  if (item.getArea () < bin.get_waste () + TOL)	//Area of piece < free area of bin
    {
      while (!placed && count_rot < try_rots.size ())
	{
	  if (try_rots[count_rot] >= 0)
	    {
	      item.set_mirror (false);
	      item.obtener_rotacion (try_rots[count_rot]);//rotation refers to original piece
	    }
	  else	//rotation refers to piece mirrored
	    {
	      item.set_mirror (true);
	      if (try_rots[count_rot] != -1000) //rotation angle is not 0. 
		item.obtener_rotacion (-1 * try_rots[count_rot]);
	    }
	  //index holds the index of the closest point to Coord (ref point) on the x axis.
	  //==============================================================================
	  if (item.Fits_Bin (bin.getL (), bin.getW ()))
	    {
	      placed = true;
	      bin.include_piece (item);
	    }
	  count_rot++;
	}
      if (!placed)
	{
	  item.obtener_rotacion (0);
	  if (item.Fits_Bin (bin.getL (), bin.getW ()))
	    {
	      placed = true;
	      bin.include_piece (item);
	    }
	}
    }
  else
    {
      cout << "Piece " << item.getID () << " is Bigger than Bin "
	  << bin.getSize () << "\n";
    }
  return;
}

bool
Place_in_NewSection (IRR_BIN &current_sect, vector<PIEZA*> &candidates,
		     EDGES &best_gc, int id)
{
  bool placed = false;
  IRR_BIN old_sec = current_sect; //Copy of the section in case no candidates fit in the new section
  vector<PUNTO> sect_points = *current_sect.get_ptos ();
  //Define the new section we are going to place pieces in
  //int id = current_sect.getID()+1;
  int bin = current_sect.getBin ();
  current_sect = Redefine_NewSection (sect_points, best_gc);
  current_sect.set_ID (id);
  current_sect.setBin (bin);
  sect_points = *current_sect.get_ptos ();
  //Check which of the candidates may fit area-wise in the new section:
  int count = 0;
  while (count < candidates.size ())
    {
      if (candidates[count]->getArea () > current_sect.getArea ()) //The candidate does not fit
	candidates.erase (candidates.begin () + count); //Eliminate the candidate from the candidate list.
      else
	{
	  count++;
	}
    }
  if (candidates.empty ()) //if no candidates left
    {
      current_sect = old_sec;
      return placed;
    }
  //=================================================================
  //There is at least one candidate that may fit in the new section (area-wise).
  int i = 0;
  while (!placed && i < candidates.size ())
    {
      placed = PieceInSect (*candidates[i], current_sect);
      i++;
    }
  if (!placed)
    current_sect = old_sec;
  return placed;
}

bool
PieceInSect (PIEZA &piece, IRR_BIN &current_sect)
{
  if (piece.getArea () > current_sect.getArea ())
    return false;

  bool placed = false;
  vector<double> angle_piece, angle_sect;
  angle_piece = piece.get_angles ();
  angle_sect = current_sect.get_angles ();
  // CONSTRUIMOS EDGES DEL IRR_BIN EN SENTIDO HORARIO (para que la pieza quede dentro):
  vector<EDGES> edges_sec;
  vector<PUNTO> ptos_sect = *current_sect.get_ptos ();
  EDGES aux;
  unsigned long last = ptos_sect.size () - 1;
  for (int i = 0; i < ptos_sect.size () - 1; i++)
    {
      aux.ini = ptos_sect[i];
      aux.fin = ptos_sect[i + 1];
      edges_sec.push_back (aux);
    }
  //Last edge
  aux.ini = ptos_sect[last];
  aux.fin = ptos_sect[0];
  edges_sec.push_back (aux);
  //=====================================================
  vector<EDGES> ed_cand = *piece.getEdges ();
  vector<EDGES> ed_pza;

  vector<double> rots; //vector of possible rotations in which the given piece fits in the given section
  int max_rots = angle_sect.size () * angle_piece.size ();
  max_rots = 2 * max_rots;
//	int max_rots = 2;
  rots = calculate_rotations_FitSection_Mirror (edges_sec, piece, max_rots,
						ALPHA);
  //===================================================================
  int count_rot = 0;
  bool is_in_sect_IpFs = false;
  bool is_in_sect_FpIs = false;
  while (!placed && count_rot < rots.size ())
    {
      //Rotate piece
      if (rots[count_rot] >= 0)
	{
	  piece.set_mirror (false);
	  piece.obtener_rotacion (rots[count_rot]);
	}
      else
	{
	  piece.set_mirror (true);
	  if (rots[count_rot] != -1000) //rotation angle is not 0.
	    piece.obtener_rotacion (-1 * rots[count_rot]);
	}
      ed_pza = *piece.getEdges ();
      vector<EDGES>::iterator it_sec, it_pza;
      //Check which edge of this rotation matches.
      for (it_pza = ed_pza.begin (); it_pza != ed_pza.end () && !placed;
	  it_pza++)
	{
	  for (it_sec = edges_sec.begin (); it_sec != edges_sec.end ();
	      it_sec++)
	    {
	      if (calculate_rotation_match (*it_pza, *it_sec) <= ALPHA)
		{
		  //Need to traslate a copy of the edges to see if it would be feasible.
		  is_in_sect_IpFs = FitInSect_IniPcFinSec (ed_pza, ptos_sect,
							   *it_sec, *it_pza); //check feasibility of matching.
		  is_in_sect_FpIs = FitInSect_FinPcIniSec (ed_pza, ptos_sect,
							   *it_sec, *it_pza);
		  if (is_in_sect_FpIs && is_in_sect_IpFs) //La pieza cabe en ambos extremos
		    {
		      int pos = BreakTie (ed_pza, ptos_sect, *it_sec, *it_pza);
		      if (pos == 0)
			is_in_sect_FpIs = false; //Aplicamos IniPz-FinSec
		      if (pos == 1)
			is_in_sect_IpFs = false; //APlicamos FinPz-IniSec

		    }
		  if (is_in_sect_IpFs)
		    {
		      trans_piece_ini_fin (piece, *it_sec, *it_pza);
		      current_sect.add_piece (piece);
		      placed = true;
		      break;
		    }
		  if (is_in_sect_FpIs)
		    {
		      trans_piece_fin_ini (piece, *it_sec, *it_pza);
		      //Place piece
		      current_sect.add_piece (piece);
		      placed = true;
		      break;
		    }
		}
	    }
	}
      count_rot++;
    }
  if (!placed) //If candidate has not been placed. We need to take it back to original position.
    {
      piece.set_mirror (false);
      piece.obtener_rotacion (0); //Not necessarily true that with this we get the candidate to original position.
    }

  return placed;
}

bool
FillBin (vector<PIEZA> &item, IRR_BIN &csect, EDGES &gc, int id,
	 vector<int> ID_disp)
{
  bool placed = false;
  IRR_BIN old_sec = csect; //Copy of the section in case no candidates fit in the new section
  vector<PUNTO> sect_points = *csect.get_ptos ();
  //Define the new section we are going to place pieces in
  int bin = csect.getBin ();
  csect = Redefine_NewSection (sect_points, gc);
  //In case it has created an unusual section with 2 points (not a polygon)
  //To avoid numerical problems.
  if ((csect.get_ptos ())->size () <= 2)
    {
      csect = old_sec;
      return false;
    }
  //==============================
  csect.set_ID (id);
  csect.setBin (bin);
  sect_points = *csect.get_ptos ();
  //Check wich of the items may fit in the new section in terms of area.
  int count = 0;
  while (count < item.size () && item[count].getArea () > csect.getArea ())
    count++;	//Find first piece that may fit in terms of area.
  while (!placed && count < item.size ())
    {
      vector<int>::iterator it;
      it = find (ID_disp.begin (), ID_disp.end (), item[count].getID ());//Piece has not been placed yet
      if (it != ID_disp.end ())
	{
	  placed = PieceInSect (item[count], csect);	//Place piece
	  count++;
	}
      else
	count++;
    }
  if (!placed)
    csect = old_sec;
  return placed;
}

bool
NextBestCG_candidates (vector<PIEZA*> &item, IRR_BIN &csect, EDGES &gc)
{
  bool placed = false;
  IRR_BIN old_sec = csect; //Copy of the section in case no candidates fit in the new section
  vector<PUNTO> sect_points = *csect.get_ptos ();
  //Define the new section we are going to place pieces in
  int id = csect.getID () + 1;
  int bin = csect.getBin ();
  csect = Redefine_NewSection (sect_points, gc);
  csect.set_ID (id);
  csect.setBin (bin);
  sect_points = *csect.get_ptos ();
  //Check wich of the items may fit in the new section in terms of area.
  int count = 0;
  while (count < item.size () && item[count]->getArea () > csect.getArea ())
    count++; //Find first piece that may fit in terms of area.
  while (!placed && count < item.size ())
    {
      if (!item[count]->is_placed ())
	{
	  placed = PieceInSect (*item[count], csect); //Place piece
	  count++; //Try next piece.
	}
      else
	count++; //Try next piece.
    }
  if (!placed)
    csect = old_sec;
  return placed;
}

