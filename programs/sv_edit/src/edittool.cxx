/*
 *  Svlis model editor tools
 *
 *    Adrian Bowyer  2 May 1995
 *
 *  This is a simple model editor that is driven by a call interface.
 *  It can thus be used by a windows system as the basis of a GUI, or
 *  by a straightforward text-driven program.
 *
 */

#include "svlis.h"
#include "edittool.h"
#include "sv_edit.h"
#if macintosh
 #pragma export on
#endif

// The current state of the world

state* now = 0;

// The Unix/X xv command

static const char* xv_command = "/usr/local/bin/xv";

//**************************************************************

// Initialize everything

void sv_edit_init()
{
	now = new state();
        sv_lightsource l;
	update_lamplist("L_0", l);
	// box_redivide_draw();
}

// Reinitialize

void cleansheet()
{
	checksave();
	delete now;
	sv_edit_init();
}	

// Shut everything down

void sv_edit_close()
{
	checksave();
	delete now;
	//sv_graph_end();
}

// Get the current model for saving

sv_model model2save()
{
	now->modified = 0;
	return(now->cur_inst->m);
}

void divide_ray(sv_integer from_scratch)
{
	sv_set s = now->cur_inst->m.set_list().set().percolate();
	now->cur_inst->m = sv_model(sv_set_list(s), now->cur_inst->m.box(), LEAF_M, sv_model());
//	set_smart_strategy(SV_MIN_MIN);
	if(from_scratch)
		now->divided_model_r = now->cur_inst->m.divide(0, &dumb_decision);
	else
		now->divided_model_r = now->divided_model_r.redivide(
			now->cur_inst->m.set_list(),0,&dumb_decision);
}

void divide_facet(sv_integer from_scratch)
{
	sv_set s = now->cur_inst->m.set_list().set().percolate();
	now->cur_inst->m = sv_model(sv_set_list(s), now->cur_inst->m.box(), LEAF_M, sv_model());
	if(from_scratch)
		now->divided_model_f = now->cur_inst->m.facet();
	else
		now->divided_model_f = now->divided_model_f.refacet(
			now->cur_inst->m.set_list());
}

// Get the current model and the current divided model

sv_model current_model()
{
	return(now->cur_inst->m);
}
sv_model current_divided_model()
{
	if(now->ray_render)
		return(now->divided_model_r);
	if(now->facet_render)
		return(now->divided_model_f);
	divide_ray(1);
	return(now->divided_model_r);
}

                 
// Print out stats on the current divided model
                 
void printstats(const char* f_name, ostream& op)
{
	current_divided_model().div_stat_report(op);
	if(f_name[0])
	op << "  The current model file is: " << f_name << SV_EL << SV_EL;
	op.flush();
}

// Picture generation controls

sv_integer get_faceting() { return(now->facet_render); }
void set_faceting(sv_integer i) { now->facet_render = i; }
sv_integer get_raytracing() { return(now->ray_render); }
void set_raytracing(sv_integer i) { now->ray_render = i; }
sv_integer get_low() { return(now->low_contents); }
void set_low(sv_integer i) { now->low_contents = i; set_low_contents(i); }
sv_real get_little() { return(now->little_box); }
void set_little(sv_real l) { now->little_box = l; set_small_volume(l); }

void ray_report(sv_real percent)
{
#ifdef SV_UNIX
   char s[STLEN];

   if(now->quickview)
   {

// Update temp file and tell xv to redisplay it

      write_image(now->pic_filename,&(now->pic));
      kill(now->xv_pid,SIGQUIT);
   }

// Use error prompt to ensure message gets through

   eprompt("    Rendering is ");
   r_to_str(s, (sv_real)round(percent));
   eprompt(s);
   eprompt("% complete: estimating finished in ");
   r_to_str(s, (sv_real)( (time(0) - now->start_time)*(100-percent)/percent ));
   eprompt(s); 
   eprompt(" seconds.       \r");
#endif
}

sv_integer get_pic_x(void)
{
	return(now->pic.x_resolution());
}
sv_integer get_pic_y(void)
{
	return(now->pic.y_resolution());
}
void set_pic_resolution(sv_integer x, sv_integer y)
{
	now->pic.resolution(x, y);
}

