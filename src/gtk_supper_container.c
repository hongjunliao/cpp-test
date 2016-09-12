#ifdef __CYGWIN_GCC__
#include "gtk_supper_container.h"

struct _SupperContainerPriv {
	GdkWindow *window;
	GdkWindow *offscreen_window;

	gint inner_border;
	gint child_offset;
};

G_DEFINE_TYPE(SupperContainer, supper_container, GTK_TYPE_BIN)

static void supper_container_get_preferred_height(GtkWidget *widget, gint *min, gint *nat) {
	SupperContainerPriv *priv;
	GtkWidget *child;
	gint child_height = 0;
	gint border_width = 0;

	priv = SUPPER_CONTAINER(widget)->priv;

	*min = 0;

	child = gtk_bin_get_child(GTK_BIN(widget));

	if (child && gtk_widget_get_visible(child)) {
		GtkRequisition req;

		gtk_widget_get_preferred_size(child, &req, NULL);

		*min = req.height;
	}

	border_width = gtk_container_get_border_width(GTK_CONTAINER(widget));
	*min += border_width * 2;
	*min += priv->inner_border * 2;

	*nat = *min;
}

static void supper_container_get_preferred_width(GtkWidget *widget, gint *min, gint *nat) {
	SupperContainerPriv *priv;
	GtkWidget *child;
	gint child_height = 0;
	gint border_width = 0;

	priv = SUPPER_CONTAINER(widget)->priv;

	*min = 0;

	child = gtk_bin_get_child(GTK_BIN(widget));

	if (child && gtk_widget_get_visible(child)) {
		GtkRequisition req;

		gtk_widget_get_preferred_size(child, &req, NULL);

		*min = req.width;
	}

	border_width = gtk_container_get_border_width(GTK_CONTAINER(widget));
	*min += border_width * 2;
	*min += priv->inner_border * 2;

	*nat = *min;
}

static void supper_container_size_allocate(GtkWidget *widget, GtkAllocation *allocation) {
	GtkWidget *child;

	gtk_widget_set_allocation(widget, allocation);

	if (gtk_widget_get_realized(widget))
		gdk_window_move_resize(gtk_widget_get_window(widget), allocation->x, allocation->y, allocation->width,
				allocation->height);

	child = gtk_bin_get_child(GTK_BIN(widget));

	if (child && gtk_widget_get_visible(child)) {
		SupperContainerPriv *priv = SUPPER_CONTAINER(widget)->priv;
		GtkAllocation alloc;
		gint height, offset;

		offset = gtk_container_get_border_width(GTK_CONTAINER(widget));
		offset += priv->inner_border;
		priv->child_offset = offset;

		alloc.x = alloc.y = 0;
		alloc.width = allocation->width - offset * 2;
		alloc.height = allocation->height - offset * 2;

		if (gtk_widget_get_realized(widget))
			gdk_window_move_resize(priv->offscreen_window, 0, 0, alloc.width, alloc.height);

		gtk_widget_size_allocate(child, &alloc);
	}
}

static void supper_container_add(GtkContainer *container, GtkWidget *widget) {
	SupperContainer *supper_container = SUPPER_CONTAINER(container);
	SupperContainerPriv *priv = supper_container->priv;

	gtk_widget_set_parent_window(widget, priv->offscreen_window);

	GTK_CONTAINER_CLASS (supper_container_parent_class)->add(container, widget);

	gtk_widget_queue_draw(GTK_WIDGET(container));
}

static void supper_container_remove(GtkContainer *container, GtkWidget *widget) {
	GTK_CONTAINER_CLASS (supper_container_parent_class)->remove(container, widget);

	gtk_widget_queue_draw(GTK_WIDGET(container));
}

static gboolean supper_container_damage(GtkWidget *widget, GdkEventExpose *event) {
	gdk_window_invalidate_rect(gtk_widget_get_window(widget), NULL, TRUE);

	return TRUE;
}

