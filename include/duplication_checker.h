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

#ifndef DUPLICATION_CHECKER_DUPLICATION_CHECKER_H
#define DUPLICATION_CHECKER_DUPLICATION_CHECKER_H

#include "config_parser.h"
#include "config_dumper.h"
#include "rule.h"
#include "item.h"
#include "keep_only.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <cassert>
#include <string>

namespace duplication_checker
{
    class duplication_checker
    {
    public:

        inline
        duplication_checker(const std::string & p_input_dir
                           ,bool p_interactive
                           );

        inline
        ~duplication_checker();

        inline
        void
        run();

    private:
        static
        void
        print_items(std::ofstream & p_file
                   ,const std::vector<item> & p_items
                   );

        void
        process_duplicated();

        void
        process_duplicated_couple();

        void
        process_duplicated_list();

        inline
        void
        generate_rm(const std::string & p_remove
                   ,const std::string & p_keep
                   );

        inline
        void
        generate_rm(const std::vector<std::string> & p_remove
                   ,const std::vector<std::string> & p_keep
                   );

        std::ifstream m_input_file;

        /**
         * List duplicated files
         */
        std::ofstream m_output_file;

        /**
         * Command generated according to rules found in config files
         */
        std::ofstream m_output_cmd_file;

        /**
         * Rules loaded from config file
         */
        std::vector<rule> m_rules;

        /**
         * Rules indicating which file to keep when there are more than 2 files
         * with same hase
         */
        std::vector<keep_only> m_keep_only;

        /**
         * Hash to ignore
         */
        std::set<std::string> m_sha1_ignore_list;

        std::vector<item> m_duplicated_items;

        /**
         * Remember couple of paths for which rules has been proposed
         */
        std::set<std::pair<std::string, std::string> > m_proposed_rules;

        /**
         * String to define which path to ignore
         */
        std::set<std::string> m_path_ignore_list;

        bool m_interactive;

        bool m_exit;
    };

    //-------------------------------------------------------------------------
    duplication_checker::duplication_checker(const std::string &p_input_dir
                                            ,bool p_interactive
                                            )
    :m_interactive{p_interactive}
    ,m_exit{false}
    {
        std::string l_input_file_name = p_input_dir + "/sorted_sha1sum.log";
        m_input_file.open(l_input_file_name);
        if(!m_input_file.is_open())
        {
            throw quicky_exception::quicky_runtime_exception(R"(Error opening input file ")" + l_input_file_name + R"(")"
                                                            ,__LINE__
                                                            ,__FILE__
                                                            );
        }

        std::string l_config_file_name = p_input_dir + "/config.xml";
        config_parser l_parser(m_rules, m_sha1_ignore_list, m_keep_only, m_path_ignore_list);
        l_parser.parse(l_config_file_name);

        std::string l_output_file_name = "duplicata.log";
        m_output_file.open(l_output_file_name);
        if(!m_output_file.is_open())
        {
            throw quicky_exception::quicky_runtime_exception(R"(Error opening output file ")" + l_output_file_name + R"(")"
                                                            ,__LINE__
                                                            ,__FILE__
                                                            );
        }

        std::string l_output_cmd_file_name = "clean_cmd.bash";
        m_output_cmd_file.open(l_output_cmd_file_name);
        if(!m_output_cmd_file.is_open())
        {
            throw quicky_exception::quicky_runtime_exception(R"(Error opening output cmd file ")" + l_output_cmd_file_name + R"(")"
                                                            ,__LINE__
                                                            , __FILE__
                                                            );
        }
        m_output_cmd_file << "#!/bin/bash" << std::endl;
    }

