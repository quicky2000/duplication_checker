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

#include <config_parser.h>
#include "rule.h"
#include "item.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <cassert>

void print_items( std::ofstream & p_file
                , const std::vector<item> & p_items
                )
{
    p_file << std::endl;
    for(auto l_iter:p_items)
    {
        p_file << l_iter.get_sha1() << "  " << l_iter.get_complete_filename() << std::endl;
    }
}

int main()
{
    try
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
        std::vector<item> l_items;
        std::vector<rule> l_rules;

        duplication_checker::config_parser l_parser("config.xml", l_rules);

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

                if(l_sha1 != l_previous_sha1)
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
                                std::cout << "<rule cmd=\"IGNORE\" file1=\"" << l_items[0].get_path() << "\" file2=\"" << l_items[1].get_path() << "\" />" << std::endl;
                                l_proposed_rules.insert(make_pair(l_items[0].get_path(), l_items[1].get_path()));
                            }
                        }
                    }

                    if(l_items.size() >= 2)
                    {
                        print_items(l_output_file, l_items);
                    }

                    l_items.clear();
                    l_previous_sha1 = l_sha1;
                }
                l_items.push_back(item(l_sha1, l_complete_filename));
            }
            else if(l_items.size() >= 2)
            {
                print_items(l_output_file, l_items);
            }
        }
        l_input_file.close();
        l_output_file.close();
        l_output_cmd_file << "#EOF" << std::endl;
        l_output_cmd_file.close();
    }
    catch(const quicky_exception::quicky_logic_exception & e)
    {
        std::cout << "ERROR : " << e.what() << std::endl;
    }
    catch(const quicky_exception::quicky_runtime_exception & e)
    {
        std::cout << "ERROR : " << e.what() << std::endl;
    }
}