void do_ray_render()
{
   char s[STLEN];
   sv_set st = now->cur_inst->set();

   if(init_raytrace_cache(st)) 
   {
        svlis_error("do_ray_render()","Cannot initialize raytracing cache",SV_FATAL);
	return;
   }

   // Set entire picture to background colour

   sv_point background_colour = get_background_colour();
   sv_pixel background_pixel_colour;
   background_pixel_colour = sv_pixel(background_colour);
   now->pic.picture_to_colour(background_pixel_colour);

#ifdef SV_UNIX

   if(now->quickview) 
   {

// Create file and start an xv

	 write_image(now->pic_filename, &(now->pic));

	    if((now->xv_pid = fork()) < 0) 
            {
	       // Fork fails
	       now->quickview = 0;
	       svlis_error("do_ray_render()",
	       	"cannot fork to start xv", SV_WARNING);
	    } else 
	    {
	       if(now->xv_pid == 0) 
               {
		  // This is the child - exec an xv
		  execl(xv_command,"xv","-name","SvLis","-viewonly",now->pic_filename,NULL);
	          svlis_error("do_ray_render()",
	       	    "cannot exec xv", SV_WARNING);
	       }
	    }
   }

#endif

   now->start_time = time(0);

#ifdef SV_UNIX
   if(now->quickview) 
	generate_quickview_picture(now->divided_model_r, now->vw, *(now->lamp_list), now->pic,
					  now->report_step,
					  ray_report);
   else
#endif
	generate_picture(now->divided_model_r, now->vw, *(now->lamp_list), now->pic,
				now->report_step,
				ray_report);

   cprompt("    Picture rendering time = ");
   r_to_str(s, (sv_real)(time(0) - now->start_time));
   cprompt(s);
   cprompt(" seconds.                            ");
   cprompt(SV_EL);
   write_image(now->pic_filename,&(now->pic));

   destroy_raytrace_cache();

#ifdef SV_UNIX
   if(now->quickview) 
   {
      if(sv_edit_message("    Remove picture? ", 0)) kill(now->xv_pid,SIGKILL);
   }
#endif
}                                           

sv_view get_view() { return(now->vw); }
sv_integer get_quickview(){ return(now->quickview); }
void set_quickview(sv_integer i) {now->quickview = i;}
sv_real get_report_step(){ return(now->report_step); }
void set_report_step(sv_real rs) 
{
	now->report_step = rs;
	if(rs < 0) now->quickview = 0;
}

void set_pic_filename(char* s)
{
	delete [] now->pic_filename;
	now->pic_filename = new char[sv_strlen(s) + 1];
	sv_strcpy(now->pic_filename, s);
}
char* get_pic_filename() {return(now->pic_filename);}

sv_light_list* get_lamp_list() { return(now->lamp_list); }
void set_lamp_list(sv_light_list* l) { now->lamp_list = l; }

void update_lamplist(const char *name,
		light_type type,
		sv_point location,
		sv_point colour,
		sv_real intensity,
		sv_point direction,
		sv_real angle_power)
{
   sv_light_list *lamplist_entry;
   sv_light_list *l;
   sv_integer new_lamp = 1;

   if(now->lamp_list) 
   {
// Check if light with this name already in list

      l = now->lamp_list;
      while((sv_strcmp(l->name,name) != 0) && (l->next))
	 l = l->next;

      if(sv_strcmp(l->name,name) == 0)
      {

// This is entry to modify

	 lamplist_entry = l;
	 new_lamp = 0;
      } else 
      {

// Add new entry to end of list

	 lamplist_entry = (struct sv_light_list *) malloc(sizeof(sv_light_list));	 
	 lamplist_entry->source = new sv_lightsource;
	 lamplist_entry->name = new char[sv_strlen(name)+1];
	 sv_strcpy(lamplist_entry->name, name);
	 l->next = lamplist_entry;
	 lamplist_entry->next = 0;
      }
   } else 
   {

// Form new lamplist;

      lamplist_entry = (struct sv_light_list *) malloc(sizeof(sv_light_list));	 
      lamplist_entry->source = new sv_lightsource;
      lamplist_entry->name = new char[sv_strlen(name)+1];
      sv_strcpy(lamplist_entry->name, name);
      now->lamp_list = lamplist_entry;
      lamplist_entry->next = 0;
   }

   // Check range of data

   colour.x = max(0.0, min(colour.x, 1.0));
   colour.y = max(0.0, min(colour.y, 1.0));
   colour.z = max(0.0, min(colour.z, 1.0));
   colour = colour.norm() * intensity;
   direction = direction.norm();
   angle_power = max(0.0,angle_power);
   

   lamplist_entry->source->type(type);
   lamplist_entry->source->location(location);
   lamplist_entry->source->colour(colour);
   lamplist_entry->source->direction(direction);
   lamplist_entry->source->angle_power(angle_power);
}

