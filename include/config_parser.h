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

#ifndef DUPLICATION_CHECKER_CONFIG_PARSER_H
#define DUPLICATION_CHECKER_CONFIG_PARSER_H

#include "xmlParser.h"
#include "rule.h"
#include "keep_only.h"
#include "quicky_exception.h"
#include <string>
#include <vector>
#include <set>

namespace duplication_checker
{
    class config_parser
    {
      public:

        config_parser( std::vector<rule> & p_rules
                     , std::map<std::string, std::string> & p_sha1_ignore_list
                     , std::vector<keep_only> & p_keep_only
                     , std::set<std::string> & p_path_ignore_list
                     );

        inline
        void
        parse(const std::string & p_file_name);

      private:

        void treat(const XMLNode & p_node);
        void treat_rule(const XMLNode & p_node);
        void treat_keep_only(const XMLNode & p_node);
        void treat_remove(const XMLNode & p_node);
        void treat_keep(const XMLNode & p_node);
        void treat_ignore(const XMLNode & p_node);

        inline
        void
        treat_ignore_path(const XMLNode & p_node);

        static
        std::string
        get_mandatory_attribute( const XMLNode & p_node
                               , const std::string & p_string
                               );

        std::vector<rule> & m_rules;
        std::map<std::string, std::string> & m_sha1_ignore_list;
        std::vector<keep_only> & m_keep_only;
        std::set<std::string> & m_path_ignore_list;
    };

    //-------------------------------------------------------------------------
    config_parser::config_parser( std::vector<rule> & p_rules
                                , std::map<std::string, std::string> & p_sha1_ignore_list
                                , std::vector<keep_only> & p_keep_only
                                , std::set<std::string> & p_path_ignore_list
                                )
    : m_rules(p_rules)
    , m_sha1_ignore_list(p_sha1_ignore_list)
    , m_keep_only(p_keep_only)
    , m_path_ignore_list(p_path_ignore_list)
    {
    }

    //-------------------------------------------------------------------------
    void
    config_parser::parse(const std::string & p_file_name)
    {
        XMLResults l_err= {eXMLErrorNone,0,0};
        XMLNode l_node = XMLNode::parseFile( p_file_name.c_str(), "duplication_checker", &l_err);

        if(eXMLErrorNone != l_err.error)
        {

            if(eXMLErrorFileNotFound == l_err.error)
            {
                throw quicky_exception::quicky_runtime_exception( "File \"" + p_file_name + "\" not found"
                        , __LINE__
                        , __FILE__
                );
            }
            else
            {
                std::string l_error_msg = XMLNode::getError(l_err.error);
                throw quicky_exception::quicky_logic_exception( "\"" + l_error_msg + "\" at line " + std::to_string(l_err.nLine) + " and column " + std::to_string(l_err.nColumn) + " of file \"" + p_file_name + "\""
                        , __LINE__
                        , __FILE__
                );
            }
        }

        treat(l_node);

        std::cout << std::to_string(m_rules.size()) + " rules imported" << std::endl;
        std::cout << std::to_string(m_sha1_ignore_list.size()) + " SHA1 ignore imported" << std::endl;
        std::cout << std::to_string(m_path_ignore_list.size()) + " path ignore imported" << std::endl;
        std::cout << std::to_string(m_keep_only.size()) + " keep only imported" << std::endl;
    }

    //-------------------------------------------------------------------------
    void
    config_parser::treat(const XMLNode & p_node)
    {
        std::string l_node_type = p_node.getName();
        if("rule" == l_node_type)
        {
            treat_rule(p_node);
        }
        else if("keep_only" == l_node_type)
        {
            treat_keep_only(p_node);
        }
        else if("ignore" == l_node_type)
        {
            treat_ignore(p_node);
        }
        else if("ignore_path" == l_node_type)
        {
            treat_ignore_path(p_node);
        }
        else if("remove" == l_node_type)
        {
            treat_remove(p_node);
        }
        else if("keep" == l_node_type)
        {
            treat_keep(p_node);
        }
        else if("duplication_checker" == l_node_type ||
                "rules" == l_node_type ||
                "sha1_ignore_list" == l_node_type ||
                "path_ignore_list" == l_node_type ||
                "keep_only_list" == l_node_type ||
                "keep_list" == l_node_type ||
                "remove_list"  == l_node_type
               )
        {
            for (int l_child_index = 0; l_child_index < p_node.nChildNode(); ++l_child_index)
            {
                treat(p_node.getChildNode(l_child_index));
            }
        }
        else
        {
            throw quicky_exception::quicky_logic_exception( "Unkown node type \"" + l_node_type + "\""
                                                          , __LINE__
                                                          , __FILE__
                                                          );
        }
    }

