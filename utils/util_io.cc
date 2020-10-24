/*************************************************************************/
/*                                                                       */
/*                Centre for Speech Technology Research                  */
/*                     University of Edinburgh, UK                       */
/*                    Copyright (c) 1994,1995,1996                       */
/*                        All Rights Reserved.                           */
/*                                                                       */
/*  Permission is hereby granted, free of charge, to use and distribute  */
/*  this software and its documentation without restriction, including   */
/*  without limitation the rights to use, copy, modify, merge, publish,  */
/*  distribute, sublicense, and/or sell copies of this work, and to      */
/*  permit persons to whom this work is furnished to do so, subject to   */
/*  the following conditions:                                            */
/*   1. The code must retain the above copyright notice, this list of    */
/*      conditions and the following disclaimer.                         */
/*   2. Any modifications must be clearly marked as such.                */
/*   3. Original authors' names are not deleted.                         */
/*   4. The authors' names are not used to endorse or promote products   */
/*      derived from this software without specific prior written        */
/*      permission.                                                      */
/*                                                                       */
/*  THE UNIVERSITY OF EDINBURGH AND THE CONTRIBUTORS TO THIS WORK        */
/*  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      */
/*  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL THE UNIVERSITY OF EDINBURGH NOR THE CONTRIBUTORS BE LIABLE     */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    */
/*  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   */
/*  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          */
/*  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       */
/*  THIS SOFTWARE.                                                       */
/*                                                                       */
/*************************************************************************/
/*                      Author :  Paul Taylor                            */
/*                      Date   :  May 1994                               */
/*-----------------------------------------------------------------------*/
/*                     File i/o utility functions                        */
/*                                                                       */
/*=======================================================================*/

#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include "EST_types.h"
#include "EST_String.h"
#include "EST_Pathname.h"
#include "EST_io_aux.h"
#include "EST_string_aux.h"
#include "EST_cutils.h"
#include "EST_Token.h"

#ifdef _WIN32
#include <io.h>
#endif

using namespace std;

EST_String make_tmp_filename()
{
    // returns tmp filename
    char *tname = cmake_tmp_filename();
    EST_String cname = tname;
    wfree(tname);
    return cname;
}

bool readable_file(char *filename)
{
    // Returns true if this is file is readable, false otherwise

    if (streq(filename,"-"))
	return true;
    else if (access(filename,R_OK) == 0)
	return true;
    else
	return false;
}

int writable_file(char *filename)
{
    // Returns true if this is afile is writable or creatable, false
    // otherwise
    // Note this is *not* guaranteed to work, if the file doesn't
    // exist the directory is checked if its writable but it can
    // lie, esp. with ro file systems and NFS.

    if (streq(filename,"-"))
	return true;
    else if (access(filename,W_OK) == 0)
	return true;
    else if ((access(filename,F_OK) == -1) &&  // doesn't exists
	     (access(EST_Pathname(filename).directory(),W_OK) == 0))
	return true;  // probably;
    else
	return false;
}

EST_String stdin_to_file()
{
    /* Copy stding to a file and return the name of that tmpfile */
    EST_String tmpname = (const char *)make_tmp_filename();
    char buff[1024];
    FILE *fd;
    unsigned int n;

    if ((fd = fopen(tmpname,"wb")) == NULL)
    {
	cerr << "Write access failed for temporary file\n";
	return tmpname;
    }	    
    while ((n=fread(buff,1,1024,stdin)) > 0)
	if (fwrite(buff,1,n,fd) != n)
	{
	    cerr << "Write error on temporary file";
	    fclose(fd);
	    return tmpname;
	}
    fclose(fd);
    return tmpname;
}

int Stringtoi(EST_String s, int &success)
{
    char *a;
    int d;

    d = strtol(s, &a, 0);
    success = (*a == '\0') ? 0: 1;

    return d;
}

int Stringtoi(EST_String s)
{
    char *a;
    int d;

    d = strtol(s, &a, 0);

    return d;
}

EST_String itoString(int n)
{
    char tmp[1000];
    
    sprintf(tmp, "%d", n);
    return EST_String(tmp);
}

