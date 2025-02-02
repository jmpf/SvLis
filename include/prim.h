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
 * SvLis - This defines the primitive class that svLis uses
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 16 March 1993
 * This version: 8 March 2000
 *
 * SvLis - This now defines the primitive block class that svLis uses
 * it's just like normal primitives, except it allows for block primitives
 *
 * Irina Voiculescu
 *
 * First version: 3 April 2001*
 */


#ifndef SVLIS_PRIMITIVE
#define SVLIS_PRIMITIVE

// primitive: a primitive in x, y, and z achieved by doing
// arithmetic on planes; primitives may have special forms
// such as cones, spheres etc indicated by shape.

// The Svlis- and user-supplied `degree' function

extern sv_integer degree_s(sv_integer);
extern sv_integer degree_user(sv_integer);

// Note there is no extern istream& operator>>(istream&, prim_kind&)
// as the kind value in hidden_prim can be any positive integer

extern istream& operator>>(istream&, prim_op&);
extern void read(istream&, prim_op&);
extern void read1(istream&, prim_op&);


class sv_primitive
{
private:

// struct containing the actual data derived
// from the reference-counting base class

   struct prim_data : public sv_refct 
   {
        friend class sv_smart_ptr<prim_data>;

        sv_integer kind;	// Indicates if this is a plane, real or compound
	sv_plane flat;		// Arithmetic is done on planes and reals
	sv_box block;           // The actual block primitive
	sv_real r;
	prim_op op;		// If compound, this says +, -, *, /, ^, or one of the monadics
	sv_integer degree;	// Highest power (trancendentals add one)
	sv_primitive *child_1;	// Children if compound
	sv_primitive *child_2;
	sv_primitive *grad_x;	// The grad vector of the primitive
	sv_primitive *grad_y;
	sv_primitive *grad_z;

        ~prim_data() { delete child_1; delete child_2; delete grad_x; delete grad_y; delete grad_z; }

// Make a block primitive -- irina

	prim_data(const sv_point& low,const sv_point& high)
	{
		kind = SV_BLOCK;
		block = sv_box(low, high);
		degree = 0;
		op = SV_ZERO;
		child_1 = new sv_primitive();
		child_2 = new sv_primitive();
		grad_x = new sv_primitive();
		grad_y = new sv_primitive();
		grad_z = new sv_primitive();
	}
     // </irina>

// Make a single-plane primitive

	prim_data(const sv_plane& a)
	{
		kind = SV_PLANE;
		flat = a;
		degree = 1;
		op = SV_ZERO;
		child_1 = new sv_primitive();
		child_2 = new sv_primitive();
		grad_x = new sv_primitive();
		grad_y = new sv_primitive();
		grad_z = new sv_primitive();
	}

// Make a single-real primitive

	prim_data(sv_real a)
	{
		kind = SV_REAL;
		r = a;
		degree = 0;
		op = SV_ZERO;
		child_1 = new sv_primitive();
		child_2 = new sv_primitive();
		grad_x = new sv_primitive();
		grad_y = new sv_primitive();
		grad_z = new sv_primitive();
	}

// Build a compound primitive from two others and a diadic operator

        prim_data(const sv_primitive& a, const sv_primitive& b, prim_op optr)
	{
		kind = SV_GENERAL;
		op = optr;
		switch (op)
		{
		case SV_PLUS:
		case SV_MINUS:
			degree = max(a.degree(), b.degree());
			break;

		case SV_TIMES:
			degree = a.degree() + b.degree();
			break;

		case SV_DIVIDE:
			degree = a.degree();	// Won't work for rationals!
			break;

		case SV_POW:
			degree = a.degree()*round(b.real());
			break;

		default:
			svlis_error("hidden_prim constructor", 
			    "dud operator",SV_CORRUPT);
		}
		child_1 = new sv_primitive(a);
		child_2 = new sv_primitive(b);
		grad_x = new sv_primitive();
		grad_y = new sv_primitive();
		grad_z = new sv_primitive();
	}


// Build a compound primitive from one other and a monadic operator

	prim_data(const sv_primitive& a,  prim_op optr)
	{
		kind = SV_GENERAL;
		op = optr;
		degree = a.degree() + 1; // Sort of convention . . .
		child_1 = new sv_primitive(a);
		child_2 = new sv_primitive();
		grad_x = new sv_primitive();
		grad_y = new sv_primitive();
		grad_z = new sv_primitive();
	}

// Make a user-primitive

