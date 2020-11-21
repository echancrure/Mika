%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%                                 Copyright 2020 Dr Christophe Meudec 
%%                                     <http://www.echancrure.eu/>
%% This file is part of Mika.
%% Mika is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
%%   the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
%% Mika is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
%%   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
%% You should have received a copy of the GNU General Public License along with Mika.  If not, see <https://www.gnu.org/licenses/>.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%A generic service to create a target all-in-one Sicstus executable 
%It uses Sicstus spld
%It creates a Target.sav, a Target.bat and calls the Target.bat which is using spld to create Target.exe
:- use_module(library(system)).                 %needed for working_directory/2
%Target is the name of the target Scistus executable to create
%called from script_createGenerator.pl and script_createEncoder.pl
createPrologExecutable(Target, Exit_code) :-
        NewWD = 'C:/Users/House/GoogleDrive/Mika/bin/',        %spld really does not like space in directory name, so using the shorthened name here
        working_directory(_, NewWD),
        atom_concat(NewWD, Target, Path_name),
        atom_concat(Path_name, '.sav', SAV_file),
        save_program(SAV_file),                         %save the Sicstus state of the loaded program into <Target>.sav (full path needed)
        atom_concat(Path_name, '.bat', BAT_file),
        open(BAT_file, write, _, [alias(bat_file)]),    %full path needed
        format(bat_file, "REM this batch file is automatically generated by script_createPrologExecutable_util.pl with the argument ~w\n", [Target]),
        %check if vsvars32.bat exists (using Visual Studio 2010) first then set to appropriate location depending whether we are at home, on the laptop or at work
        %current version of spld (distributed with Sicstus 3.12.11) only works with Visual Studio 2010 
        ((VsDevCmd_path = 'C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\Common7\\Tools\\vsvars32.bat', 
         file_exists(VsDevCmd_path)) ->          %on work pc
                true
        ;
                (format(user_output, "Error script_createPrologExecutable_util.pl does not know where to find vsvars32.bat the C compiler bat setup file\n", []),
                 !,
                 fail
                )
        ),
        format(bat_file, "CALL ""~w""\n", [VsDevCmd_path]),
        format(bat_file, "CALL spld --output=~w.exe --static ~w.sav", [Target, Target]),
        close(bat_file),                %<Target>.bat file created full path needed
        system(BAT_file, Exit_code).    %call the bat file created just above