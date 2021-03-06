<?php
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

require_once("openmediavault/error.inc");
require_once("openmediavault/notify.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/initscript.inc");

class OMVRpcServiceMediaBrowser extends OMVRpcServiceAbstract
{

    public function getName()
    {
        return "MediaBrowser";
    }

    private function getPluginName()
    {
        return strtolower($this->getName());
    }

    private function getXPath()
    {
        return sprintf("//services/%s", $this->getPluginName());
    }

    private function getEventMessagePath()
    {
        return sprintf("org.openmediavault.services.%s", $this->getPluginName());
    }

    public function initialize() {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
        $this->registerMethod("doRestart");
    }

    /**
     * Get the settings of the service.
     *
     * @param array $params The method paramaters.
     * @param array $context The context of the caller.
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     *
     * @return mixed
     */
    public function getSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $object = $xmlConfig->get($this->getXpath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        // Modify result data
        $settingsObject["enable"] = boolval($object["enable"]);
        $settingsObject["showtab"] = boolval($object["showtab"]);
        $settingsObject["mntentref"] = $object["mntentref"];
        $settingsObject["db-folder"] = $object["db-folder"];

        return $settingsObject;
    }

    /**
     * Set the settings of the service.
     *
     * @param array $params The method paramaters.
     * @param array $context The context of the caller.
     *
     * @throws E_CONFIG_SET_OBJECT_FAILED
     *
     * @return mixed
     */
    public function setSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type": "object",
                "properties": {
                    "enable": { "type" : "boolean" },
                    "showtab": { "type" : "boolean" },
                    "mntentref": { ' . $GLOBALS['OMV_JSONSCHEMA_UUID_NONE'] . ', "optional" : true },
                    "db-folder": { "type" : "string", "optional" : true }
                }
            }'
        );

        // Get path of mntentref and set db-folder to this path
        if ( empty($params['mntentref']) == false )
        {
            $xPath = sprintf("//system/fstab/mntent[uuid='%s']", $params['mntentref']);
            $deviceDir = $xmlConfig->get($xPath);
            $deviceDir = $deviceDir['dir'];

            $dbFolder = "{$deviceDir}/mediabrowser";
            $params['db-folder'] = $dbFolder;
        } else {
            $params['db-folder'] = "/var/lib/mediabrowser";
        }

        // Gets the stored folder to see if a data movement is required
        $object = $xmlConfig->get($this->getXpath());
        if (!is_null($object)) {
            $old_folder = ($params['db-folder'] == $object["db-folder"]) ? "" : $object["db-folder"];
        } else {
            $old_folder = "";
        }

        // Prepare configuration data
        $object = array(
            "enable"    => array_boolval($params, "enable"),
            "showtab"   => array_boolval($params, "showtab"),
            "mntentref" => $params['mntentref'],
            "db-folder" => $params['db-folder'],
            "old_db-folder" => $old_folder
        );

        // Set configuration object
        if (false === $xmlConfig->replace($this->getXpath(), $object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            $object
        );

        return $object;
    }

    public function doRestart($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        if (!boolval($object["enable"]))
        {
            return;
        }

        $initd = new OMVSysVInitScript($this->getPluginName());
        $initd->invoke("restart");
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceMediaBrowser());
