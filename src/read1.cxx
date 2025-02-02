/* 
 *  The SvLis Geometric Modelling Kernel
 *  ------------------------------------
 *
 *  Copyright (C) 1993, 1997, 1998, 2000 
 *  University of Bath & Information Geometers Ltd
 *
 *  http://www.bath.ac.uk/
 *  http://www.inge.com/
 *
 *  Principal author:
 *
 *     Adrian Bowyer
 *     Department of Mechanical Engineering
 *     Faculty of Engineering and Design
 *     University of Bath
 *     Bath BA2 7AY
 *     U.K.
 *
 *     e-mail: A.Bowyer@bath.ac.uk
 *        web: http://www.bath.ac.uk/~ensab/
 *
 *   SvLis is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   Licence as published by the Free Software Foundation; either
 *   version 2 of the Licence, or (at your option) any later version.
 *
 *   SvLis is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public Licence for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   Licence along with svLis; if not, write to the Free
 *   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA,
 *   or see
 *
 *      http://www.gnu.org/
 * 
 * =====================================================================
 *
 * SvLis version 3 read procedures
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 7 September 1996
 * This version: 23 September 2000
 *
 */


#include "sv_std.h"
#include "enum_def.h"
#include "flag.h"
#include "sums.h"
#include "geometry.h"
#include "interval.h"
#include "sv_b_cls.h"
#include "prim.h"
#include "attrib.h"
#include "sv_set.h"
#include "decision.h"
#include "polygon.h"
#include "model.h"
#include "picture.h"
#if macintosh
 #pragma export on
#endif

void read1(istream& s, sv_point& p)
{
	sv_real x,y,z;
	sv_integer di;

	check_token(s, SVT_POINT);
	check_token(s, SVT_OB);
	if(get_token(s, di, x, 0) != SVT_REAL)
		svlis_error("read1(.. sv_point)","dud x token", SV_WARNING);
	if(get_token(s, di, y, 0) != SVT_REAL)
		svlis_error("read1(.. sv_point)","dud y token", SV_WARNING);
	if(get_token(s, di, z, 0) != SVT_REAL)
		svlis_error("read1(.. sv_point)","dud z token", SV_WARNING);
	check_token(s, SVT_CB);
	p = sv_point(x,y,z);
}

void read1(istream& s, sv_line& l)
{
	check_token(s, SVT_LINE);
	check_token(s, SVT_OB);
	read1(s, l.direction);
	read1(s, l.origin);
	check_token(s, SVT_CB);
}

void read1(istream& s, sv_plane& f)
{
	sv_integer id;

	check_token(s, SVT_PLANE);
	check_token(s, SVT_OB);
	read1(s, f.normal);
	if(get_token(s, id, f.d, 0) != SVT_REAL)
		svlis_error("read1(.. sv_plane)", "dud d-term token", SV_WARNING);
	check_token(s, SVT_CB);
}

void read1(istream& s, sv_interval& i)
{
	sv_integer id;
        sv_real ilo, ihi;

	check_token(s, SVT_INTERVAL);
	check_token(s, SVT_OB);
	if(get_token(s, id, ilo, 0) != SVT_REAL)
		svlis_error("read1(.. sv_interval)","dud low token", SV_WARNING);
	if(get_token(s, id, ihi, 0) != SVT_REAL)
		svlis_error("read1(.. sv_interval)","dud high token", SV_WARNING);
	check_token(s, SVT_CB);
        i = sv_interval(ilo, ihi);
}

void read1(istream& s, sv_box& b)
{
	check_token(s, SVT_BOX);
	check_token(s, SVT_OB);
	read1(s, b.xi);
	read1(s, b.yi);
	read1(s, b.zi);
	check_token(s, SVT_CB);
}

// Read a primitive operator

void read1(istream& s, prim_op& o)
{
	char c;

	junk_junk(s);
	s.get(c);
	switch(c)
	{
	case '+': o = SV_PLUS; break;
	case '-': o = SV_MINUS; break;
	case '*': o = SV_TIMES; break;
	case '/': o = SV_DIVIDE; break;
	case '^': o = SV_POW; break;
	case '~': o = SV_COMP; break;
	case '|': o = SV_ABS; break;
	case 'S': o = SV_SIN; break;
	case 'C': o = SV_COS; break;
	case 'E': o = SV_EXP; break;
	case '@': o = SV_SSQRT; break;
        case '%': o = SV_SIGN; break;

	default:
		svlis_error("prim_op::operator>>","dud value read",SV_CORRUPT);
		break;
	}
}