    //-------------------------------------------------------------------------
    void
    config_parser::treat_rule(const XMLNode & p_node)
    {
        const unsigned int l_nb_attribute = p_node.nAttribute();
        if(3 != l_nb_attribute)
        {
            throw quicky_exception::quicky_logic_exception("Node \"" + std::string(p_node.getName()) + "\" has " + std::to_string(l_nb_attribute) + " instead of 3"
                                                          , __LINE__
                                                          , __FILE__
                                                          );
        }

        std::string l_cmd_attribute = get_mandatory_attribute(p_node, "cmd");
        std::string l_file1_attribute = get_mandatory_attribute(p_node, "file1");
        std::string l_file2_attribute = get_mandatory_attribute(p_node, "file2");
        rule::t_rule_cmd l_rule_cmd = rule::to_rule_cmd(l_cmd_attribute);
        m_rules.emplace_back(l_rule_cmd, l_file1_attribute, l_file2_attribute);
    }

    //-------------------------------------------------------------------------
    void
    config_parser::treat_ignore(const XMLNode & p_node)
    {
        std::string l_sha1_attribute = get_mandatory_attribute(p_node, "sha1");
        std::string l_comment = p_node.getAttribute("comment") != nullptr ? p_node.getAttribute("comment") : "";
        m_sha1_ignore_list.emplace(l_sha1_attribute, l_comment);
    }

    //-------------------------------------------------------------------------
    void
    config_parser::treat_ignore_path(const XMLNode & p_node)
    {
        std::string l_str_attribute = get_mandatory_attribute(p_node, "str");
        m_path_ignore_list.emplace(l_str_attribute);
    }

    //-------------------------------------------------------------------------
    std::string
    config_parser::get_mandatory_attribute( const XMLNode & p_node
                                          , const std::string & p_string
                                          )
    {
        XMLCSTR l_attribute = p_node.getAttribute(p_string.c_str());
        if(nullptr == l_attribute)
        {
            throw quicky_exception::quicky_logic_exception( "Missing mandatory " + p_string + " attribute"
                                                          , __LINE__
                                                          , __FILE__
                                                          );
        }
        return {l_attribute};
    }

    //-------------------------------------------------------------------------
    void
    config_parser::treat_keep_only(const XMLNode & p_node)
    {
        int l_nb_child = p_node.nChildNode();
        if(2 != l_nb_child)
        {
            throw quicky_exception::quicky_logic_exception("Node keep_only should have 2 children instead of " + std::to_string(l_nb_child), __LINE__, __FILE__);
        }
        std::string l_child_name = p_node.getChildNode(0).getName();
        if("keep_list" != l_child_name)
        {
            throw quicky_exception::quicky_logic_exception(R"(Node keep_only first child should be name "keep_list" instead of ")" + l_child_name + R"(")", __LINE__, __FILE__);
        }
        l_child_name = p_node.getChildNode(1).getName();
        if("remove_list" != l_child_name)
        {
            throw quicky_exception::quicky_logic_exception(R"(Node keep_only second child should be name "remove_list" instead of ")" + l_child_name + R"(")", __LINE__, __FILE__);
        }
        m_keep_only.emplace_back();
        treat(p_node.getChildNode(0));
        treat(p_node.getChildNode(1));
    }

    //-------------------------------------------------------------------------
    void
    config_parser::treat_remove(const XMLNode & p_node)
    {
        std::string l_path = get_mandatory_attribute(p_node, "path");
        assert(!m_keep_only.empty());
        m_keep_only.back().add_to_remove(l_path);
    }

    //-------------------------------------------------------------------------
    void
    config_parser::treat_keep(const XMLNode & p_node)
    {
        std::string l_path = get_mandatory_attribute(p_node, "path");
        assert(!m_keep_only.empty());
        m_keep_only.back().add_to_keep(l_path);
    }

}
#endif //DUPLICATION_CHECKER_CONFIG_PARSER_H
// EOF