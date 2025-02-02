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
 * SvLis primitive procedures
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 11 April 1993
 * This version: 8 March 2000
 *
 */


#include "sv_std.h"
#include "enum_def.h"
#include "sums.h"
#include "flag.h"
#include "geometry.h"
#include "interval.h"
#include "sv_b_cls.h"
#include "prim.h"
#if macintosh
 #pragma export on
#endif

// The second operand of divisions is treated as a general primitive
// rather than a real, to allow for rationals (one day).  The second operand
// of an exponentiation is assumed to be real (and indeed is sometimes
// rounded to an integer, which it ought to be).

// Unique tag

sv_integer sv_primitive::tag() const { return(SVT_F*SVT_PRIM); }

static const sv_primitive zero = sv_primitive(0);

void lazy_grad(const sv_primitive& p, sv_primitive& x, sv_primitive& y, sv_primitive& z)
{
	sv_primitive x1, y1, z1, x2, y2, z2, pm;
	sv_integer k = p.kind();

	switch(k)
	{
	case SV_REAL:
		x = zero;
		y = zero;
		z = zero;
		return;

	case SV_PLANE:
		x = sv_primitive(p.plane().normal.x);
		y = sv_primitive(p.plane().normal.y);
		z = sv_primitive(p.plane().normal.z);
		return;

	case SV_CYLINDER:
	case SV_SPHERE:
	case SV_CONE:
	case SV_TORUS:
	case SV_CYCLIDE:
	case SV_GENERAL:

		if( diadic(p.op()) )
		{
			lazy_grad(p.child_1(), x1, y1, z1);
			lazy_grad(p.child_2(), x2, y2, z2);
		} else
			lazy_grad(p.child_1(), x1, y1, z1);

		switch(p.op())
		{
		case SV_PLUS:
			x = x1 + x2;
			y = y1 + y2;
			z = z1 + z2;
			return;

		case SV_MINUS:
			x = x1 - x2;
			y = y1 - y2;
			z = z1 - z2;
			return;	

		case SV_TIMES:
			x = p.child_1()*x2 + p.child_2()*x1;
			y = p.child_1()*y2 + p.child_2()*y1;
			z = p.child_1()*z2 + p.child_2()*z1;
			return;	

		case SV_DIVIDE:
			x = x1/p.child_2();
			y = y1/p.child_2();
			z = z1/p.child_2();
			return;

		case SV_POW:
			switch(sv_round(p.child_2().real()))
			{
			case 0:
				svlis_error("lazy_grad(...)","zero exponent",SV_WARNING);
				return;

			case 1:
				svlis_error("lazy_grad(...)","exponent of 1",SV_WARNING);
				return;

			case 2:
				pm = p.child_1()*sv_primitive(2.0);
				break;

			default:
				pm = ( p.child_1()^sv_primitive(sv_round(p.child_2().real() - 1)) )*p.child_2();
			}
			x = pm*x1;
			y = pm*y1;
			z = pm*z1;
			return;

		case SV_COMP:
			x = -x1;
			y = -y1;
			z = -z1;
			return;

		case SV_ABS:
		        pm = sign(p);
			x = x1*pm;
			y = y1*pm;
			z = z1*pm;
			return;

		case SV_SIN:
			pm = cos(p.child_1());
			x = x1*pm;
			y = y1*pm;
			z = z1*pm;
			return;

		case SV_COS:
			pm = sin(-p.child_1());
			x = x1*pm;
			y = y1*pm;
			z = z1*pm;
			return;

		case SV_EXP:
			pm = exp(p.child_1());
			x = x1*pm;
			y = y1*pm;
			z = z1*pm;
			return;

// Note that the s_sqrt grad value is the grad of the underlying
// function.  This prevents the magnitude going to
// infinity at the surface of a primitive; Svlis is only concerned with
// the directions of grads, not their magnitude, so this doesn't
// matter.  Beware if your application does use magnitudes,
// though.
	
		case SV_SSQRT:
			x = x1;
			y = y1;
			z = z1;
			return;

		case SV_SIGN:
			x = zero;
			y = zero;
			z = zero;
			return;	

		default:
			svlis_error("lazy_grad", "dud operator", SV_CORRUPT);
		}

	default:
		if (k <= S_U_PRIM)
				;
			else
				;
		svlis_error("lazy_grad", "user primitive", SV_WARNING);

	}
}

// The grad at a point

sv_point sv_primitive::grad(const sv_point& p) const
{
	return( sv_point( this->grad_x().value(p), this->grad_y().value(p), 
		this->grad_z().value(p) ) );
}

// The grad at a point for drawing pictures - the absolute value function
// here returns the grad of its argument.

sv_point sv_primitive::p_grad(const sv_point& p) const
{
        sv_primitive q = *this;
	if(q.op() == SV_ABS) q = q.child_1();
	return( sv_point( q.grad_x().value(p), q.grad_y().value(p), 
		q.grad_z().value(p) ) );
}

// The range of grads in a box

sv_box sv_primitive::grad(const sv_box& b) const
{
	sv_box result;

	if (kind() == SV_PLANE)
	{
		result.xi = sv_interval(grad_x().real(), grad_x().real());
		result.yi = sv_interval(grad_y().real(), grad_y().real());
		result.zi = sv_interval(grad_z().real(), grad_z().real());
	} else                             
		result = sv_box( this->grad_x().range(b), this->grad_y().range(b), 
			 this->grad_z().range(b));

	return(result);
}

               
// Normal user i/o functions

ostream& operator<<(ostream& s, sv_primitive& p)
{
	unwrite(p);
	write_svlis_header(s);
	write(s, p, 0);
	s.flush();
	return(s);
}

istream& operator>>(istream& s, sv_primitive& p)
{
	sv_clear_input_tables();
	check_svlis_header(s);
	read(s, p);
	return(s);
}

// Set some flag bits

void sv_primitive::set_flags(sv_integer f)
{
	set_flags_priv(f & FLAG_MASK);
}
void sv_primitive::reset_flags(sv_integer f)
{
	reset_flags_priv(f & FLAG_MASK);
}



//************************************************************************

// Hard-coded primitives

sv_primitive read_s(istream& s, sv_integer up)
{
	sv_primitive result;

	switch (up)
	{
	default:
		svlis_error("read_s","attempt to read an undefined primitive",SV_CORRUPT);
	}

	return(result);
}

void write_s(ostream& s, sv_integer up)
{
	switch (up)
	{
	default:
		svlis_error("write_s","attempt to write an undefined primitive",SV_WARNING);
	}
}



#define HC_PRIM 100
static sv_point hcp[HC_PRIM];
static sv_real hcr[HC_PRIM];
static sv_integer hc_f = 16;

sv_primitive s_sphere(const sv_point& c, sv_real r)
{
#if 0

//Experimental...

	sv_integer sp = hc_f*4;

	hcp[hc_f] = c;
	hcr[hc_f++] = r;
	if (hc_f > HC_PRIM)
	{
		svlis_error("s_sphere","out of hard-code space",SV_FATAL);
	}
#endif
	sv_primitive result = sv_primitive(0, 1, 2, 3);          // g++ bug workround
	return(result);
}


// Return the bogus degree value

sv_integer degree_s(sv_integer up)
{
	return(2); // well, why not?
}

// Translate a primitive