	prim_data(sv_integer up, sv_integer upx, sv_integer upy, sv_integer upz)
	{
		kind = up;
		if (up < S_U_PRIM)
			degree = degree_s(up);
		else
			degree = degree_user(up);
		op = SV_ZERO;
		child_1 = new sv_primitive();
		child_2 = new sv_primitive();
		grad_x = new sv_primitive();
		grad_y = new sv_primitive();
		grad_z = new sv_primitive();
	}
   }; // prim_data

// This is the pointer that gets ref counted

   sv_smart_ptr<prim_data> prim_info;

// Build a compound primitive from two others and a diadic operator

        sv_primitive(const sv_primitive& a, const sv_primitive& b, prim_op optr)
	{
	  prim_info = new prim_data(a, b, optr);
	}

// Build a compound primitive from one other and a monadic operator

	sv_primitive(const sv_primitive& a,  prim_op optr)
	{
	  prim_info = new prim_data(a, optr);
	}

// Priveleged (Re)Set flag bit(s)

    void set_flags_priv(sv_integer a) { prim_info->set_flags(a); }
    void reset_flags_priv(sv_integer a) { prim_info->reset_flags(a); }

// Set the special shapes

    void set_kind(sv_integer k) { prim_info->kind = k; }

    friend void lazy_grad(const sv_primitive&, sv_primitive&, sv_primitive&, sv_primitive&);

public:

// Null primitive

	sv_primitive() { }

// Make one from a plane or a real

	sv_primitive(const sv_plane& a) { prim_info = new prim_data(a); }
	sv_primitive(sv_real a) { prim_info = new prim_data(a); }
	// -- irina :
	sv_primitive(sv_point a, sv_point b) {
	  //cerr <<"making primitive out of "<<a << " and "<<b <<endl;
	  prim_info = new prim_data(a,b); 
	}

// Make a user-primitive

	sv_primitive(sv_integer up, sv_integer upx, sv_integer upy, sv_integer upz)
	{
	  prim_info = new prim_data(up, upx, upy, upz);
	}

// Build one primitive from another

	sv_primitive(const sv_primitive& a) { *this = a; }

// Functions to return the hidden data

	sv_integer flags() const { return(prim_info->flags()); }
	sv_plane plane() const { return(prim_info->flat); }
	sv_real real() const { return(prim_info->r); }
	sv_box block() const { return(prim_info->block); } // --irina
	sv_integer kind() const { return(prim_info->kind); }
	prim_op parameters(sv_integer*, sv_real*, sv_real*, sv_real*, sv_plane*, 
		sv_point*, sv_line*) const;
	prim_op op() const { return(prim_info->op); }
	sv_integer degree() const  { return(prim_info->degree); }
	sv_primitive child_1() const { return(*(prim_info->child_1)); }
	sv_primitive child_2() const { return(*(prim_info->child_2)); }
	sv_primitive grad_x() const;
	sv_primitive grad_y() const;
	sv_primitive grad_z() const;

// Deep copy

	sv_primitive deep() const;

// The simplifier for the same test

	sv_primitive dump_scales() const;

// Characteristic point

	sv_point point() const;

// Unique tag

	sv_integer tag() const;

// The 5 arithmetic operations

	friend sv_primitive operator+(const sv_primitive&, const sv_primitive&);
	friend sv_primitive operator-(const sv_primitive&, const sv_primitive&);
	friend sv_primitive operator*(const sv_primitive&, const sv_primitive&);
	friend sv_primitive operator/(const sv_primitive&, const sv_primitive&);
	friend sv_primitive operator^(const sv_primitive&, const sv_primitive&);

// Test for an undefined set

	int exists() const { return(prim_info.exists()); }

// Unique value (effectively the pointer to this set)
// This is explicitly long not sv_integer

	long unique() const { return(prim_info.unique()); }

// Primitives are equal if they point to the same thing.
// I mean int not sv_integer here:

	friend int operator==(const sv_primitive& a, const sv_primitive& b) { return(a.unique() == b.unique()); }
	friend int operator!=(const sv_primitive& a, const sv_primitive& b) { return(!(a == b)); } 
	
// Value of a primitive for a point

	sv_real value(const sv_point&) const;

// Value of a box in a primitive

	sv_interval range(const sv_box&) const;

// Make special shapes

	friend sv_primitive p_cylinder(const sv_line&, sv_real);
	friend sv_primitive p_cone(const sv_line&, sv_real);
	friend sv_primitive p_sphere(const sv_point&, sv_real);
	friend sv_primitive p_torus(const sv_line&, sv_real, sv_real);
	friend sv_primitive p_cyclide(const sv_line&, const sv_point&, 
	    sv_real, sv_real, sv_real);

// Translate a primitive

