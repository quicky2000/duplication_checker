/*
      This file is part of duplication_checker
      Copyright (C) 2024  Julien Thevenon ( julien_thevenon at yahoo.fr )

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

#ifndef DUPLICATION_CHECKER_CONFIG_DUMPER_H
#define DUPLICATION_CHECKER_CONFIG_DUMPER_H

#include "xmlParser.h"
#include "rule.h"
#include "keep_only.h"
#include <set>
#include <string>
#include <vector>

namespace duplication_checker
{
    class config_dumper
    {
    public:

        inline static
        void
        dump(const std::string & p_file_name
            ,const std::vector<rule> & p_rules
            ,const std::vector<keep_only> & p_keep_only
            ,const std::set<std::string> & p_sha1_ignore_list
            ,const std::set<std::string> & p_path_ignore_list
            );

    private:
    };

    //-------------------------------------------------------------------------
    void
    config_dumper::dump(const std::string & p_file_name
                       ,const std::vector<rule> & p_rules
                       ,const std::vector<keep_only> & p_keep_only
                       ,const std::set<std::string> & p_sha1_ignore_list
                       ,const std::set<std::string> & p_path_ignore_list
                       )
    {
        XMLNode l_root = XMLNode::createXMLTopNode("duplication_checker");
        if(!p_sha1_ignore_list.empty())
        {
            XMLNode l_list = l_root.addChild("sha1_ignore_list");
            for(const auto & l_iter: p_sha1_ignore_list)
            {
                XMLNode l_node = l_list.addChild("ignore");
                l_node.addAttribute("sha1", l_iter.c_str());
            }
        }
        if(!p_path_ignore_list.empty())
        {
            XMLNode l_list = l_root.addChild("path_ignore_list");
            for(const auto & l_iter: p_path_ignore_list)
            {
                XMLNode l_node = l_list.addChild("ignore_path");
                l_node.addAttribute("str", l_iter.c_str());
            }
        }
        if(!p_rules.empty())
        {
            XMLNode l_list = l_root.addChild("rules");
            for(const auto & l_iter: p_rules)
            {
                XMLNode l_node = l_list.addChild("rule");
                l_node.addAttribute("cmd", rule::to_string(l_iter.get_cmd()).c_str());
                l_node.addAttribute("file1", l_iter.get_path_1().c_str());
                l_node.addAttribute("file2", l_iter.get_path_2().c_str());
            }
        }
        if(!p_keep_only.empty())
        {
            XMLNode l_list = l_root.addChild("keep_only_list");
            for(const auto & l_iter: p_keep_only)
            {
                XMLNode l_node = l_list.addChild("keep_only");
                XMLNode l_keep_list = l_node.addChild("keep_list");
                std::function<void(const std::string & p_str)> l_keep_func = [&](const std::string & p_str) -> void
                {
                    l_keep_list.addChild("keep").addAttribute("path", p_str.c_str());
                };
                l_iter.apply_to_keep(l_keep_func);
                XMLNode l_remove_list = l_node.addChild("remove_list");
                std::function<void(const std::string & p_str)> l_rm_func = [&](const std::string & p_str) -> void
                {
                    l_remove_list.addChild("remove").addAttribute("path", p_str.c_str());
                };
                l_iter.apply_to_remove(l_rm_func);
            }
        }
        l_root.writeToFile(p_file_name.c_str());
    }

}
#endif //DUPLICATION_CHECKER_CONFIG_DUMPER_H