sv_primitive translate_s(sv_integer up, const sv_point& q)
{
	sv_primitive result;

	switch(up)
	{
	default:
		svlis_error("translate_s","dud tag supplied",SV_WARNING);
	}

	return(result);
}

// Rotate about a line

sv_primitive spin_s(sv_integer up, const sv_line& l, sv_real angle)
{
	sv_primitive result;

	switch(up)
	{
	default:
		svlis_error("spin_s","dud tag supplied",SV_WARNING);
	}

	return(result);
}

// Scale change

sv_primitive scale_s(sv_integer up, const sv_point& c, sv_real s)
{
	sv_primitive result;

	if(s == 0)
		svlis_error("scale_s","zero scale factor",SV_WARNING);

	switch(up)
	{
	default:
		svlis_error("scale_s","dud tag supplied",SV_WARNING);
	}

	return(result);
}

sv_primitive scale_s(sv_integer up, const sv_line& s_ax, sv_real s)
{
	sv_primitive result;

	if(s == 0)
		svlis_error("scale_s","zero scale factor",SV_WARNING);

	switch(up)
	{
	default:
		svlis_error("scale_s","dud tag supplied",SV_WARNING);
	}

	return(result);
}

// Mirror in a plane

sv_primitive mirror_s(sv_integer up, const sv_plane& m)
{
	sv_primitive result;

	switch(up)
	{
	default:
		svlis_error("mirror_s","dud tag supplied",SV_WARNING);
	}

	return(result);
}

// Value of a primitive for a point

sv_real value_s(sv_integer up, const sv_point& q)
{
	sv_integer aptr = up/4;

	switch(up & 3)
	{
	case 0: return(  (q - hcp[aptr]).mod() - hcr[aptr] );

	case 1: return( q.x - hcp[aptr].x );

	case 2: return( q.y - hcp[aptr].y );

	case 3: return( q.z - hcp[aptr].z );

	default:
		svlis_error("value_s","dud tag supplied",SV_WARNING);
	}

	return(0.0);
}

// Value of a box in a primitive

sv_interval range_s(sv_integer up, const sv_box& b)
{
	sv_integer aptr = up/4;

	sv_box bb = b - hcp[aptr];

	switch(up & 3)
	{
	case 0: return(pow(bb.xi,2) + pow(bb.yi,2) + pow(bb.zi,2) -
			hcr[aptr]*hcr[aptr]);

	case 1: return( bb.xi );

	case 2: return( bb.yi );

	case 3: return( bb.zi );

	default:
		svlis_error("range_s","dud tag supplied",SV_WARNING);
	}

	return(sv_interval(1,0));
}

// Complement a primitive

sv_primitive complement_s(sv_integer up)
{
	sv_primitive result;

	switch(up)
	{
	default:
		svlis_error("complement_s","dud tag supplied",SV_WARNING);
	}

	return(result);
}

// ************************************************************************


// Primitive I/O

istream& operator>>(istream& s, prim_op& o)
{
	if(get_read_version() == SV_VER)
		read(s, o);
	else
	{
		if(get_read_version() != (SV_VER - 1))
		svlis_error("operator>>(.. prim_op)",
			"file is too old",
			SV_WARNING);
		read1(s,o);
	}
	return(s);
}
 
// Flag a primitive as not written.

void unwrite(sv_primitive& p)
{
	sv_primitive p_temp;
	p.reset_flags_priv(WRIT_BIT);
	if(p.child_1().exists())
	{
		p_temp = p.child_1();
		unwrite(p_temp);
		if(diadic(p.op()))
		{
			p_temp = p.child_2();
			unwrite(p_temp);
		}
	}
	/*if(p.grad_x.exists())
	{	
		p_temp = p.grad_x();
		unwrite(p_temp);
		p_temp = p.grad_y();
		unwrite(p_temp);
		p_temp = p.grad_z();
		unwrite(p_temp);
	}*/
}

void write(ostream& s, const prim_op& o, sv_integer level)
{
	put_white(s, level);
	switch(o)
	{
	case SV_PLUS: s << '+'; break;
	case SV_MINUS: s << '-'; break;
	case SV_TIMES: s << '*'; break;
	case SV_DIVIDE: s << '/'; break;
	case SV_POW: s << '^'; break;
	case SV_COMP: s << '~'; break;
	case SV_ABS: s << '|'; break;
	case SV_SIN: s << 'S'; break;
	case SV_COS: s << 'C'; break;
	case SV_EXP: s << 'E'; break;
	case SV_SSQRT: s << '@'; break; // Well, any better ideas?
        case SV_SIGN: s << '%'; break;  //  "     "     "     "  ?

	default:
		svlis_error("prim_op::operator<<","dud value",SV_CORRUPT);
		break;
	}
}

// Write a primitive to a stream.  NB - no need to write out the degree

void write(ostream& s, sv_primitive& p, sv_integer level)
{
	put_white(s,level);
	put_token(s, SVT_PRIM, 0, 0);
	s << SV_EL;
	put_white(s,level);
	put_token(s, SVT_OB_P, 0, 0);
	s << SV_EL;

	long p_ptr = p.unique();
	sv_primitive p_temp;
	sv_integer k = p.kind();
	sv_integer nxl = level+1;

	writei(s, p_ptr, nxl); s << ' ';
	if (!(p.flags() & WRIT_BIT))
	{
		writei(s, k, 0); s << ' ';
		writei(s, p.flags(), 0); s << ' ';
		p.set_flags_priv(WRIT_BIT);
		switch(k)
		{
// Bug here?  If the real or plane is a complement the comp
// doesn't get written.
		case SV_REAL: s << SV_EL; writer(s, p.real(), nxl); s << SV_EL; break;
		case SV_PLANE: s << SV_EL; write(s, p.plane(), nxl); s << SV_EL; break;

		case SV_CYLINDER:
		case SV_SPHERE:
		case SV_CONE:
		case SV_TORUS:
		case SV_CYCLIDE:
		case SV_GENERAL:
			write(s, p.op(), 0); s << SV_EL;
			if (diadic(p.op()))
			{
				p_temp = p.child_1();
				write(s, p_temp, nxl);
				p_temp = p.child_2();
				write(s, p_temp, nxl);
			} else
			{
				p_temp = p.child_1();
				write(s, p_temp, nxl);
			}

			// Don't bother to write grads - lazy evaluator will rebuild them

			writei(s, 0, nxl); s << SV_EL;

			break;

	// Hard or User-prim
	
		default:
			if (k <= S_U_PRIM)
				write_s(s, k);
			else
				write_user(s, k);
			s << SV_EL;
			break;
		}
	} else
		s << SV_EL;
	put_white(s,level);
	put_token(s, SVT_CB_P, 0, 0);
	s << SV_EL;
}

// The look-up table for primitive input

look_up<sv_primitive> p_write_list; 

void clean_primitive_lookup()
{
	p_write_list.clean();
}

// Special shapes

// Real distance potential function flag

static sv_integer real_d = 0;

void real_distance(sv_integer f) { real_d = f; }

