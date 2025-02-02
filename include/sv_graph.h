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
 * SvLis - OpenGL graphics procedures
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 20 April 1995
 * This version: 18 March 2000
 *
 */


#ifndef SVLIS_GRAPHICS
#define SVLIS_GRAPHICS

// actually plot the stuff on the screen

extern void sv_draw(int);

// Compile a model into OpenGL polygons etc

extern void sv_render_a_model(const sv_model& m);

// Initialize GLUT and run its open loop

extern void sv_run_glut(void *);

// Compile all the models in the list and their transforms

extern void sv_compile_model();

// Get and release the lock on the display lists

extern void get_display_list();
extern void release_display_list();

void sv_delete_picture_list(); //imc

/*
 *  Linked list of svLis models for rendering
 */

class sv_model_list
{
 private:
  sv_model org;        // The original model
  sv_model m;          // The faceted model
  int m_changed;       // Flag to remember that the model's changed
  sv_point trans;      // Offset to apply
  sv_line ax;          // Rotation axis
  sv_real spn;         // Rotation angle
  int x_changed;       // Flag to remember the transform's changed
  sv_model_list* n;    // Next in the linked list
  int vis;             // Display it?
  GLuint mlist;        // OpenGL display list for the model
  GLuint xlist;        // OpenGL display list for the transforms
  void* pointer_;      // For the user's data (if any)

// Callback for when OpenGL wants to move the model

  void (*xf_cb)(sv_model_list*, const sv_point&, const sv_line&, sv_real th, sv_xform_action);

  void compile_model()
  {
    if(!m_changed) return;

    if(!glIsList(mlist)) mlist = glGenLists(1);

    glNewList(mlist, GL_COMPILE);
     sv_render_a_model(m);
    glEndList();

    m_changed = 0;
  }

  void compile_transform()
  {
    if(!x_changed) return;

    if(!glIsList(xlist)) xlist = glGenLists(1);

    glNewList(xlist, GL_COMPILE);
     glTranslatef(trans.x + ax.origin.x, trans.y + ax.origin.y, trans.z + ax.origin.z);
     glRotatef(spn*180/M_PI, ax.direction.x, ax.direction.y, ax.direction.z);
     glTranslatef(-ax.origin.x, -ax.origin.y, -ax.origin.z);
    glEndList();

    x_changed = 0; 
  }

// User should never call the destructor

  ~sv_model_list()
  {
    sv_model null;
    m = null;
    org = null;
    if(glIsList(mlist)) glDeleteLists(mlist,1);
    if(glIsList(xlist)) glDeleteLists(xlist,1);
  }

// Directly overwrite the faceted model

  void faceted_model(const sv_model& mm) 
  {
    m = mm; 
    m_changed = 1;
  }

  friend void ray_enquire(int x, int y, int button);
  friend void menuFacet(int value);

 public:

// Only constructor

  sv_model_list() 
  {
    n = 0; 
    spn = 0;
    ax = SV_ZL;
    trans = SV_OO;
    m_changed = 0;
    x_changed = 0; 
    mlist = 0;    // Funny numbers probably not needed
    xlist = 0;    // But do no harm
    xf_cb = 0;
    vis = 1;
    pointer_ = 0;
  }

// Return and modify the model

  sv_model model() { return org; }
  sv_model faceted_model() { return m; }
  void model(const sv_model& mm)
  {
    org = mm;
    m = mm.facet();
    m_changed = 1;
  }

// Return the model's name - the name attribute (if any) of
// the first set in the model's list

  char* name() { return(m.name()); }

// Return and modify the transform

  sv_point offset() {return trans;}
  sv_line axis() {return ax;}
  sv_real angle() {return spn;}
  void new_transform(const sv_point& p, const sv_line& a, sv_real th)
  {
    trans = p;
    ax = a;
    spn = th;
    x_changed = 1;    
  }

// Register and return the callback

  void call_back(void (*cb) (sv_model_list*, const sv_point&, 
	      const sv_line&, sv_real, sv_xform_action) ) { xf_cb = cb; }

  void (*call_back()) (sv_model_list*, const sv_point&, 
	      const sv_line&, sv_real, sv_xform_action) {return xf_cb;}

// Register and return the pointer

