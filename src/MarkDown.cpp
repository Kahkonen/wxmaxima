// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2015      Gunter Königsmann <wxMaxima@physikbuch.de>
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

#include "MarkDown.h"
#include "Config.h"

MarkDownParser::~MarkDownParser()
{
  while(!regexReplaceList.empty())
  {
    delete regexReplaceList.front();
    regexReplaceList.pop_front();
  }
}

MarkDownParser :: MarkDownParser()
{
  m_flowedTextRequested = Config::Get()->m_flowedTextRequested;
}
  
wxString MarkDownParser::MarkDown(wxString str)
{
  // Replace all markdown equivalents of arrows and similar symbols by the
  // according symbols
  for(replaceList::iterator it=regexReplaceList.begin();
      it!=regexReplaceList.end();
      ++it)
    (*it)->DoReplace(&str);
  
  // The result of this action
  wxString result=wxEmptyString;

  // The list of indentation levels for bullet lists we found
  // so far
  std::list <int> indentationLevels;

  // Now process the input string line-by-line.
  wxStringTokenizer lines(str,wxT("\n"),wxTOKEN_RET_EMPTY_ALL);
  while(lines.HasMoreTokens())
  {
    // Determine the amount of indentation and the contents of the rest
    // of the line.
    wxString line = lines.GetNextToken();

    // We will add our own newline characters when needed.
    line.Replace(NewLine(),wxT(" "));

    // Trailing whitespace doesn't help much.
    line=line.Trim();

    wxString str = line;
    str = str.Trim(false);
    size_t index = line.Length()-str.Length();

    // Does the line contain anything other than spaces?
    if(str != wxEmptyString)
    {
      // The line contains actual text..
      
      // If the line begins with a star followed by a space it is part
      // of a bullet list
      if(str.Left(2) == wxT("* "))
      {
        // We are part of a bullet list.

        // Remove the bullet list start marker from our string.
        str = str.Right(str.Length()-2);
        str = str.Trim(false);
        
        // Let's see if this is the first item in the list
        if(indentationLevels.empty())
        {
          // This is the first item => Start the itemization.
          result += itemizeBegin()+itemizeItem();
          indentationLevels.push_back(index );
        }
        else
        {
          // We are inside a bullet list.

          // Are we on a new indentation level?
          if(indentationLevels.back()<index)
          {
            // A new identation level => add the itemization-start-command.
            result += itemizeEndItem() + itemizeBegin();
            indentationLevels.push_back(index);
          }
          
          // End lists if we are at a old indentation level.
          while(!indentationLevels.empty() && (indentationLevels.back() > index))
            {
              result += itemizeEnd();
              indentationLevels.pop_back();
            }
          
          // Add a new item marker.
          result += itemizeEndItem() + itemizeItem();
        }
        result += str;
      }
      else
      {
        // Ordinary text.
        //
        // If we are at a old indentation level we need to end some lists
        // and add a new item if we still are inside a list.
        if(!indentationLevels.empty())
        {
          if(indentationLevels.back() > index)
          {
            if(NewLineBreaksLine() && !m_flowedTextRequested)

            result += itemizeEndItem();
            while((!indentationLevels.empty())&&
                  (indentationLevels.back()>index))
            {
              result += itemizeEnd();
              indentationLevels.pop_back();
            }
            if(!indentationLevels.empty()) result += itemizeItem();
          }
          line = line.Right(line.Length() - index);
        }
        
        // Add the text to the output.        
        result += line + "\n";
      }
    }
    else
    {
      if(lines.HasMoreTokens()) result += NewLine();
    }
  }

  // Close all item lists
  while(!indentationLevels.empty())
  {
    result += itemizeEnd();
    indentationLevels.pop_back();
  }
  return result;
}
  
MarkDownTeX::MarkDownTeX() : MarkDownParser()
{
  regexReplaceList.push_back(
    new RegexReplacer(wxT("\\\\verb\\|<\\|=\\\\verb\\|>\\|"),wxT("\\\\ensuremath{\\\\Longleftrightarrow}")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("=\\\\verb\\|>\\|"),wxT("\\\\ensuremath{\\\\Longrightarrow}")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("\\\\verb\\|<\\|-\\\\verb\\|>\\|"),wxT("\\\\ensuremath{\\\\longleftrightarrow}")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("-\\\\verb\\|>\\|"),wxT("\\\\ensuremath{\\\\longrightarrow}")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("\\\\verb\\|<\\|-"),wxT("\\\\ensuremath{\\\\longleftarrow}")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("\\\\verb\\|<\\|="),wxT("\\\\ensuremath{\\\\leq}")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("\\\\verb\\|>\\|="),wxT("\\\\ensuremath{\\\\geq}")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("\\+/-"),wxT("\\\\ensuremath{\\\\pm}")));
}

MarkDownHTML::MarkDownHTML() : MarkDownParser()
{
  regexReplaceList.push_back(
    new RegexReplacer(wxT("\\&lt;=\\&gt;"),wxT("\\&hArr;")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("=\\&gt;"),wxT("\\&rArr;")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("&lt;-\\&gt;"),wxT("\\&harr;")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("-\\&gt;"),wxT("\\&rarr;")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("\\&lt;-"),wxT("\\&larr;")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("\\&lt;="),wxT("\\&le;")));
  regexReplaceList.push_back(
    new RegexReplacer(wxT("\\&gt;="),wxT("\\&ge;")));;
  regexReplaceList.push_back(
    new RegexReplacer(wxT("\\+/-"),wxT("\\&plusmn;")));;
}