static gboolean supper_container_draw(GtkWidget *widget, cairo_t *cr) {
	SupperContainerPriv *priv = SUPPER_CONTAINER(widget)->priv;
	GtkStyleContext *context;
	GtkStateFlags state;
	gint width, height;

	context = gtk_widget_get_style_context(widget);
	state = gtk_widget_get_state_flags(widget);

	gtk_style_context_set_state(context, state);

	if (gtk_cairo_should_draw_window(cr, gtk_widget_get_window(widget))) {
		GtkWidget *child;
		GdkRGBA color;

		gdk_window_get_geometry(gtk_widget_get_window(widget), NULL, NULL, &width, &height);

		gtk_render_background(context, cr, 0, 0, width, height);

		child = gtk_bin_get_child(GTK_BIN(widget));

		if (child && gtk_widget_get_visible(child)) {
			gdk_cairo_set_source_window(cr, priv->offscreen_window, priv->child_offset, priv->child_offset);
			cairo_rectangle(cr, priv->child_offset, priv->child_offset, gdk_window_get_width(priv->offscreen_window),
					gdk_window_get_height(priv->offscreen_window));
			cairo_fill(cr);

			/* draw frame border */
			gtk_style_context_get_color(context, state, &color);
			gdk_cairo_set_source_rgba(cr, &color);

			cairo_set_line_width(cr, priv->inner_border);
			cairo_rectangle(cr, priv->inner_border, priv->inner_border, width - priv->inner_border * 2,
					height - priv->inner_border * 2);
			cairo_stroke(cr);
		}
	} else if (gtk_cairo_should_draw_window(cr, priv->offscreen_window)) {
		GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));

		gtk_render_background(context, cr, 0, 0, gdk_window_get_width(priv->offscreen_window),
				gdk_window_get_height(priv->offscreen_window));

		if (child)
			gtk_container_propagate_draw(GTK_CONTAINER(widget), child, cr);
	}

	return FALSE;
}

static inline void to_child(SupperContainer *bin, double widget_x, double widget_y, double *x_out, double *y_out) {
	SupperContainerPriv *priv = SUPPER_CONTAINER(bin)->priv;

	*x_out = widget_x - priv->child_offset;
	*y_out = widget_y - priv->child_offset;
}

static inline void to_parent(SupperContainer *bin, double offscreen_x, double offscreen_y, double *x_out, double *y_out) {
	SupperContainerPriv *priv = SUPPER_CONTAINER(bin)->priv;

	*x_out = offscreen_x + priv->child_offset;
	*y_out = offscreen_y + priv->child_offset;
}

static GdkWindow*
pick_offscreen_child(GdkWindow *window, double widget_x, double widget_y, SupperContainer *bin) {
	SupperContainerPriv *priv = SUPPER_CONTAINER(bin)->priv;
	GtkWidget *child = gtk_bin_get_child(GTK_BIN(bin));

	if (child && gtk_widget_get_visible(child)) {
		GtkAllocation child_area;
		double x, y;

		to_child(bin, widget_x, widget_y, &x, &y);

		gtk_widget_get_allocation(child, &child_area);

		if (x >= 0 && x < child_area.width && y >= 0 && y < child_area.height)
			return priv->offscreen_window;
	}

	return NULL;
}

static void offscreen_window_to_parent(GdkWindow *offscreen_window, double offscreen_x, double offscreen_y,
		double *parent_x, double *parent_y, SupperContainer *bin) {
	to_parent(bin, offscreen_x, offscreen_y, parent_x, parent_y);
}

static void offscreen_window_from_parent(GdkWindow *window, double parent_x, double parent_y, double *offscreen_x,
		double *offscreen_y, SupperContainer *bin) {
	to_child(bin, parent_x, parent_y, offscreen_x, offscreen_y);
}

