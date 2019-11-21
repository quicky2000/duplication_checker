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

#ifndef DUPLICATION_CHECKER_RULE_H
#define DUPLICATION_CHECKER_RULE_H

#include <string>

class rule
{
  public:

    typedef enum class rule_cmd
    { RM_FIRST
    , RM_SECOND
    , IGNORE
    } t_rule_cmd;

    inline
    rule( t_rule_cmd p_cmd
        , const std::string & p_path_1
        , const std::string & p_path_2
        );

    inline
    t_rule_cmd get_cmd() const;

    inline
    const std::string & get_path_1() const;

    inline
    const std::string & get_path_2() const;

    inline
    bool match( const std::string & p_path_1
              , const std::string & p_path_2
              ) const;

  private:

    t_rule_cmd m_cmd;
    std::string m_path_1;
    std::string m_path_2;
};

//-----------------------------------------------------------------------------
rule::rule( t_rule_cmd p_cmd
          , const std::string & p_path_1
          , const std::string & p_path_2
          )
: m_cmd(p_cmd)
, m_path_1(p_path_1)
, m_path_2(p_path_2)
{
}

//-----------------------------------------------------------------------------
rule::t_rule_cmd rule::get_cmd() const
{
    return m_cmd;
}

//-----------------------------------------------------------------------------
const std::string & rule::get_path_1() const
{
    return m_path_1;
}

//-----------------------------------------------------------------------------
const std::string & rule::get_path_2() const
{
    return m_path_2;
}

//-----------------------------------------------------------------------------
bool rule::match( const std::string & p_path_1
                , const std::string & p_path_2
                ) const
{
    return m_path_1 == p_path_1 && m_path_2 == p_path_2;
}

#endif //DUPLICATION_CHECKER_RULE_H
// EOF