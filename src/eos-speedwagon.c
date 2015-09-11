#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>
#include <string.h>

#include "speedwagon-service.h"

#define SPLASH_SCREEN_COMPLETE_TIME 0.2
#define SPLASH_SCREEN_FADE_OUT 0.2

#define QUIT_TIMEOUT 15
#define SPLASH_BACKGROUND_DESKTOP_KEY "X-Endless-SplashBackground"
#define DEFAULT_SPLASH_SCREEN_BACKGROUND "resource:///com/endlessm/Speedwagon/splash-background-default.jpg"


typedef struct {
  GtkApplication parent;

  EosSpeedwagon *skeleton;
  GHashTable *splashes;
} EosSpeedwagonApp;

typedef struct {
  GtkApplicationClass parent_class;
} EosSpeedwagonAppClass;

typedef struct {
  GtkApplicationWindow parent;
  GDesktopAppInfo *app_info;

  GtkWidget *base_box;
  GtkWidget *spinner;

  guint ramp_out_id;
  guint tick_id;

  gint64 fade_out_start_time;
} EosSpeedwagonWindow;

typedef struct {
  GtkApplicationWindowClass parent_class;
} EosSpeedwagonWindowClass;

GType eos_speedwagon_window_get_type (void) G_GNUC_CONST;
G_DEFINE_TYPE (EosSpeedwagonWindow, eos_speedwagon_window, GTK_TYPE_APPLICATION_WINDOW)