    //-------------------------------------------------------------------------
    void
    duplication_checker::run()
    {
        std::string l_previous_sha1;
        while(!m_input_file.eof() && !m_exit)
        {
            std::string l_line;
            getline(m_input_file, l_line);
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
                    process_duplicated();
                    l_previous_sha1 = l_sha1;
                }
                if(m_sha1_ignore_list.end() == m_sha1_ignore_list.find(l_sha1))
                {
                    auto l_iter = std::find_if(m_path_ignore_list.begin()
                                              ,m_path_ignore_list.end()
                                              ,[&](const std::string & p_string) -> bool
                                               {
                                                  return std::string::npos != l_complete_filename.find(p_string);
                                               }
                                              );
                    if(l_iter == m_path_ignore_list.end())
                    {
                        m_duplicated_items.emplace_back(l_sha1, l_complete_filename);
                    }
                }
            }
            else if(m_duplicated_items.size() >= 2)
            {
                process_duplicated();
            }
        }

        config_dumper::dump("updated_config.xml", m_rules, m_keep_only, m_sha1_ignore_list, m_path_ignore_list);
    }

    //-------------------------------------------------------------------------
    void
    duplication_checker::generate_rm(const std::string & p_remove
                                    ,const std::string & p_keep
                                    )
    {
        m_output_cmd_file << "if [ ! -L " << p_keep << " -a -f " << p_keep << " ]" << std::endl;
        m_output_cmd_file << "then" << std::endl;
        m_output_cmd_file << "    rm " << p_remove << std::endl;
        m_output_cmd_file << "elif [ -L " << p_keep << "  ]" << std::endl;
        m_output_cmd_file << "then" << std::endl;
        m_output_cmd_file << R"(    echo ")" << p_keep << R"(" is a link)" << std::endl;
        m_output_cmd_file << "else" << std::endl;
        m_output_cmd_file << R"(    echo ")" << p_keep << R"(" do not exist)" << std::endl;
        m_output_cmd_file << "fi" << std::endl;
    }

    //-------------------------------------------------------------------------
    void
    duplication_checker::generate_rm(const std::vector<std::string> & p_remove
                                    ,const std::vector<std::string> & p_keep
                                    )
    {
        m_output_cmd_file << "ok_to_rm=1" << std::endl;
        for(const auto & l_iter: p_keep)
        {
            m_output_cmd_file << std::endl << R"(# Keep only : ")" << l_iter << R"(")" << std::endl;
            m_output_cmd_file << "if [ ! -f " << l_iter << " -o -L " << l_iter << " ]" << std::endl;
            m_output_cmd_file << "then" << std::endl;
            m_output_cmd_file << "    ok_to_rm=0" << std::endl;
            m_output_cmd_file << "    if [ ! -f " << l_iter << " ]" << std::endl;
            m_output_cmd_file << "    then" << std::endl;
            m_output_cmd_file << R"(        echo "File )" << l_iter << R"( is missing")" << std::endl;
            m_output_cmd_file << "    else" << std::endl;
            m_output_cmd_file << R"(        echo "File )" << l_iter << R"( is a link")" << std::endl;
            m_output_cmd_file << "    fi" << std::endl;
            m_output_cmd_file << "fi" << std::endl;
        }
        m_output_cmd_file << "if [ $ok_to_rm -eq 1  ]" << std::endl;
        m_output_cmd_file << "then" << std::endl;
        for(const auto & l_iter: p_remove)
        {
            m_output_cmd_file << "    rm " << l_iter << std::endl;
        }
        m_output_cmd_file << "fi" << std::endl;
    }

    //-------------------------------------------------------------------------
    void
    duplication_checker::process_duplicated()
    {
        // 2 items with same Sha1
        if(2 == m_duplicated_items.size())
        {
            process_duplicated_couple();
        }
        else if(m_duplicated_items.size() >= 2)
        {
            process_duplicated_list();
        }
        m_duplicated_items.clear();
    }

    //-------------------------------------------------------------------------
    void
    duplication_checker::process_duplicated_couple()
    {
        assert(m_duplicated_items.size() == 2);
        bool l_matched = false;
        // Search if there is a rule for this items
        for(const auto & l_iter_rule: m_rules)
        {
            if(l_iter_rule.match(m_duplicated_items[0].get_path(), m_duplicated_items[1].get_path()))
            {
                l_matched = true;
                // Apply rule
                switch(l_iter_rule.get_cmd())
                {
                    case rule::t_rule_cmd::RM_FIRST:
                        m_output_cmd_file << std::endl << "# Rule : RM_FIRST \"" << l_iter_rule.get_path_1() << "\" \"" << l_iter_rule.get_path_2() << "\"" << std::endl;
                        generate_rm(m_duplicated_items[0].get_despecialised_complete_filename()
                                   , m_duplicated_items[1].get_despecialised_complete_filename()
                                   );
                        break;
                    case rule::t_rule_cmd::RM_SECOND:
                        m_output_cmd_file << std::endl << "# Rule : RM_SECOND \"" << l_iter_rule.get_path_1() << "\" \"" << l_iter_rule.get_path_2() << "\"" << std::endl;
                        generate_rm(m_duplicated_items[1].get_despecialised_complete_filename()
                                   , m_duplicated_items[0].get_despecialised_complete_filename()
                                   );
                        break;
                    case rule::t_rule_cmd::IGNORE:
                        break;
                    case rule::t_rule_cmd::SKIP:
                        print_items(m_output_file, m_duplicated_items);
                        break;
                    default:
                        throw quicky_exception::quicky_logic_exception(R"(Unkown value ")" + rule::to_string(l_iter_rule.get_cmd()) + R"(")"
                                                                      ,__LINE__
                                                                      ,__FILE__
                                                                      );
                }
                m_duplicated_items.clear();
                break;
            }
        }
        if(!l_matched)
        {
            print_items(m_output_file, m_duplicated_items);
            if(m_interactive)
            {
                std::cout << m_duplicated_items[0].get_path() << std::endl;
                std::cout << m_duplicated_items[1].get_path() << std::endl;
                bool l_valid_cmd = true;
                rule::t_rule_cmd l_cmd;
                do
                {
                    std::cout << "Create a rule ? [s/i/rf/rs/q]" << std::endl;
                    std::string l_choice;
                    std::cin >> l_choice;
                    if(l_choice == "s" || l_choice.empty())
                    {
                        l_cmd = rule::t_rule_cmd::SKIP;
                    }
                    else if(l_choice == "i")
                    {
                        l_cmd = rule::t_rule_cmd::IGNORE;
                    }
                    else if(l_choice == "rf")
                    {
                        l_cmd = rule::t_rule_cmd::RM_FIRST;
                    }
                    else if(l_choice == "rs")
                    {
                        l_cmd = rule::t_rule_cmd::RM_SECOND;
                    }
                    else if(l_choice == "q")
                    {
                        m_exit = true;
                        return;
                    }
                    else
                    {
                        l_valid_cmd = false;
                    }
                } while(!l_valid_cmd);
                m_rules.emplace_back(l_cmd, m_duplicated_items[0].get_path(), m_duplicated_items[1].get_path());
            }
            // If there were no rules propose 1 that do nothing
            else if(m_proposed_rules.end() == m_proposed_rules.find(make_pair(m_duplicated_items[0].get_path(), m_duplicated_items[1].get_path())))
            {
                std::cout << R"(<rule cmd="IGNORE" file1=")" << m_duplicated_items[0].get_path() << R"(" file2=")" << m_duplicated_items[1].get_path() << R"(" />)" << std::endl;
                m_proposed_rules.insert(make_pair(m_duplicated_items[0].get_path(), m_duplicated_items[1].get_path()));
            }
        }
    }

    //-------------------------------------------------------------------------
    void
    duplication_checker::process_duplicated_list()
    {
        assert(m_duplicated_items.size() > 2);
        // Create a list of paths corresponding to items
        std::vector<std::string>  l_paths(m_duplicated_items.size());
        unsigned int l_index = 0;
        for(auto const & l_iter:m_duplicated_items)
        {
            l_paths[l_index] = l_iter.get_path();
            ++l_index;
        }
        bool l_matched = false;
        // Search for a rule corresponding to this list of path
        for(auto l_iter: m_keep_only)
        {
            if(l_iter.match(l_paths))
            {
                l_matched = true;
                // If there is a rule generate the corresponding commands
                std::vector<std::string> l_to_keep;
                std::vector<std::string> l_to_remove;
                for(auto const & l_iter_item:m_duplicated_items)
                {
                    if(l_iter.is_to_keep(l_iter_item.get_path()))
                    {
                        l_to_keep.emplace_back(l_iter_item.get_complete_filename());
                    }
                    else
                    {
                        l_to_remove.emplace_back(l_iter_item.get_complete_filename());
                    }
                }
                generate_rm(l_to_remove, l_to_keep);
                break;
            }
        }
        // If there is no rule, log the items as duplicated
        if(!l_matched)
        {
            print_items(m_output_file, m_duplicated_items);
        }
    }

    //-------------------------------------------------------------------------
    void
    duplication_checker::print_items(std::ofstream & p_file
                                    ,const std::vector<item> & p_items
                                    )
    {
        p_file << std::endl;
        for(const auto & l_iter:p_items)
        {
            p_file << l_iter.get_sha1() << "  " << l_iter.get_complete_filename() << std::endl;
        }
    }

    //-------------------------------------------------------------------------
    duplication_checker::~duplication_checker()
    {
        m_input_file.close();
        m_output_file.close();
        m_output_cmd_file << "#EOF" << std::endl;
        m_output_cmd_file.close();
    }
}
#endif //DUPLICATION_CHECKER_DUPLICATION_CHECKER_H
// EOF