	friend sv_primitive operator+(const sv_primitive&, const sv_point&);
	friend sv_primitive operator-(const sv_primitive&, const sv_point&);

// Rotate and mirror

	sv_primitive spin(const sv_line&, sv_real) const;
	sv_primitive mirror(const sv_plane&) const;

// Scale

	sv_primitive scale(const sv_point&, sv_real) const;

// 1D Scale along a line

	sv_primitive scale(const sv_line&, sv_real) const;

// Complement a sv_primitive

	friend sv_primitive operator-(const sv_primitive&);

// The monadic operators on primitives as functions

	friend sv_primitive abs(const sv_primitive&);
	friend sv_primitive sin(const sv_primitive&);
	friend sv_primitive cos(const sv_primitive&);
	friend sv_primitive exp(const sv_primitive&);
	friend sv_primitive s_sqrt(const sv_primitive&);
	friend sv_primitive sign(const sv_primitive&);

// The grad at a point

	sv_point grad(const sv_point& p) const;

// Special grad at a point for graphics - grad defined at 0s of
// primitives that are abs()

	sv_point p_grad(const sv_point& p) const;



// The range of grads in a box

	sv_box grad(const sv_box& b) const;

// Set flag bit(s)

	void set_flags(sv_integer);
	void reset_flags(sv_integer);

// Raytracer's flag setter
	
	friend sv_integer set_prim_flags(sv_integer, sv_set s);

// I/O

// Write and read are public so that the other classes can get at them
// without everything having to be a friend of everything else.  The
// user shouldn't normally call these.

	friend void unwrite(sv_primitive&);
	friend void write(ostream&, sv_primitive&, sv_integer);
	friend void read(istream&, sv_primitive&);
	friend void read1(istream&, sv_primitive&);
};



// ************************************************************************

// *************** External functions

/* external function prototypes added by imc to satisfy gcc 4 */
sv_primitive p_cylinder(const sv_line&, sv_real);
sv_primitive p_cone(const sv_line&, sv_real);
sv_primitive p_sphere(const sv_point&, sv_real);
sv_primitive p_cyclide(const sv_line&, const sv_point&, sv_real, sv_real, sv_real);
sv_primitive p_torus(const sv_line&, sv_real, sv_real);

// All the corners of a box the same sign for a primitive?

extern void definite(const sv_primitive&, const sv_box&, mem_test*, sv_integer*);

// **************** Svlis-primitive external functions
// I/O

extern sv_primitive read_s(istream&, sv_integer);
extern void write_s(ostream&, sv_integer);

// Translate a primitive

extern sv_primitive translate_s(sv_integer, const sv_point&);

// Rotate and mirror

extern sv_primitive spin_s(sv_integer, const sv_line&, sv_real);
extern sv_primitive mirror_s(sv_integer, const sv_plane&);

// Scale about a point

extern sv_primitive scale_s(sv_integer, const sv_point&, sv_real);

// Scale along a line

extern sv_primitive scale_s(sv_integer, const sv_line&, sv_real);

// Value of a primitive for a point

extern sv_real value_s(sv_integer, const sv_point&);

// Value of a box in an sv_primitive

extern sv_interval range_s(sv_integer, const sv_box&);

// Complement a primitive

extern sv_primitive complement_s(sv_integer);

// **************** User-primitive external functions

// I/O

extern sv_primitive read_user(istream&, sv_integer);
extern void write_user(ostream&, sv_integer);

// Translate a primitive

extern sv_primitive translate_user(sv_integer, const sv_point&);

// Rotate and mirror

extern sv_primitive spin_user(sv_integer, const sv_line&, sv_real);
extern sv_primitive mirror_user(sv_integer, const sv_plane&);

// Scale

extern sv_primitive scale_user(sv_integer, const sv_point&, sv_real);

// Scale along a line

extern sv_primitive scale_user(sv_integer, const sv_line&, sv_real);

// Value of a primitive for a point

extern sv_real value_user(sv_integer, const sv_point&);

// Value of a box in an primitive

extern sv_interval range_user(sv_integer, const sv_box&);

// Complement a primitive

extern sv_primitive complement_user(sv_integer);

// Flag that spheres, cylinders, & cones (?) are to have linear
// Distance functions

extern void real_distance(sv_integer);

// Normal user i/o functions

extern ostream& operator<<(ostream&, sv_primitive&);

extern istream& operator>>(istream&, sv_primitive&);

// *************** Inlines

inline sv_primitive sv_primitive::grad_x() const 
{
        sv_primitive x, y, z;
	if (!prim_info->grad_x->exists()) 
	{
	  lazy_grad(*this, x, y, z);
	  *(prim_info->grad_x) = x;
	  *(prim_info->grad_y) = y;
	  *(prim_info->grad_z) = z;
	}
	return(*(prim_info->grad_x));
}