// Read a primitive from a stream

void read1(istream& s, sv_primitive& p)
{
	long p_ptr;
	sv_primitive result, c_1, c_2, g_x, g_y, g_z;
	sv_integer k, id;
	prim_op o;
	sv_integer fl;
	sv_real r, rd;
	sv_plane f;

	if(check_token(s, SVT_PRIM))
	{
		check_token(s, SVT_OB_P);
		get_token(s, p_ptr, rd, 1);
		if(!(result = p_write_list.find(p_ptr)).exists())
		{
			get_token(s, k, rd, 1);
			get_token(s, fl, rd, 1);
			switch(k)
			{
			case SV_REAL: 
				get_token(s, id, r, 0);
				result = sv_primitive(r); 
				break;
			case SV_PLANE: 
				s >> f; 
				result = sv_primitive(f); 
				break;

			case SV_CYLINDER:
			case SV_SPHERE:
			case SV_CONE:
			case SV_TORUS:
			case SV_CYCLIDE:
			case SV_GENERAL:
				junk_junk(s);
				s >> o;
				if (diadic(o))
				{
					read1(s, c_1);
 					read1(s, c_2);
				} else
					read1(s, c_1);

				get_token(s, id, rd, 1);
				if (diadic(o))
					result = sv_primitive(c_1, c_2, o);
				else
					result = sv_primitive(c_1, o);

				result.set_kind(k);
				break;

		// Hard or User-primitive

			default: 
				if (k <= S_U_PRIM)
				  result = read_s(s, k); // read1_s ??? - AB
				else
					result = read_user(s, k); // read1_user ??? - AB
				break;
			}

			result.set_flags_priv(fl);

			p_write_list.add(result, p_ptr);
		}
		check_token(s, SVT_CB_P);
	}

	p = result;
}

// Read a set from a stream

void read1(istream& s, set_op& o)
{
	char c;

	junk_junk(s);
	s.get(c);
	switch(c)
	{
	case '|': o = SV_UNION; break;
	case '&': o = SV_INTERSECTION; break;

	default:
		svlis_error("read1(.. set_op)","dud value read",SV_CORRUPT);
		break;
	}
}

void read1(istream& s, sv_set& sr)
{
	long s_ptr;
	sv_primitive p;
	sv_integer c, f, id;
	set_op o;
	sv_attribute at;
	sv_set result, c_1, c_2;
	sv_real rd;

	
	if (check_token(s, SVT_SET))
	{
		check_token(s, SVT_OB_S);
		get_token(s, s_ptr, rd, 1);
		if (!(result = s_write_list.find(s_ptr)).exists())
		{
			get_token(s, c, rd, 1);
			get_token(s, f, rd, 1);
			switch(c)
			{
			case SV_NOTHING: result = sv_set(SV_NOTHING); break;
			case SV_EVERYTHING: result = sv_set(SV_EVERYTHING); break;

			case 1:	read1(s, p);
				result = sv_set(p);
				break;

			default:
				read1(s, o);
				read1(s, c_1);
				read1(s, c_2);
				result = sv_set(c_1, c_2, o);
			}

			result.set_flags_priv(f);

			get_token(s, id, rd, 1);
			if (id)
			{
				read1(s, at);
				result = result.attribute(at);
			}

			get_token(s, id, rd, 1);
			if (id)
			{
				read1(s, c_1);
				result.set_info->set_complement(c_1);
				c_1.set_info->set_complement(result);
			}

			s_write_list.add(result, s_ptr);
		}
		check_token(s, SVT_CB_S);
	}
	sr = result;
}

// Read a set list from a stream - use recursion to re-create the order

sv_set_list read_sl_r1(istream& s)
{
	sv_set_list sl, n;
	long sl_ptr = 0;
	sv_integer f;
	sv_set a;
	sv_real rd;

	if (check_token(s, SVT_SET_LIST))
	{
		check_token(s, SVT_OB_L);
		get_token(s, sl_ptr, rd, 1);
		if(sl_ptr)
		{
		   if( !(sl = sl_write_list.find(sl_ptr)).exists() )
		   {
			get_token(s, f, rd, 1);
			read1(s, a);
			sl = sv_set_list(a);
			sl.set_flags_priv(f);
		   }
		}
		check_token(s, SVT_CB_L);
	}

	if(sl_ptr)
	{
		n = read_sl_r1(s);
		if (n.exists())
			sl = merge(n, sl.set());
		if(!sl_write_list.find(sl_ptr).exists())
			sl_write_list.add(sl, sl_ptr);
	}
	return(sl);
}

