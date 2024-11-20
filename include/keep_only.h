/*
      This file is part of duplication_checker
      Copyright (C) 2020  Julien Thevenon ( julien_thevenon at yahoo.fr )

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

#ifndef DUPLICATION_CHECKER_KEEP_ONLY_H
#define DUPLICATION_CHECKER_KEEP_ONLY_H

#include <string>
#include <vector>
#include <set>
#include <cassert>
#include <functional>

namespace duplication_checker
{
    class keep_only
    {

      public:

        inline
        keep_only() = default;

        inline
        void add_to_remove(const std::string & p_name);

        inline
        void add_to_keep(const std::string & p_name);

        inline
        bool match(const std::vector<std::string> & p_list);

        inline
        bool is_to_keep(const std::string & p_name) const;

        inline
        bool is_to_remove(const std::string & p_name) const;

        inline
        void
        apply_to_remove(std::function<void(const std::string &)> & p_func) const;

        inline
        void
        apply_to_keep(std::function<void(const std::string &)> & p_func) const;

    private:

        /**
         * Path tho keep
         */
        std::set<std::string> m_to_keep;

        /**
         * Paths to remove
         */
        std::set<std::string> m_to_remove;
    };

    //-------------------------------------------------------------------------
    void
    keep_only::add_to_remove(const std::string & p_name)
    {
        // Ensure there is no contradiction
        assert(m_to_keep.end() == m_to_keep.find(p_name));
        m_to_remove.insert(p_name);
    }

    //-------------------------------------------------------------------------
    void
    keep_only::add_to_keep(const std::string & p_name)
    {
        // Ensure there is no contradiction
        assert(m_to_remove.end() == m_to_remove.find(p_name));
        m_to_keep.insert(p_name);
    }

    //-------------------------------------------------------------------------
    bool
    keep_only::match(const std::vector<std::string> & p_list)
    {
        if(p_list.size() != (m_to_remove.size() + m_to_keep.size()))
        {
            return false;
        }
        return std::all_of(p_list.begin()
                          , p_list.end()
                          , [&](const std::string & p_str) -> bool
                           {
                               return m_to_keep.end() != m_to_keep.find(p_str) ||
                               m_to_remove.end() != m_to_remove.find(p_str);
                           }
                          );
    }

    //-------------------------------------------------------------------------
    bool
    keep_only::is_to_keep(const std::string & p_name) const
    {
        return m_to_keep.end() != m_to_keep.find(p_name);
    }

    //-------------------------------------------------------------------------
    bool
    keep_only::is_to_remove(const std::string & p_name) const
    {
        return m_to_remove.end() != m_to_remove.find(p_name);
    }

    //-------------------------------------------------------------------------
    void
    keep_only::apply_to_remove(std::function<void(const std::string &)> & p_func) const
    {
        for(const auto & l_iter: m_to_remove)
        {
            p_func(l_iter);
        }
    }

    //-------------------------------------------------------------------------
    void
    keep_only::apply_to_keep(std::function<void(const std::string &)> & p_func) const
    {
        for(const auto & l_iter: m_to_keep)
        {
            p_func(l_iter);
        }
    }

}
#endif //DUPLICATION_CHECKER_KEEP_ONLY_H
