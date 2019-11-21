/*
      This file is part of duplication_checker
      Copyright (C) 2019  Julien Thevenon ( julien_thevenon at yahoo.fr )

      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 3 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef DUPLICATION_CHECKER_ITEM_H
#define DUPLICATION_CHECKER_ITEM_H

#include <string>

class item
{
  public:

    inline
    item( const std::string & p_sha1
        , const std::string & p_complete_filename
        );

    inline
    const std::string & get_sha1() const;

    inline
    const std::string & get_complete_filename() const;

    inline
    const std::string & get_path() const;

    inline
    const std::string & get_filename() const;

    inline
    std::string get_despecialised_complete_filename() const;

  private:

    std::string m_sha1;
    std::string m_complete_filename;
    std::string m_path;
    std::string m_filename;
};

//-----------------------------------------------------------------------------
item::item( const std::string & p_sha1
          , const std::string & p_complete_filename
          )
: m_sha1(p_sha1)
, m_complete_filename(p_complete_filename)
{
    size_t l_last_separator_pos = m_complete_filename.rfind('/');
    if(std::string::npos != l_last_separator_pos)
    {
        m_path = m_complete_filename.substr(0, l_last_separator_pos);
        m_filename = m_complete_filename.substr(l_last_separator_pos + 1);
    }
    else
    {
        m_filename = m_complete_filename;
    }
    //std::cout << "\"" << m_path << "\"" << std::endl;
    //std::cout << "\"" << m_filename << "\"" << std::endl;
}

//-----------------------------------------------------------------------------
const std::string & item::get_sha1() const
{
    return m_sha1;
}

//-----------------------------------------------------------------------------
const std::string & item::get_complete_filename() const
{
    return m_complete_filename;
}

//-----------------------------------------------------------------------------
const std::string & item::get_path() const
{
    return m_path;
}

//-----------------------------------------------------------------------------
const std::string & item::get_filename() const
{
    return m_filename;
}

//-----------------------------------------------------------------------------
std::string item::get_despecialised_complete_filename() const
{
    std::string l_result = m_complete_filename;
    size_t l_pos = l_result.size() - 1;
    while((l_pos = l_result.find_last_of("'` $()&;", l_pos)) != std::string::npos)
    {
        l_result.insert(l_pos, "\\");
    }
    return l_result;
}

#endif //DUPLICATION_CHECKER_ITEM_H
// EOF