void update_lamplist(const char *name, const sv_lightsource& l)
{
	update_lamplist(name, l.type(),
		l.location(),
		l.colour().norm(),
		l.colour().mod(),
		l.direction(),
		l.angle_power());
}

// Get/set the surface to be used

sv_surface current_surface()
{
	return(now->surf);
}
void new_surface(const sv_surface& s)
{
	now->surf = s;
}

void new_cyl(const sv_line& axis, sv_real r)
{
	now->cyl_axis = axis;
	now->radius = r;
}
void current_cyl(sv_line* axis, sv_real* r)
{
	*axis = now->cyl_axis;
	*r = now->radius;
}

char* current_name()
{
	return(now->filename);
}

void new_name(char* s)
{
	delete [] now->filename;
	now->filename = new char[sv_strlen(s) + 1];
	sv_strcpy(now->filename, s);
}

void new_sphere(const sv_point& cen, sv_real r)
{
	now->sph_cen = cen;
	now->radius = r;
}
void current_sphere(sv_point* cen, sv_real* r)
{
	*cen = now->sph_cen;
	*r = now->radius;
}

void new_plane(const sv_plane& f)
{
	now->cur_plane = f;
}
sv_plane current_plane()
{
	return(now->cur_plane);
}

// Set/Return the current cuboid

void new_cuboid(const sv_box& b)
{
	now->cur_cuboid = b;
}
sv_box current_cuboid()
{
	return(now->cur_cuboid);
}


sv_line get_ray()
{
	sv_line l;
	svlis_error("get_ray()","not implemented under OpenGL",SV_WARNING);
	return(l);
}


// Model is about to be destroyed for some reason
// Warn if it might need to be saved.

void checksave()
{
	if (!now->modified) return;
	sv_model m;
	if(sv_edit_message("Do you want to save the current model? ", 0))
	{
		m = model2save();
		save_model(m);
	}
}

// Change box plotting

void plot_box(sv_integer i)
{
	now->plt_b = i;
}

// Indirection for call to plotting in case anything else is
// needed.

void do_facet_render()
{
	// if(now->plt_b)
	//	plot_m_boxes(now->divided_model_f, SV_M_NE, 
	//		"SvLis editor: model boxes");
	// else
		plot_m_p_gons(now->divided_model_f, "SvLis editor");
}
	
void redraw()
{
	if(now->ray_render) do_ray_render();
	if(now->facet_render) do_facet_render();
}

// Force redivide if box changes

void box_redivide_draw()
{
	if(now->facet_render)
		divide_facet(1);
	if(now->ray_render)
		divide_ray(1);
	redraw();
}

// Force redivide if set changes

void set_redivide_draw()
{
	if(now->facet_render)
		divide_facet(1);
	if(now->ray_render)
		divide_ray(1);
	redraw();
}

// Membership test a point against the current model

mem_test membership(const sv_point& p)
{
	sv_primitive ks;
	return(current_divided_model().member(p, &ks));
}


// Compute the area of the current model

sv_real m_area()
{
	sv_real fac = get_swell_fac();
	if(fac > 0.0001) set_swell_fac(0.0001);
	if((!now->facet_render) || (fac > 0.0001))
		divide_facet(1);
	sv_real r = area(now->divided_model_f);
	if(fac > 0.0001) set_swell_fac(fac);
	if((!now->facet_render) && (fac > 0.0001))
		divide_facet(1);
	return(r);
}

void m_vol(sv_real accy, sv_real& v, sv_point& c, sv_point& mxyz, sv_point& nxyz)
{
	sv_real fac = get_swell_fac();
	if(fac > 0.0001) set_swell_fac(0.0001);
	if((now->facet_render) && (fac > 0.0001))
		divide_facet(1);
	else
	{
		if(fac > 0.0001) divide_ray(1);
	}
	integral(current_divided_model(), accy, v, c, mxyz, nxyz);
	if(fac > 0.0001) set_swell_fac(fac);
	if((!now->facet_render) && (fac > 0.0001))
		divide_facet(1);
	if(now->ray_render  && (fac > 0.0001))
		divide_ray(1);
}

// Raytrace a line into the model