sv_primitive p_cylinder(const sv_line& axis, sv_real radius)
{

//  This returns an infinitely long cylinder aligned with the line axis
//  and of radius radius.
    
	sv_point ax = axis.direction;
	sv_point cent = axis.origin;

//   Generate a vector perpendicular to the cylinder's axis.

	sv_point srad0 = right(ax);

//   And another perpendicular to both

	sv_point srad1 = srad0^ax;

//  Generate two perpendicular planes intersecting in the axis

	sv_primitive hs0 = sv_plane(srad0, cent);
	sv_primitive hs1 = sv_plane(srad1, cent);

//  The product of their squares - radius^2 is the cylinder

	sv_primitive c = (hs0^2) + (hs1^2) - sv_primitive(radius*radius);
	c.set_kind(SV_CYLINDER);
	if (real_d)
		return(s_sqrt(c));
	else
		return(c);
}


// Note the next is a double cone, meeting at the apex.

sv_primitive p_cone(const sv_line& axis, sv_real angle)
{
	sv_point ax = axis.direction;
	sv_point cent = axis.origin;

//   Generate a vector perpendicular to the cone's axis.

	sv_point srad0 = right(ax);

//   And another perpendicular to both

	sv_point srad1 = srad0^ax;

//  Generate two perpendicular planes intersecting in the axis

	sv_primitive hs0 = sv_plane(srad0, cent);
	sv_primitive hs1 = sv_plane(srad1, cent);

//  Generate a third through the vertex

	sv_primitive hs2 = sv_plane(ax,cent);

//  The product of their squares - radius^2 is the cone; radius is
//  the distance from hs2 times the tan of half the angle

	sv_real rfac = (sv_real)tan(0.5*angle);

	sv_primitive c;

	c = (hs0^2) + (hs1^2) - ((hs2*rfac)^2);

	c.set_kind(SV_CONE);
	if (real_d)
		return(s_sqrt(c)); // This ain't quite right
	else
		return(c);
}

sv_primitive p_sphere(const sv_point& centre, sv_real radius)
{
//	return(s_sphere(centre, radius));

	sv_primitive xhs = sv_plane(SV_X,centre);
	sv_primitive yhs = sv_plane(SV_Y,centre);
	sv_primitive zhs = sv_plane(SV_Z,centre);

	sv_primitive s;

	s = (xhs^2) + (yhs^2) + (zhs^2) - sv_primitive(radius*radius);

	s.set_kind(SV_SPHERE);
	if (real_d)
		return(s_sqrt(s));
	else
		return(s);
}

sv_primitive p_cyclide(const sv_line& axis, const sv_point& sym, sv_real a, sv_real m, sv_real c)
{
        sv_point ax, cent, srad1, srad2;
        sv_primitive hs1, hs2, hs3;
        sv_primitive  t;
	sv_real b2 = a*a + c*c;
        ax = axis.direction;
        cent = axis.origin;
        srad2 = ax^(sym - axis.origin);
        srad1 = srad2^ax;
        hs1 = sv_primitive(sv_plane(srad1,cent));
        hs2 = sv_primitive(sv_plane(srad2,cent));
        hs3 = sv_primitive(sv_plane(ax,cent));
        t = (((hs1^2) + (hs2^2) + (hs3^2) + sv_primitive(b2 - m*m))^2) - 
		((sv_primitive(2*a)*hs1 - sv_primitive(2*c*m))^2) -
		sv_primitive(4*b2)*(hs2^2);
	t.set_kind(SV_CYCLIDE);
        return(t);
}

// Jakob's much better torus formula

sv_primitive p_torus(const sv_line& axis, sv_real rr, sv_real r)
{
        sv_point ax, cent, srad1, srad2;
        sv_primitive hs1, hs2, hs3, x, y, z;
        sv_primitive  t, t1;

// tg is to get the grad right.

	sv_primitive tg = p_cyclide(axis, axis.origin + right(axis.direction), 
			rr, r, 0);
	lazy_grad(tg, x, y, z);

// Now the torus

        ax = axis.direction;
        cent = axis.origin;
	srad1 = right(ax);
        srad2 = srad1^ax;
        hs1 = sv_primitive(sv_plane(srad1,cent));
        hs2 = sv_primitive(sv_plane(srad2,cent));
        hs3 = sv_primitive(sv_plane(ax,cent));
        t = (s_sqrt((hs2^2) + (hs1^2)) - sv_primitive(rr));
        t = (hs3^2) + (t^2) - sv_primitive(r*r);
	t.set_kind(SV_TORUS);

// explicitly set the gradients

	*(t.prim_info->grad_x) = x;
	*(t.prim_info->grad_y) = y;
	*(t.prim_info->grad_z) = z;

        return(t);
}




// Return the parameters of a primitive
//
// 		k is the kind; if this is SV_GENERAL no other information is returned
//		r0 is the real for reals, and the (major) radius for other kinds
//		r1 is the minor radius for tori
//		f is the plane for planes
//		cen is the centre for spheres
//		axis is the axis for cones and cylinders
//
// The prim_op returned is one of:
//
//    SV_PLUS - straight primitive
//    SV_COMP - the primitive is a complement (i.e. a hollow sphere or whatever)
//    SV_TIMES - Special case: scaled plane; the factor is in r0
//    SV_ABS - the primitive is thin
//    SV_SSQRT - the primitive has been signed-square-rooted
//    SV_SIGN - the primitive has been signed

