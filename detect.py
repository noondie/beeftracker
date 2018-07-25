
import os;
import string;

class Detect: # an empty object

	def __init__(self):
		self.set_data({})

	def __setattr__(self, n, v):
		self.__dict__['__data'][n] = v

	def __getattr__(self, n):
		return self.__dict__['__data'][n]

	def __str__(self):
		return str(self.__dict__['__data'])

	def set_data(self, data):
		self.__dict__['__data'] = data;

def compile_cpp(program, cxxflags):
	# Compiles a program with the 'c++' command.
	#
	# Takes two arguments:
	#
	# program - The text of the C++ program.
	#
	# cxxflags - The flags to be passed to the C++ compiler
	#
	# Returns 1 if the program compiles.
	# Returns 0 if the program does not compile. Errors will be found in
	# config_errors.log.
	#
	# The program text is left in ./test.cpp, and is compiled into
	# the executable program ./test.

	f=open("test.cpp", "w");
	f.write(program);
	f.close();
	res=os.system("c++ test.cpp -fPIC " + cxxflags + " -o test 2>>config_errors.log");
	if (res != 0):
		os.system("echo Failed program was: >> config_errors.log");
		os.system("cat test.cpp >> config_errors.log");
		return 0;
	return 1;

def check_cpp_compile(program, cxxflags):
	# Checks if a program will compile with the 'c++' command.
	#
	# Takes two arguments:
	#
	# program - The text of the C++ program.
	#
	# cxxflags - The flags to be passed to the C++ compiler
	#
	# Returns 1 if the program compiles.
	# Returns 0 if the program does not compile. Errors will be found in
	# config_errors.log.
	#
	# Does not actually run the program.
	#
	# The program and source file are deleted after the test is
	# completed.

	res=compile_cpp(program, cxxflags);
	os.system("rm -f test.cpp test");
	return res;

def check_cpp_output(program, cxxflags):
	did_compile = compile_cpp(program, cxxflags);
	if(did_compile == 0):
		return 0;
	res = os.popen("./test").readlines();
	os.system("rm -f test.cpp test");
	return res;

def parse_libs(cmd):

        ret = { 'flags' : [],  'libs' : [] }

        parts = string.split(os.popen(cmd).readlines()[0])
	for part in parts:
                if part[:2] == "-l":
                        ret['libs'].append(part[2:])
                else:
                        ret['flags'].append(part)

        return ret

def check_pkg_config():

	print "Detecting if PKG-CONFIG is installed... ";

	errorval=os.system("pkg-config --version");


	if (errorval):
		print "Error: cant execute pkg-config, please install pkg-config!"
		return 1;
	else:
		print "pkg-config found!";
		return 0;

def check_libdl(libdata):

	print "Checking for libdl...";
	dl_search_dirs=[ \
		"/sw", \
		"/usr", \
		"/usr/local", \
	];

	# search for extra include dirs to add
	for x in dl_search_dirs:
		if (os.path.isfile(x + "/include/dlfcn.h")):
			print "Found dlfcn.h in " + x + "/include";
			libdata.dl_flags=["-I" + x + "/include"];
			break;

	f=open("test.cpp","w");
	f.write("#include <dlfcn.h>\n#include <stdio.h>\nint main() { printf(\"Testing dlfcn.\"); return 0; }\n");
	f.close();

	for x in dl_search_dirs:
		execline="c++ -L" + x + "/lib " + libdata.dl_flags[0] + " test.cpp -o test -ldl 2>>config_errors.log";
		res=os.system(execline);

		if (res == 0):
			libdata.dl_libs=['dl'];
			libdata.dl_link_flags=["-L" + x + "/lib"];
			os.system("rm test.cpp");
			os.system("rm test");
			return 0;
	return 1;

