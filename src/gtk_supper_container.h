#ifndef __SUPPER_CONTAINER_H__
#define __SUPPER_CONTAINER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define SUPPER_CONTAINER_TYPE (supper_container_get_type ())
#define SUPPER_CONTAINER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUPPER_CONTAINER_TYPE, SupperContainer))
#define SUPPER_CONTAINER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SUPPER_CONTAINER_TYPE, SupperContainerClass))
#define IS_SUPPER_CONTAINER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUPPER_CONTAINER_TYPE))
#define IS_SUPPER_CONTAINER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SUPPER_CONTAINER_TYPE))
#define SUPPER_CONTAINER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), SUPPER_CONTAINER_TYPE, SupperContainerClass))

typedef struct _SupperContainer SupperContainer;
typedef struct _SupperContainerPriv SupperContainerPriv;
typedef struct _SupperContainerClass SupperContainerClass;

struct _SupperContainer {
	GtkBin parent_instance;

	SupperContainerPriv *priv;
};

struct _SupperContainerClass {
	GtkBinClass parent_class;
};

GType supper_container_get_type(void) G_GNUC_CONST;

GtkWidget* supper_container_new(void);

G_END_DECLS

#endif