  void pointer(void* p) { pointer_ = p; }
  void* pointer() { return pointer_; }

// Deal with visibility

  void visible(int v) { vis = v; }
  int visible() const { return vis; }

// Next in the list

  sv_model_list* next() {return n;}

// Force a future recompilation of all the models in the list

  void force_recompile()
  {
    sv_model_list* svml = this;
    while(svml)
    {
      svml->x_changed = 1;
      svml->m_changed = 1;
      svml = svml->n;
    }
  }

// Merge in a new model (unfaceted)

  friend sv_model_list* add_model_to_picture(const sv_model&, const sv_point& p, 
	   const sv_line& a, sv_real th, void (*cb)(sv_model_list*, const sv_point&, 
	   const sv_line&, sv_real, sv_xform_action) );

// Merge in a new model (faceted)

  friend sv_model_list* add_model_to_picture(const sv_model&, const sv_model&, const sv_point& p, 
	   const sv_line& a, sv_real th, void (*cb)(sv_model_list*, const sv_point&, 
	   const sv_line&, sv_real, sv_xform_action) );

// Transform a ray and a point in scene coordinates to one in model coordinates

  sv_line model_line(const sv_line& ray) const
  {
     sv_line result;
     result = ray - trans;
     result = result.spin(ax, -spn);
     return(result);
  }

  sv_point model_point(const sv_point& p) const
  {
     sv_point result;
     result = p - trans;
     result = result.spin(ax, -spn);
     return(result);
  }

// Transform a ray or point in model coordinates to one in scene coordinates

  sv_line scene_line(const sv_line& ray) const
  {
     sv_line result = ray.spin(ax, spn) + trans;
     return(result);
  }

  sv_point scene_point(const sv_point& p) const
  {
     sv_point result = p.spin(ax, spn) + trans;
     return(result);
  }

  friend void recompile_list(sv_model_list*);
  friend void sv_delete_picture_list();
  friend void gl_call_picture_list(sv_model_list*);

};

// plot all changes to a model list

extern void sv_draw_all();

// The plot facets procedure (historical)

extern void plot_m_p_gons(const sv_model& m, const char*); // For backwards compatibility

// Fire a ray into the entire scene of models in the list

extern sv_model_list *sv_ray_into_scene(
	const sv_line& ray,       // The ray
	sv_set& hit_set,          // The hit set (undefined if a miss)
	sv_point& hit_point,      // The point hit in the _set_'s coordinate system
	sv_real& hit_param);      // The ray parameter in the view space coordinate system


// Initialize and return the enclosing box

extern void sv_enclosure(const sv_box& b);

extern sv_box sv_enclosure();

// Null callback for individual model transformations

extern void xform_model(sv_model_list* ml, const sv_point& p, const sv_line& ax, sv_real ang);

// Find the model list entry with a given name

extern sv_model_list* sml_named(char*);

// Function to change the size of a cross 
 
extern void set_user_cross(sv_real uc);

// Functions to set and return the speed of the mouse

extern void sv_mouse_speed(sv_integer);
extern sv_integer sv_mouse_speed();

// Functions to turn on (or off - send 0) logging each
// new image into a collection of files

extern void sv_animate_root(char*);
extern char* sv_animate_root();

inline void plot_m_p_gons(const sv_model& m)  // For backwards compatibility
{
  plot_m_p_gons(m, "svLis");
}

// Plot boxes - mainly diagnostic

extern void plot_m_boxes(const sv_model&, sv_integer, const char*); // For backwards compatibility

inline void plot_m_boxes(const sv_model& m, sv_integer pb)  // For backwards compatibility
{
  plot_m_boxes(m, pb, "svLis");
}

// Return the current display list

extern sv_model_list* all_the_models();


// Merge in a new model (unfaceted)

extern sv_model_list* add_model_to_picture(const sv_model&, const sv_point& p, 
	   const sv_line& a, sv_real th, void (*cb)(sv_model_list*, const sv_point&, 
	   const sv_line&, sv_real, sv_xform_action) );

// Merge in a new model (faceted)

extern sv_model_list* add_model_to_picture(const sv_model&, const sv_model&, const sv_point& p, 
	   const sv_line& a, sv_real th, void (*cb)(sv_model_list*, const sv_point&, 
	   const sv_line&, sv_real, sv_xform_action) );
#endif