def check_need_gmp(libdata):
	print "Checking if GMP is needed...",
	if(libdata.have_stdint_h):
		stdint = "#define HAVE_STDINT_H\n";
	elif(libdata.have_msint):
		stdint = "#define HAVE_MSINT\n";
	res=check_cpp_compile(
		"#include <new>\n" +
		stdint +
		"#include \"common/defines/typedefs.h\"\n"+
		"int main() {\n"		+
		"	char DontNeedIt[(sizeof(size_t) < sizeof(Uint64)) ? 1 : -1];\n"	+
		"}\n", "");
	if(res == 0):
		print "Yes."
		print "Checking if GMP is available...",
		res = check_cpp_compile(
			"#include <gmp.h>\n" +
			"int main() {return 0;}\n", "-lgmp");
		if(res == 0):
			print "No."
			print "\n\n**** CANNOT FIND GMP LIBRARY ****\n\n";
			print "\n\nGMP is required to build CheeseTracker on your system."
			sys.exit(1);
		else:
			print "Yes."
			libdata.need_gmp=1;
			return 1;
	else:
		print "No."
		libdata.need_gmp = 0;
		return 0;

# On some systems (namely, those systems running G++ 4.1.2 and newer
# compiler versions), <stdint.h> gets silently included when other C++
# headers are included. As a result, defining __STDC_LIMIT_MACROS just
# before our inclusion of <stdint.h> does not give us the limit macros,
# because <stdint.h> is already included.

def check_stdc_limit_macros(libdata):
	print "Checking if the macro __STDC_LIMIT_MACROS must be"
	print "  defined on the command line...",
	res=check_cpp_compile(
		"#include <iostream>\n"		+
		"#define __STDC_LIMIT_MACROS\n"	+
		"#include <stdint.h>\n"		+
		"int main() {\n"		+
		"	return INT16_MAX;\n"	+
		"}\n", "");
	if(res == 0):
		print "Yup.";
		libdata.need_limit_macros_define=1;
		return 1;
	print "Nope.\n";
	libdata.need_limit_macros_define=0;
	return 0;

def check_madvise(libdata):
	print "Checking for madvise... ",
	res=check_cpp_compile(
		"#include <sys/mman.h>\n"	+
		"int main() {\n"		+
		"	madvise(0, 0, MADV_SEQUENTIAL);\n" +
		"}\n", "");
	if (res == 0):
		print " no. Oh well.";
		libdata.have_madvise=0;
		return 0;
	print " yes.";
	libdata.have_madvise=1;
	return 1;

def check_libaudiofile(libdata):
	print "Checking for libaudiofile...",
	res = check_cpp_compile(
		"#include <audiofile.h>\n" +
		"int main() {\n" +
		"	afNewFileSetup();\n" +
		"	return 0;\n" +
		"}\n", "-laudiofile -lm");
	if(res == 0):
		print " no. Access to lots of file formats is lost.";
		libdata.have_libaudiofile=0;
		return 0;
	print "Yes.";
	libdata.have_libaudiofile=1;
	return 1;

def check_mmap(libdata):
	print "Checking for mmap...",
	res=check_cpp_compile(
		"#define _POSIX_MAPPED_FILES\n" +
		"#include <unistd.h>\n"		+
		"#include <sys/mman.h>\n"	+
		"int main() {\n"		+
		"	mmap(NULL, 0, 0, 0, 0, 0);\n" +
		"	return 0;" +
		"}\n", "");
	if (res == 0):
		print " no. Files won't load as quickly.";
		libdata.have_mmap=0;
		return 0;
	print " yes.";
	libdata.have_mmap=1;
	return 1;

def check_bits_per_byte(libdata):
	print "Checking the number of bits per byte... ",
	res=check_cpp_output(
		"#include <cstdio>\n" +
		"int main() {\n" +
		"	char moby=0;\n"+
		"	char oldmoby=1;\n"+
		"	int ix=0;\n"+
		"	for(ix=0; moby != oldmoby; ix++) {\n"+
		"		oldmoby=moby;\n"+
		"		moby |= 1 << ix;\n"+
		"	}\n"+
		"	printf(\"%d\\n\", ix-1);\n"+
		"}\n", "");
	libdata.bits_per_byte=int(res[0]);
	if(res == 0):
		print "Test program did not compile!";
		return 0;
	print int(res[0]);
	return 1;

