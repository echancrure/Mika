#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <direct.h>
#include <stdio.h>
#include <errno.h>
#include <transform_path_to_prolog.c>

//mika_ada_generator.exe is a wrapper around generator.exe (our test inputs generator compiled separately) 
int main(int argc, char* argv[])
{
	int i;
    char install_dir[_MAX_PATH];
	char parsed_dir[_MAX_PATH];
	char source_file_name[_MAX_FNAME];
    char subprogram_name[_MAX_FNAME];
    char line_no[_MAX_FNAME];
	char driver_file_name[_MAX_FNAME];
	char driver_subprogram_name[_MAX_FNAME];
    char strategy[_MAX_FNAME];
    char check_coverage[_MAX_FNAME];
	char coverage_thoroughness[_MAX_FNAME];
    char context[_MAX_FNAME];
    char create_time_stamped_directory[_MAX_FNAME];
    char debug_mode[_MAX_FNAME];
	char mika_dg[_MAX_PATH];
    int debugMode = 0;
	char driver[_MAX_FNAME];

    char full_path_to_generator_exe[_MAX_PATH];

	char tmp_s[_MAX_PATH*10];        /* temporary string holder */
	char *parsed_dir_prolog;
    char *install_dir_prolog;
    int generator_exit_code;

    strcpy_s(install_dir, "");
    _getcwd(parsed_dir, 256);
    strcpy_s(source_file_name, "");
    strcpy_s(subprogram_name, "");
    strcpy_s(line_no, "_");
	strcpy_s(driver_file_name, "");
	strcpy_s(driver_subprogram_name, "");
    strcpy_s(strategy, "");
    strcpy_s(check_coverage, "no");
	strcpy_s(coverage_thoroughness, "S");	// default : subprogram only
    strcpy_s(context, "");
    strcpy_s(create_time_stamped_directory, "yes");
    strcpy_s(debug_mode, "release");
	strcpy_s(mika_dg, "no");

	for(i=1; i<argc-1; i++) { //processing switches
		if(argv[i][0] == '-') {
			switch(argv[i][1]) {
			  case 'M' :  //specifies the install directory of the parser executable i.e. the full path to the bin directory of Mika file e.g. - M"C:\Program Files\Midoan Software Engineering Solutions\Mika\bin"
			    if (_access(&argv[i][2], 0) == -1) {    //checks if &argv[i][2] is a valid directory
				  fprintf(stdout, "Mika Generator ERROR: indicated install directory (via -M switch) : %s , cannot be accessed\n", &argv[i][2]);
                  fflush(stdout);
                  exit(1);
				}
                strcpy_s(install_dir, &argv[i][2]);
				break;
              case 'c' :    //unsure what this for
                strcpy_s(check_coverage, "yes");
                break;
              case 'C': //Context switch: either ignore the elaboration phase or take it into account
                    if (!strcmp(&argv[i][2], "ignored") || !strcmp(&argv[i][2], "not_ignored"))
                      strcpy_s(context, &argv[i][2]);
                    else {
                      fprintf(stdout, "Mika Generator Warning: invalid -C context switch argument is ignored: %s. Should be '-Cignored' or '-Cnot_ignored'.\n", argv[i]);
			          fflush(stdout);
                    }
                    break;
              case 'd' :
				  if (argv[i][2] == 'f') {			//-df : driver file name
					strcpy_s(driver_file_name, &argv[i][3]);
					_strlwr_s(driver_file_name);
				  }
				  else if (argv[i][2] == 's') {		//-ds : driver subprogram name
					strcpy_s(driver_subprogram_name, &argv[i][3]);
					_strlwr_s(driver_subprogram_name);
				  }
				  else {    //local debug mode
					strcpy_s(debug_mode, "debug");
					debugMode = 1;
				  }
                break;
              case 'l' :    //-l overloaded switch It must be used to provide the line number of the first occurrence of the subprogram in the code (could be in its specification or its body) e.g. - l96
                strcpy_s(line_no, &argv[i][2]);
                break;
              case 'S' :    //-S : the name of the subprogram for which test inputs generation is desired e.g. -SCalcEngineCoolingAir
                strcpy_s(subprogram_name, &argv[i][2]);
				_strlwr_s(subprogram_name);
                break;
              case 'T' :    //-T : the testing strategy switch
                    if (!strcmp(&argv[i][2], "branch") || !strcmp(&argv[i][2], "decision") || !strcmp(&argv[i][2], "mcdc") || !strcmp(&argv[i][2], "query"))
                      strcpy_s(strategy, &argv[i][2]);
                    else if (!strcmp(&argv[i][2], "exception"))
                      strcpy_s(strategy, "rune_coverage");
                    else {
                      fprintf(stdout, "Mika Generator Warning: invalid -T switch argument is ignored: %s. Should be '-Tbranch', '-Tdecision', '-Tmcdc', '-Tquery' or '-Texception'.\n", argv[i]);
			          fflush(stdout);
                    }
                    break;
              case 'o' :    //-o provides the full path of the parsed file
					if (_access(&argv[i][2], 0) == -1) {    //checks if &argv[i][2] is a valid directory
						fprintf(stdout, "Mika Generator ERROR: indicated target path to parsed file (via -o switch) : %s , cannot be accessed\n", &argv[i][2]);
                        fflush(stdout);
                        exit(1);
					}
					strcpy_s(parsed_dir, &argv[i][2]);
					break;
              case 't' :    //-t : disables the time stamped directory feature of the output directory
                strcpy_s(create_time_stamped_directory, "no");
                break;
			  case 'u' :    //-u<S|F|A> indicates the level of coverage desired: S : Subprogram Only, F : Local Call Tree, A : Entire Call Tree.E.g. - uF.If it is not specified - uS is the default.
                strcpy_s(coverage_thoroughness, &argv[i][2]);
                break;
              case 'z' :    //hidden option, mika_dg flags passed to generator -z[g|m]* g sets garbage collection off, m sets message_mode to debug
                strcpy_s(mika_dg, &argv[i][2]);
                break;
			  default :
			    fprintf(stdout, "Mika Generator Warning: unknown option is ignored : %s\n", argv[i]);
                fflush(stdout);
			}
		}
		else {
			fprintf(stdout, "Mika Generator Warning: unknown parameter is ignored : %s\n", argv[i]);
			fflush(stdout);
        }
	}	//end processing switches
	strcpy_s(source_file_name, argv[argc-1]);	//getting the filename which should always be the last argument
	_strlwr_s(source_file_name);				//to lower case
	if (!strcmp(source_file_name, "")) {
		fprintf(stdout, "Mika Generator ERROR: the target source file name has not been set: it should be the last argument in the command line\n");
		fflush(stdout);
        exit(1);
	}
    if (!strcmp(install_dir, "")) {
      fprintf(stdout, "Mika Generator ERROR: the installation directory of the generator tool has not been set: use -M switch on the command line\n");
      fflush(stdout);
      exit(1);
    }
    if (!strcmp(subprogram_name, "")) {
      fprintf(stdout, "Mika Generator ERROR: the subprogram name has not been set: use -S switch on the command line\n");
      fflush(stdout);
      exit(1);
    }
    if (!strcmp(strategy, "")) {
      fprintf(stdout, "Mika Generator ERROR: the test inputs generation strategy has not been set: use -T switch on the command line\n");
      fflush(stdout);
      exit(1);
    }
    if (!strcmp(context, "")) {
      fprintf(stdout, "Mika Generator ERROR: the test inputs generation context has not been set: use -C switch on the command line\n");
      fflush(stdout);
      exit(1);
    }
	if (!strcmp(driver_file_name, "") && !strcmp(driver_subprogram_name, "")) strcpy_s(driver, "no_driver no_driver");
	else if (!strcmp(driver_file_name, "") && strcmp(driver_subprogram_name, "")) {
      fprintf(stdout, "Mika Generator ERROR: the driver subprogram name has been specified but not the driver file name: use -df switch on the command line\n");
      fflush(stdout);
      exit(1);
    }
	else if (strcmp(driver_file_name, "") && !strcmp(driver_subprogram_name, "")) {
      fprintf(stdout, "Mika Generator ERROR: the driver file name has been specified but not the driver subprogram name: use -ds switch on the command line\n");
      fflush(stdout);
      exit(1);
    }
	else {
	  strcpy_s(driver, driver_file_name);
	  strcat_s(driver, " ");
	  strcat_s(driver, driver_subprogram_name);
	}
    strcpy_s(full_path_to_generator_exe, install_dir);
    strcat_s(full_path_to_generator_exe, "\\generator.exe");
    if (_access(full_path_to_generator_exe, 0) == -1) {
      fprintf(stdout, "Mika Generator ERROR: Mika generator executable does not exist : %s , ensure install_dir via the -M switch is properly set\n", full_path_to_generator_exe);
      fflush(stdout);
      exit(1);
    }
	transform_path_to_prolog(parsed_dir, &parsed_dir_prolog);
    transform_path_to_prolog(install_dir, &install_dir_prolog);
	
	strcpy_s(tmp_s, "\"\"");              //the entire string is surrounded with "" because of spaces in install_dir within full_path_to_generator_exe
    strcat_s(tmp_s, full_path_to_generator_exe); 
    strcat_s(tmp_s,  "\" \"");
    strcat_s(tmp_s, install_dir_prolog);
    strcat_s(tmp_s, "\" \"");
	strcat_s(tmp_s, parsed_dir_prolog);
    strcat_s(tmp_s, "\" ");
    strcat_s(tmp_s, source_file_name);
    strcat_s(tmp_s, " ");
    strcat_s(tmp_s, subprogram_name);
    strcat_s(tmp_s, " ");
    strcat_s(tmp_s, line_no);
    strcat_s(tmp_s, " ");
	strcat_s(tmp_s, driver);
	strcat_s(tmp_s, " ");
    strcat_s(tmp_s, strategy);
    strcat_s(tmp_s, " ");
    strcat_s(tmp_s, check_coverage);
    strcat_s(tmp_s, " ");
    strcat_s(tmp_s, context);
    strcat_s(tmp_s, " ");
    strcat_s(tmp_s, create_time_stamped_directory);
    strcat_s(tmp_s, " ");
    strcat_s(tmp_s, debug_mode);
	strcat_s(tmp_s, " ");
    strcat_s(tmp_s, mika_dg);
	strcat_s(tmp_s, " ");
    strcat_s(tmp_s, coverage_thoroughness);
    strcat_s(tmp_s, "\" ");
	if (debugMode) {
		fprintf(stdout, "Mika Generator DEBUG: call to generator: %s\n", tmp_s);
		fflush(stdout);
	}
    generator_exit_code = system(tmp_s);      //generator called
    if (generator_exit_code != 0) {
      fprintf(stdout, "Mika Generator ERROR");
      if (debugMode) fprintf(stdout, ": call was: %s", tmp_s);
      fflush(stdout);
      exit(generator_exit_code);
    }
	return 0;
}