inline sv_primitive sv_primitive::grad_y() const 
{
        sv_primitive x, y, z;
	if (!prim_info->grad_y->exists()) 
	{
	  lazy_grad(*this, x, y, z);
	  *(prim_info->grad_x) = x;
	  *(prim_info->grad_y) = y;
	  *(prim_info->grad_z) = z;
	}
	return(*(prim_info->grad_y));
}

inline sv_primitive sv_primitive::grad_z() const 
{
        sv_primitive x, y, z;
	if (!prim_info->grad_z->exists()) 
	{
	  lazy_grad(*this, x, y, z);
	  *(prim_info->grad_x) = x;
	  *(prim_info->grad_y) = y;
	  *(prim_info->grad_z) = z;
	}
	return(*(prim_info->grad_z));
}


extern look_up<sv_primitive> p_write_list; 

// Monadic functions

inline sv_primitive operator-(const sv_primitive& a)
{
        if(a.op() == SV_COMP) return(a.child_1());

	sv_primitive b = sv_primitive(a, SV_COMP);

	b.prim_info->kind = a.kind();	// Shape's the same
	if(a.kind() == SV_REAL) b.prim_info->r = -a.real();
	if(a.kind() == SV_PLANE) b.prim_info->flat = -a.plane();
	return(b);
}

// The grad of the sign of a primitive is the same as that of the primitive

inline sv_primitive sign(const sv_primitive& a)
{
	sv_primitive b;

	if(a.kind() == SV_REAL)
		b = sv_primitive(sign(a.real()));
	else
		b = sv_primitive(a, SV_SIGN);

	return(b);
}

inline sv_primitive abs(const sv_primitive& a)
{
	sv_primitive b, x, y, z;

	if(a.kind() == SV_REAL)
		b = sv_primitive(fabs(a.real()));
	else
		b = sv_primitive(a, SV_ABS);

	return(b);
}

inline sv_primitive sin(const sv_primitive& a)
{
	sv_primitive b;

	if(a.kind() == SV_REAL)
		b = sv_primitive((sv_real)sin(a.real()));
	else
		b = sv_primitive(a, SV_SIN);
	return(b);
}

inline sv_primitive cos(const sv_primitive& a)
{
	sv_primitive b;

	if(a.kind() == SV_REAL)
		b = sv_primitive((sv_real)cos(a.real()));
	else
		b = sv_primitive(a, SV_COS);
	return(b);
}

inline sv_primitive exp(const sv_primitive& a)
{
	sv_primitive b;

	if(a.kind() == SV_REAL)
		b = sv_primitive((sv_real)exp(a.real()));
	else
		b = sv_primitive(a, SV_EXP);
	return(b);
}

// Note that the s_sqrt grad value is the grad of the underlying
// function.  This prevents the magnitude going to
// infinity at the surface of a primitive; Svlis is only concerned with
// the directions of grads, not their magnitude, so this doesn't
// matter.  Beware if your application does use magnitudes,
// though.

inline sv_primitive s_sqrt(const sv_primitive& a)
{
	sv_primitive b;

	if(a.kind() == SV_REAL)
		b = sv_primitive(s_sqrt(a.real()));
	else
		b = sv_primitive(a, SV_SSQRT);

	return(b);
}


// Are two primitives the same?

// The answer is yes if they are set-theoretically the same; thus
// if their potential functions have the same 0s and the same sign
// then they're the same, if not, not.  If SV_ABS is returned, then one
// is the absolute value of the other.  If SV_COMP is returned, then
// one is the complement of the other.

// This procedure is conservative: if two primitives are different
// it will always say so.  If two are really complicated and actually
// the same, then occasionally this won't be spotted.

extern prim_op same(const sv_primitive&, const sv_primitive&);

// Simplest (prim - point) translation is to negate the point and add

inline sv_primitive operator-(const sv_primitive& a, const sv_point& b)
{
	return(a + (-b));
}

// Make translation commute (NB point - prim is meaningless)

inline sv_primitive operator+(const sv_point& a, const sv_primitive& b)
{
	return(b + a);
}

// Flag up abs-type primitives

inline int p_thin(const sv_primitive& q)
{
	sv_primitive p;
	if(q.op() == SV_COMP)
		p = q.child_1();
	else
		p = q;

	int result = (p.op() == SV_ABS); // NB op on leaves is SV_ZERO

	return(result);
}

// sv_primitive
// **************************************************************************

#endif




