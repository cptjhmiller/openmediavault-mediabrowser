/**
 * Copyright (C) 2014 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")

Ext.define("OMV.module.admin.service.mediabrowser.Settings", {
    extend : "OMV.workspace.form.Panel",

    rpcService   : "MediaBrowser",
    rpcGetMethod : "getSettings",
    rpcSetMethod : "setSettings",

    getFormItems : function() {
        return [{
            xtype    : "fieldset",
            title    : _("General settings"),
            defaults : {
                labelSeparator : ""
            },
            items    : [{
                xtype      : "checkbox",
                name       : "enable",
                fieldLabel : _("Enable"),
                checked    : false
            },{
                xtype      : "checkbox",
                name       : "showtab",
                fieldLabel : _("Show Client"),
                boxLabel   : _("Show tab containing Web Client frame."),
                checked    : false
            },{
                xtype   : "button",
                name    : "openmanage",
                text    : _("Media Browser Web Client"),
                scope   : this,
                margin  : "0 0 5 0",
                handler : function() {
                    var link = 'http://' + location.hostname + ':8096/mediabrowser/dashboard/dashboard.html';
                    window.open(link, '_blank');
                }
            }]
        }];
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "settings",
    path      : "/service/mediabrowser",
    text      : _("Settings"),
    position  : 10,
    className : "OMV.module.admin.service.mediabrowser.Settings"
});