def check_stdint_h(libdata):
	print "Checking for <stdint.h> and making sure that integers with 8, 16, 32, and 64";
	print "\tbits are defined...",
	res=check_cpp_compile(
		"#include <stdint.h>\n" +
		"int main() {\n" +
		"	int8_t eight;\n" +
		"	int16_t sixteen;\n" +
		"	int32_t thirtytwo;\n" +
		"	int64_t sixtyfour;\n" +
		"}\n", "");
	if (res == 0):
		print " no... the build MIGHT still work.";
		libdata.have_stdint_h=0;
		return 0;
	print " yes.";
	libdata.have_stdint_h=1;
	return 1;

def check_microsoft_ints(libdata):
	print "Checking for Microsoft-style exact-width integers...",
	res=check_cpp_compile(
		"#include <limits.h>\n" +
		"int main() {\n" +
		"	__int8	eight;\n"+
		"	__int16	sixteen;\n"+
		"	__int32	thirtytwo;\n"+
		"	__int64	sixtyfour;\n"+
		"}\n", "");
	if (res == 0):
		print " no... the build MIGHT still work.";
		libdata.have_msint=0;
		return 0;
	print " UGH! This feels like a Micro$hit environment!";
	libdata.have_msint=1;
	return 1;

def check_xpg_basename(libdata):
	print "Checking for function 'basename' in <libgen.h>...",
	res=check_cpp_compile(
		"#include <libgen.h>\n" +
		"int main() {\n" +
		"	char *buffer;\n" +
		"	basename(buffer);\n" +
		"	return 0;\n"+
		"}\n", "");
	if (res == 0):
		print " no. We'll use our own\n\timplementation.";
		libdata.have_xpg_basename=0;
		return 0;
	print " yes. We'll use a great, big C++ wrapper.";
	libdata.have_xpg_basename=1;
	return 1;

def check_gnu_basename(libdata):
	print "Checking for GNU 'basename' in <cstring>...",
	res=check_cpp_compile(
		"#include <cstring>\n"+
		"int main() {\n" +
		"	const char *buffer;\n"+
		"	basename(buffer);\n"+
		"	return 0;\n"+
		"}\n", "");
	if (res == 0):
		print " no. We'll check for the horrible XPG version.";
		libdata.have_gnu_basename=0;
		return 0;
	print " yes! We can call it directly."; 
	libdata.have_gnu_basename=1;
	return 1;

def check_alsa(libdata):

	print "Checking for ALSA... ",

	errorval=os.system("pkg-config alsa --atleast-version 0.9");

	if (errorval):
		libdata.has_alsa=0;
		print("Alsa v0.9 or greater not found in pkg-config, disabling support");
		return 1;

	print "ALSA found!";

	res=parse_libs("pkg-config alsa --cflags");
	libdata.alsa_flags=res['flags'];
	res=parse_libs("pkg-config alsa --libs");
	libdata.alsa_link_flags=res['flags'];
	libdata.alsa_libs=res['libs'];
	libdata.has_alsa=1;

	return 0;

def check_jack(libdata):

	print "Checking for JACK...",

	errorval=os.system("pkg-config jack --atleast-version 0.72");

	if (errorval):
		libdata.has_jack=0;
		print("Jack v0.72 or greater not found in pkg-config, disabling support");
		return 1;

	print " JACK found!";

	res=parse_libs("pkg-config jack --cflags");
	libdata.jack_flags=res['flags'];
	res=parse_libs("pkg-config jack --libs");
	libdata.jack_link_flags=res['flags'];
	libdata.jack_libs=res['libs'];
	libdata.has_jack=1;

	return 0;

def check_sigc(libdata):

	print "Checking for libsigc++-1.2... ",

	errorval=os.system("pkg-config sigc++-1.2 --modversion");

	if (errorval):
		return 1;

	print "libsigc++-1.2 found!";

	res=parse_libs("pkg-config sigc++-1.2 --cflags");
	libdata.sigc_flags=res['flags'];
	res=parse_libs("pkg-config sigc++-1.2 --libs");
	libdata.sigc_link_flags=res['flags'];
	libdata.sigc_libs=res['libs'];

	return 0;

