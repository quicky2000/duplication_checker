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

#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <cassert>

class rule
{
  public:

    typedef enum class rule_cmd { RM_FIRST, RM_SECOND, IGNORE} t_rule_cmd;
    inline rule(t_rule_cmd p_cmd, const std::string & p_path_1, const std::string & p_path_2);
    inline t_rule_cmd get_cmd() const;
    inline const std::string & get_path_1() const;
    inline const std::string & get_path_2() const;
    inline bool match(const std::string & p_path_1, const std::string & p_path_2) const;

  private:

    t_rule_cmd m_cmd;
    std::string m_path_1;
    std::string m_path_2;
}
;

rule::rule(t_rule_cmd p_cmd, const std::string & p_path_1, const std::string & p_path_2)
: m_cmd(p_cmd)
, m_path_1(p_path_1)
, m_path_2(p_path_2)
{
}

rule::t_rule_cmd rule::get_cmd() const
{
    return m_cmd;
}

const std::string & rule::get_path_1() const
{
    return m_path_1;
}

const std::string & rule::get_path_2() const
{
    return m_path_2;
}

bool rule::match(const std::string & p_path_1, const std::string & p_path_2) const
{
    return m_path_1 == p_path_1 && m_path_2 == p_path_2;
}

class item
{
  public:
    inline item( const std::string & p_sha1
               , const std::string & p_complete_filename
               );
    inline const std::string & get_sha1() const;
    inline const std::string & get_complete_filename() const;
    inline const std::string & get_path() const;
    inline const std::string & get_filename() const;
    inline std::string get_despecialised_complete_filename() const;
  private:
    std::string m_sha1;
    std::string m_complete_filename;
    std::string m_path;
    std::string m_filename;
}
;

item::item( const std::string & p_sha1
          , const std::string & p_complete_filename
          )
: m_sha1(p_sha1)
, m_complete_filename( p_complete_filename)
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

const std::string & item::get_sha1() const
{
    return m_sha1;
}

const std::string & item::get_complete_filename() const
{
    return m_complete_filename;
}

const std::string & item::get_path() const
{
    return m_path;
}

const std::string & item::get_filename() const
{
    return m_filename;
}

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

int main()
{
  std::ifstream l_input_file;
  l_input_file.open("sorted_sha1sum.log");
  if(!l_input_file.is_open())
  {
	std::cout << "Error opening input file" << std::endl;
        return -1;
  }
  std::ofstream l_output_file;
  l_output_file.open("duplicata.log");
  if(!l_output_file.is_open())
  {
	std::cout << "Error opening output file" << std::endl;
        return -1;
  }
  std::ofstream l_output_cmd_file;
  l_output_cmd_file.open("clean_cmd.bash");
  if(!l_output_cmd_file.is_open())
  {
	std::cout << "Error opening output cmd file" << std::endl;
        return -1;
  }
  l_output_cmd_file << "#!/bin/bash" << std::endl;
  std::string l_previous_sha1;
  std::string l_previous_complete_filename;
  std::string l_previous_line;
  bool l_new_duplicata = false;
  std::vector<item> l_items;
  std::vector<rule> l_rules;

  std::set<std::pair<std::string, std::string> > l_proposed_rules;
  while(!l_input_file.eof())
  {
     std::string l_line;
     getline(l_input_file, l_line);
     //std::cout << "\"" << l_line << "\"" << std::endl;
     if("" != l_line)
     {
         size_t l_space_pos = l_line.find(' ');
         assert(std::string::npos != l_space_pos);
         std::string l_sha1 = l_line.substr(0, l_space_pos);
         std::string l_complete_filename = l_line.substr(l_space_pos + 2);
         //std::cout << "\"" << l_sha1 << "\"" << std::endl;
         //std::cout << "\"" << l_complete_filename << "\"" << std::endl;
         if(l_sha1 == l_previous_sha1)
         {
             if(l_new_duplicata)
             {
                 l_output_file << std::endl;
                 l_output_file << l_previous_line << std::endl;
                 l_items.push_back(item(l_previous_sha1, l_previous_complete_filename));
                 l_new_duplicata = false;
             }
             l_output_file << l_line << std::endl;
             l_items.push_back(item(l_sha1, l_complete_filename));
         }
         else
         {
            if(2 == l_items.size())
            {
                bool l_matched = false;
                for(auto l_iter_rule: l_rules)
                {
                   if(l_iter_rule.match(l_items[0].get_path(), l_items[1].get_path()))
                   {
                       l_matched = true;
                       switch(l_iter_rule.get_cmd())
                       {
                          case rule::t_rule_cmd::RM_FIRST:
                              l_output_cmd_file << "# Rule : \"" << l_iter_rule.get_path_1() << "\" \"" << l_iter_rule.get_path_2() << "\"" << std::endl;
                              l_output_cmd_file << "rm " << l_items[0].get_despecialised_complete_filename() << std::endl;
                          break;
                          case rule::t_rule_cmd::RM_SECOND:
                              l_output_cmd_file << "# Rule : \"" << l_iter_rule.get_path_1() << "\" \"" << l_iter_rule.get_path_2() << "\"" << std::endl;
                              l_output_cmd_file << "rm " << l_items[1].get_despecialised_complete_filename() << std::endl;
                          break;
                          case rule::t_rule_cmd::IGNORE:
                          break;
                       }
		       break;
                   }
                }
                if(!l_matched)
                {
                     if(l_proposed_rules.end() == l_proposed_rules.find(make_pair(l_items[0].get_path(), l_items[1].get_path())))
                     {
                         std::cout << "  l_rules.push_back(rule(rule::t_rule_cmd::IGNORE, \"" << l_items[0].get_path() << "\", \"" << l_items[1].get_path() << "\"));" << std::endl;
                         l_proposed_rules.insert(make_pair(l_items[0].get_path(), l_items[1].get_path()));
                     }
                } 
            }
            l_new_duplicata = true;
            l_items.clear();
         }
         l_previous_line = l_line;
         l_previous_sha1 = l_sha1;
         l_previous_complete_filename = l_complete_filename;
     }
  }
  l_input_file.close();
  l_output_file.close();
  l_output_cmd_file << "#EOF" << std::endl;
  l_output_cmd_file.close();
}
