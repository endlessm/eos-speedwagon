// -*- mode: js; js-indent-level: 4; indent-tabs-mode: nil -*-

const Format = imports.format;
const Lang = imports.lang;

const Config = imports.config;
const GLib = imports.gi.GLib;
const Gio = imports.gi.Gio;
const Gdk = imports.gi.Gdk;
// Needed to be imported so that get_window() returns a proper wrapper.
const GdkX11 = imports.gi.GdkX11;
const Gtk = imports.gi.Gtk;

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
const SPLASH_SCREEN_FADE_OUT = 0.2;

// This needs to match the time of the transitions in speedwagon.css
const SPLASH_SCREEN_COMPLETE_TIME = 0.2;

const SPLASH_BACKGROUND_DESKTOP_KEY = 'X-Endless-SplashBackground';
const DEFAULT_SPLASH_SCREEN_BACKGROUND = 'resource:///com/endlessm/Speedwagon/splash-background-default.jpg';

const QUIT_TIMEOUT = 15;

const Window = new Lang.Class({
    Name: 'EosSpeedwagonWindow',
    Extends: Gtk.ApplicationWindow,

    _init: function(app, appInfo) {
        this._appInfo = appInfo;

        this.parent({ application: app,
                      role: 'eos-speedwagon',
                      title: appInfo.get_name() });
        this.maximize();
        this.set_keep_above(true);

        this._buildUI();
    },

    _buildUI: function() {
        this._baseBox = new Gtk.Box({ visible: true });
        let context = this._baseBox.get_style_context();
        context.add_class('speedwagon-bg');

        this._spinner = new Gtk.Spinner({ visible: true,
                                          width_request: 220,
                                          height_request: 220,
                                          hexpand: true, vexpand: true,
                                          halign: Gtk.Align.CENTER,
                                          valign: Gtk.Align.CENTER });
        this._spinner.get_style_context().add_class('speedwagon-spinner');
        this._spinner.start();

        let spinnerBox = new Gtk.Box({ visible: true });
        spinnerBox.get_style_context().add_class('speedwagon-spinner-bg');

        let overlay = new Gtk.Overlay({ visible: true });
        overlay.add(spinnerBox);
        overlay.add_overlay(this._spinner);
        this._baseBox.add(overlay);

        let gicon = this._appInfo.get_icon();
        let image = new Gtk.Image({ visible: true,
                                    hexpand: true, vexpand: true,
                                    halign: Gtk.Align.CENTER,
                                    valign: Gtk.Align.CENTER,
                                    pixel_size: 64,
                                    gicon: gicon });
        spinnerBox.add(image);

        let bgPath;
        if (this._appInfo.has_key(SPLASH_BACKGROUND_DESKTOP_KEY)) {
            bgPath = this._appInfo.get_string(SPLASH_BACKGROUND_DESKTOP_KEY);
        } else {
            bgPath = DEFAULT_SPLASH_SCREEN_BACKGROUND;
        }

        let provider = new Gtk.CssProvider();
        provider.load_from_data(Format.vprintf('.speedwagon-bg { background-image: url("%s"); }', [bgPath]));
        context.add_provider(provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);

        this.add(this._baseBox);
    },

    _fadeOutTick: function(window, clock) {
        let time = clock.get_frame_time();
        let dt = (time - this._fadeOutStartTime) / GLib.USEC_PER_SEC;
        let opacity = 1.0 - (dt / SPLASH_SCREEN_FADE_OUT);
        if (opacity > 0)
            this.opacity = opacity;
        else
            this.destroy();
        return true;
    },
    _fadeOut: function() {
        this._fadeOutStartTime = this.get_frame_clock().get_frame_time();
        this.add_tick_callback(Lang.bind(this, this._fadeOutTick));
    },

    rampOut: function() {
        this._baseBox.get_style_context().add_class('glow');

        GLib.timeout_add(GLib.PRIORITY_DEFAULT, SPLASH_SCREEN_COMPLETE_TIME * 1000, Lang.bind(this, function() {
            this._fadeOut();
            return false;
        }));
    },
});

const EosSpeedwagon = new Lang.Class({
    Name: 'EosSpeedwagon',
    Extends: Gtk.Application,

    _init: function() {
        this.parent({ application_id: 'com.endlessm.Speedwagon',
                      flags: Gio.ApplicationFlags.IS_SERVICE,
                      inactivity_timeout: QUIT_TIMEOUT * 1000 });

        this._dbusImpl = Gio.DBusExportedObject.wrapJSObject(SpeedwagonIface, this);
        this._dbusImpl.export(Gio.DBus.session, '/com/endlessm/Speedwagon');

        this._splashes = {};
    },

    vfunc_startup: function() {
        this.parent();

        let resource = Gio.Resource.load(Config.RESOURCE_DIR + '/speedwagon.gresource');
        resource._register();

        let provider = new Gtk.CssProvider();
        provider.load_from_file(Gio.File.new_for_uri('resource:///com/endlessm/Speedwagon/speedwagon.css'));
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
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
                this._splashes[appID] = null;
                return false;
            }));
        }

        this._splashes[appID].present();
    },

    HideSplash: function(appID) {
        if (this._splashes[appID])
            this._splashes[appID].rampOut();

        this._splashes[appID] = null;
    },
});

function main(argv) {
    let app = new EosSpeedwagon();
    if (GLib.getenv('EOS_SPEEDWAGON_PERSIST'))
        app.hold();
    return app.run(null);
}