EST_String ftoString(float n, int pres, int width, int right_justify)
{
    (void)right_justify;
    EST_String val;
    char tmp[10000];
    char spec[100];
    if (width != 0)
        sprintf(spec, "%%%d.%df", width, pres);
    else
        sprintf(spec, "%%.%df", pres);
    sprintf(tmp, spec, n);
    val = tmp;
    return val;
}

// Carry out equivalent of Bourne shell basename command, i.e. strip of
// leading path and optionally remove extension. It assumes directories
// are separated by a forward "/".  This wont work on deviant OSs.
EST_String basename(EST_String full, EST_String ext)
{
    if (full.contains("/"))
    {
	full= full.after(full.index("/", -1));
//	full= full.after("/"); //- don't know why this was here
    }

    if (ext == "*") 
    {
	if (full.contains("."))
	    full = full.before(".", -1); // everything apart from last extension
    }
    else if (ext == "?")
    {
	if (full.contains("."))
	    full = full.before("."); // everything up to first extension
    }
    else if (ext != "")
	full = full.before(ext);
    
    return full;
}

void strip_quotes(EST_String &s, const EST_String quote_char)
{
    // if s is has quote_char as first and/or last char, strip them
    if (s == "")
	return;

    if (quote_char(0) == s(0))
	s = s.after(0);
    if (quote_char(0) == s(s.length()-1))
	s = s.before((int)(s.length()-1));
}

// uncompression via temporary file
EST_String
uncompress_file_to_temporary(const EST_String &filename, const EST_String &prog_name)
{
    
    EST_String new_filename = (const char *)make_tmp_filename();
    EST_String sysstr = prog_name + " " + filename + " > " + new_filename;
    
    //cerr << "Uncompressing file : " << sysstr << endl;
    int stat = system(sysstr);

    if(stat != 0)
    {
	(void)delete_file(new_filename);
	new_filename = "";
    }

    return new_filename;
}

int compress_file_in_place(const EST_String &filename, 
			   const EST_String &prog_name)
{
    return system(prog_name + " " + filename);
}

int compress_file(const EST_String &filename,
		  const EST_String &new_filename, 
		  const EST_String &prog_name)
{

    EST_String sysstr;
    if(new_filename == "-")
	sysstr = prog_name + " " + filename;
    else
	sysstr = prog_name + " " + filename + " > " + new_filename;
    return system(sysstr);
}

/*

EST_read_status load_TList_of_StrVector(EST_TList<EST_StrVector> &w,
					const EST_String &filename,
					const int vec_len)
{

    EST_TokenStream ts;
    EST_String s;
    EST_StrVector v;
    int c;

    if(ts.open(filename) != 0){
	cerr << "Can't open EST_TList<EST_StrVector> file " << filename << endl;
	return misc_read_error;
    }
    
    v.resize(vec_len);
//    ts.set_SingleCharSymbols("");
//    ts.set_PunctuationSymbols("");

    c=0;
    while (!ts.eof())
    {

	s = ts.get().string();
	if(s != "")
	{
	    if(c == vec_len)
	    {
		cerr << "Too many points in line - expected " << vec_len << endl;
		return wrong_format;
	    }
	    else
		v[c++] = s;
	}

	if(ts.eoln())
	{
	    if(c != vec_len)
	    {
		cerr << "Too few points in line - got "
		    << c << ", expected " << vec_len << endl;
		return wrong_format;
	    }
	    else
	    {
		w.append(v);
		c=0;
	    }
	}
    }

    ts.close();
    return format_ok;

}

int ilist_member(const EST_IList &l,int i)
{
    EST_Litem *p;
    for (p = l.head(); p != 0; p = p->next())
	if (l.item(p) == i)
	    return true;

    return false;
}

int ilist_index(const EST_IList &l,int i)
{
    EST_Litem *p;
    int j=0;
    for (p = l.head(); p != 0; p = p->next())
    {
	if (l.item(p) == i)
	    return j;
	j++;
    }

    return -1;
}

int strlist_member(const EST_StrList &l,const EST_String &s)
{
    EST_Litem *p;
    for (p = l.head(); p != 0; p = p->next())
	if (l.item(p) == s)
	    return true;

    return false;
}

int strlist_index(const EST_StrList &l,const EST_String &s)
{
    EST_Litem *p;
    int j=0;
    for (p = l.head(); p != 0; p = p->next())
    {
	if (l.item(p) == s)
	    return j;
	j++;
    }

    return -1;
}

*/
