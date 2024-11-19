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
#include "duplication_checker.h"

int main(int argc,char ** argv)
{
    try
    {
        // Defining application command line parameters
        parameter_manager::parameter_manager l_param_manager("duplication_checker.exe", "--", 0);
        parameter_manager::parameter_if l_input_dir_param("input_dir", true);
        l_param_manager.add(l_input_dir_param);
        parameter_manager::parameter_if l_interactive_param("interactive", true);
        l_param_manager.add(l_interactive_param);

        // Treating parameters
        l_param_manager.treat_parameters(argc,argv);

        std::string l_input_dir = l_input_dir_param.value_set() ? l_input_dir_param.get_value<std::string>() : ".";
        bool l_interactive = l_interactive_param.value_set() ? l_interactive_param.get_value<bool>() : false;

        duplication_checker::duplication_checker l_checker(l_input_dir, l_interactive);
        l_checker.run();
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