static void
eos_speedwagon_window_build_ui (EosSpeedwagonWindow *self)
{
  GtkStyleContext *context;

  self->base_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_show (self->base_box);
  context = gtk_widget_get_style_context (self->base_box);
  gtk_style_context_add_class (context, "speedwagon-bg");

  self->spinner = gtk_spinner_new ();
  gtk_widget_show (self->spinner);
  gtk_widget_set_size_request (self->spinner, 220, 220);
  gtk_widget_set_hexpand (self->spinner, TRUE);
  gtk_widget_set_vexpand (self->spinner, TRUE);
  gtk_widget_set_halign (self->spinner, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (self->spinner, GTK_ALIGN_CENTER);
  context = gtk_widget_get_style_context (self->spinner);
  gtk_style_context_add_class (context, "speedwagon-spinner");
  gtk_spinner_start (GTK_SPINNER (self->spinner));

  GtkWidget *spinner_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_show (spinner_box);
  context = gtk_widget_get_style_context (spinner_box);
  gtk_style_context_add_class (context, "speedwagon-spinner-bg");

  GtkWidget *overlay = gtk_overlay_new ();
  gtk_widget_show (overlay);
  gtk_container_add (GTK_CONTAINER (overlay), spinner_box);
  gtk_overlay_add_overlay (GTK_OVERLAY (overlay), self->spinner);
  gtk_container_add (GTK_CONTAINER (self->base_box), overlay);

  GIcon *icon = g_app_info_get_icon (G_APP_INFO (self->app_info));
  GtkWidget *image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_image_set_pixel_size (GTK_IMAGE (image), 64);
  gtk_widget_set_hexpand (image, TRUE);
  gtk_widget_set_vexpand (image, TRUE);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_CENTER);
  gtk_container_add (GTK_CONTAINER (spinner_box), image);

  gchar *bg_path;
  if (g_desktop_app_info_has_key (self->app_info,
                                  SPLASH_BACKGROUND_DESKTOP_KEY))
    bg_path = g_desktop_app_info_get_string (self->app_info,
                                             SPLASH_BACKGROUND_DESKTOP_KEY);
  else
    bg_path = g_strdup (DEFAULT_SPLASH_SCREEN_BACKGROUND);

  GError *error = NULL;
  GtkCssProvider *provider = gtk_css_provider_new ();
  char *css_data =
    g_strdup_printf (".speedwagon-bg { background-image: url(\"%s\"); }",
                     bg_path);
  gtk_css_provider_load_from_data (provider, css_data, -1, &error);
  if (error != NULL)
    {
      g_warning ("Unable to load CSS for custom splash: %s", error->message);
      g_error_free (error);
    }
  else
    {
      context = gtk_widget_get_style_context (self->base_box);
      gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (provider),
                                      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

  g_free (css_data);
  g_free (bg_path);
  g_object_unref (provider);

  gtk_container_add (GTK_CONTAINER (self), self->base_box);
}

static gboolean
fade_out_tick (GtkWidget *widget,
               GdkFrameClock *frame_clock,
               gpointer user_data)
{
  EosSpeedwagonWindow *self = user_data;
  gint64 time = gdk_frame_clock_get_frame_time (frame_clock);
  gint64 dt = (gint64) (time - self->fade_out_start_time) / G_USEC_PER_SEC;
  gdouble opacity = 1.0 - (gdouble) (dt / SPLASH_SCREEN_FADE_OUT);

  if (opacity > 0)
    gtk_widget_set_opacity (widget, opacity);
  else
    gtk_widget_destroy (widget);

  return G_SOURCE_CONTINUE;
}

static void
eos_speedwagon_window_fade_out (EosSpeedwagonWindow *self)
{
  self->fade_out_start_time = gdk_frame_clock_get_frame_time
    (gtk_widget_get_frame_clock (GTK_WIDGET (self)));

  self->tick_id =
    gtk_widget_add_tick_callback (GTK_WIDGET (self), fade_out_tick, self, NULL);
}

static gboolean
ramp_out_timeout (gpointer user_data)
{
  EosSpeedwagonWindow *self = user_data;
  self->ramp_out_id = 0;
  eos_speedwagon_window_fade_out (self);

  return G_SOURCE_REMOVE;
}

static void
eos_speedwagon_window_ramp_out (EosSpeedwagonWindow *self)
{
  if (self->ramp_out_id != 0)
    return;

  GtkStyleContext *context = gtk_widget_get_style_context (self->base_box);
  gtk_style_context_add_class (context, "glow");

  self->ramp_out_id =
    g_timeout_add (SPLASH_SCREEN_COMPLETE_TIME,
                   ramp_out_timeout, self);
}

static void
eos_speedwagon_window_finalize (GObject *object)
{
  EosSpeedwagonWindow *self = (EosSpeedwagonWindow *) object;

  g_clear_object (&self->app_info);
  if (self->ramp_out_id != 0)
    g_source_remove (self->ramp_out_id);
  if (self->tick_id != 0)
    gtk_widget_remove_tick_callback (GTK_WIDGET (self), self->tick_id);

  G_OBJECT_CLASS (eos_speedwagon_window_parent_class)->finalize (object);
}

static void
eos_speedwagon_window_init (EosSpeedwagonWindow *self)
{
}

static void
eos_speedwagon_window_class_init (EosSpeedwagonWindowClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->finalize = eos_speedwagon_window_finalize;
}

static GtkWindow *
eos_speedwagon_window_new (EosSpeedwagonApp *app,
                           GDesktopAppInfo *app_info)
{
  EosSpeedwagonWindow *window =
    g_object_new (eos_speedwagon_window_get_type (),
                  "application", app,
                  "role", "eos-speedwagon",
                  "title", g_app_info_get_name (G_APP_INFO (app_info)),
                  NULL);
  GtkWindow *gtk_window = GTK_WINDOW (window);

  window->app_info = g_object_ref (app_info);
  gtk_window_maximize (gtk_window);
  gtk_window_set_keep_above (gtk_window, TRUE);

  gtk_widget_realize (GTK_WIDGET (window));
  GdkWindow *gdk_window = gtk_widget_get_window (GTK_WIDGET (window));
  const char *desktop_id = g_app_info_get_id (G_APP_INFO (app_info));
  char *app_id = g_strndup (desktop_id, strlen (desktop_id) - 8);
  gdk_x11_window_set_utf8_property (gdk_window, "_GTK_APPLICATION_ID", app_id);
  g_free (app_id);

  eos_speedwagon_window_build_ui (window);

  return gtk_window;
}

GType eos_speedwagon_app_get_type (void) G_GNUC_CONST;
G_DEFINE_TYPE (EosSpeedwagonApp, eos_speedwagon_app, GTK_TYPE_APPLICATION)

static gboolean
speedwagon_window_delete_event (GtkWidget *widget,
                                GdkEvent *event,
                                gpointer user_data)
{
  EosSpeedwagonApp *self = user_data;
  EosSpeedwagonWindow *window = (EosSpeedwagonWindow *) widget;
  const char *desktop_id;

  desktop_id = g_app_info_get_id (G_APP_INFO (window->app_info));
  eos_speedwagon_emit_splash_closed (self->skeleton, desktop_id);
  g_hash_table_remove (self->splashes, desktop_id);

  return GDK_EVENT_PROPAGATE;
}

static gboolean
speedwagon_service_show_splash (EosSpeedwagon *skeleton,
                                GDBusMethodInvocation *invocation,
                                const char *desktop_id,
                                EosSpeedwagonApp *self)
{
  GtkWindow *window = g_hash_table_lookup (self->splashes, desktop_id);

  if (!window)
    {
      GDesktopAppInfo *info = g_desktop_app_info_new (desktop_id);
      if (info == NULL)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR,
                                                 G_DBUS_ERROR_FILE_NOT_FOUND,
                                                 "Desktop ID %s does not exist",
                                                 desktop_id);
          return TRUE;
        }


      window = eos_speedwagon_window_new (self, info);
      g_signal_connect (window, "delete-event",
                        G_CALLBACK (speedwagon_window_delete_event), self);
      g_hash_table_insert (self->splashes, g_strdup (desktop_id), window);
      g_object_unref (info);
    }

  gtk_window_present (window);
  eos_speedwagon_complete_show_splash (skeleton, invocation);

  return TRUE;
}

