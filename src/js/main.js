// -*- mode: js; js-indent-level: 4; indent-tabs-mode: nil -*-

const GLib = imports.gi.GLib;
const Gio = imports.gi.Gio;
const Gdk = imports.gi.Gdk;

// Needed to be imported so that get_window() returns a proper wrapper.
const GdkX11 = imports.gi.GdkX11;

const Gtk = imports.gi.Gtk;
const Speedwagon = imports.gi.Speedwagon;

const Format = imports.format;
const Lang = imports.lang;

const SpeedwagonIface = '<node> \
<interface name="com.endlessm.Speedwagon"> \
<method name="ShowSplash"> \
    <arg type="s" direction="in" name="desktopFile" /> \
</method> \
<method name="HideSplash"> \
    <arg type="s" direction="in" name="desktopFile" /> \
</method> \
<signal name="SplashClosed"> \
    <arg type="s" name="desktopFile" /> \
</signal> \
</interface> \
</node>';

const SPLASH_CIRCLE_PERIOD = 2;
const SPLASH_SCREEN_FADE_OUT = 1;
const SPLASH_SCREEN_COMPLETE_TIME = 1;

const SPLASH_BACKGROUND_DESKTOP_KEY = 'X-Endless-SplashBackground';
const DEFAULT_SPLASH_SCREEN_BACKGROUND = 'resource:///com/endlessm/Speedwagon/splash-background-default.jpg';

const Spinner = new Lang.Class({
    Name: 'EosSpeedwagonSpinner',
    Extends: Speedwagon.Spinner,

    _init: function(params) {
        this.parent(params);

        this.get_style_context().add_class('speedwagon-spinner');

        this._spinner = new Gtk.Image({ visible: true, resource: '/com/endlessm/Speedwagon/splash-spinner.png' });
        this._spinner.set_parent(this);
        this._glowChannel = new Gtk.Image({ visible: true, resource: '/com/endlessm/Speedwagon/splash-spinner-channel-glow.png' });
        this._glowChannel.set_parent(this);
        this._glowSpinner = new Gtk.Image({ visible: true, resource: '/com/endlessm/Speedwagon/splash-spinner-glow.png' });
        this._glowSpinner.set_parent(this);
        this.set_internal_widgets([this._spinner, this._glowChannel, this._glowSpinner]);

        this._rotation = 0;
        this.glowOpacity = 0;
    },

    set glowOpacity(value) {
        this._glowOpacity = value;
        this._glowChannel.opacity = value;
        this._glowSpinner.opacity = value;
    },
    get glowOpacity() {
        return this._glowOpacity;
    },

    vfunc_size_allocate: function(rect) {
        this.parent(rect);
        this._spinner.size_allocate(rect);
        this._glowChannel.size_allocate(rect);
        this._glowSpinner.size_allocate(rect);
    },

    vfunc_draw: function(cr) {
        // Draw the background -- the standard channel
        let context = this.get_style_context();
        let alloc = this.get_allocation();
        Gtk.render_background(this.get_style_context(), cr, 0, 0, alloc.width, alloc.height);

        // Draw all the normal elements
        this.foreach(Lang.bind(this, function(child) {
            this.propagate_draw(child, cr);
        }));

        // Draw the channel glow
        this.propagate_draw(this._glowChannel, cr);

        // Draw the spinner
        cr.translate(alloc.width / 2, alloc.height / 2);
        cr.rotate(this._rotation * Math.PI * 2);
        cr.translate(-alloc.width / 2, -alloc.height / 2);
        this.propagate_draw(this._glowSpinner, cr);
        this.propagate_draw(this._spinner, cr);

        cr.$dispose();
    },

    tick: function(clock) {
        let time = clock.get_frame_time() / GLib.USEC_PER_SEC;
        if (!this._startTime)
            this._startTime = time;

        // 1s = 1 full turn of the spinner
        this._rotation = (time - this._startTime) / SPLASH_CIRCLE_PERIOD;
        this.queue_draw();
    },
});