prim_op sv_primitive::parameters(sv_integer* k, sv_real* r0, sv_real* r1, sv_real* r2, sv_plane* f, 
			sv_point* cen, sv_line* axis) const
{
	prim_op oo;
	prim_op o = op();	
	if ((o == SV_ABS) || ( o == SV_SSQRT) || (o == SV_COMP) || (o == SV_SIGN))
	{
		oo = child_1().parameters(k,r0,r1,r2,f,cen,axis);
		switch(oo)
		{
		case SV_ABS:
			return(oo);
		case SV_COMP:
			if(o == SV_ABS)
				return(o);
			else
				return(oo);
		default:
			return(o);
		}
	}
	
	*k = kind();
	switch(*k)
	{
	case SV_REAL:
		*r0 = real();
		return(SV_PLUS);		
		
	case SV_PLANE:
	case SV_CYLINDER:
	case SV_SPHERE:
	case SV_CONE:
	case SV_TORUS:
	case SV_CYCLIDE:
	case SV_GENERAL:
		// Check for scaled planes
		if (o == SV_TIMES)
		{
			if ((child_1().kind() == SV_REAL) && (child_2().kind() == SV_PLANE))
			{
			      child_2().parameters(k,r0,r1,r2,f,cen,axis);
			      *r0 = child_1().real();
			      return(SV_TIMES);
			} else
			{			
			  if ((child_2().kind() == SV_REAL) && (child_1().kind() == SV_PLANE))
			  {
			      child_1().parameters(k,r0,r1,r2,f,cen,axis);
			      *r0 = child_2().real();
			      return(SV_TIMES);
			  }			
			}
		}
	    break;
		
	default:
		return(SV_PLUS);
	}

	sv_plane f1, f2, f3;
	sv_primitive p1, p2, p3;
	sv_real t;
	flag_val flag;
	
	switch(*k)
	{
	case SV_PLANE:
		*f = plane();
		break;
		
	case SV_CYLINDER:
		p1 = child_1().child_1().child_1();
		if (p1.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud cylinder plane 1",SV_CORRUPT);
		f1 = p1.plane();
		
		p2 = child_1().child_2().child_1();
		if (p2.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud cylinder plane 2",SV_CORRUPT);
		f2 = p2.plane();
		*axis = planes_line(f1, f2);

		p3 = child_2();
		if (p3.kind() != SV_REAL)
			svlis_error("sv_primitive::parameters(...)","dud cylinder radius",SV_CORRUPT);
		*r0 = sqrt(p3.real());		
		break;
				
	case SV_SPHERE:
		p1 = child_1().child_1().child_1().child_1();
		if (p1.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud sphere plane 1",SV_CORRUPT);
		f1 = p1.plane();
		
		p2 = child_1().child_1().child_2().child_1();
		if (p2.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud sphere plane 2",SV_CORRUPT);
		f2 = p2.plane();
		
		p3 = child_1().child_2().child_1();
		if (p3.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud sphere plane 3",SV_CORRUPT);
		f3 = p3.plane();
				
		*cen = planes_point(f1, f2, f3);

		p3 = child_2();
		if (p3.kind() != SV_REAL)
			svlis_error("sv_primitive::parameters(...)","dud sphere radius",SV_CORRUPT);
		*r0 = sqrt(p3.real());		
		break;
			
	case SV_CONE:
		p1 = child_1().child_1().child_1();
		if (p1.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud cone plane 1",SV_CORRUPT);
		f1 = p1.plane();
		
		p2 = child_1().child_2().child_1();
		if (p2.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud cone plane 2",SV_CORRUPT);
		f2 = p2.plane();
		*axis = planes_line(f1, f2);

		p3 = child_2().child_1().child_1();
		if (p3.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud cone plane 3",SV_CORRUPT);
		f3 = p3.plane();
		t = plane_line_t(f3, *axis, flag);
		if (flag)
			svlis_error("sv_primitive::parameters(...)","dud cone plane/line intersection",
				SV_CORRUPT);
		axis->origin = line_point(*axis, t);
		
		p3 = child_2().child_1().child_2();
		if (p3.kind() != SV_REAL)
			svlis_error("sv_primitive::parameters(...)","dud cone angle",SV_CORRUPT);
		t = p3.real();
		*r0 = 2.0*atan(t);				
		break;
				
	case SV_TORUS:        
		p1 = child_1().child_2().child_1().child_1().child_1().child_2().child_1();
		if (p1.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud torus plane 1",SV_CORRUPT);
		f1 = p1.plane();
		
		p2 = child_1().child_2().child_1().child_1().child_1().child_1().child_1();
		if (p2.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud torus plane 2",SV_CORRUPT);
		f2 = p2.plane();
		*axis = planes_line(f1, f2);

		p3 = child_1().child_1().child_1();
		if (p3.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud torus plane 3",SV_CORRUPT);
		f3 = p3.plane();
		t = plane_line_t(f3, *axis, flag);
		if (flag)
			svlis_error("sv_primitive::parameters(...)","dud torus plane/line intersection",
				SV_CORRUPT);
		axis->origin = line_point(*axis, t);
		
		p3 = child_1().child_2().child_1().child_2();
		if (p3.kind() != SV_REAL)
			svlis_error("sv_primitive::parameters(...)","dud torus R",SV_CORRUPT);
		*r0 = p3.real();
		
		p3 = child_2();
		if (p3.kind() != SV_REAL)
			svlis_error("sv_primitive::parameters(...)","dud torus r",SV_CORRUPT);
		*r1 = sqrt(p3.real());						
		break;
					
	case SV_CYCLIDE:
		p1 = child_1().child_1().child_1().child_1().child_1().child_1().child_1();
		if (p1.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud cyclide plane 1",SV_CORRUPT);
		f1 = p1.plane();
		
		p2 = child_1().child_1().child_1().child_1().child_1().child_2().child_1();
		if (p2.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud cyclide plane 2",SV_CORRUPT);
		f2 = p2.plane();
		*axis = planes_line(f1, f2);

		p3 = child_1().child_1().child_1().child_1().child_2().child_1();
		if (p3.kind() != SV_PLANE)
			svlis_error("sv_primitive::parameters(...)","dud cyclide plane 3",SV_CORRUPT);
		f3 = p3.plane();
		t = plane_line_t(f3, *axis, flag);
		if (flag)
			svlis_error("sv_primitive::parameters(...)","dud cyclide plane/line intersection",
				SV_CORRUPT);
		axis->origin = line_point(*axis, t);
		
		*cen = axis->origin + f2.normal;

		p3 = child_2().child_1();
		if (p3.kind() != SV_REAL)
			svlis_error("sv_primitive::parameters(...)","dud cyclide 4b2",SV_CORRUPT);
		t = p3.real()*0.25;
		
		p3 = child_1().child_1().child_1().child_2();
		if (p3.kind() != SV_REAL)
			svlis_error("sv_primitive::parameters(...)","dud cyclide b2 - m^2",SV_CORRUPT);
		*r1 = sqrt(t - p3.real());

		p3 = child_1().child_2().child_1().child_1().child_1();
		if (p3.kind() != SV_REAL)
			svlis_error("sv_primitive::parameters(...)","dud cyclide 2a",SV_CORRUPT);
		*r0 = p3.real()*0.5;

		*r2 = sqrt(t - (*r0)*(*r0));
					
		break;
	
	default:
		break;	
	}
	return(SV_PLUS);
}

// Get rid of signed square roots, constant positive multipliers,
// raising to odd powers, etc.

sv_primitive sv_primitive::dump_scales() const
{
	switch(kind())
	{
	case SV_REAL:
		return(*this);
		
	case SV_PLANE:		
	case SV_CYLINDER:
	case SV_SPHERE:
	case SV_CONE:
	case SV_TORUS:
	case SV_CYCLIDE:
	case SV_GENERAL:
		// Check for scaling factors, odd powers, sign, and signed square roots
		switch(op())
		{
		case SV_TIMES:
			if (child_1().kind() == SV_REAL)
			{
				if (child_1().real() < 0)
					return( -child_2().dump_scales() );
				else
					return( child_2().dump_scales() );
			} else
			{			
				if (child_2().kind() == SV_REAL)
				{
					if (child_2().real() < 0)
						return( -child_1().dump_scales()); 
					else
						return(child_1().dump_scales());
				}
			}
			return(*this);

		case SV_DIVIDE:	
			if (child_2().kind() == SV_REAL)
			{
				if (child_2().real() < 0)
					return( -child_1().dump_scales()); 
				else
					return(child_1().dump_scales());
			} else
				svlis_error("dump_scales","rational primitive", SV_WARNING);
			break;

		case SV_POW:
			if (sv_round( child_2().real() )%2)
				return( child_1().dump_scales() );
			else
				return(*this);
			
		case SV_SSQRT:
			return( child_1().dump_scales() );

		case SV_SIGN:
			return( child_1().dump_scales() );
		
		default:
		    return(*this);
        	}
        
	default:
		return(*this);
	}
	
// Should never get here

	return(*this);
}

// Are two primitives the same?

// Possible answers:

// SV_ZERO - No
// SV_PLUS - Yes
// SV_COMP - One is the complement of the other
// SV_ABS  - One is the absolute value of the other

prim_op same(const sv_primitive& aa, const sv_primitive& bb)
{
	sv_integer k_a, k_b;
	sv_real r0_a, r0_b;
	sv_real r1_a, r1_b;
	sv_real r2_a, r2_b;
	sv_plane f_a, f_b;
	sv_point cen_a, cen_b;
	sv_line axis_a, axis_b;
	prim_op param_a, param_b, result, r2, op_a, op_b;
	int flip = 0;
	
	if (aa == bb) return(SV_PLUS); // Well, that bit was easy...
	
	sv_primitive a = aa.dump_scales();
	sv_primitive b = bb.dump_scales();
	
	param_a = a.parameters(&k_a, &r0_a, &r1_a, &r2_a, &f_a, &cen_a, &axis_a);
	param_b = b.parameters(&k_b, &r0_b, &r1_b, &r2_b, &f_b, &cen_b, &axis_b);
	if (k_a != k_b) return(SV_ZERO);
	
	switch(k_a)
	{
	case SV_REAL:
		result = same(a.real(), b.real());
		break;

	case SV_PLANE:
		result = same(a.plane(), b.plane());
		break;
		
	case SV_CYLINDER:
		result = same(axis_a, axis_b);
		if (!result) return(SV_ZERO);
		if ((result == SV_COMP) || (result == SV_TIMES)) result = SV_PLUS;
		if (same(r0_a, r0_b) != SV_PLUS) return(SV_ZERO);
		break;

	case SV_CONE:  // N.B. Double cone, so complemented axis doesn't matter
		result = same(axis_a, axis_b);
		if (!result) return(SV_ZERO);
		if (result == SV_COMP) result = SV_PLUS;
		if (result == SV_TIMES) return(SV_ZERO);
		if (same(r0_a, r0_b) != SV_PLUS) return(SV_ZERO);
		break; 
		
	case SV_SPHERE:
		result = same(cen_a, cen_b);
		if (!result) return(SV_ZERO);
		if (result == SV_COMP) return(SV_ZERO);
		if (same(r0_a, r0_b) != SV_PLUS) return(SV_ZERO);
		break;

	case SV_TORUS:
		result = same(axis_a, axis_b);
		if (!result) return(SV_ZERO);
		if (result == SV_COMP) result = SV_PLUS;
		if (result == SV_TIMES) return(SV_ZERO);
		if (same(r0_a, r0_b) != SV_PLUS) return(SV_ZERO);
		if (same(r1_a, r1_b) != SV_PLUS) return(SV_ZERO);
		break; 
		
	case SV_CYCLIDE:
		result = same(axis_a, axis_b);
		if (!result) return(SV_ZERO);
		if (result == SV_COMP) result = SV_PLUS;
		if (result == SV_TIMES) return(SV_ZERO);
		if (same(r0_a, r0_b) != SV_PLUS) return(SV_ZERO);
		if (same(r1_a, r1_b) != SV_PLUS) return(SV_ZERO);
		if (same(r2_a, r2_b) != SV_PLUS) return(SV_ZERO);
		break;

	case SV_GENERAL:
		op_a = a.op();
		op_b = b.op();
		if (op_a != op_b)
			return(SV_ZERO);
		else
		{
			if(diadic(op_a))
			{
				result = same(a.child_1(), b.child_1());
				if ( (!result) && ( (op_a == SV_PLUS) || (op_a == SV_TIMES) ) )
				{
					result = same(a.child_1(), b.child_2());
					flip = 1;
				}
				if (!result) return(SV_ZERO);
				if (flip)
					r2 = same(a.child_2(), b.child_1());
				else
					r2 = same(a.child_2(), b.child_2());
				if (!r2) return(SV_ZERO);
				if (r2 != result) result = SV_ZERO;			
			} else
			{
				result = same(a.child_1(), b.child_1());
			}
		}
		
	default:
		break;	
	}
	if (!result) return(SV_ZERO);
	if (param_a == param_b) return(result);
	if ((param_a == SV_ABS) || (param_b == SV_ABS)) return(SV_ABS);
	if (result == SV_PLUS)   // either param_a or param_b must be SV_COMP
		return(SV_COMP);
	else
		return(SV_PLUS);	
}

/*
 * The 5 arithmetic operations
 *
 * These fold simple real arithmetic wherever possible.
 */

sv_primitive operator+(const sv_primitive& a, const sv_primitive& b)
{
	sv_primitive c;
	sv_plane fa, fb;
	sv_point n;
	sv_real d, scale;
	
	if ((a.kind() == SV_REAL) && (b.kind() == SV_REAL))
		c = sv_primitive(a.real() + b.real());
	else
	{
	    if ( (a.kind() == SV_PLANE) && (b.kind() == SV_PLANE) )
	    {
		// 2 planes make a plane, but with scaled distance
		
			fa = a.plane();
			fb = b.plane();
			n = fa.normal + fb.normal;
			d = fa.d + fb.d;
			scale = n.mod();
			c = sv_primitive(scale)*sv_primitive(sv_plane(n, d));
	    } else
	    {
		// Plane + real is just a shift
		
			if ( (a.kind() == SV_PLANE) && (b.kind() == SV_REAL) )
			{
		    	fa = a.plane();
		    	fa.d = fa.d + b.real();
		    	c = sv_primitive(fa);		
			} else
			{
		    		if ( (a.kind() == SV_REAL) && (b.kind() == SV_PLANE) )
		    		{
					fb = b.plane();
					fb.d = fb.d + a.real();
					c = sv_primitive(fb);		    
		    		} else
					c = sv_primitive(a, b, SV_PLUS);
			}
	    }
	}
	return(c);
}

sv_primitive operator-(const sv_primitive& a, const sv_primitive& b)
{
	sv_primitive c;
	sv_plane fa, fb;
	sv_point n;
	sv_real d, scale;
	
	if ((a.kind() == SV_REAL) && (b.kind() == SV_REAL))
		c = sv_primitive(a.real() - b.real());
	else
	{
	    if ( (a.kind() == SV_PLANE) && (b.kind() == SV_PLANE) )
	    {
		// 2 planes make a plane, but with scaled distance
		
			fa = a.plane();
			fb = b.plane();
			n = fa.normal - fb.normal;
			d = fa.d - fb.d;
			scale = n.mod();
			c = sv_primitive(scale)*sv_primitive(sv_plane(n, d));
	    } else
	    {
		// Plane - real is just a shift
		
			if ( (a.kind() == SV_PLANE) && (b.kind() == SV_REAL) )
			{
		    		fa = a.plane();
		    		fa.d = fa.d - b.real();
		    		c = sv_primitive(fa);		
			} else
			{
		    		if ( (a.kind() == SV_REAL) && (b.kind() == SV_PLANE) )
		    		{
					fb = b.plane();
					fb.d = fb.d - a.real();
					c = sv_primitive(-fb);		    
		    		} else
					c = sv_primitive(a, b, SV_MINUS);
			}
	    }
	}
	return(c);
}

sv_primitive operator*(const sv_primitive& a, const sv_primitive& b)
{
	sv_primitive c;
	if ((a.kind() == SV_REAL) && (b.kind() == SV_REAL))
		c = sv_primitive(a.real()*b.real());
	else
		c = sv_primitive(a, b, SV_TIMES);
	return(c);
}

sv_primitive operator/(const sv_primitive& a, const sv_primitive& b)
{
	sv_primitive c;
	if (b.kind() != SV_REAL) svlis_error("sv_primitive operator/",
			"rational primitives not supported",SV_WARNING);
	if (b.real() == 0.0) svlis_error("sv_primitive operator/",
			"division by 0",SV_WARNING);
	if (a.kind() == SV_REAL)
		c = sv_primitive(a.real()/b.real());
	else
		c = sv_primitive(a, b, SV_DIVIDE);
	return(c);
}

sv_primitive operator^(const sv_primitive& a, const sv_primitive& b)
{
	sv_primitive c;

	if (b.kind() != SV_REAL) svlis_error("sv_primitive operator^",
		"primitive^primitive not supported",SV_WARNING);
	if (a.kind() == SV_REAL)
		c = sv_primitive(pow( a.real(),sv_round(b.real()) ));
	else
	{
		if (b.real() < 0)
			svlis_error("sv_primitive operator^",
				"negative exponents not supported",SV_WARNING);
		switch(sv_round(b.real()))
		{
		case 0:
			return(sv_primitive(1));

		case 1:
			return(a);

		default:
			;
		}
		c = sv_primitive(a, b, SV_POW);
	}
	return(c);
}


// Deep copy

sv_primitive sv_primitive::deep() const
{
	sv_primitive c;
	sv_integer k;

	switch(k = kind())
	{
	case SV_REAL:
		c = sv_primitive(real());
		break;

	case SV_PLANE:
		c = sv_primitive(plane());
		break;

	case SV_CYLINDER:
	case SV_SPHERE:
	case SV_CONE:
	case SV_TORUS:
	case SV_CYCLIDE:
	case SV_GENERAL:
		switch(op())
		{
		case SV_PLUS:
			c = child_1().deep() + child_2().deep();
			break;

		case SV_MINUS:
			c = child_1().deep() - child_2().deep();
			break;

		case SV_TIMES:
			c = child_1().deep()*child_2().deep();
			break;

		case SV_DIVIDE:
			c = child_1().deep()/child_2().deep();
			break;

		case SV_POW:
			c = (child_1().deep())^(child_2().deep());
			break;

		case SV_COMP:
			c = -child_1().deep();
			break;

		case SV_ABS:
			c = abs(child_1().deep());
			break;

		case SV_SIN:
			c = sin(child_1().deep());
			break;

		case SV_COS:
			c = cos(child_1().deep());
			break;

		case SV_EXP:
			c = exp(child_1().deep());
			break;

		case SV_SSQRT:
			c = s_sqrt(child_1().deep());
			break;

		case SV_SIGN:
			c = sign(child_1().deep());
			break;

		default:
			svlis_error("sv_primitive::deep()", "dud operator", SV_CORRUPT);
		}

// Now set the shape, which must have been unaffected by a deep copy

		c.set_kind(k);
		break;

	default:

		if (k < S_U_PRIM)
			;
		else
			;
		break;
		svlis_error("sv_primitive::deep()", "user primitive", SV_CORRUPT);
	}

	return(c);
}

// Translation

sv_primitive operator+(const sv_primitive& a, const sv_point& q)
{
	sv_primitive c;
	sv_integer k;

	switch(k = a.kind())
	{
	case SV_REAL:
		c = a;		// Reals unaffected by translation
		break;

	case SV_PLANE:
		c = sv_primitive(a.plane() + q);
		break;

	case SV_CYLINDER:
	case SV_SPHERE:
	case SV_CONE:
	case SV_TORUS:
	case SV_CYCLIDE:
	case SV_GENERAL:
		switch(a.op())
		{
		case SV_PLUS:
			c = (a.child_1() + q) + (a.child_2() + q);
			break;

		case SV_MINUS:
			c = (a.child_1() + q) - (a.child_2() + q);
			break;

		case SV_TIMES:
			c = (a.child_1() + q)*(a.child_2() + q);
			break;

		case SV_DIVIDE:
			c = (a.child_1() + q)/(a.child_2() + q);
			break;

		case SV_POW:
			c = (a.child_1() + q)^a.child_2();
			break;

		case SV_COMP:
			c = -(a.child_1() + q);
			break;

		case SV_ABS:
			c = abs(a.child_1() + q);
			break;

		case SV_SIN:
			c = sin(a.child_1() + q);
			break;

		case SV_COS:
			c = cos(a.child_1() + q);
			break;

		case SV_EXP:
			c = exp(a.child_1() + q);
			break;

		case SV_SSQRT:
			c = s_sqrt(a.child_1() + q);
			break;

		case SV_SIGN:
			c = sign(a.child_1() + q);
			break;

		default:
			svlis_error("primitive + point", "dud operator", SV_CORRUPT);
		}

// Now set the shape, which must have been unaffected by a translation

		c.set_kind(k);
		break;

	default:

		if (k < S_U_PRIM)
			c = translate_s(k, q);
		else
			c = translate_user(k, q);
		break;
	}

	return(c);
}

// Scale change

sv_primitive sv_primitive::scale(const sv_point& cen, sv_real s) const
{
	sv_primitive c;
	sv_integer k;
	sv_plane f;

	if (s == 0.0)
	{
		svlis_error("scale(sv_primitive)","zero scaling factor",SV_WARNING);
		return(sv_primitive(0.0));
	}

	switch(k = kind())
	{
	case SV_REAL:
		c = *this;		// Reals unaffected by scaling
		break;

	case SV_PLANE:
		f = plane().scale(cen, s);
		c = sv_primitive(f)*sv_primitive(1/s);	// Can't just scale the plane, as
							// potential fn must change too
		break;

	case SV_CYLINDER:
	case SV_SPHERE:
	case SV_CONE:
	case SV_TORUS:
	case SV_CYCLIDE:
	case SV_GENERAL:
		switch(op())
		{
		case SV_PLUS:
			c = child_1().scale(cen, s) + child_2().scale(cen, s);
			break;

		case SV_MINUS:
			c = child_1().scale(cen, s) - child_2().scale(cen, s);
			break;

		case SV_TIMES:
			c = child_1().scale(cen, s)*child_2().scale(cen, s);
			break;

		case SV_DIVIDE:
			c =  child_1().scale(cen, s)/child_2().scale(cen, s);
			break;

		case SV_POW:
			c = child_1().scale(cen, s)^child_2().scale(cen, s); // Defensive
			break;

		case SV_COMP:
			c = -(child_1().scale(cen, s));
			break;

		case SV_ABS:
			c = abs(child_1().scale(cen, s));
			break;

		case SV_SIN:
			c = sin(child_1().scale(cen, s));
			break;

		case SV_COS:
			c = cos(child_1().scale(cen, s));
			break;

		case SV_EXP:
			c = exp(child_1().scale(cen, s));
			break;

		case SV_SSQRT:
			c = s_sqrt(child_1().scale(cen, s));
			break;

		case SV_SIGN:
			c = sign(child_1().scale(cen, s));
			break;

		default:
			svlis_error("sv_primitive::scale()", "dud operator", SV_CORRUPT);
		}

// Now set the shape, which must have been unaffected by a scaling

		c.set_kind(k);
		break;

	default:

		if (k < S_U_PRIM)
			c = scale_s(k, cen, s);
		else
			c = scale_user(k, cen, s);
		break;
	}

	return(c);
}

// 1-D Scale change

sv_primitive sv_primitive::scale(const sv_line& s_ax, sv_real s) const
{
	sv_primitive c;
	sv_integer k;
	sv_plane f, g;
	sv_point n;
	sv_real a1, b1, c1, d1, s1;

	if (s == 0.0)
	{
		svlis_error("scale(sv_primitive)","zero scaling factor",SV_WARNING);
		return(sv_primitive(0.0));
	}

	sv_real sf;

	switch(k = kind())
	{
	case SV_REAL:
		c = *this;		// Reals unaffected by scaling
		break;

	case SV_PLANE:
                 f = plane();
                 g = sv_plane(s_ax.direction, s_ax.origin);
                 d1 = (1-1/s)*f.normal*g.normal;
                 a1 = f.normal.x - d1*g.normal.x;
                 b1 = f.normal.y - d1*g.normal.y;
                 c1 = f.normal.z - d1*g.normal.z;
                 d1 = f.d - d1*g.d;
                 s1 = sqrt(a1*a1 + b1*b1 + c1*c1);
                 c = sv_primitive(sv_plane(a1,b1,c1,d1))*sv_primitive(s1);   	       
		break;

	case SV_CYLINDER:
	case SV_SPHERE:
	case SV_CONE:
	case SV_TORUS:
	case SV_CYCLIDE:
	case SV_GENERAL:
		switch(op())
		{
		case SV_PLUS:
			c = child_1().scale(s_ax, s) + child_2().scale(s_ax, s);
			break;

		case SV_MINUS:
			c = child_1().scale(s_ax, s) - child_2().scale(s_ax, s);
			break;

		case SV_TIMES:
			c = child_1().scale(s_ax, s)*child_2().scale(s_ax, s);
			break;

		case SV_DIVIDE:
			c =  child_1().scale(s_ax, s)/child_2().scale(s_ax, s);
			break;

		case SV_POW:
			c = child_1().scale(s_ax, s)^child_2().scale(s_ax, s); // Defensive
			break;

		case SV_COMP:
			c = -(child_1().scale(s_ax, s));
			break;

		case SV_ABS:
			c = abs(child_1().scale(s_ax, s));
			break;

		case SV_SIN:
			c = sin(child_1().scale(s_ax, s));
			break;

		case SV_COS:
			c = cos(child_1().scale(s_ax, s));
			break;

		case SV_EXP:
			c = exp(child_1().scale(s_ax, s));
			break;

		case SV_SSQRT:
			c = s_sqrt(child_1().scale(s_ax, s));
			break;

		case SV_SIGN:
			c = sign(child_1().scale(s_ax, s));
			break;

		default:
			svlis_error("sv_primitive::scale()", "(1D); dud operator", SV_CORRUPT);
		}

// Now set the shape, which must have been changed by a 1D scaling

		c.set_kind(SV_GENERAL);
		break;

	default:

		if (k < S_U_PRIM)
			c = scale_s(k, s_ax, s);
		else
			c = scale_user(k, s_ax, s);
		break;
	}

	return(c);
}


// Rotation

sv_primitive sv_primitive::spin(const sv_line& l, sv_real angle) const
{
	sv_primitive c;
	sv_integer k;

	switch(k = kind())
	{
	case SV_REAL:
		c = *this;		// Reals unaffected by rotation
		break;

	case SV_PLANE:
		c = sv_primitive( plane().spin(l,angle) );
		break;

	case SV_CYLINDER:
	case SV_SPHERE:
	case SV_CONE:
	case SV_TORUS:
	case SV_CYCLIDE:
	case SV_GENERAL:
		switch(op())
		{
		case SV_PLUS:
			c = child_1().spin(l,angle) + child_2().spin(l,angle);
			break;

		case SV_MINUS:
			c = child_1().spin(l,angle) - child_2().spin(l,angle);
			break;

		case SV_TIMES:
			c = child_1().spin(l,angle)*child_2().spin(l,angle);
			break;

		case SV_DIVIDE:
			c = child_1().spin(l,angle)/child_2().spin(l,angle);
			break;

		case SV_POW:
			c = child_1().spin(l,angle)^child_2().spin(l,angle);
			break;

		case SV_COMP:
			c = -(child_1().spin(l,angle));
			break;

		case SV_ABS:
			c = abs(child_1().spin(l,angle));
			break;

		case SV_SIN:
			c = sin(child_1().spin(l,angle));
			break;

		case SV_COS:
			c = cos(child_1().spin(l,angle));
			break;

		case SV_EXP:
			c = exp(child_1().spin(l,angle));
			break;

		case SV_SSQRT:
			c = s_sqrt(child_1().spin(l,angle));
			break;

		case SV_SIGN:
			c = sign(child_1().spin(l, angle));
			break;

		default:
			svlis_error("sv_primitive::spin()", "dud operator", SV_CORRUPT);
		}

// Now set the shape, which must have been unaffected by a rotation

		c.set_kind(k);
		break;

	default:
		if (k < S_U_PRIM)
			c = spin_s(k, l, angle);
		else
			c = spin_user(k, l, angle);
		break;
	}

	return(c);
}

// Mirror a primitive in a plane

sv_primitive sv_primitive::mirror(const sv_plane& m) const
{
	sv_primitive c;
	sv_integer k;

	switch(k = kind())
	{
	case SV_REAL:
		c = *this;		// Reals unaffected by mirroring
		break;

	case SV_PLANE:
		c = sv_primitive( plane().mirror(m) );
		break;

	case SV_CYLINDER:
	case SV_SPHERE:
	case SV_CONE:
	case SV_TORUS:
	case SV_CYCLIDE:
	case SV_GENERAL:
		switch(op())
		{
		case SV_PLUS:
			c = child_1().mirror(m) + child_2().mirror(m);
			break;

		case SV_MINUS:
			c = child_1().mirror(m) - child_2().mirror(m);
			break;

		case SV_TIMES:
			c = child_1().mirror(m)*child_2().mirror(m);
			break;

		case SV_DIVIDE:
			c = child_1().mirror(m)/child_2().mirror(m);
			break;

		case SV_POW:
			c = child_1().mirror(m)^child_2().mirror(m);
			break;

		case SV_COMP:
			c = -(child_1().mirror(m));
			break;

		case SV_ABS:
			c = abs(child_1().mirror(m));
			break;

		case SV_SIN:
			c = sin(child_1().mirror(m));
			break;

		case SV_COS:
			c = cos(child_1().mirror(m));
			break;

		case SV_EXP:
			c = exp(child_1().mirror(m));
			break;

		case SV_SSQRT:
			c = s_sqrt(child_1().mirror(m));
			break;

		case SV_SIGN:
			c = sign(child_1().mirror(m));
			break;

		default:
		    svlis_error("primitive::mirror()", "dud operator", SV_CORRUPT);
		}

// Now set the shape, which must have been unaffected by a mirroring

		c.set_kind(k);
		break;

	default:
		if (k < S_U_PRIM)
			c = mirror_s(k, m);
		else
			c = mirror_user(k, m);

	}

	return(c);
}

// Value of a primitive for a point

sv_real sv_primitive::value(const sv_point& q) const
{
	sv_real c;
	sv_integer k;

	switch(k = kind())
	{
	case SV_REAL:
		c = real();
		break;

	case SV_PLANE:
		if(op() == SV_ZERO) // At the bottom of ABS COMP etc?
		{
			c = plane().value(q);
			break;
		} // NO break here

	case SV_CYLINDER:
	case SV_SPHERE:
	case SV_CONE:
	case SV_TORUS:
	case SV_CYCLIDE:
	case SV_GENERAL:
		switch(op())
		{
		case SV_PLUS:
			c = child_1().value(q) + child_2().value(q);
			break;

		case SV_MINUS:
			c = child_1().value(q) - child_2().value(q);
			break;

		case SV_TIMES:
			c = child_1().value(q)*child_2().value(q);
			break;

		case SV_DIVIDE:
			c = child_1().value(q)/child_2().value(q);
			break;

		case SV_POW:
			c = pow( child_1().value(q), sv_round( child_2().real() ) );
			break;

		case SV_COMP:
			c = -(child_1().value(q));
			break;

		case SV_ABS:
			c = fabs(child_1().value(q));
			break;

		case SV_SIN:
			c = (sv_real)sin( child_1().value(q) );
			break;

		case SV_COS:
			c = (sv_real)cos( child_1().value(q) );
			break;

		case SV_EXP:
			c = (sv_real)exp( child_1().value(q) );
			break;

		case SV_SSQRT:
			c = s_sqrt(child_1().value(q));
			break;

		case SV_SIGN:
			c = sign(child_1().value(q));
			break;

		default:
			svlis_error("sv_primitive::value(point)", "dud operator", SV_CORRUPT);
		}
		break;

	default:
		if (k < S_U_PRIM)
			c = value_s(k, q);
		else
			c = value_user(k, q);
		break;
	}

	return(c);
}

// Value of a box in a primitive

sv_interval sv_primitive::range(const sv_box& b) const
{
	sv_interval c;
	sv_integer k;
	int c_1, c_2;			// Logical - T if child is a real

	switch(k = kind())
	{
	case SV_REAL:
		svlis_error("sv_primitive::range(box)","primitive is a single constant",
			SV_WARNING);
		c = sv_interval(real(), real()); // Best we can do
		break;

	case SV_PLANE:
		if(op() == SV_ZERO) // Any ABS, COMP etc?
		{ 
			c = plane().range(b);
			break;
		} // NO break here

// If the primitive has a real child, then that child must be put into
// the interval arithmetic inline because, as this function returns an
// interval, there is no way that it can also return a real.
// BUT, you say, WHY NOT JUST MAKE AN INTERVAL WITH COINCIDENT ENDS?
// Because, gentle comment reader, intervals with swapped ends are
// defined to be empty, and rounding error might cause trouble.

	case SV_CYLINDER:
	case SV_SPHERE:
	case SV_CONE:
	case SV_TORUS:
	case SV_CYCLIDE:
	case SV_GENERAL:
		if (diadic(op()))
		{
			c_1 = (child_1().kind() == SV_REAL);
			c_2 = (child_2().kind() == SV_REAL);
		} else
			c_1 = (child_1().kind() == SV_REAL);

		switch(op())
		{
		case SV_PLUS:
			if (c_1)
			{
				if (c_2)
				{
					svlis_error("sv_primitive::range(box)","primitive is real+real",
						SV_WARNING);
					c = sv_interval(child_1().real() + child_2().real(), 
							child_1().real() + child_2().real() );
				}	
				else
					c = child_1().real() + child_2().range(b);
			} else
			{
				if (c_2)
					c = child_1().range(b) + child_2().real();
				else
					c = child_1().range(b) + child_2().range(b);
			}
			break;

		case SV_MINUS:
			if (c_1)
			{
				if (c_2)
				{
					svlis_error("sv_primitive::range(box)","primitive is real-real",
						SV_WARNING);
					c = sv_interval(child_1().real() - child_2().real(), 
							child_1().real() - child_2().real() );
				}	
				else
					c = child_1().real() - child_2().range(b);
			} else
			{
				if (c_2)
					c = child_1().range(b) - child_2().real();
				else
					c = child_1().range(b) - child_2().range(b);
			}
			break;

		case SV_TIMES:
			if (c_1)
			{
				if (c_2)
				{
					svlis_error("sv_primitive::range(box)","primitive is real*real",
						SV_WARNING);
					c = sv_interval(child_1().real()*child_2().real(), 
							child_1().real()*child_2().real() );
				}	
				else
					c = child_1().real()*child_2().range(b);
			} else
			{
				if (c_2)
					c = child_1().range(b)*child_2().real();
				else
					c = child_1().range(b)*child_2().range(b);
			}
			break;

		case SV_DIVIDE:
			if (!c_2)
				svlis_error("sv_primitive::range(box)","rational primitive",
					SV_CORRUPT);
			else
			{
				if (c_1)
				{
					svlis_error("sv_primitive::range(box)","primitive is real/real",
						SV_WARNING);
					c = sv_interval(child_1().real()/child_2().real(), 
							child_1().real()/child_2().real() );
				}
				else
					c = child_1().range(b)/child_2().real();
			}
			break;

		case SV_POW:
			if (!c_2)
				svlis_error("sv_primitive::range(box)","exponent is a primitive",
					SV_WARNING);
			else
			{
				if (c_1)
				{
					svlis_error("sv_primitive::range(box)","primitive is real^real",
						SV_WARNING);
					c = sv_interval(pow(child_1().real(), sv_round(child_2().real())),
							pow(child_1().real(), sv_round(child_2().real())) );
				}
				else
					c = pow( child_1().range(b), sv_round( child_2().real() ) );
			}
			break;

		case SV_COMP:
			c = -(child_1().range(b));
			break;

		case SV_ABS:
			c = abs(child_1().range(b));
			break;

		case SV_SIN:
			c = sin(child_1().range(b));
			break;

		case SV_COS:
			c = cos(child_1().range(b));
			break;

		case SV_EXP:
			c = exp(child_1().range(b));
			break;

		case SV_SSQRT:
			c = s_sqrt(child_1().range(b));
			break;

		case SV_SIGN:
			c = sign(child_1().range(b));
			break;

		default:
			svlis_error("sv_primitive::range(box)", "dud operator", SV_CORRUPT);
		}
		break;

	default:
		if (k < S_U_PRIM)
			c = range_s(k, b);
		else
			c = range_user(k, b);
		break;
	}

	return(c);
}



// This is probably balls and needs to go.
// All the corners of a box the same sign for a primitive?

void definite(const sv_primitive& q, const sv_box& b, mem_test* result, 
		sv_integer* corner)
{
	mem_test temp;
	sv_primitive p;

	if((q.kind() != SV_REAL) && (q.kind() != SV_PLANE))
	{
		if(q.op() == SV_ABS)
			p = q.child_1();	// Rather pointless, otherwise.
		else
			p = q;
	} else
		p = q;

	*result = member( p.value( b.corner(0) ) );

	if(*result == SV_SURFACE)
	{
		*corner = 1;
		return;
	}

	for(sv_integer i = 1; i <= 7; i++)
	{
		temp = member( p.value( b.corner(i) ) );
		if (temp != *result) 
		{
			*corner = i;
			*result = SV_SURFACE;
			return;
		}
	}
	*corner = 7;
	return;
}
#if macintosh
 #pragma export off
#endif









