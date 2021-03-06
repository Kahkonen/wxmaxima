// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2009-2015 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//            (C) 2014-2015 Gunter Königsmann <wxMaxima@physikbuch.de>
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

#include "Structure.h"

#include <wx/sizer.h>
#include <wx/regex.h>

Structure::Structure(wxWindow* parent, int id) : wxPanel(parent, id)
{
  m_displayedItems = new wxListBox(this, structure_ctrl_id);
  m_regex = new wxTextCtrl(this, structure_regex_id);
  wxFlexGridSizer * box = new wxFlexGridSizer(1);
  box->AddGrowableCol(0);
  box->AddGrowableRow(0);

  box->Add(m_displayedItems, 0, wxEXPAND | wxALL, 0);
  box->Add(m_regex, 0, wxEXPAND | wxALL, 1);

  SetSizer(box);
  box->Fit(this);
  box->SetSizeHints(this);
}

Structure::~Structure()
{
  delete m_regex;
  delete m_displayedItems;
}

void Structure::Update(MathCell* tree, GroupCell *cursorPosition)
{
  int selection = -1;
  if(IsShown())
    {
      GroupCell* cell=  dynamic_cast<GroupCell*>(tree);
      int pos=0;
      m_structure.clear();
      
      // Get a new list of tokens.
      while(cell != NULL)
	{
	  int groupType = cell->GetGroupType();
	  if(
	     (groupType == GC_TYPE_TITLE) ||
	     (groupType == GC_TYPE_SECTION) ||
	     (groupType == GC_TYPE_SUBSECTION) ||
	     (groupType == GC_TYPE_SUBSUBSECTION)
            )
	    m_structure.push_back((MathCell *)cell);
          
          if(cell == cursorPosition)
          {
            if(!m_structure.empty())
              selection = m_structure.size()-1;
          }

	  cell = dynamic_cast<GroupCell*>(cell->m_next);
	}
      
      UpdateDisplay();
      if((selection >= 0)&&(m_displayedItems->GetSelection()!=selection))
        m_displayedItems->SetSelection(selection);
    }
}

void Structure::UpdateDisplay()
{
  wxLogNull disableWarnings;

  wxString regex = m_regex->GetValue();
  wxArrayString items;
  wxRegEx matcher;

  if (regex != wxEmptyString)
    matcher.Compile(regex);

  for (unsigned int i=0; i<m_structure.size(); i++)
  {
    // Indentation further reduces the screen real-estate. So it is to be used
    // sparingly. But we should perhaps add at least a little bit of it to make
    // the list more readable.
    wxString curr;
    switch(dynamic_cast<GroupCell*>(m_structure[i])->GetGroupType())
      {
      case GC_TYPE_TITLE:
	curr = m_structure[i]->ToString();
	break;
      case GC_TYPE_SECTION:
	curr = wxT("  ") + m_structure[i]->ToString();
	break;
      case GC_TYPE_SUBSECTION:
	curr = wxT("    ") + m_structure[i]->ToString();
	m_structure[i]->ToString();
	break;
      case GC_TYPE_SUBSUBSECTION:
	curr = wxT("      ") + m_structure[i]->ToString();
	m_structure[i]->ToString();
	break;
      }

    // Respecting linebreaks doesn't make much sense here.
    curr.Replace(wxT("\n"),wxT(" "));
    
    if (regex.Length()>0 && matcher.IsValid())
      {
	if (matcher.Matches(curr))
	  items.Add(curr);
      }
    else
      items.Add(curr);
  }

  if(items!=m_items_old)
    m_displayedItems->Set(items);
}

void Structure::OnRegExEvent(wxCommandEvent &ev)
{
  UpdateDisplay();
}

BEGIN_EVENT_TABLE(Structure, wxPanel)
  EVT_TEXT(structure_regex_id, Structure::OnRegExEvent)
END_EVENT_TABLE()
