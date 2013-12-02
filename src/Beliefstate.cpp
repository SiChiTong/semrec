#include <Beliefstate.h>


namespace beliefstate {
  Beliefstate::Beliefstate(int argc, char** argv) {
    m_psPlugins = NULL;
    m_bRun = true;
    m_argc = argc;
    m_argv = argv;
    m_strBaseDataDirectory = "";
  }
  
  Beliefstate::~Beliefstate() {
    if(m_psPlugins) {
      delete m_psPlugins;
    }
  }
  
  Result Beliefstate::init(string strConfigFile) {
    Result resInit = defaultResult();
    
    list<string> lstConfigFileLocations;
    lstConfigFileLocations.push_back("./");
    lstConfigFileLocations.push_back("~/.beliefstate/");
    lstConfigFileLocations.push_back(ros::package::getPath("beliefstate"));
    
    // Do the actual init here.
    m_psPlugins = new PluginSystem(m_argc, m_argv);
    
    bool bConfigLoaded = false;
    if(strConfigFile != "") {
      bConfigLoaded = this->loadConfigFile(strConfigFile);
    }
    
    if(!bConfigLoaded) {
      for(list<string>::iterator itPath = lstConfigFileLocations.begin();
	  itPath != lstConfigFileLocations.end();
	  itPath++) {
	string strPath = *itPath;
	
	if(this->loadConfigFile(strPath + "/config.cfg")) {
	  bConfigLoaded = true;
	  break;
	}
      }
    }
    
    if(bConfigLoaded) {
      for(list<string>::iterator itPluginName = m_lstPluginsToLoad.begin();
	  itPluginName != m_lstPluginsToLoad.end();
	  itPluginName++) {
	m_psPlugins->loadPluginLibrary(*itPluginName, true);
      }
    } else {
      cerr << "Failed to load a valid config file.";
      resInit.bSuccess = false;
    }
    
    return resInit;
  }
  
  Result Beliefstate::deinit() {
    Result resInit = defaultResult();
    
    // Do the actual deinit here.
    
    return resInit;
  }
  
  bool Beliefstate::loadConfigFile(string strConfigFile) {
    Config cfgConfig;
    
    try {
      cfgConfig.readFile(strConfigFile.c_str());
      
      // Section: Persistent data storage
      Setting &sPersistentDataStorage = cfgConfig.lookup("persistent-data-storage");
      sPersistentDataStorage.lookupValue("base-data-directory", m_strBaseDataDirectory);
      sPersistentDataStorage.lookupValue("use-mongodb", m_bUseMongoDB);
      
      if(m_bUseMongoDB) {
	Setting &sMongoDB = cfgConfig.lookup("persistent-data-storage.mongodb");
	sMongoDB.lookupValue("host", m_strMongoDBHost);
	sMongoDB.lookupValue("port", m_nMongoDBPort);
	sMongoDB.lookupValue("database", m_strMongoDBDatabase);
      }
      
      // Section: Plugins
      Setting &sPluginsLoad = cfgConfig.lookup("plugins.load");
      for(int nI = 0; nI < sPluginsLoad.getLength(); nI++) {
	string strLoad = sPluginsLoad[nI];
	m_lstPluginsToLoad.push_back(strLoad);
      }
      
      Setting &sPluginsPaths = cfgConfig.lookup("plugins.search-paths");
      for(int nI = 0; nI < sPluginsPaths.getLength(); nI++) {
	string strPath = sPluginsPaths[nI];
	m_psPlugins->addPluginSearchPath(strPath);
      }
      
      return true;
    } catch(ParseException e) {
      cerr << "Error while parsing config file '" << strConfigFile << "': " << e.getError() << endl;
    } catch(...) {
      cerr << "Undefined error while parsig config file '" << strConfigFile << "'" << endl;
    }
    
    return false;
  }
  
  void Beliefstate::spreadEvent(Event evEvent) {
    if(m_psPlugins->spreadEvent(evEvent) == 0) {
      cerr << "[beliefstate] Unhandled event dropped." << endl;
      
      if(evEvent.cdDesignator) {
	cerr << "[beliefstate] Content was:" << endl;
	evEvent.cdDesignator->printDesignator();
      } else {
	cerr << "[beliefstate] No content given." << endl;
      }
    }
  }
  
  void Beliefstate::spreadServiceEvent(ServiceEvent seServiceEvent) {
    if(m_psPlugins->spreadServiceEvent(seServiceEvent) == 0) {
      cerr << "[beliefstate] Unhandled service event ('" << seServiceEvent.strServiceName << "') dropped." << endl;
      
      if(seServiceEvent.cdDesignator) {
	cerr << "[beliefstate] Content was:" << endl;
	seServiceEvent.cdDesignator->printDesignator();
      } else {
	cerr << "[beliefstate] No content given." << endl;
      }
    }
  }
  
  bool Beliefstate::cycle() {
    bool bContinue = true;
    
    if(m_bRun) {
      Result resCycle = m_psPlugins->cycle();
      
      if(resCycle.bSuccess) {
	// Events
	for(list<Event>::iterator itEvent = resCycle.lstEvents.begin();
	    itEvent != resCycle.lstEvents.end();
	    itEvent++) {
	  Event evEvent = *itEvent;
	  
	  // Distribute the event
	  this->spreadEvent(evEvent);
	  
	  // Clean up
	  if(evEvent.cdDesignator) {
	    delete evEvent.cdDesignator;
	  }
	}
	
	// Services
	for(list<ServiceEvent>::iterator itEvent = resCycle.lstServiceEvents.begin();
	    itEvent != resCycle.lstServiceEvents.end();
	    itEvent++) {
	  ServiceEvent seServiceEvent = *itEvent;
	  
	  // Distribute the event
	  this->spreadServiceEvent(seServiceEvent);
	  
	  // Clean up
	  if(seServiceEvent.cdDesignator) {
	    delete seServiceEvent.cdDesignator;
	  }
	}
      }
    } else {
      bContinue = false;
    }
    
    return bContinue;
  }
  
  void Beliefstate::triggerShutdown() {
    m_bRun = false;
  }
  
  void Beliefstate::setBaseDataDirectory(string strBaseDataDirectory) {
    m_strBaseDataDirectory = strBaseDataDirectory;
    m_psPlugins->setBaseDataDirectory(strBaseDataDirectory);
  }
  
  string Beliefstate::baseDataDirectory() {
    return m_strBaseDataDirectory;
  }
}
