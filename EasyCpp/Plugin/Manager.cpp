#include "Manager.h"
#include "Plugin.h"
#include <fstream>

#include "IPluginDatabaseProvider.h"
#include "../Database/DatabaseDriverManager.h"

namespace EasyCpp
{
	namespace Plugin
	{
		Manager::Manager()
		{
		}

		Manager::~Manager()
		{
		}

		void Manager::registerInterface(InterfacePtr iface)
		{
			auto name = iface->getName();
			auto rev = iface->getVersion();
			if (_server_ifaces.count(name) && _server_ifaces[name].count(rev))
			{
				throw std::logic_error("Interface already registered");
			}
			_server_ifaces[name][rev] = iface;
		}

		void Manager::loadPlugin(const std::string & name, const std::string & path, const std::vector<InterfacePtr>& server_ifaces)
		{
			if (_plugins.count(name))
				throw std::runtime_error("Plugin name already used");
			interface_map_t tmap = _server_ifaces;
			for (auto& e : server_ifaces)
				tmap[e->getName()][e->getVersion()] = e;
			auto plugin = std::make_shared<Plugin>(name, path, tmap);
			_plugins[name] = plugin;

			if (_autoregister) {
				// Autoregister EasyCpp Extensions
				if (plugin->hasInterface<IPluginDatabaseProvider>())
				{
					auto iface = plugin->getInterface<IPluginDatabaseProvider>();
					for (auto& e : iface->getDriverMap()) {
						Database::DatabaseDriverManager::registerDriver(e.first, e.second);
					}
				}
			}
		}

		void Manager::loadPluginFromMemory(const std::string & name, const std::vector<uint8_t>& data, const std::vector<InterfacePtr>& server_ifaces)
		{
			// Write data to a file and load it.
			char fname[L_tmpnam];
			tmpnam((char*)&fname);
			{
				std::ofstream stream;
				stream.open(fname, std::ofstream::out | std::ofstream::binary);
				stream.write((const char*)data.data(), data.size());
				stream.close();
			}
			this->loadPlugin(name, fname, server_ifaces);
		}

		InterfacePtr Manager::getInterface(const std::string & pluginname, const std::string & ifacename, uint64_t version)
		{
			if (_plugins.count(pluginname) == 0)
				throw std::runtime_error("Plugin not found");
			return _plugins.at(pluginname)->getInterface(ifacename, version);
		}

		bool Manager::hasInterface(const std::string & pluginname, const std::string & ifacename, uint64_t version)
		{
			if (_plugins.count(pluginname) == 0)
				throw std::runtime_error("Plugin not found");
			return _plugins.at(pluginname)->hasInterface(ifacename, version);
		}

		void Manager::unloadPlugin(const std::string & name)
		{
			if (!_plugins.count(name))
				throw std::runtime_error("Plugin not found");
			if (!_plugins.at(name)->canUnload())
				throw std::runtime_error("Plugin is in use");
			if (_autoregister) {
				// Autoregister EasyCpp Extensions
				if (_plugins.at(name)->hasInterface<IPluginDatabaseProvider>())
				{
					auto iface = _plugins.at(name)->getInterface<IPluginDatabaseProvider>();
					for (auto& e : iface->getDriverMap()) {
						Database::DatabaseDriverManager::deregisterDriver(e.first);
					}
				}
			}
			_plugins.at(name)->deinit();
			_plugins.erase(name);
		}

		std::vector<std::string> Manager::getPlugins()
		{
			std::vector<std::string> res;
			for (auto e : _plugins)
				res.push_back(e.first);
			return res;
		}

		void Manager::setAutoRegisterExtensions(bool v)
		{
			_autoregister = v;
		}

		bool Manager::isAutoRegisterExtensions() const
		{
			return _autoregister;
		}
	}
}