static gboolean
speedwagon_service_hide_splash (EosSpeedwagon *skeleton,
                                GDBusMethodInvocation *invocation,
                                const char *desktop_id,
                                EosSpeedwagonApp *self)
{
  EosSpeedwagonWindow *window = g_hash_table_lookup (self->splashes, desktop_id);

  if (!window)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR,
                                             G_DBUS_ERROR_UNKNOWN_OBJECT,
                                             "Splash for desktop ID %s does not exist",
                                             desktop_id);
      return TRUE;
    }

  eos_speedwagon_window_ramp_out (window);
  g_hash_table_remove (self->splashes, desktop_id);
  eos_speedwagon_complete_hide_splash (skeleton, invocation);

  return TRUE;
}

static void
eos_speedwagon_app_startup (GApplication *app)
{
  G_APPLICATION_CLASS (eos_speedwagon_app_parent_class)->startup (app);

  GtkCssProvider *provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_resource (provider,
                                       "/com/endlessm/Speedwagon/speedwagon.css");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref (provider);
}

static void
eos_speedwagon_app_activate (GApplication *app)
{
}

static gboolean
eos_speedwagon_app_dbus_register (GApplication *app,
                                  GDBusConnection *connection,
                                  const char *object_path,
                                  GError **error)
{
  EosSpeedwagonApp *self = (EosSpeedwagonApp *) app;

  self->skeleton = eos_speedwagon_skeleton_new ();
  g_signal_connect (self->skeleton, "handle-show-splash",
                    G_CALLBACK (speedwagon_service_show_splash), self);
  g_signal_connect (self->skeleton, "handle-hide-splash",
                    G_CALLBACK (speedwagon_service_hide_splash), self);

  return g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (self->skeleton),
                                           connection, object_path, error);
}

static void
eos_speedwagon_app_finalize (GObject *object)
{
  EosSpeedwagonApp *self = (EosSpeedwagonApp *) object;
  g_clear_object (&self->skeleton);
  g_hash_table_unref (self->splashes);

  G_OBJECT_CLASS (eos_speedwagon_app_parent_class)->finalize (object);
}

static void
eos_speedwagon_app_init (EosSpeedwagonApp *self)
{
  self->splashes = g_hash_table_new_full (g_str_hash, g_str_equal,
                                          g_free, NULL);
}

static void
eos_speedwagon_app_class_init (EosSpeedwagonAppClass *klass)
{
  GApplicationClass *aclass = G_APPLICATION_CLASS (klass);
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  aclass->startup = eos_speedwagon_app_startup;
  aclass->activate = eos_speedwagon_app_activate;
  aclass->dbus_register = eos_speedwagon_app_dbus_register;

  oclass->finalize = eos_speedwagon_app_finalize;
}

static GApplication *
eos_speedwagon_app_new (void)
{
  return g_object_new (eos_speedwagon_app_get_type (),
                       "application-id", "com.endlessm.Speedwagon",
                       "flags", G_APPLICATION_IS_SERVICE,
                       "inactivity-timeout", QUIT_TIMEOUT * 1000,
                       NULL);
}

int
main (int argc,
      char **argv)
{
  GApplication *app = eos_speedwagon_app_new ();
  if (g_getenv ("EOS_SPEEDWAGON_PERSIST") != NULL)
    g_application_hold (app);

  int retval = g_application_run (app, argc, argv);
  g_object_unref (app);

  return retval;
}