void read1(istream& s, sv_set_list& slr)
{
	slr = read_sl_r1(s);
}

// Read an attribute list from a stream - use recursion to re-create the order

sv_attribute read_at_r1(istream& s)
{
	sv_attribute at, n;
	long a_ptr;
	sv_integer tg;
	sv_user_attribute* u;
	sv_integer f;
	sv_real rd;

	if (check_token(s, SVT_ATTRIBUTE))
	{
		check_token(s, SVT_OB_P);
		get_token(s, a_ptr, rd, 1);
		if(a_ptr)
		{
		  if(!(at = a_write_list.find(a_ptr)).exists())
		  {
			get_token(s, tg, rd, 1);
			read1(s, &u);
			get_token(s, f, rd, 1);
			at = sv_attribute(tg, u);
			at.set_flags_priv(f);
		  }
		}
		check_token(s, SVT_CB_P);
	}

	if(a_ptr)
	{
		n = read_at_r1(s);
		if (n.exists())
			at = merge(n, at);  // BUG 1 (AFW)
        	if(!(a_write_list.find(a_ptr)).exists() )
               		a_write_list.add(at, a_ptr);
	}
	return(at);
}

void read1(istream& s, sv_attribute& atr)
{
	atr = read_at_r1(s);
}

void read1(istream& s, mod_kind& k)
{
	char c;

	junk_junk(s);
	s.get(c);
	switch(c)
	{
	case 'L': k = LEAF_M; break;
	case 'X': k = X_DIV; break;
	case 'Y': k = Y_DIV; break;
	case 'Z': k = Z_DIV; break;

	default:
		svlis_error("read1(.. mod_kind)","dud value read",SV_CORRUPT);
		k = LEAF_M;
		break;
	}
}

// Read a model from a stream

void read1(istream& s, sv_model& mr)
{
	long m_ptr;
	sv_set_list sl;
	sv_integer f, id;
	mod_kind k;
	sv_box b;
	sv_model result, c_1, c_2, pp, m_temp;
	sv_real cut, rd;

	if (check_token(s, SVT_MODEL))
	{
		check_token(s, SVT_OB_M);
		get_token(s, m_ptr, rd, 1);
		if (!(result = m_write_list.find(m_ptr)).exists())
		{
			read1(s, k);
			get_token(s, f, rd, 1);
			read1(s, b);
			read1(s, sl);
			get_token(s, f, rd, 1);
			if(f) read1(s, pp);
			m_temp = sv_model(sl, b, LEAF_M, pp);
			switch(k)
			{
			case LEAF_M: result = m_temp; break;

			case X_DIV:
			case Y_DIV:
			case Z_DIV:
				get_token(s, id, cut, 0);
				read1(s, c_1);
				read1(s, c_2);
				result = sv_model(m_temp, c_1, c_2, k, cut);
				break;

			default:
				svlis_error("read1(sv_model)", "dud kind", SV_CORRUPT);
			}

			result.set_flags_priv(f);

			m_write_list.add(result, m_ptr);

		}
		check_token(s, SVT_CB_M);
	}
	mr = result;
}

// Polygon input

void read1(istream& s, sv_p_gon_kind& pk)
{
	char c;
	junk_junk(s);
	s.get(c);
	switch(c)
	{
	case 'P': pk = PT_SET; break;
	case 'L': pk = P_LINE; break;
	case 'O': pk = P_GON; break;

	default:
		svlis_error("read1(.. sv_p_gon_kind)",
			"dud polygon kind",
			SV_CORRUPT); 
	}
}