static void supper_container_realize(GtkWidget *widget) {
	SupperContainerPriv *priv;
	GtkStyleContext *context;
	GdkWindowAttr attributes;
	GtkWidget *child;
	gint attributes_mask, border_width;
	GtkAllocation allocation;

	priv = SUPPER_CONTAINER(widget)->priv;

	gtk_widget_set_realized(widget, TRUE);

	gtk_widget_get_allocation(widget, &allocation);
	border_width = gtk_container_get_border_width(GTK_CONTAINER(widget));

	attributes.x = allocation.x + border_width;
	attributes.y = allocation.y + border_width;
	attributes.width = allocation.width - 2 * border_width;
	attributes.height = allocation.height - 2 * border_width;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK
			| GDK_BUTTON_PRESS_MASK;
	attributes.visual = gtk_widget_get_visual(widget);
	attributes.wclass = GDK_INPUT_OUTPUT;

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

	priv->window = gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask);
	gtk_widget_set_window(widget, priv->window);
	gdk_window_set_user_data(priv->window, widget);

	g_signal_connect(priv->window, "pick-embedded-child", G_CALLBACK (pick_offscreen_child), widget);

	child = gtk_bin_get_child(GTK_BIN(widget));
	attributes.window_type = GDK_WINDOW_OFFSCREEN;
	attributes.x = attributes.y = 0;

	if (child && gtk_widget_get_visible(child)) {
		GtkAllocation alloc;

		gtk_widget_get_allocation(child, &alloc);
		attributes.width = allocation.width;
		attributes.height = allocation.height;
	} else
		attributes.width = attributes.height = 0;

	priv->offscreen_window = gdk_window_new(gtk_widget_get_root_window(widget), &attributes, attributes_mask);
	gdk_window_set_user_data(priv->offscreen_window, widget);

	if (child)
		gtk_widget_set_parent_window(child, priv->offscreen_window);

	gdk_offscreen_window_set_embedder(priv->offscreen_window, priv->window);

	g_signal_connect(priv->offscreen_window, "to-embedder", G_CALLBACK (offscreen_window_to_parent), widget);
	g_signal_connect(priv->offscreen_window, "from-embedder", G_CALLBACK (offscreen_window_from_parent), widget);

	context = gtk_widget_get_style_context(widget);
	gtk_style_context_set_background(context, priv->window);
	gtk_style_context_set_background(context, priv->offscreen_window);
	gdk_window_show(priv->offscreen_window);
}

static void supper_container_unrealize(GtkWidget *widget) {
	SupperContainerPriv *priv = SUPPER_CONTAINER(widget)->priv;

	if (priv->offscreen_window) {
		gdk_window_set_user_data(priv->offscreen_window, NULL);
		gdk_window_destroy(priv->offscreen_window);
		priv->offscreen_window = NULL;
	}

	GTK_WIDGET_CLASS(supper_container_parent_class)->unrealize(widget);
}

static void supper_container_init(SupperContainer *supper) {
	SupperContainerPriv *priv;

	supper->priv = priv = G_TYPE_INSTANCE_GET_PRIVATE(supper, SUPPER_CONTAINER_TYPE, SupperContainerPriv);

	priv->inner_border = 2;

	gtk_widget_set_has_window(GTK_WIDGET(supper), TRUE);
}

static void supper_container_class_init(SupperContainerClass *klass) {
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkContainerClass *container_class;

	object_class = G_OBJECT_CLASS(klass);
	widget_class = GTK_WIDGET_CLASS(klass);
	container_class = GTK_CONTAINER_CLASS(klass);

	container_class->add = supper_container_add;
	container_class->remove = supper_container_remove;

	widget_class->realize = supper_container_realize;
	widget_class->unrealize = supper_container_unrealize;
	widget_class->draw = supper_container_draw;
	widget_class->get_preferred_width = supper_container_get_preferred_width;
	widget_class->get_preferred_height = supper_container_get_preferred_height;
	widget_class->size_allocate = supper_container_size_allocate;

	g_signal_override_class_closure(g_signal_lookup("damage-event", GTK_TYPE_WIDGET), SUPPER_CONTAINER_TYPE,
			g_cclosure_new(G_CALLBACK(supper_container_damage),
			NULL, NULL));

	g_type_class_add_private(object_class, sizeof(SupperContainerPriv));
}

GtkWidget*
supper_container_new(void) {
	return g_object_new(SUPPER_CONTAINER_TYPE, NULL);
}
#endif //__CYGWIN_GCC__