#list of dirs I can test..

def check_qt(libdata):

	#list of dirs I can test..
	qt_unix_library_dirs = [\
		"",\
		"/usr/lib",\
		"/usr/X11R6/lib",\
		"/usr/lib/x86_64-linux-gnu",\
		"/usr/local/lib",\
	];

	qt_unix_bin_dirs = [\
                "",\
		"/usr/bin",\
		"/usr/X11R6/bin",\
		"/usr/local/bin",\
	];

	qt_unix_include_dirs = [\
		"/usr/include",\
		"/usr/X11R6/include",\
		"/usr/include/x86_64-linux-gnu/qt5",\
		"/usr/local/include",\
	];

	print "QT Check:";

	# FIXME: In QT 4, <qtglobal.h> changes to <Qt/qglobal.h>, and
	# the QT_VERSION_STR macro is defined indirectly in another include
	# file. This means that the 'grep' method used below is useless
	# for QT 4!

	if (os.environ.has_key('QTDIR')):
                qtdir=os.environ['QTDIR'];
		print "$QTDIR exists at, using QTDIR instead of harcdoded pathlist " + qtdir;
		include_qtdir=qtdir+'/include';
		qt_unix_include_dirs=[];
		qt_unix_include_dirs.append(include_qtdir);
		qt_unix_include_dirs.append(include_qtdir + '/qt');
		qt_unix_include_dirs.append(include_qtdir + '/qt5');

		lib_qtdir=qtdir+'/lib';
		qt_unix_library_dirs=[];
		qt_unix_library_dirs.append(lib_qtdir);

		bin_qtdir=qtdir+'/bin';
		qt_unix_bin_dirs=[];
		qt_unix_bin_dirs.append(bin_qtdir);
	else:
		print "$QTDIR not found, you could define this pointing to a proper QT location if not found";
		print "I will try to check if you have Qt in a bunch of paths..";

	print "Looking for QT 5.x Includes:";

        qt_inc_found=0;
        for x in qt_unix_include_dirs:

		file_to_check = "QtCore/qglobal.h"
		current_file=x + "/" + file_to_check;

		print current_file;
		if (os.path.isfile(current_file)):
			print x;
			print "Checking QT version.. \n";
			version=check_cpp_output(
				"#include <QtCore/qglobal.h>\n"   +
				"#include <iostream>\n"   +
				"using namespace std;\n"  +
				"int main() {        \n"  +
				"	cout << QT_VERSION_STR << endl;\n"+
				"	return 0;                      \n" +
				"}\n", "-I" + x);
			if (version == 0):
				print "Cant determine QT version! (Qt/qglobal.h not found at " +x+ ")\n";
				continue;

			ver_str=version[0];
			pos=ver_str.find("5.");
			if (pos >=0 ): #found QT header
				print "Found Qt Version " + ver_str;
				libdata.qt_flags=["-I"+x,"-I"+x+"/QtCore","-I"+x+"/QtGui","-I"+x+"/QtWidgets","-DQT_NO_EMIT"];
				qt_inc_found=1;
				break;

	print "Looking for QT 5.x Libraries:";

	qt_lib_found=0;

        for x in qt_unix_library_dirs:
		if (not qt_inc_found):
			print "Skipping library test because the include files couldn't be found.";
			break;

		test_program="#include <QtCore/qglobal.h>\n #include<stdio.h>\n int main() { \n printf(\"Testing QT: %s\\n\",QT_VERSION);\n return 0;\n }\n";

		f=open("test.cpp","w");
		f.write(test_program);
                f.close();

		auxlibpath="";
		if (len(x)>0):
			auxlibpath="-L"+x;

		print "Searching for libraries in " + x;
		execline="c++ " + auxlibpath + " " + ' '.join(libdata.qt_flags) + " -fPIC  test.cpp -o test -lQtCore -lQtGui 2>> config_errors.log";
		res=os.system(execline);
		print execline;
		os.system("rm test.cpp");
		if (res==0):
			os.system("rm test");
			print "using: -lQtCore -lQtGui " + auxlibpath;
			qt_lib_found=1;
			libdata.qt_link_flags=[auxlibpath];
			libdata.qt_libs=['QtCore', 'QtGui'];
			break;

	print "Looking for QT 5.x 'moc' Binary:";

	qt_found=0;
	
        for x in qt_unix_bin_dirs:
		print x;
		if (not qt_lib_found):
			print "Skipping moc test because libraries weren't found";
			break;

		command="moc";
		if (len(x)):
			command=x+"/moc";

		print "c:" + command;
		res=os.system(command + " -v 2>/dev/null");
		print res;
		#if (res!=256):
                #        continue;

       		version=os.popen(command +" -v 2>&1").readlines();
                if (not len(version)):
                        continue;

		print version;
                pos=version[0].find(" 5.");
		print pos;
                if (pos<0):
                        print("Not version 5:" + command);
                        continue;

       		qt_found=1;
       		print "found moc command: " + command;
       		libdata.moc_bin=command;
		break;


        if (not qt_found):
		print("I Couldnt find QT in your system :(\n");
		print("If you think it is actually installed, you could try the following:\n");
		print("-Define/undefine the $QTDIR env var. Some distros/unixes dont place Qt in standard locations (Like Debian)\n");
		print("-Check if the path where your Qt 5.x stuff is and add it to the list at detect.py!\n");
		print("-detect.py may be buggy. If you fixed it, please send in patches at http://sf.net/projects/cheesetracker\n");

		return 1;

	else:
		print("QT was found!\n");
		return 0;