void read1(istream& s, sv_p_gon** pg)
{
	sv_point pt, gr;
	short e;
	sv_p_gon_kind k, k0;
	sv_p_gon pg_dum;
	sv_integer id;
	sv_real rd;

	if (check_token(s, SVT_POLYGON))
	{
	    check_token(s, SVT_OB);
	    get_token(s, id, rd, 1);

	    if(id)
	    {
		get_token(s, id, rd, 1);
		if(id)
		{
			read1(s, k0);
			read1(s, pt);
			read1(s, gr);
			get_token(s, id, rd, 1);
			e = (short)id;
			*pg = first_point(pt, k0);
			(*pg)->edge = e;
			(*pg)->g = gr;
			get_token(s, id, rd, 1);
		}
		while (id)
		{
			read1(s, k);
			read1(s, pt);
			read1(s, gr);
			get_token(s, id, rd, 1);
			e = (short)id;
			if (k != k0) 
				svlis_error("read1(.. sv_p_gon**)", 
					"kind changes", 
					SV_WARNING);
			*pg = add_edge(*pg, pt);
			(*pg)->edge = e;
			(*pg)->g = gr;
			get_token(s, id, rd, 1);
		}
	    } else
	    {
		*pg = 0;
	    }
	    check_token(s, SVT_CB);
	} else
		*pg = 0;
}

// Read a picture from a file

// Read a ppm file

void read_ppm_pic1(istream& s, sv_picture** svp)
{
   char buffer[1025];
   char testch;
   sv_integer maxval;

   r_to_eoln(s, buffer, 1024);

// Check for comments

   testch = '\n';
   while( newline(testch) ) s.get(testch);
   if(s.eof())
   {
      svlis_error("read_ppm_pic1(...)",
      	"EOF when checking for comment character",SV_WARNING);
      return;
   }

   while(testch == '#') 
   {
// Read comment up to end of line
      do 
      {
	 s.get(testch);
      } while( !newline(testch) && !s.eof() );
      s.get(testch);
      if( newline(testch) )s.get(testch);
   }
   if(s.eof())
   {
   	svlis_error("read_ppm_pic1(...)",
      	"ppm file is entirely comments",SV_WARNING);
      return;
   }
   if(!newline(testch))s.putback(testch);

   *svp = new sv_picture();
   s >> (*svp)->x_res;
   s >> (*svp)->y_res;
   if(s.rdstate() & ios::badbit)
   {
   	svlis_error("read_ppm_pic1(...)",
      	"cannot read image width and height",SV_WARNING);
	delete *svp;
        *svp = 0;
      return;
   }
   s >> maxval;
   if(s.rdstate() & ios::badbit)
   {
	svlis_error("read_ppm_pic1(...)",
      	"cannot read max pixel value",SV_WARNING);
	delete *svp;
	*svp = 0;
      return;
   }

   testch = '\n';
   while(newline(testch))s.get(testch);
   s.putback(testch);

// Get memory for image
   sv_integer count = ((*svp)->x_res)*((*svp)->y_res);
   if(((*svp)->image_bitmap = 
	(sv_pixel*)malloc(count*sizeof(sv_pixel))) == NULL)
   {
	svlis_error("read_ppm_pic1(...)",
      	"cannot get memory for image",SV_WARNING);
	delete *svp;
	*svp = 0;
      return;
   }


// Read image

   sv_pixel *ptr = (*svp)->image_bitmap;
   sv_real sf = 255.9/(sv_real)maxval;
   sv_integer ct = count;
   char bt;

   while(ct--) {
      s.get(bt);
      ptr->r = (GLubyte)(sf*(sv_real)((unsigned char)bt));
      s.get(bt);
      ptr->g = (GLubyte)(sf*(sv_real)((unsigned char)bt));
      s.get(bt);
      ptr->b = (GLubyte)(sf*(sv_real)((unsigned char)bt));
      if(s.rdstate() & ios::badbit)
      {
			svlis_error("read_ppm_pic1(...)",
      			"error reading image data",SV_WARNING);
			delete *svp;
			*svp = 0;
	 		return;
      }
      ptr++;
   }
}

// Read a BMP file

short get_short1(istream& ifs)
{
   short s, c0, c1;
   char b0, b1;
   ifs.get(b0);
   ifs.get(b1);
   c0 = b0;
   c1 = b1;
   s = (c1 << 8) | c0;
   return(s);
}

long get_long1(istream& ifs)
{
   long l, c0, c1, c2, c3;
   char b0, b1, b2, b3;
   ifs.get(b0);
   ifs.get(b1);
   ifs.get(b2);
   ifs.get(b3);
   c0 = b0;
   c1 = b1;
   c2 = b2;
   c3 = b3;
   l = (c3 << 24) | (c2 << 16) | (c1 << 8) | c0;
   return(l);
}