const Window = new Lang.Class({
    Name: 'EosSpeedwagonWindow',
    Extends: Gtk.ApplicationWindow,

    _init: function(app, appInfo) {
        this._appInfo = appInfo;

        this.parent({ application: app,
                      role: 'eos-speedwagon',
                      title: appInfo.get_name() });
        this.maximize();

        this.realize();
        let gdkWindow = this.get_window();
        let appID = appInfo.get_id().replace('.desktop', '');
        gdkWindow.set_utf8_property('_GTK_APPLICATION_ID', appID);

        this._buildUI();

        this._rampOutStartTime = 0;

        // Start the spinning...
        this.add_tick_callback(Lang.bind(this, this._tick));
    },

    _buildUI: function() {
        let baseBox = new Gtk.Box({ visible: true });
        let context = baseBox.get_style_context();
        context.add_class('speedwagon-bg');

        this._spinner = new Spinner({ visible: true,
                                      width_request: 220,
                                      height_request: 220,
                                      hexpand: true, vexpand: true,
                                      halign: Gtk.Align.CENTER,
                                      valign: Gtk.Align.CENTER });
        baseBox.add(this._spinner);

        let gicon = this._appInfo.get_icon();
        let image = new Gtk.Image({ visible: true,
                                    hexpand: true, vexpand: true,
                                    halign: Gtk.Align.CENTER,
                                    valign: Gtk.Align.CENTER,
                                    pixel_size: 64,
                                    gicon: gicon });
        this._spinner.add(image);

        let bgPath;
        if (this._appInfo.has_key(SPLASH_BACKGROUND_DESKTOP_KEY)) {
            bgPath = this._appInfo.get_string(SPLASH_BACKGROUND_DESKTOP_KEY);
        } else {
            bgPath = DEFAULT_SPLASH_SCREEN_BACKGROUND;
        }

        let provider = new Gtk.CssProvider();
        provider.load_from_data(Format.vprintf('.speedwagon-bg { background-image: url("%s"); }', [bgPath]));
        context.add_provider(provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);

        this.add(baseBox);
    },

    _tick: function(window, clock) {
        this._spinner.tick(clock);

        if (this._rampOutStartTime) {
            let time = clock.get_frame_time();
            let dt = (time - this._rampOutStartTime) / GLib.USEC_PER_SEC;

            if (dt < SPLASH_SCREEN_COMPLETE_TIME) {
                this._spinner.glowOpacity = dt / SPLASH_SCREEN_COMPLETE_TIME;
            } else if (dt - SPLASH_SCREEN_COMPLETE_TIME < SPLASH_SCREEN_FADE_OUT) {
                this.opacity = 1.0 - (dt - SPLASH_SCREEN_COMPLETE_TIME) / SPLASH_SCREEN_FADE_OUT;
            } else {
                this.destroy();
            }
        }

        return true;
    },

    rampOut: function() {
        let clock = this.get_frame_clock();
        this._rampOutStartTime = clock.get_frame_time();
    },
});

const EosSpeedwagon = new Lang.Class({
    Name: 'EosSpeedwagon',
    Extends: Gtk.Application,

    _init: function() {
        this.parent({ application_id: 'com.endlessm.Speedwagon' });

        this._dbusImpl = Gio.DBusExportedObject.wrapJSObject(SpeedwagonIface, this);
        this._dbusImpl.export(Gio.DBus.session, '/com/endlessm/Speedwagon');

        this._splashes = {};
    },

    vfunc_startup: function() {
        this.parent();

        let resource = Gio.Resource.load('speedwagon.gresource');
        resource._register();

        let provider = new Gtk.CssProvider();
        provider.load_from_file(Gio.File.new_for_uri('resource:///com/endlessm/Speedwagon/speedwagon.css'));
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);

        this.hold();
    },

    vfunc_activate: function() {
    },

    _emitSplashClosed: function(appID) {
        let connection = this._dbusImpl.get_connection();
        let info = this._dbusImpl.get_info();
        connection.emit_signal(null, this._dbusImpl.get_object_path(), info.name, 'SplashClosed', GLib.Variant.new('(s)', [appID]));
    },

    ShowSplash: function(appID) {
        if (!this._splashes[appID]) {
            let appInfo = Gio.DesktopAppInfo.new(appID);
            this._splashes[appID] = new Window(this, appInfo);
            this._splashes[appID].connect('delete-event', Lang.bind(this, function() {
                this._emitSplashClosed(appID);
            }));
        }

        this._splashes[appID].present();
    },

    HideSplash: function(appID) {
        this._splashes[appID].rampOut();
        this._splashes[appID] = null;
    },
});

let app = new EosSpeedwagon();
app.run(null);