def check_system(libdata):
	f=os.popen("uname");
	sysname=f.readlines()[0];
	libdata.os_is_cygwin=(sysname.find("CYGWIN")>=0);
	libdata.os_is_macosx=(sysname.find("Darwin")>=0);


def check_oss(libdata):

	print "Detecting if OSS exists on the system..",

	test_program="#include <sys/soundcard.h>\n #include <stdio.h>\n int main() { \n printf(\"Testing OSS: \\n\");\n return 0;\n }\n";

	f=open("test.c","w");
	f.write(test_program);
	f.close();

	execline="cc test.c -o test  2>> config_errors.log"; # 2>/dev/null";
	res=os.system(execline);
	libdata.is_oss_installed=(res==0);
	if (libdata.is_oss_installed):
		print(" yes.");
	else:
		print(" OSS was not detected.");
	os.system("rm test.c");


def read_dep_cache(libdata):
	import os
	if not os.path.exists("detect_cache.py"):
		return 0;

	import detect_cache
	osid = os.popen("uname -a").readlines()[0];
	if (not detect_cache.cached_data.has_key('os_id')) or osid != detect_cache.cached_data['os_id']:
		return 0
	libdata.set_data(detect_cache.cached_data)

	return 1

def write_dep_cache(libdata):

	libdata.os_id = os.popen("uname -a").readlines()[0];

	file = open("detect_cache.py",'wt')
	file.write("cached_data = " + str(libdata))
	file.close()

	return 0;

def check_dependences(libdata):

	check_system(libdata);

	if (read_dep_cache(libdata)):
		return 0;

	if ( check_libdl(libdata) ):
		return 1;

	if ( check_pkg_config() ):
		return 1;

	if ( check_sigc(libdata) ):
		return 1;

	if ( check_qt(libdata) ):
		return 1;


	if( check_bits_per_byte(libdata) == 0 ):
		return 1;
	check_jack(libdata);
	check_alsa(libdata);
	if (check_stdint_h(libdata) != 1):
		check_microsoft_ints(libdata);
	else:
		libdata.have_msint=0;
	if (check_gnu_basename(libdata) != 1):
		check_xpg_basename(libdata);
	else:
		libdata.have_xpg_basename=0;

	if(check_mmap(libdata)):
		check_madvise(libdata);
	else:
		libdata.have_madvise=0;

	check_stdc_limit_macros(libdata);
	check_need_gmp(libdata);
	check_libaudiofile(libdata);

	check_oss(libdata);
	print "Dependency check successful, writing cache";

	write_dep_cache(libdata);

	return 0;