void read_bmp_pic1(istream& s, sv_picture** svp)
{
    unsigned long off_bytes, head_bytes, dummy, i, j, col;

    dummy = get_long1(s);      // File size in bytes
    dummy = get_short1(s);     // X hotspot
    dummy = get_short1(s);     // Y hotspot 
    off_bytes = get_long1(s);  // Offset to bitmap
    head_bytes = get_long1(s); // Size of the header we're reading


//    if( (head_bytes < SV_BMP_OLDHSZ) || (head_bytes > SV_BMP_OS2HSZ) )
    if(head_bytes != SV_BMP_OS2HSZ) // Force 24 bit BMP
    {
        svlis_error("read_bmp_pic1(...)","file is not a 24-bit BMP image", SV_WARNING);
        return;
    }

// Read the rest of the bitmap header

   unsigned long w, h, d, n_cols, pic_bytes, planes, comp;
   char r, g, b;
   sv_pixel palette[256];

   if( head_bytes == SV_BMP_OLDHSZ )
   {
      w = get_short1(s);        // Width in pixels
      h = get_short1(s);        // Height in pixels
      planes = get_short1(s);   // Number of planes
      d = get_short1(s);        // Bits per pixel
      n_cols = (off_bytes - head_bytes - SV_BMP_FHSZ)/3; // number of colours

// Read the colour palette

      for(i=0; i < n_cols; i++)
      {
         s.get(b);
         s.get(g);
         s.get(r);
         palette[i] = sv_pixel(sv_point((float)r/255.0, (float)g/255.0, (float)b/255.0));
      }
   } else
   {
      w = get_long1(s);         // Width in pixels
      h = get_long1(s);         // Height in pixels
      planes = get_short1(s);   // Number of planes
      d = get_short1(s);        // Bits per pixel
      comp = get_long1(s);      // Compression
      if(comp)
      {
	svlis_error("read_bmp_pic1(...)","sorry - can't read compressed BMP images", 
		SV_WARNING);
        return;
      }        
      for(i = 0; i < (head_bytes - 20); i++) s.get(r);
      n_cols = (off_bytes - head_bytes - SV_BMP_FHSZ)/4;
      if( n_cols > 0 )
         for(i=0; i < n_cols; i++)
	 {
         	col = get_long1(s);
         	palette[i] = sv_pixel(sv_point(((float) (col & 0xff))/255.0, 
			((float) ((col >> 8) & 0xff))/255.0, 
			((float) ((col >> 16) & 0xff))/255.0));
	}
   }

// Create the picture

   *svp = new sv_picture();
   (*svp)->x_res = w;
   (*svp)->y_res = h;
   if(((*svp)->image_bitmap = 
	(sv_pixel*)malloc(w*h*sizeof(sv_pixel))) == NULL)
   {
	svlis_error("read_bmp_picture(...)",
      	"cannot get memory for image",SV_WARNING);
	delete *svp;
	*svp = 0;
        return;
   }

// Read the bitmap

   char index;
   long rowleft = ((d*w + 31)/32)*4 - w*(d/8);
   for(i = 0; i < h; i++)
   {
     for(j = 0; j < w; j++)
     {
	if(n_cols > 0)
	{
		s.get(index);
		(*svp)->pixel(j, h-i-1, palette[index]);
	}
	else
	{
		s.get(b);
		s.get(g);
		s.get(r);
        }
	(*svp)->pixel(j, h-i-1, sv_pixel(sv_point((float)r/255.0, 
		(float)g/255.0, (float)b/255.0)));
     }
     if(rowleft > 0) 
        for(int k = 0; k < rowleft; k++) 
          s.get(r);
   }
}

void read1(istream& s, sv_picture** svp)
{
   *svp = 0;
   junk_junk(s);

// Process header

// Check header: "BM" or "P6"

    char c0, c1;
    s.get(c0);
    s.get(c1);

    if((c0 == 'B') && (c1 == 'M'))
    {
	read_bmp_pic1(s, svp);
	return;
    }
	
    if((c0 == 'P') && (c1 == '6'))
    {
	read_ppm_pic1(s, svp);
	return;
    }
	
    svlis_error("read1(...)","picture file is neither a raw PPM nor a BMP image", 
	SV_WARNING);
}
#if macintosh
 #pragma export off
#endif
