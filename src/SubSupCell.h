//
//  Copyright (C) 2007-2014 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef SUBSUPCELL_H
#define SUBSUPCELL_H

#include "CellList.h"

class SubSupCell : public Cell
{
public:
  SubSupCell();
  ~SubSupCell();
  Cell* Copy();
  void Destroy();
  void SetBase(CellList *base);
  void SetIndex(CellList *index);
  void SetExponent(CellList *expt);
  void RecalculateSize(CellParser& parser, int fontsize);
  void RecalculateWidths(CellParser& parser, int fontsize);
  void Draw(CellParser& parser, wxPoint point, int fontsize);
  wxString ToString();
  wxString ToTeX();
  wxString ToXML();
  void SelectInner(wxRect& rect, Cell** first, Cell** last);
  void SetParent(Cell *parent);
protected:
  CellList *m_baseCell;
  CellList *m_exptCell;
  CellList *m_indexCell;
};

#endif // SUBSUPCELL_H