sv_set raytrace(const sv_line& ray, sv_real* t)
{
	sv_set s;
	sv_interval t_int = line_box(ray,current_divided_model().box());
	if (t_int.empty()) return(s); 
	s = current_divided_model().fire_ray(ray, t_int, t);
	return(s);
}


void home_e_view(){ //now->vw=set_scene(now->cur_inst->box(),0); 
}

void e_view_change(const sv_view& new_view){now->vw=new_view; redraw();}

// This instant has changed 

void do_inst()
{
    box_redivide_draw();
    now->modified = 1;
}

// Step back/forward along the edit chain	

void undo()
{
    instant* n = now->cur_inst->previous;
    if (!n) return;    
    now->cur_inst = n;
    do_inst();
}
void redo()
{
	instant* n = now->cur_inst->next;
    if (!n) return;    
    now->cur_inst = n;
    do_inst();
}

#if 0

// Move e_view sideways by an angle

void rotate_h(sv_real angle)
{
	sv_view t = now->vw;
	sv_line spin_axis = sv_line(t.get_up_vector() - t.get_centre(), t.get_centre());
	t.set_eye_point(t.get_eye_point().spin(spin_axis, angle));
	e_view_change(t);	
}


// Move e_view vertically by an angle

void rotate_v(sv_real angle)
{
	sv_view t = now->vw;
	sv_line spin_axis = sv_line((t.get_up_vector() - t.get_centre())^(t.get_eye_point() - t.get_centre()), 
		t.get_centre());
	t.set_eye_point(t.get_eye_point().spin(spin_axis, angle));
	e_view_change(t);	
}

// Spin the e_view by one revolution

void spin()
{
	sv_real a;
	for (a = 0; a < 360.0; a = a + 1.0)
		rotate_h(a*M_PI/180.0);
}

void e_view_left() { rotate_h(-0.1); }
void e_view_right() { rotate_h(0.1); }
void e_view_up()  { rotate_v(-0.1); }
void e_view_down()  { rotate_v(0.1); }

void e_view_forward() 
{  
	sv_view t = now->vw;
	
	t.set_eye_point(t.get_eye_point() + (t.get_centre() - t.get_eye_point())*0.1);
	e_view_change(t);
}

void e_view_back()
{  
	sv_view t = now->vw;
	
	t.set_eye_point(t.get_eye_point() - (t.get_centre() - t.get_eye_point())*0.1);
	e_view_change(t);
}
#endif

// The model has changed

void model_change(instant* new_i)
{
	instant* n = now->cur_inst->next;
	instant* temp;
	
	while(n)
	{
		temp = n;
		n = n->next;
		delete temp;
	}

	now->cur_inst->next = new_i;
	now->cur_inst = now->cur_inst->next;
	now->modified = 1;
}

void model_change(const sv_set& new_set)
{
	instant* n =  new instant(sv_model(sv_set_list(new_set), 
		now->cur_inst->box(), LEAF_M, sv_model()), now->cur_inst, 0);
	model_change(n);
	set_redivide_draw();
}

void model_change(const sv_box& new_box, sv_integer replot)
{
	instant* n =  new instant(sv_model(sv_set_list(
	  now->cur_inst->set()), new_box, LEAF_M, sv_model()), now->cur_inst, 0);
	model_change(n);
	if(replot)box_redivide_draw();
}
void model_change(const sv_set& new_set, const sv_box& new_box)
{
	instant* n =  new instant(sv_model(sv_set_list(
	  new_set), new_box, LEAF_M, sv_model()), now->cur_inst, 0);
	model_change(n);
	box_redivide_draw();
}

// Load in a new model

void model2load(const sv_model& m)
{
	model_change(m.set_list().set(), m.box());
	now->modified = 0;
}

// The user has put in a new set

void add_set(const sv_set& new_set, sv_ed_op op)
{
	sv_set s;
		
	switch(op)
	{
	case UNI:
		s = now->cur_inst->set() | new_set;
		break;
		
	case INTS:
		s = now->cur_inst->set() & new_set;
		break;
			        
	case DIF:
		s = now->cur_inst->set() - new_set;
		break;
			
	case S_DIF:
		s = now->cur_inst->set() ^ new_set;
		break;	
	
	default:
		svlis_error("add_set", "illegal set operator", SV_WARNING);
	}
    model_change(s.surface(now->surf));
}

// Get/change the current model's box

sv_box current_box()
{
	return(now->cur_inst->box());
}

void new_box(const sv_box& nb, sv_integer replot)
{
	model_change(nb, replot);
}
#if macintosh
 #pragma export off
#endif
 












