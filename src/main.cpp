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

#include "parameter_manager.h"
#include "config_parser.h"
#include "rule.h"
#include "item.h"
#include "keep_only.h"
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
    for(const auto & l_iter:p_items)
    {
        p_file << l_iter.get_sha1() << "  " << l_iter.get_complete_filename() << std::endl;
    }
}

int main(int argc,char ** argv)
{
    try
    {
         // Defining application command line parameters
        parameter_manager::parameter_manager l_param_manager("duplication_checker.exe", "--", 0);
        parameter_manager::parameter_if l_input_dir_param("input_dir", true);
        l_param_manager.add(l_input_dir_param);

        // Treating parameters
        l_param_manager.treat_parameters(argc,argv);

        std::string l_input_dir = l_input_dir_param.value_set() ? l_input_dir_param.get_value<std::string>() : ".";

        std::string l_input_file_name = l_input_dir + "/sorted_sha1sum.log";
        std::ifstream l_input_file;
        l_input_file.open(l_input_file_name);
        if(!l_input_file.is_open())
        {
            std::cout << R"(Error opening input file ")" << l_input_file_name << R"(")" << std::endl;
            return -1;
        }

        std::string l_output_file_name = "duplicata.log";
        std::ofstream l_output_file;
        l_output_file.open(l_output_file_name);
        if(!l_output_file.is_open())
        {
            std::cout << R"(Error opening output file ")" << l_output_file_name << R"(")" << std::endl;
            return -1;
        }

        std::string l_output_cmd_file_name = "clean_cmd.bash";
        std::ofstream l_output_cmd_file;
        l_output_cmd_file.open(l_output_cmd_file_name);
        if(!l_output_cmd_file.is_open())
        {
            std::cout << R"(Error opening output cmd file ")" << l_output_cmd_file_name << R"(")" << std::endl;
            return -1;
        }
        l_output_cmd_file << "#!/bin/bash" << std::endl;
        std::string l_previous_sha1;
        std::vector<item> l_items;
        std::vector<rule> l_rules;
        std::vector<duplication_checker::keep_only> l_keep_only;
        std::set<std::string> l_sha1_ignore_list;
        std::string l_config_file_name = l_input_dir + "/config.xml";
        duplication_checker::config_parser l_parser(l_config_file_name, l_rules, l_sha1_ignore_list, l_keep_only);

        std::set<std::pair<std::string, std::string> > l_proposed_rules;

        while(!l_input_file.eof())
        {
            std::string l_line;
            getline(l_input_file, l_line);
            //std::cout << "\"" << l_line << "\"" << std::endl;
            if(!l_line.empty())
            {
                // Extract SHA1 and file name
                size_t l_space_pos = l_line.find(' ');
                assert(std::string::npos != l_space_pos);
                std::string l_sha1 = l_line.substr(0, l_space_pos);
                std::string l_complete_filename = l_line.substr(l_space_pos + 2);
                //std::cout << "\"" << l_sha1 << "\"" << std::endl;
                //std::cout << "\"" << l_complete_filename << "\"" << std::endl;

                // Sha1 change detection
                if(l_sha1 != l_previous_sha1)
                {
                    // 2 items with same Sha1
                    if(2 == l_items.size())
                    {
                        bool l_matched = false;
                        // Search if there is a rule for this items
                        for(const auto & l_iter_rule: l_rules)
                        {
                            if(l_iter_rule.match(l_items[0].get_path(), l_items[1].get_path()))
                            {
                                l_matched = true;
                                // Apply rule
                                switch(l_iter_rule.get_cmd())
                                {
                                    case rule::t_rule_cmd::RM_FIRST:
                                        l_output_cmd_file << std::endl << "# Rule : \"" << l_iter_rule.get_path_1() << "\" \"" << l_iter_rule.get_path_2() << "\"" << std::endl;
                                        l_output_cmd_file << "rm " << l_items[0].get_despecialised_complete_filename() << std::endl;
                                        break;
                                    case rule::t_rule_cmd::RM_SECOND:
                                        l_output_cmd_file << std::endl << "# Rule : \"" << l_iter_rule.get_path_1() << "\" \"" << l_iter_rule.get_path_2() << "\"" << std::endl;
                                        l_output_cmd_file << "rm " << l_items[1].get_despecialised_complete_filename() << std::endl;
                                        break;
                                    case rule::t_rule_cmd::IGNORE:
                                        break;
                                }
                                l_items.clear();
                                break;
                            }
                        }
                        if(!l_matched)
                        {
                            // If there were no rules propose 1 that do nothing
                            if(l_proposed_rules.end() == l_proposed_rules.find(make_pair(l_items[0].get_path(), l_items[1].get_path())))
                            {
                                std::cout << "<rule cmd=\"IGNORE\" file1=\"" << l_items[0].get_path() << "\" file2=\"" << l_items[1].get_path() << "\" />" << std::endl;
                                l_proposed_rules.insert(make_pair(l_items[0].get_path(), l_items[1].get_path()));
                            }
                        }
                    }

                    if(l_items.size() >= 2)
                    {
                        // Create a list of paths corresponding to items
                        std::vector<std::string>  l_paths(l_items.size());
                        unsigned int l_index = 0;
                        for(auto const & l_iter:l_items)
                        {
                            l_paths[l_index] = l_iter.get_path();
                            ++l_index;
                        }
                        bool l_matched = false;
                        // Search for a rule correspindg to this list of path
                        for(auto l_iter: l_keep_only)
                        {
                            if(l_iter.match(l_paths))
                            {
                                l_matched = true;
                                // If there is a rule generate the corresponding commands
                                for(auto const & l_iter_item:l_items)
                                {
                                    if(l_iter.is_to_keep(l_iter_item.get_path()))
                                    {
                                        l_output_cmd_file << std::endl << R"(# Keep only : ")" << l_iter_item.get_complete_filename() << R"(")" << std::endl;
                                        break;
                                    }
                                }
                                for(auto const & l_iter_item:l_items)
                                {
                                    if(!l_iter.is_to_keep(l_iter_item.get_path()))
                                    {
                                        l_output_cmd_file << "rm " << l_iter_item.get_despecialised_complete_filename() << std::endl;
                                    }
                                }
                                break;
                            }
                        }
                        // If there is no rule, log the items as duplicated
                        if(!l_matched)
                        {
                            print_items(l_output_file, l_items);
                        }
                    }

                    l_items.clear();
                    l_previous_sha1 = l_sha1;
                }
                if(l_sha1_ignore_list.end() == l_sha1_ignore_list.find(l_sha1))
                {
                    l_items.push_back(item(l_sha1, l_complete_filename));
                }
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
        std::cout << "ERROR : " << e.what() << " from " << e.get_file() << ":" << e.get_line() << std::endl;
    }
    catch(const quicky_exception::quicky_runtime_exception & e)
    {
        std::cout << "ERROR : " << e.what() << " from " << e.get_file() << ":" << e.get_line() << std::endl;
    }
}
